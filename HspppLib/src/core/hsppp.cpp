// HspppLib/src/core/hsppp.cpp
// HSP互換APIのファサード実装（Direct2D 1.1対応）
//
// このファイルは以下の構成で分割されています：
//   - hsppp.cpp         : グローバル状態とinit/close_system
//   - hsppp_screen.inl  : Screenクラスメンバ関数
//   - hsppp_factory.inl : screen/buffer/bgscr生成関数
//   - hsppp_drawing.inl : 描画系関数（color, pos, mes, boxf, line, circle, pset, pget, redraw, await, end）
//   - hsppp_ginfo.inl   : ginfo, font, sysfont, title, width
//   - hsppp_copy.inl    : gsel, gmode, gcopy, gzoom
//   - hsppp_interrupt.inl : 割り込みハンドラ（onclick, oncmd, onerror, onexit, onkey）
//   - hsppp_string.inl  : 文字列操作関数（instr, strmid, strtrim, strf, getpath）

// グローバルモジュールフラグメント
module;

#define NOMINMAX
#include <windows.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>
#include <string_view>
#include <source_location>
#include <format>
#include <stdexcept>
#include <filesystem>
#include <system_error>
#include <map>
#include <memory>
#include <fstream>
#include <cstdio>
#include <cctype>
#include <random>
#include <shlobj.h>
#include <lmcons.h>
#include <shellapi.h>
#include <commdlg.h>

#include "Internal.h"

// モジュール実装
module hsppp;

// ============================================================
// グローバル状態（anonymous namespace内）
// ============================================================
namespace {
    using namespace hsppp::internal;

    // Surface管理
    std::map<int, std::shared_ptr<HspSurface>> g_surfaces;
    std::weak_ptr<HspSurface> g_currentSurface;

    // ID自動採番カウンター（負の値を使用してHSP互換ID 0〜との衝突を避ける）
    int g_nextAutoId = -1;

    // 次の自動採番IDを取得
    int getNextAutoId() {
        return g_nextAutoId--;
    }

    // システム状態
    bool g_shouldQuit = false;
    DWORD g_lastAwaitTime = 0;

    // マウスホイール状態
    int g_mouseWheelDelta = 0;

    // IDからSurfaceを取得するヘルパー
    std::shared_ptr<HspSurface> getSurfaceById(int id) {
        auto it = g_surfaces.find(id);
        if (it != g_surfaces.end()) {
            return it->second;
        }
        return nullptr;
    }

    // 生のSurfaceポインタを取得（GUI命令用）
    HspSurface* getSurface(int id) {
        auto ptr = getSurfaceById(id);
        return ptr.get();
    }

    // 現在のスクリーンIDを保持（GUI命令用）
    int g_currentScreenId = 0;

    // 遅延初期化: カレントサーフェスがなければデフォルトウィンドウを作成
    void ensureDefaultScreen() {
        auto current = g_currentSurface.lock();
        if (!current) {
            // デフォルトウィンドウを作成: screen 0, 640, 480, 0 (normal)
            (void)hsppp::screen(0, 640, 480, 0, -1, -1, 0, 0, "HSPPP Window");
        }
    }

    // カレントサーフェスを取得（なければ自動的にデフォルトウィンドウを作成）
    std::shared_ptr<HspSurface> getCurrentSurface() {
        auto current = g_currentSurface.lock();
        if (!current) {
            // デフォルトウィンドウを自動作成
            ensureDefaultScreen();
            current = g_currentSurface.lock();
        }
        return current;
    }

    // ============================================================
    // safe_call - 例外を適切なHspError/HspWeakErrorに変換するラッパー
    // ============================================================
    // std::exceptionの継承階層を網羅し、適切なエラー種別に変換して再スローする。
    // 既存のHspErrorBase派生例外はそのまま通過させる。
    
    template<typename Func>
    auto safe_call(const std::source_location& loc, Func&& func) -> decltype(func()) {
        using ReturnType = decltype(func());
        
        try {
            return func();
        }
        // HspErrorBase派生はそのまま通過
        catch (const hsppp::HspErrorBase&) {
            throw;
        }
        // --- 致命的エラー (HspError) ---
        // メモリ不足
        catch (const std::bad_alloc& e) {
            throw hsppp::HspError(hsppp::ERR_OUT_OF_MEMORY, e, loc);
        }
        // 配列境界外アクセス
        catch (const std::out_of_range& e) {
            throw hsppp::HspError(hsppp::ERR_OUT_OF_ARRAY, e, loc);
        }
        // 長さエラー（コンテナサイズ超過など）
        catch (const std::length_error& e) {
            throw hsppp::HspError(hsppp::ERR_BUFFER_OVERFLOW, e, loc);
        }
        // 無効引数
        catch (const std::invalid_argument& e) {
            throw hsppp::HspError(hsppp::ERR_OUT_OF_RANGE, e, loc);
        }
        // ドメインエラー（数学関数など）
        catch (const std::domain_error& e) {
            throw hsppp::HspError(hsppp::ERR_OUT_OF_RANGE, e, loc);
        }
        // オーバーフロー
        catch (const std::overflow_error& e) {
            throw hsppp::HspError(hsppp::ERR_BUFFER_OVERFLOW, e, loc);
        }
        // アンダーフロー
        catch (const std::underflow_error& e) {
            throw hsppp::HspError(hsppp::ERR_OUT_OF_RANGE, e, loc);
        }
        // --- 復帰可能エラー (HspWeakError) ---
        // ファイルシステムエラー
        catch (const std::filesystem::filesystem_error& e) {
            throw hsppp::HspWeakError(hsppp::ERR_FILE_IO, e, loc);
        }
        // 範囲エラー（計算結果が範囲外など - 復帰可能なケースが多い）
        catch (const std::range_error& e) {
            throw hsppp::HspWeakError(hsppp::ERR_OUT_OF_RANGE, e, loc);
        }
        // システムエラー（errnoベース）- 復帰可能（runtime_error派生なので先にcatch）
        catch (const std::system_error& e) {
            throw hsppp::HspWeakError(hsppp::ERR_SYSTEM_ERROR, e, loc);
        }
        // ランタイムエラー（その他）- 復帰可能として扱う
        catch (const std::runtime_error& e) {
            throw hsppp::HspWeakError(hsppp::ERR_SYSTEM_ERROR, e, loc);
        }
        // ロジックエラー（プログラムバグ）- 致命的
        catch (const std::logic_error& e) {
            throw hsppp::HspError(hsppp::ERR_INTERNAL, e, loc);
        }
        // その他すべてのstd::exception - 致命的として扱う
        catch (const std::exception& e) {
            throw hsppp::HspError(hsppp::ERR_INTERNAL, e, loc);
        }
        // 非std例外（滅多にないが念のため）
        catch (...) {
            throw hsppp::HspError(hsppp::ERR_INTERNAL, "Unknown exception caught", loc);
        }
    }
}

// ============================================================
// 分割された実装ファイルを #include
// ============================================================
// 注意: hsppp_interrupt.inl は割り込み処理を定義するため最初に含める
#include "hsppp_interrupt.inl"
// 注意: hsppp_copy.inl はgcopy_impl/gzoom_impl等のヘルパーを定義するため、
//       hsppp_screen.inlより前に含める必要がある
#include "hsppp_copy.inl"
#include "hsppp_screen.inl"
#include "hsppp_factory.inl"
#include "hsppp_drawing.inl"
#include "hsppp_ginfo.inl"
#include "hsppp_input.inl"
#include "hsppp_image.inl"
#include "hsppp_cel.inl"
#include "hsppp_math.inl"
#include "hsppp_string.inl"
#include "hsppp_system.inl"
#include "hsppp_file.inl"
#include "hsppp_easing.inl"
#include "hsppp_gui.inl"
#include "hsppp_media.inl"

// ============================================================
// init_system / close_system
// ============================================================
namespace hsppp::internal {

    // MediaManager への外部関数宣言（MediaManager.cpp で定義）
    void MediaManager_initialize();
    void MediaManager_shutdown();

    void init_system(const std::source_location& location) {
        // COM初期化
        CoInitialize(nullptr);

        // ウィンドウマネージャーの初期化
        WindowManager& windowManager = WindowManager::getInstance();
        if (!windowManager.registerWindowClass()) {
            MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
            return;
        }

        // Direct2D 1.1 デバイスマネージャーの初期化
        D2DDeviceManager& deviceManager = D2DDeviceManager::getInstance();
        if (!deviceManager.initialize()) {
            MessageBoxW(nullptr, L"Failed to initialize Direct2D 1.1 device", L"Error", MB_OK | MB_ICONERROR);
            return;
        }

        // マルチメディアマネージャーの初期化
        MediaManager_initialize();
    }

    void close_system(const std::source_location& location) {
        // マルチメディアマネージャーの終了
        MediaManager_shutdown();

        // すべてのサーフェスを解放
        g_surfaces.clear();
        g_currentSurface.reset();

        // Direct2D 1.1 デバイスマネージャーの終了
        D2DDeviceManager::getInstance().shutdown();

        // WindowManagerはスタティック変数なので明示的な削除は不要

        // COM終了処理
        CoUninitialize();
    }

    // ============================================================
    // Screen ID から HWND を取得（Media系から使用）
    // ============================================================
    void* getWindowHwndById(int id) {
        auto surface = getSurfaceById(id);
        if (!surface) return nullptr;
        
        auto* window = dynamic_cast<HspWindow*>(surface.get());
        if (!window) return nullptr;
        
        return static_cast<void*>(window->getHwnd());
    }

} // namespace hsppp::internal
