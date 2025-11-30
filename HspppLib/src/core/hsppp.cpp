// HspppLib/src/core/hsppp.cpp
// HSP互換APIのファサード実装（Direct2D 1.1対応）

// グローバルモジュールフラグメント
module;

#include <windows.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>
#include <string_view>
#include <map>
#include <memory>

#include "Internal.h"

// モジュール実装
module hsppp;

// グローバル変数
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

    // 描画モード管理（HSP互換）
    int g_redrawMode = 1;
    bool g_isDrawing = false;

    // gmode設定（HSP互換）
    int g_gmodeMode = 0;
    int g_gmodeSizeX = 32;
    int g_gmodeSizeY = 32;
    int g_gmodeBlendRate = 0;

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

    Screen& Screen::line(int x2, int y2) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        int startX = surface->getCurrentX();
        int startY = surface->getCurrentY();

        if (g_redrawMode == 1) {
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->line(x2, y2, startX, startY, false);
            endDrawAndPresent();
        }
        else {
            surface->line(x2, y2, startX, startY, false);
        }
        return *this;
    }

    Screen& Screen::line(int x2, int y2, int x1, int y1) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        if (g_redrawMode == 1) {
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->line(x2, y2, x1, y1, true);
            endDrawAndPresent();
        }
        else {
            surface->line(x2, y2, x1, y1, true);
        }
        return *this;
    }

    Screen& Screen::circle(int x1, int y1, int x2, int y2, int fillMode) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        if (g_redrawMode == 1) {
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->circle(x1, y1, x2, y2, fillMode);
            endDrawAndPresent();
        }
        else {
            surface->circle(x1, y1, x2, y2, fillMode);
        }
        return *this;
    }

    Screen& Screen::pset(int x, int y) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        if (g_redrawMode == 1) {
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->pset(x, y);
            endDrawAndPresent();
        }
        else {
            surface->pset(x, y);
        }
        return *this;
    }

    Screen& Screen::pset() {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        int px = surface->getCurrentX();
        int py = surface->getCurrentY();

        if (g_redrawMode == 1) {
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->pset(px, py);
            endDrawAndPresent();
        }
        else {
            surface->pset(px, py);
        }
        return *this;
    }

    Screen& Screen::pget(int x, int y) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        int r, g, b;

        if (g_redrawMode == 1) {
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->pget(x, y, r, g, b);
            endDrawAndPresent();
        }
        else {
            surface->pget(x, y, r, g, b);
        }
        return *this;
    }

    Screen& Screen::pget() {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        int px = surface->getCurrentX();
        int py = surface->getCurrentY();
        int r, g, b;

        if (g_redrawMode == 1) {
            g_currentSurface = surface;
            beginDrawIfNeeded();
            surface->pget(px, py, r, g, b);
            endDrawAndPresent();
        }
        else {
            surface->pget(px, py, r, g, b);
        }
        return *this;
    }


    // ============================================================
    // screen 関数の実装
    // ============================================================

    // 内部実装：ウィンドウ作成の共通処理
    static Screen createWindowInternal(
        int id,
        int width,
        int height,
        int mode,
        int pos_x,
        int pos_y,
        int client_w,
        int client_h,
        std::string_view title
    ) {
        using namespace internal;

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

        // Direct2D 1.1リソースの初期化
        if (!window->initialize()) {
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

    // OOP版：構造体パラメータ（ID自動採番）
    Screen screen(const ScreenParams& params) {
        int id = getNextAutoId();
        return createWindowInternal(
            id,
            params.width,
            params.height,
            params.mode,
            params.pos_x,
            params.pos_y,
            params.client_w,
            params.client_h,
            params.title
        );
    }

    // OOP版：引数なし（ID自動採番、デフォルト設定）
    Screen screen() {
        return screen(ScreenParams{});
    }

    // HSP互換版：ID明示指定
    Screen screen(
        int id,
        OptInt width,
        OptInt height,
        OptInt mode,
        OptInt pos_x,
        OptInt pos_y,
        OptInt client_w,
        OptInt client_h,
        std::string_view title
    ) {
        return createWindowInternal(
            id,
            width.value_or(640),
            height.value_or(480),
            mode.value_or(0),
            pos_x.value_or(-1),
            pos_y.value_or(-1),
            client_w.value_or(0),
            client_h.value_or(0),
            title
        );
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

    // プログラム終了 (HSP互換)
    [[noreturn]] void end(int exitcode) {
        // 描画中の場合は終了処理
        if (g_isDrawing) {
            endDrawAndPresent();
        }

        // リソースのクリーンアップ
        internal::close_system();

        // プロセス終了
        ExitProcess(static_cast<UINT>(exitcode));
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

    // ============================================================
    // line - 直線を描画（HSP互換）
    // ============================================================
    void line(OptInt x2, OptInt y2, OptInt x1, OptInt y1) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int endX = x2.value_or(0);
        int endY = y2.value_or(0);
        
        // x1, y1が省略されたかどうかを判定
        bool useStartPos = !x1.is_default() && !y1.is_default();
        int startX = x1.value_or(currentSurface->getCurrentX());
        int startY = y1.value_or(currentSurface->getCurrentY());

        // 描画モードに応じて処理
        if (g_redrawMode == 1) {
            beginDrawIfNeeded();
            currentSurface->line(endX, endY, startX, startY, useStartPos);
            endDrawAndPresent();
        }
        else {
            currentSurface->line(endX, endY, startX, startY, useStartPos);
        }
    }

    // ============================================================
    // circle - 円を描画（HSP互換）
    // ============================================================
    void circle(OptInt x1, OptInt y1, OptInt x2, OptInt y2, OptInt fillMode) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int p1 = x1.value_or(0);
        int p2 = y1.value_or(0);
        int p3 = x2.value_or(currentSurface->getWidth());
        int p4 = y2.value_or(currentSurface->getHeight());
        int p5 = fillMode.value_or(1);

        // 描画モードに応じて処理
        if (g_redrawMode == 1) {
            beginDrawIfNeeded();
            currentSurface->circle(p1, p2, p3, p4, p5);
            endDrawAndPresent();
        }
        else {
            currentSurface->circle(p1, p2, p3, p4, p5);
        }
    }

    // ============================================================
    // pset - 1ドットの点を描画（HSP互換）
    // ============================================================
    void pset(OptInt x, OptInt y) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int px = x.is_default() ? currentSurface->getCurrentX() : x.value();
        int py = y.is_default() ? currentSurface->getCurrentY() : y.value();

        // 描画モードに応じて処理
        if (g_redrawMode == 1) {
            beginDrawIfNeeded();
            currentSurface->pset(px, py);
            endDrawAndPresent();
        }
        else {
            currentSurface->pset(px, py);
        }
    }

    // ============================================================
    // pget - 1ドットの色を取得（HSP互換）
    // ============================================================
    void pget(OptInt x, OptInt y) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int px = x.is_default() ? currentSurface->getCurrentX() : x.value();
        int py = y.is_default() ? currentSurface->getCurrentY() : y.value();

        int r, g, b;
        
        // 描画モードに応じて処理
        if (g_redrawMode == 1) {
            beginDrawIfNeeded();
            currentSurface->pget(px, py, r, g, b);
            endDrawAndPresent();
        }
        else {
            currentSurface->pget(px, py, r, g, b);
        }
    }

    // ============================================================
    // ginfo - ウィンドウ情報の取得（HSP互換）
    // ============================================================
    int ginfo(int type) {
        using namespace internal;
        
        auto currentSurface = getCurrentSurface();
        auto pWindow = currentSurface ? std::dynamic_pointer_cast<HspWindow>(currentSurface) : nullptr;
        
        switch (type) {
        case 0:  // スクリーン上のマウスカーソルX座標
        {
            POINT pt;
            GetCursorPos(&pt);
            return pt.x;
        }
        case 1:  // スクリーン上のマウスカーソルY座標
        {
            POINT pt;
            GetCursorPos(&pt);
            return pt.y;
        }
        case 2:  // アクティブなウィンドウID
        {
            HWND hwndActive = GetForegroundWindow();
            // g_surfacesを検索してウィンドウIDを返す
            for (const auto& pair : g_surfaces) {
                auto pWin = std::dynamic_pointer_cast<HspWindow>(pair.second);
                if (pWin && pWin->getHwnd() == hwndActive) {
                    return pair.first;
                }
            }
            return -1;  // HSP以外のウィンドウがアクティブ
        }
        case 3:  // 操作先ウィンドウID
        {
            auto current = g_currentSurface.lock();
            if (current) {
                for (const auto& pair : g_surfaces) {
                    if (pair.second == current) {
                        return pair.first;
                    }
                }
            }
            return 0;
        }
        case 4:  // ウィンドウの左上X座標
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.left;
            }
            return 0;
        }
        case 5:  // ウィンドウの左上Y座標
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.top;
            }
            return 0;
        }
        case 6:  // ウィンドウの右下X座標
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.right;
            }
            return 0;
        }
        case 7:  // ウィンドウの右下Y座標
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.bottom;
            }
            return 0;
        }
        case 8:  // ウィンドウの描画基点X座標（スクロール未対応のため常に0）
            return 0;
        case 9:  // ウィンドウの描画基点Y座標（スクロール未対応のため常に0）
            return 0;
        case 10:  // ウィンドウ全体のXサイズ
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.right - rect.left;
            }
            return currentSurface ? currentSurface->getWidth() : 0;
        }
        case 11:  // ウィンドウ全体のYサイズ
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.bottom - rect.top;
            }
            return currentSurface ? currentSurface->getHeight() : 0;
        }
        case 12:  // クライアント領域Xサイズ
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetClientRect(pWindow->getHwnd(), &rect);
                return rect.right;
            }
            return currentSurface ? currentSurface->getWidth() : 0;
        }
        case 13:  // クライアント領域Yサイズ
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetClientRect(pWindow->getHwnd(), &rect);
                return rect.bottom;
            }
            return currentSurface ? currentSurface->getHeight() : 0;
        }
        case 14:  // メッセージの出力Xサイズ（未実装）
            return 0;
        case 15:  // メッセージの出力Yサイズ（未実装）
            return 0;
        case 16:  // 現在設定されているカラーコード(R)
        {
            if (currentSurface) {
                auto color = currentSurface->getCurrentColor();
                return static_cast<int>(color.r * 255.0f);
            }
            return 0;
        }
        case 17:  // 現在設定されているカラーコード(G)
        {
            if (currentSurface) {
                auto color = currentSurface->getCurrentColor();
                return static_cast<int>(color.g * 255.0f);
            }
            return 0;
        }
        case 18:  // 現在設定されているカラーコード(B)
        {
            if (currentSurface) {
                auto color = currentSurface->getCurrentColor();
                return static_cast<int>(color.b * 255.0f);
            }
            return 0;
        }
        case 19:  // デスクトップのカラーモード（常にフルカラー）
            return 0;
        case 20:  // デスクトップ全体のXサイズ
            return GetSystemMetrics(SM_CXSCREEN);
        case 21:  // デスクトップ全体のYサイズ
            return GetSystemMetrics(SM_CYSCREEN);
        case 22:  // カレントポジションのX座標
            return currentSurface ? currentSurface->getCurrentX() : 0;
        case 23:  // カレントポジションのY座標
            return currentSurface ? currentSurface->getCurrentY() : 0;
        case 24:  // メッセージ割り込み時のウィンドウID（未実装）
            return 0;
        case 25:  // 未使用ウィンドウID
        {
            for (int i = 0; ; ++i) {
                if (g_surfaces.find(i) == g_surfaces.end()) {
                    return i;
                }
            }
        }
        case 26:  // 画面の初期化Xサイズ
            return currentSurface ? currentSurface->getWidth() : 0;
        case 27:  // 画面の初期化Yサイズ
            return currentSurface ? currentSurface->getHeight() : 0;
        default:
            return 0;
        }
    }

    // ginfo_r, ginfo_g, ginfo_b マクロの代わりとなる関数
    int ginfo_r() {
        return ginfo(16);
    }

    int ginfo_g() {
        return ginfo(17);
    }

    int ginfo_b() {
        return ginfo(18);
    }

    // ============================================================
    // buffer - 仮想画面を初期化
    // ============================================================

    // 内部実装：バッファ作成の共通処理
    static Screen createBufferInternal(int id, int width, int height, int mode) {
        using namespace internal;

        // 既存のサーフェスがある場合の処理
        // HSPでは既存のIDに対してbuffer()を呼ぶと上書きされる（エラーではない）
        auto it = g_surfaces.find(id);
        if (it != g_surfaces.end()) {
            g_surfaces.erase(it);
        }

        // HspBufferインスタンスの作成
        auto buf = std::make_shared<HspBuffer>(width, height);

        // Direct2D 1.1リソースの初期化
        if (!buf->initialize()) {
            MessageBoxW(nullptr, L"Failed to initialize buffer", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};  // 無効なScreenを返す
        }

        // Surfaceマップに追加
        g_surfaces[id] = buf;

        // カレントサーフェスとして設定
        g_currentSurface = buf;

        // Screen ハンドルを返す
        return Screen{id, true};
    }

    // OOP版：構造体パラメータ（ID自動採番）
    Screen buffer(const BufferParams& params) {
        int id = getNextAutoId();
        return createBufferInternal(id, params.width, params.height, params.mode);
    }

    // OOP版：引数なし（ID自動採番、デフォルト設定）
    Screen buffer() {
        return buffer(BufferParams{});
    }

    // HSP互換版：ID明示指定
    Screen buffer(int id, OptInt width, OptInt height, OptInt mode) {
        return createBufferInternal(
            id,
            width.value_or(640),
            height.value_or(480),
            mode.value_or(0)
        );
    }

    // ============================================================
    // bgscr - 枠のないウィンドウを初期化
    // ============================================================

    // 内部実装：枠なしウィンドウ作成の共通処理
    static Screen createBgscrInternal(
        int id,
        int width,
        int height,
        int mode,
        int pos_x,
        int pos_y,
        int client_w,
        int client_h
    ) {
        using namespace internal;

        // 既存のサーフェスを削除
        if (g_surfaces.find(id) != g_surfaces.end()) {
            g_surfaces.erase(id);
        }

        // ウィンドウスタイル: 枠なし（WS_POPUP）
        DWORD dwStyle = WS_POPUP;
        DWORD dwExStyle = 0;

        // 非表示ウィンドウ（mode & 2）
        bool isHidden = (mode & 2) != 0;

        // クライアントサイズの決定
        int clientWidth = (client_w > 0) ? client_w : width;
        int clientHeight = (client_h > 0) ? client_h : height;

        // HspWindowインスタンスの作成（タイトルは空）
        auto window = std::make_shared<HspWindow>(width, height, "");

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
            MessageBoxW(nullptr, L"Failed to create borderless window", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};
        }

        // Direct2D 1.1リソースの初期化
        if (!window->initialize()) {
            MessageBoxW(nullptr, L"Failed to initialize borderless window", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};
        }

        // Surfaceマップに追加
        g_surfaces[id] = window;

        // カレントサーフェスとして設定
        g_currentSurface = window;

        // 非表示フラグが立っていなければウィンドウを表示
        if (!isHidden) {
            ShowWindow(window->getHwnd(), SW_SHOW);
            UpdateWindow(window->getHwnd());
        }

        g_shouldQuit = false;

        return Screen{id, true};
    }

    // OOP版：構造体パラメータ（ID自動採番）
    Screen bgscr(const BgscrParams& params) {
        int id = getNextAutoId();
        return createBgscrInternal(
            id,
            params.width,
            params.height,
            params.mode,
            params.pos_x,
            params.pos_y,
            params.client_w,
            params.client_h
        );
    }

    // OOP版：引数なし（ID自動採番、デフォルト設定）
    Screen bgscr() {
        return bgscr(BgscrParams{});
    }

    // HSP互換版：ID明示指定
    Screen bgscr(int id, OptInt width, OptInt height, OptInt mode,
                 OptInt pos_x, OptInt pos_y, OptInt client_w, OptInt client_h) {
        return createBgscrInternal(
            id,
            width.value_or(640),
            height.value_or(480),
            mode.value_or(0),
            pos_x.value_or(-1),
            pos_y.value_or(-1),
            client_w.value_or(0),
            client_h.value_or(0)
        );
    }

    // ============================================================
    // gsel - 描画先指定、ウィンドウ最前面、非表示設定（HSP互換）
    // ============================================================
    void gsel(OptInt id, OptInt mode) {
        using namespace internal;

        int p1 = id.value_or(0);
        int p2 = mode.value_or(0);

        // 指定されたIDのサーフェスを取得
        auto it = g_surfaces.find(p1);
        if (it == g_surfaces.end()) {
            return;  // 存在しないIDは無視
        }

        auto surface = it->second;

        // カレントサーフェスとして設定
        g_currentSurface = surface;

        // HspWindowの場合はウィンドウ操作
        auto pWindow = std::dynamic_pointer_cast<HspWindow>(surface);
        if (pWindow) {
            HWND hwnd = pWindow->getHwnd();
            switch (p2) {
            case -1:
                // 非表示にする
                ShowWindow(hwnd, SW_HIDE);
                break;
            case 0:
                // 特に影響なし（描画先のみ変更）
                break;
            case 1:
                // アクティブにする
                ShowWindow(hwnd, SW_SHOW);
                SetForegroundWindow(hwnd);
                break;
            case 2:
                // アクティブ＋最前面
                ShowWindow(hwnd, SW_SHOW);
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                SetForegroundWindow(hwnd);
                break;
            }
        }
    }

    // ============================================================
    // gmode - 画面コピーモード設定（HSP互換）
    // ============================================================
    void gmode(OptInt mode, OptInt size_x, OptInt size_y, OptInt blend_rate) {
        g_gmodeMode = mode.value_or(0);
        g_gmodeSizeX = size_x.value_or(32);
        g_gmodeSizeY = size_y.value_or(32);
        g_gmodeBlendRate = blend_rate.value_or(0);
    }

    // ============================================================
    // gcopy - 画面コピー（HSP互換）
    // Direct2D 1.1: 共有ビットマップを使用して異なるサーフェス間でコピー
    // ============================================================
    void gcopy(OptInt src_id, OptInt src_x, OptInt src_y, OptInt size_x, OptInt size_y) {
        using namespace internal;

        int p1 = src_id.value_or(0);
        int p2 = src_x.value_or(0);
        int p3 = src_y.value_or(0);
        int p4 = size_x.value_or(g_gmodeSizeX);
        int p5 = size_y.value_or(g_gmodeSizeY);

        // コピー元サーフェスを取得
        auto srcIt = g_surfaces.find(p1);
        if (srcIt == g_surfaces.end()) return;
        auto srcSurface = srcIt->second;

        // カレントサーフェス（コピー先）を取得
        auto destSurface = getCurrentSurface();
        if (!destSurface) return;

        // コピー元のビットマップを取得（Direct2D 1.1の共有ビットマップ）
        auto srcBitmap = srcSurface->getTargetBitmap();
        if (!srcBitmap) return;

        // コピー先のDeviceContextを取得
        auto destContext = destSurface->getDeviceContext();
        if (!destContext) return;

        // カレントポジションを取得
        int destX = destSurface->getCurrentX();
        int destY = destSurface->getCurrentY();

        // 描画モードに応じて処理
        bool wasDrawing = g_isDrawing;
        if (g_redrawMode == 1 && !wasDrawing) {
            beginDrawIfNeeded();
        }

        // コピー元の領域
        D2D1_RECT_F srcRect = D2D1::RectF(
            static_cast<FLOAT>(p2),
            static_cast<FLOAT>(p3),
            static_cast<FLOAT>(p2 + p4),
            static_cast<FLOAT>(p3 + p5)
        );

        // コピー先の領域（カレントポジションから）
        D2D1_RECT_F destRect = D2D1::RectF(
            static_cast<FLOAT>(destX),
            static_cast<FLOAT>(destY),
            static_cast<FLOAT>(destX + p4),
            static_cast<FLOAT>(destY + p5)
        );

        // コピーモードに応じた処理
        FLOAT opacity = 1.0f;
        D2D1_PRIMITIVE_BLEND primitiveBlend = D2D1_PRIMITIVE_BLEND_SOURCE_OVER;

        if (g_gmodeMode >= 3 && g_gmodeMode <= 6) {
            // 半透明・加算・減算モードの場合はブレンド率を適用
            opacity = g_gmodeBlendRate / 256.0f;
        }

        if (g_gmodeMode == 5) {
            // 加算ブレンド
            primitiveBlend = D2D1_PRIMITIVE_BLEND_ADD;
        } else if (g_gmodeMode == 6) {
            // 減算ブレンド（Direct2Dに直接対応がないためMINで近似）
            primitiveBlend = D2D1_PRIMITIVE_BLEND_MIN;
        }

        destContext->SetPrimitiveBlend(primitiveBlend);

        // Direct2D 1.1では同じDeviceから作成されたビットマップを直接描画可能
        destContext->DrawBitmap(
            srcBitmap,
            destRect,
            opacity,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            srcRect
        );

        // ブレンドモードをリセット
        if (primitiveBlend != D2D1_PRIMITIVE_BLEND_SOURCE_OVER) {
            destContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
        }

        if (g_redrawMode == 1 && !wasDrawing) {
            endDrawAndPresent();
        }
    }

    // ============================================================
    // gzoom - 変倍して画面コピー（HSP互換）
    // Direct2D 1.1: 共有ビットマップを使用して異なるサーフェス間でコピー
    // ============================================================
    void gzoom(OptInt dest_w, OptInt dest_h, OptInt src_id, OptInt src_x, OptInt src_y,
               OptInt src_w, OptInt src_h, OptInt mode) {
        using namespace internal;

        int p1 = dest_w.value_or(g_gmodeSizeX);
        int p2 = dest_h.value_or(g_gmodeSizeY);
        int p3 = src_id.value_or(0);
        int p4 = src_x.value_or(0);
        int p5 = src_y.value_or(0);
        int p6 = src_w.value_or(g_gmodeSizeX);
        int p7 = src_h.value_or(g_gmodeSizeY);
        int p8 = mode.value_or(0);

        // コピー元サーフェスを取得
        auto srcIt = g_surfaces.find(p3);
        if (srcIt == g_surfaces.end()) return;
        auto srcSurface = srcIt->second;

        // カレントサーフェス（コピー先）を取得
        auto destSurface = getCurrentSurface();
        if (!destSurface) return;

        // コピー元のビットマップを取得（Direct2D 1.1の共有ビットマップ）
        auto srcBitmap = srcSurface->getTargetBitmap();
        if (!srcBitmap) return;

        // コピー先のDeviceContextを取得
        auto destContext = destSurface->getDeviceContext();
        if (!destContext) return;

        // カレントポジションを取得
        int destX = destSurface->getCurrentX();
        int destY = destSurface->getCurrentY();

        // 描画モードに応じて処理
        bool wasDrawing = g_isDrawing;
        if (g_redrawMode == 1 && !wasDrawing) {
            beginDrawIfNeeded();
        }

        // コピー元の領域
        D2D1_RECT_F srcRect = D2D1::RectF(
            static_cast<FLOAT>(p4),
            static_cast<FLOAT>(p5),
            static_cast<FLOAT>(p4 + p6),
            static_cast<FLOAT>(p5 + p7)
        );

        // コピー先の領域（変倍、カレントポジションから）
        D2D1_RECT_F destRect = D2D1::RectF(
            static_cast<FLOAT>(destX),
            static_cast<FLOAT>(destY),
            static_cast<FLOAT>(destX + p1),
            static_cast<FLOAT>(destY + p2)
        );

        // コピーモードに応じた処理（gcopyと同様にgmodeを尊重）
        FLOAT opacity = 1.0f;
        D2D1_PRIMITIVE_BLEND primitiveBlend = D2D1_PRIMITIVE_BLEND_SOURCE_OVER;

        if (g_gmodeMode >= 3 && g_gmodeMode <= 6) {
            opacity = g_gmodeBlendRate / 256.0f;
        }

        if (g_gmodeMode == 5) {
            // 加算ブレンド
            primitiveBlend = D2D1_PRIMITIVE_BLEND_ADD;
        } else if (g_gmodeMode == 6) {
            // 減算ブレンド（Direct2Dに直接対応がないためMINで近似）
            primitiveBlend = D2D1_PRIMITIVE_BLEND_MIN;
        }

        // 補間モード
        D2D1_BITMAP_INTERPOLATION_MODE interpMode =
            (p8 == 1) ? D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
                      : D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;

        destContext->SetPrimitiveBlend(primitiveBlend);

        // Direct2D 1.1では同じDeviceから作成されたビットマップを直接描画可能
        destContext->DrawBitmap(
            srcBitmap,
            destRect,
            opacity,
            interpMode,
            srcRect
        );

        // ブレンドモードをリセット
        if (primitiveBlend != D2D1_PRIMITIVE_BLEND_SOURCE_OVER) {
            destContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
        }

        if (g_redrawMode == 1 && !wasDrawing) {
            endDrawAndPresent();
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

            // Direct2D 1.1 デバイスマネージャーの初期化
            D2DDeviceManager& deviceManager = D2DDeviceManager::getInstance();
            if (!deviceManager.initialize()) {
                MessageBoxW(nullptr, L"Failed to initialize Direct2D 1.1 device", L"Error", MB_OK | MB_ICONERROR);
                return;
            }
        }

        void close_system() {
            // すべてのサーフェスを解放
            g_surfaces.clear();
            g_currentSurface.reset();

            // Direct2D 1.1 デバイスマネージャーの終了
            D2DDeviceManager::getInstance().shutdown();

            // WindowManagerはスタティック変数なので明示的な削除は不要

            // COM終了処理
            CoUninitialize();
        }

    } // namespace internal

} // namespace hsppp
