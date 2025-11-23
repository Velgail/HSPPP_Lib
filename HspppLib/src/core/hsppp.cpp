// HspppLib/src/core/hsppp.cpp

// グローバルモジュールフラグメント
// モジュール化されていないヘッダーはここでincludeする
module;

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <vector>

// モジュール実装
module hsppp;

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

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

    // Direct2D / DirectWrite リソース
    ID2D1Factory* g_pD2DFactory = nullptr;
    ID2D1HwndRenderTarget* g_pRenderTarget = nullptr;
    IDWriteFactory* g_pDWriteFactory = nullptr;
    IDWriteTextFormat* g_pTextFormat = nullptr;
    ID2D1SolidColorBrush* g_pBrush = nullptr;

    // 描画状態
    bool g_isDrawing = false; // BeginDraw/EndDraw の状態管理
    int g_windowWidth = 0;
    int g_windowHeight = 0;
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
        // 既存のRenderTargetを解放
        if (g_pBrush) {
            g_pBrush->Release();
            g_pBrush = nullptr;
        }
        if (g_pRenderTarget) {
            g_pRenderTarget->Release();
            g_pRenderTarget = nullptr;
        }

        // 既存のウィンドウがあれば破棄
        if (g_hwnd) {
            DestroyWindow(g_hwnd);
            g_hwnd = nullptr;
        }

        g_shouldQuit = false;
        g_windowWidth = width;
        g_windowHeight = height;

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

        // Direct2D RenderTarget の作成
        if (g_pD2DFactory) {
            D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
            D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(
                g_hwnd,
                D2D1::SizeU(width, height)
            );

            HRESULT hr = g_pD2DFactory->CreateHwndRenderTarget(
                props,
                hwndProps,
                &g_pRenderTarget
            );

            if (FAILED(hr)) {
                MessageBoxW(nullptr, L"Failed to create render target", L"Error", MB_OK | MB_ICONERROR);
            }

            // デフォルトのブラシを作成（黒色）
            if (g_pRenderTarget) {
                g_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                    &g_pBrush
                );
            }
        }

        // ウィンドウを表示
        ShowWindow(g_hwnd, SW_SHOW);
        UpdateWindow(g_hwnd);
    }

    // 描画制御
    void redraw(int p1) {
        if (!g_pRenderTarget) return;

        if (p1 == 0) {
            // 描画開始
            if (!g_isDrawing) {
                g_pRenderTarget->BeginDraw();
                g_isDrawing = true;
            }
        }
        else {
            // 描画終了＆画面反映
            if (g_isDrawing) {
                HRESULT hr = g_pRenderTarget->EndDraw();
                g_isDrawing = false;

                // デバイスロスト時は再作成が必要（今は無視）
                if (hr == D2DERR_RECREATE_TARGET) {
                    // TODO: RenderTargetの再作成
                }
            }
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

        // ブラシの色を更新
        if (g_pBrush && g_pRenderTarget) {
            g_pBrush->SetColor(D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f));
        }
    }

    // 描画位置設定
    void pos(int x, int y) {
        g_currentX = x;
        g_currentY = y;
    }

    // 文字列描画
    void mes(const char* text) {
        if (!g_pRenderTarget || !g_pBrush || !g_pTextFormat) return;
        if (!g_isDrawing) return; // 描画中でなければ何もしない

        // UTF-8からUTF-16に変換
        std::wstring wideText = Utf8ToWide(text);

        // テキスト描画領域（とりあえず大きめに）
        D2D1_RECT_F layoutRect = D2D1::RectF(
            static_cast<FLOAT>(g_currentX),
            static_cast<FLOAT>(g_currentY),
            static_cast<FLOAT>(g_currentX + 1000),
            static_cast<FLOAT>(g_currentY + 1000)
        );

        g_pRenderTarget->DrawText(
            wideText.c_str(),
            static_cast<UINT32>(wideText.length()),
            g_pTextFormat,
            layoutRect,
            g_pBrush
        );
    }

    // 矩形塗りつぶし（座標指定版）
    void boxf(int x1, int y1, int x2, int y2) {
        if (!g_pRenderTarget || !g_pBrush) return;
        if (!g_isDrawing) return; // 描画中でなければ何もしない

        D2D1_RECT_F rect = D2D1::RectF(
            static_cast<FLOAT>(x1),
            static_cast<FLOAT>(y1),
            static_cast<FLOAT>(x2),
            static_cast<FLOAT>(y2)
        );

        g_pRenderTarget->FillRectangle(rect, g_pBrush);
    }

    // 矩形塗りつぶし（全画面版）
    void boxf() {
        // 画面全体を塗りつぶす
        boxf(0, 0, g_windowWidth, g_windowHeight);
    }

    namespace internal {

        void init_system() {
            // COM初期化
            CoInitialize(nullptr);

            // インスタンスハンドルの取得
            g_hInstance = GetModuleHandle(nullptr);

            // Direct2D Factory の作成
            HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
            if (FAILED(hr)) {
                MessageBoxW(nullptr, L"Failed to create Direct2D factory", L"Error", MB_OK | MB_ICONERROR);
            }

            // DirectWrite Factory の作成
            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&g_pDWriteFactory)
            );
            if (FAILED(hr)) {
                MessageBoxW(nullptr, L"Failed to create DirectWrite factory", L"Error", MB_OK | MB_ICONERROR);
            }

            // デフォルトのテキストフォーマット作成
            if (g_pDWriteFactory) {
                g_pDWriteFactory->CreateTextFormat(
                    L"MS Gothic",                       // フォント名
                    nullptr,                            // フォントコレクション
                    DWRITE_FONT_WEIGHT_NORMAL,
                    DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL,
                    14.0f,                              // フォントサイズ
                    L"ja-jp",                           // ロケール
                    &g_pTextFormat
                );
            }

            // ウィンドウクラスの登録（Unicode版）
            WNDCLASSEXW wc = {};
            wc.cbSize = sizeof(WNDCLASSEXW);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = WindowProc;
            wc.hInstance = g_hInstance;
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hbrBackground = nullptr; // Direct2Dで描画するため背景ブラシは不要
            wc.lpszClassName = g_windowClassName;

            if (!RegisterClassExW(&wc)) {
                MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
            }
        }

        void close_system() {
            // Direct2D/DirectWrite リソースの解放
            if (g_pBrush) {
                g_pBrush->Release();
                g_pBrush = nullptr;
            }
            if (g_pTextFormat) {
                g_pTextFormat->Release();
                g_pTextFormat = nullptr;
            }
            if (g_pRenderTarget) {
                g_pRenderTarget->Release();
                g_pRenderTarget = nullptr;
            }
            if (g_pDWriteFactory) {
                g_pDWriteFactory->Release();
                g_pDWriteFactory = nullptr;
            }
            if (g_pD2DFactory) {
                g_pD2DFactory->Release();
                g_pD2DFactory = nullptr;
            }

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
