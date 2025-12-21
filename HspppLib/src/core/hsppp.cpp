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
#include <map>
#include <memory>
#include <fstream>
#include <cstdio>
#include <cctype>
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

    // gmode設定（HSP互換）
    int g_gmodeMode = 0;
    int g_gmodeSizeX = 32;
    int g_gmodeSizeY = 32;
    int g_gmodeBlendRate = 0;

    // IDからSurfaceを取得するヘルパー
    std::shared_ptr<HspSurface> getSurfaceById(int id) {
        auto it = g_surfaces.find(id);
        if (it != g_surfaces.end()) {
            return it->second;
        }
        return nullptr;
    }

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
}

// ============================================================
// 分割された実装ファイルを #include
// ============================================================
// 注意: hsppp_interrupt.inl は割り込み処理を定義するため最初に含める
#include "hsppp_interrupt.inl"
#include "hsppp_screen.inl"
#include "hsppp_factory.inl"
#include "hsppp_drawing.inl"
#include "hsppp_ginfo.inl"
#include "hsppp_copy.inl"
#include "hsppp_input.inl"
#include "hsppp_image.inl"
#include "hsppp_cel.inl"
#include "hsppp_math.inl"
#include "hsppp_string.inl"
#include "hsppp_system.inl"
#include "hsppp_file.inl"

// ============================================================
// init_system / close_system
// ============================================================
namespace hsppp::internal {

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
    }

    void close_system(const std::source_location& location) {
        // すべてのサーフェスを解放
        g_surfaces.clear();
        g_currentSurface.reset();

        // Direct2D 1.1 デバイスマネージャーの終了
        D2DDeviceManager::getInstance().shutdown();

        // WindowManagerはスタティック変数なので明示的な削除は不要

        // COM終了処理
        CoUninitialize();
    }

} // namespace hsppp::internal
