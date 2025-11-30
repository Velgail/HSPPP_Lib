// HspppLib/src/core/hsppp.cpp
// HSP互換APIのファサード実装

// グローバルモジュールフラグメント
// モジュール化されていないヘッダーはここでincludeする
module;

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>
#include <string_view>
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

    // Direct2D / DirectWrite リソース（COMスマートポインタで管理）
    ComPtr<ID2D1Factory> g_pD2DFactory;
    ComPtr<IDWriteFactory> g_pDWriteFactory;

    // Surface管理
    std::map<int, std::shared_ptr<HspSurface>> g_surfaces;
    std::weak_ptr<HspSurface> g_currentSurface;  // weak_ptrで安全に管理

    // システム状態
    bool g_shouldQuit = false;
    DWORD g_lastAwaitTime = 0; // 前回のawait呼び出し時刻

    // 描画モード管理（HSP互換）
    int g_redrawMode = 1;      // 0: 仮想画面のみ, 1: 即座に反映（デフォルト）
    bool g_isDrawing = false;  // BeginDraw中かどうか

    // 遅延初期化: カレントサーフェスがなければデフォルトウィンドウを作成
    void ensureDefaultScreen() {
        auto current = g_currentSurface.lock();
        if (!current) {
            // デフォルトウィンドウを作成: screen 0, 640, 480, 0 (normal)
            hsppp::screen(0, 640, 480, 0, -1, -1, 0, 0, "HSPPP Window");
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

    // 描画開始（内部ヘルパー）
    void beginDrawIfNeeded() {
        if (!g_isDrawing) {
            auto currentSurface = getCurrentSurface();
            if (currentSurface) {
                currentSurface->beginDraw();
                g_isDrawing = true;
            }
        }
    }

    // 描画終了＆画面反映（内部ヘルパー）
    void endDrawAndPresent() {
        if (g_isDrawing) {
            auto currentSurface = g_currentSurface.lock();
            if (currentSurface) {
                currentSurface->endDraw();

                // HspWindowの場合は画面に転送
                auto pWindow = std::dynamic_pointer_cast<HspWindow>(currentSurface);
                if (pWindow) {
                    pWindow->present();
                }

                g_isDrawing = false;
            }
        }
    }
}

namespace hsppp {

    // ============================================================
    // Screen クラスのメンバ関数実装
    // IDからグローバルマップを経由してSurfaceを取得する
    // ============================================================

    namespace {
        // IDからSurfaceを取得するヘルパー
        std::shared_ptr<internal::HspSurface> getSurfaceById(int id) {
            auto it = g_surfaces.find(id);
            if (it != g_surfaces.end()) {
                return it->second;
            }
            return nullptr;
        }
    }

    Screen& Screen::color(int r, int g, int b) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->color(r, g, b);
        }
        return *this;
    }

    Screen& Screen::pos(int x, int y) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->pos(x, y);
        }
        return *this;
    }

    Screen& Screen::mes(std::string_view text) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        // 描画モードに応じて処理
        if (g_redrawMode == 1) {
            // このScreenをカレントに設定してから描画
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->mes(text);
            endDrawAndPresent();
        }
        else {
            surface->mes(text);
        }
        return *this;
    }

    Screen& Screen::boxf(int x1, int y1, int x2, int y2) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        if (g_redrawMode == 1) {
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->boxf(x1, y1, x2, y2);
            endDrawAndPresent();
        }
        else {
            surface->boxf(x1, y1, x2, y2);
        }
        return *this;
    }

    Screen& Screen::boxf() {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        if (g_redrawMode == 1) {
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->boxf(0, 0, surface->getWidth(), surface->getHeight());
            endDrawAndPresent();
        }
        else {
            surface->boxf(0, 0, surface->getWidth(), surface->getHeight());
        }
        return *this;
    }

    Screen& Screen::redraw(int mode) {
        // このScreenをカレントにしてからredraw
        select();
        hsppp::redraw(mode);
        return *this;
    }

    Screen& Screen::select() {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            g_currentSurface = surface;
        }
        return *this;
    }

    int Screen::width() const {
        auto surface = getSurfaceById(m_id);
        return surface ? surface->getWidth() : 0;
    }

    int Screen::height() const {
        auto surface = getSurfaceById(m_id);
        return surface ? surface->getHeight() : 0;
    }


    // ============================================================
    // screen 関数の実装（Screen を返す版）
    // ============================================================

    // ウィンドウ初期化（構造体版）- Screen を返す
    Screen screen(const ScreenParams& params) {
        using namespace internal;

        int id = params.id;
        int width = params.width;
        int height = params.height;
        int mode = params.mode;
        int pos_x = params.pos_x;
        int pos_y = params.pos_y;
        int client_w = params.client_w;
        int client_h = params.client_h;
        std::string_view title = params.title;

        // 既存のサーフェスを削除
        if (g_surfaces.find(id) != g_surfaces.end()) {
            g_surfaces.erase(id);
        }

        // ID0はデフォルトでサイズ固定
        if (id == 0) {
            mode |= screen_fixedsize;
        }

        // ウィンドウスタイルの決定
        DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        DWORD dwExStyle = 0;

        // サイズ固定でない場合は、サイズ変更可能なスタイルを追加
        if (!(mode & screen_fixedsize)) {
            dwStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;
        }

        // ツールウィンドウ
        if (mode & screen_tool) {
            dwExStyle |= WS_EX_TOOLWINDOW;
        }

        // 深い縁のあるウィンドウ
        if (mode & screen_frame) {
            dwExStyle |= WS_EX_CLIENTEDGE;
        }

        // 非表示ウィンドウ（初期状態では非表示、後でgselで表示可能）
        bool isHidden = (mode & screen_hide) != 0;

        // クライアントサイズの決定（client_w, client_hが0ならwidth, heightと同じ）
        int clientWidth = (client_w > 0) ? client_w : width;
        int clientHeight = (client_h > 0) ? client_h : height;

        // HspWindowインスタンスの作成
        auto window = std::make_shared<HspWindow>(width, height, title);

        // ウィンドウマネージャーの取得
        WindowManager& windowManager = WindowManager::getInstance();

        // ウィンドウの作成
        if (!window->createWindow(
            windowManager.getHInstance(),
            windowManager.getClassName(),
            dwStyle,
            dwExStyle,
            pos_x,
            pos_y,
            clientWidth,
            clientHeight)) {
            MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};  // 無効なScreenを返す
        }

        // Direct2Dリソースの初期化
        if (!window->initialize(g_pD2DFactory, g_pDWriteFactory)) {
            MessageBoxW(nullptr, L"Failed to initialize window", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};  // 無効なScreenを返す
        }

        // Surfaceマップに追加
        g_surfaces[id] = window;

        // カレントサーフェスとして設定（weak_ptrを使用）
        g_currentSurface = window;

        // 非表示フラグが立っていなければウィンドウを表示
        if (!isHidden) {
            ShowWindow(window->getHwnd(), SW_SHOW);
            UpdateWindow(window->getHwnd());
        }

        g_shouldQuit = false;

        // Screen ハンドルを返す（IDと有効フラグのみ）
        return Screen{id, true};
    }

    // ウィンドウ初期化（HSP互換・省略可能版）
    Screen screen(
        OptInt id,
        OptInt width,
        OptInt height,
        OptInt mode,
        OptInt pos_x,
        OptInt pos_y,
        OptInt client_w,
        OptInt client_h,
        std::string_view title
    ) {
        // デフォルト値を適用して構造体版を呼び出す
        return screen(ScreenParams{
            .id       = id.value_or(0),
            .width    = width.value_or(640),
            .height   = height.value_or(480),
            .mode     = mode.value_or(0),
            .pos_x    = pos_x.value_or(-1),
            .pos_y    = pos_y.value_or(-1),
            .client_w = client_w.value_or(0),
            .client_h = client_h.value_or(0),
            .title    = title
        });
    }

    // 描画制御（HSP互換）
    void redraw(int p1) {
        // カレントサーフェス取得（自動的にデフォルトウィンドウ作成）
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        // p1の値に応じて描画モードを設定
        // 0: モード0に設定（仮想画面のみ）
        // 1: モード1に設定＋画面更新
        // 2: モード0に設定のみ（画面更新なし）
        // 3: モード1に設定のみ（画面更新なし）

        bool shouldUpdate = (p1 % 2 == 1);  // 奇数なら画面更新
        int newMode = p1 % 2;                // 0 or 1

        if (newMode == 0) {
            // モード0に切り替え: 仮想画面のみに描画
            // 既に描画中でなければBeginDrawを呼ぶ
            if (!g_isDrawing) {
                beginDrawIfNeeded();
            }
            g_redrawMode = 0;
        }
        else {
            // モード1に切り替え: 即座に反映
            g_redrawMode = 1;

            // shouldUpdateがtrueなら即座に画面更新
            if (shouldUpdate && g_isDrawing) {
                endDrawAndPresent();
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
        auto currentSurface = getCurrentSurface();
        if (currentSurface) {
            currentSurface->color(r, g, b);
        }
    }

    // 描画位置設定
    void pos(int x, int y) {
        auto currentSurface = getCurrentSurface();
        if (currentSurface) {
            currentSurface->pos(x, y);
        }
    }

    // 文字列描画
    void mes(std::string_view text) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        // 描画モードに応じて処理
        if (g_redrawMode == 1) {
            // モード1: 即座に反映
            beginDrawIfNeeded();
            currentSurface->mes(text);
            endDrawAndPresent();
        }
        else {
            // モード0: 仮想画面のみ（BeginDraw済み）
            currentSurface->mes(text);
        }
    }

    // 矩形塗りつぶし（座標指定版）
    void boxf(int x1, int y1, int x2, int y2) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        // 描画モードに応じて処理
        if (g_redrawMode == 1) {
            // モード1: 即座に反映
            beginDrawIfNeeded();
            currentSurface->boxf(x1, y1, x2, y2);
            endDrawAndPresent();
        }
        else {
            // モード0: 仮想画面のみ（BeginDraw済み）
            currentSurface->boxf(x1, y1, x2, y2);
        }
    }

    // 矩形塗りつぶし（全画面版）
    void boxf() {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        // 描画モードに応じて処理
        if (g_redrawMode == 1) {
            // モード1: 即座に反映
            beginDrawIfNeeded();
            currentSurface->boxf(0, 0, currentSurface->getWidth(), currentSurface->getHeight());
            endDrawAndPresent();
        }
        else {
            // モード0: 仮想画面のみ（BeginDraw済み）
            currentSurface->boxf(0, 0, currentSurface->getWidth(), currentSurface->getHeight());
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

            // Direct2D Factory の作成（ComPtrで自動管理）
            HRESULT hr = D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                g_pD2DFactory.GetAddressOf()
            );
            if (FAILED(hr)) {
                MessageBoxW(nullptr, L"Failed to create Direct2D factory", L"Error", MB_OK | MB_ICONERROR);
                return;
            }

            // DirectWrite Factory の作成（ComPtrで自動管理）
            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                &g_pDWriteFactory
            );
            if (FAILED(hr)) {
                MessageBoxW(nullptr, L"Failed to create DirectWrite factory", L"Error", MB_OK | MB_ICONERROR);
                return;
            }
        }

        void close_system() {
            // すべてのサーフェスを解放
            g_surfaces.clear();
            g_currentSurface.reset();  // weak_ptrをリセット

            // ComPtrは自動的にReleaseされるので明示的な解放は不要
            g_pDWriteFactory.Reset();
            g_pD2DFactory.Reset();

            // WindowManagerはスタティック変数なので明示的な削除は不要
            // デストラクタでウィンドウクラスの登録解除が行われる

            // COM終了処理
            CoUninitialize();
        }

    } // namespace internal

} // namespace hsppp
