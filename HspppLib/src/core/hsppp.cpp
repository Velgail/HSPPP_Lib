// HspppLib/src/core/hsppp.cpp
module hsppp;

#include <windows.h>
#include <string>
#include <vector>

// ヘルパー関数: UTF-8文字列をUTF-16(wchar_t)に変換
namespace {
    std::wstring Utf8ToWide(const char* utf8str) {
        if (!utf8str || *utf8str == '\0') {
            return L"";
        }

        // 必要なバッファサイズを取得
        int wideSize = MultiByteToWideChar(CP_UTF8, 0, utf8str, -1, nullptr, 0);
        if (wideSize == 0) {
            return L"";
        }

        // 変換
        std::wstring wideStr(wideSize, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8str, -1, &wideStr[0], wideSize);

        // 末尾のnull文字を除去
        if (!wideStr.empty() && wideStr.back() == L'\0') {
            wideStr.pop_back();
        }

        return wideStr;
    }
}

// グローバル変数（一時的なスタブ実装用）
namespace {
    HWND g_hwnd = nullptr;
    HINSTANCE g_hInstance = nullptr;
    int g_currentX = 0;
    int g_currentY = 0;
    COLORREF g_currentColor = RGB(0, 0, 0);
    bool g_shouldQuit = false;
    const wchar_t* g_windowClassName = L"HspppWindowClass";
    DWORD g_lastAwaitTime = 0; // 前回のawait呼び出し時刻
}

// ウィンドウプロシージャ
namespace {
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // TODO: Direct2D描画処理
            // 現在は何も描画しない（黒いウィンドウ）
            EndPaint(hwnd, &ps);
            return 0;
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
}

namespace hsppp {

// ウィンドウ初期化
void screen(int id, int width, int height, int mode, const char* title) {
    // 既存のウィンドウがあれば破棄
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = nullptr;
    }

    g_shouldQuit = false;

    // UTF-8タイトルをUTF-16に変換
    std::wstring wideTitle = Utf8ToWide(title);

    // ウィンドウスタイルの決定
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    if (mode == 1) {
        // 枠なしウィンドウ
        dwStyle = WS_POPUP | WS_VISIBLE;
    }

    // クライアント領域のサイズを指定されたサイズにするため、
    // ウィンドウ全体のサイズを計算
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, dwStyle, FALSE);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    // ウィンドウの作成（Unicode版）
    g_hwnd = CreateWindowExW(
        0,                      // 拡張スタイル
        g_windowClassName,      // ウィンドウクラス名
        wideTitle.c_str(),      // ウィンドウタイトル（UTF-16）
        dwStyle,                // ウィンドウスタイル
        CW_USEDEFAULT,          // X座標
        CW_USEDEFAULT,          // Y座標
        windowWidth,            // 幅
        windowHeight,           // 高さ
        nullptr,                // 親ウィンドウ
        nullptr,                // メニュー
        g_hInstance,            // インスタンスハンドル
        nullptr                 // 追加パラメータ
    );

    if (!g_hwnd) {
        // ウィンドウ作成失敗
        MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // ウィンドウを表示
    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
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

// 待機＆メッセージ処理 (HSP互換)
// 前回のawait呼び出しから指定時間が経過するまで待機する
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
    // COM初期化
    CoInitialize(nullptr);

    // インスタンスハンドルの取得
    g_hInstance = GetModuleHandle(nullptr);

    // ウィンドウクラスの登録（Unicode版）
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = g_windowClassName;

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
    }
}

void close_system() {
    // ウィンドウの破棄
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = nullptr;
    }

    // ウィンドウクラスの登録解除（Unicode版）
    UnregisterClassW(g_windowClassName, g_hInstance);

    // COM終了処理
    CoUninitialize();
}

} // namespace internal

} // namespace hsppp
