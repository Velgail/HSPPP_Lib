// HspppLib/src/core/hsppp.cpp
module hsppp;

#include <windows.h>

// グローバル変数（一時的なスタブ実装用）
namespace {
    HWND g_hwnd = nullptr;
    int g_currentX = 0;
    int g_currentY = 0;
    COLORREF g_currentColor = RGB(0, 0, 0);
    bool g_shouldQuit = false;
}

namespace hsppp {

// ウィンドウ初期化
void screen(int id, int width, int height, int mode, const char* title) {
    // 最小限のスタブ実装
    // TODO: 実際のウィンドウ作成処理を実装
    g_shouldQuit = false;
}

// 描画制御
void redraw(int p1) {
    // TODO: バックバッファの制御を実装
    if (p1 == 1 && g_hwnd) {
        // Present処理のスタブ
        InvalidateRect(g_hwnd, nullptr, FALSE);
        UpdateWindow(g_hwnd);
    }
}

// 待機＆メッセージ処理
int await(int time_ms) {
    // メッセージポンプの実装
    MSG msg;
    DWORD startTime = GetTickCount();

    while (GetTickCount() - startTime < (DWORD)time_ms) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_shouldQuit = true;
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Sleep(1);
        }
    }

    return g_shouldQuit ? 0 : 1;
}

// 描画色設定
void color(int r, int g, int b) {
    g_currentColor = RGB(r, g, b);
}

// 描画位置設定
void pos(int x, int y) {
    g_currentX = x;
    g_currentY = y;
}

// 文字列描画
void mes(const char* text) {
    // TODO: Direct2D実装
    // 現在はスタブ
}

// 矩形塗りつぶし（座標指定版）
void boxf(int x1, int y1, int x2, int y2) {
    // TODO: Direct2D実装
    // 現在はスタブ
}

// 矩形塗りつぶし（全画面版）
void boxf() {
    // TODO: 画面サイズを取得して boxf(0, 0, width, height) を呼ぶ
    // 現在はスタブ
}

namespace internal {

void init_system() {
    // TODO: COM初期化、Direct2D Factory作成など
    // 現在はスタブ
    CoInitialize(nullptr);
}

void close_system() {
    // TODO: リソース解放
    // 現在はスタブ
    CoUninitialize();
}

} // namespace internal

} // namespace hsppp
