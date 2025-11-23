// HspppLib/src/core/hsppp.cpp
// HSP互換APIのファサード実装

// グローバルモジュールフラグメント
// モジュール化されていないヘッダーはここでincludeする
module;

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <map>
#include <memory>

#include "Internal.h"

// モジュール実装
module hsppp;

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

// グローバル変数
namespace {
    using namespace hsppp::internal;

    // Direct2D / DirectWrite リソース
    ID2D1Factory* g_pD2DFactory = nullptr;
    IDWriteFactory* g_pDWriteFactory = nullptr;

    // Surface管理
    std::map<int, std::shared_ptr<HspSurface>> g_surfaces;
    HspSurface* g_currentSurface = nullptr;

    // システム状態
    bool g_shouldQuit = false;
    DWORD g_lastAwaitTime = 0; // 前回のawait呼び出し時刻
}

namespace hsppp {

    // ウィンドウ初期化
    void screen(int id, int width, int height, int mode, const char* title) {
        using namespace internal;

        // 既存のサーフェスを削除
        if (g_surfaces.find(id) != g_surfaces.end()) {
            g_surfaces.erase(id);
        }

        // ウィンドウスタイルの決定
        DWORD dwStyle = WS_OVERLAPPEDWINDOW;
        if (mode == 1) {
            // 枠なしウィンドウ
            dwStyle = WS_POPUP | WS_VISIBLE;
        }

        // HspWindowインスタンスの作成
        auto window = std::make_shared<HspWindow>(width, height, title);

        // ウィンドウマネージャーの取得
        WindowManager& windowManager = WindowManager::getInstance();

        // ウィンドウの作成
        if (!window->createWindow(
            windowManager.getHInstance(),
            windowManager.getClassName(),
            dwStyle)) {
            MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
            return;
        }

        // Direct2Dリソースの初期化
        if (!window->initialize(g_pD2DFactory, g_pDWriteFactory)) {
            MessageBoxW(nullptr, L"Failed to initialize window", L"Error", MB_OK | MB_ICONERROR);
            return;
        }

        // Surfaceマップに追加
        g_surfaces[id] = window;

        // カレントサーフェスとして設定
        g_currentSurface = window.get();

        g_shouldQuit = false;
    }

    // 描画制御
    void redraw(int p1) {
        if (!g_currentSurface) return;

        if (p1 == 0) {
            // 描画開始
            g_currentSurface->beginDraw();
        }
        else {
            // 描画終了＆画面反映
            g_currentSurface->endDraw();

            // HspWindowの場合は画面に転送
            HspWindow* pWindow = dynamic_cast<HspWindow*>(g_currentSurface);
            if (pWindow) {
                pWindow->present();
            }
        }
    }

    // 待機＆メッセージ処理 (HSP互換)
    void await(int time_ms) {
        MSG msg;
        DWORD currentTime = GetTickCount();

        // 初回呼び出し、または時刻が巻き戻った場合は現在時刻を基準にする
        if (g_lastAwaitTime == 0 || currentTime < g_lastAwaitTime) {
            g_lastAwaitTime = currentTime;
        }

        // 前回からの経過時間を計算
        DWORD elapsed = currentTime - g_lastAwaitTime;

        // 指定時間に満たない場合は待機
        if (elapsed < (DWORD)time_ms) {
            DWORD waitTime = time_ms - elapsed;
            DWORD endTime = currentTime + waitTime;

            // 待機中もメッセージを処理
            while (GetTickCount() < endTime) {
                if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) {
                        g_shouldQuit = true;
                        return;
                    }
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else {
                    Sleep(1);
                }
            }
        }
        else {
            // すでに指定時間を超過している場合もメッセージ処理だけ行う
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    g_shouldQuit = true;
                    return;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        // 次回のawaitのために現在時刻を記録
        g_lastAwaitTime = GetTickCount();
    }

    // 描画色設定
    void color(int r, int g, int b) {
        if (g_currentSurface) {
            g_currentSurface->color(r, g, b);
        }
    }

    // 描画位置設定
    void pos(int x, int y) {
        if (g_currentSurface) {
            g_currentSurface->pos(x, y);
        }
    }

    // 文字列描画
    void mes(const char* text) {
        if (g_currentSurface) {
            g_currentSurface->mes(text);
        }
    }

    // 矩形塗りつぶし（座標指定版）
    void boxf(int x1, int y1, int x2, int y2) {
        if (g_currentSurface) {
            g_currentSurface->boxf(x1, y1, x2, y2);
        }
    }

    // 矩形塗りつぶし（全画面版）
    void boxf() {
        if (g_currentSurface) {
            // 画面全体を塗りつぶす
            g_currentSurface->boxf(0, 0, g_currentSurface->getWidth(), g_currentSurface->getHeight());
        }
    }

    namespace internal {

        void init_system() {
            // COM初期化
            CoInitialize(nullptr);

            // ウィンドウマネージャーの初期化
            WindowManager& windowManager = WindowManager::getInstance();
            if (!windowManager.registerWindowClass()) {
                MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
                return;
            }

            // Direct2D Factory の作成
            HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
            if (FAILED(hr)) {
                MessageBoxW(nullptr, L"Failed to create Direct2D factory", L"Error", MB_OK | MB_ICONERROR);
                return;
            }

            // DirectWrite Factory の作成
            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&g_pDWriteFactory)
            );
            if (FAILED(hr)) {
                MessageBoxW(nullptr, L"Failed to create DirectWrite factory", L"Error", MB_OK | MB_ICONERROR);
                return;
            }
        }

        void close_system() {
            // すべてのサーフェスを解放
            g_surfaces.clear();
            g_currentSurface = nullptr;

            // Direct2D/DirectWrite リソースの解放
            if (g_pDWriteFactory) {
                g_pDWriteFactory->Release();
                g_pDWriteFactory = nullptr;
            }
            if (g_pD2DFactory) {
                g_pD2DFactory->Release();
                g_pD2DFactory = nullptr;
            }

            // WindowManagerはスタティック変数なので明示的な削除は不要
            // デストラクタでウィンドウクラスの登録解除が行われる

            // COM終了処理
            CoUninitialize();
        }

    } // namespace internal

} // namespace hsppp
