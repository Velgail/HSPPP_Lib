// HspppLib/src/core/Window.cpp
// ウィンドウ管理とメッセージループの実装

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

#include "Internal.h"

// モジュール実装
module hsppp;

namespace hsppp {
namespace internal {

// ========== WindowManager 実装 ==========

WindowManager::WindowManager()
    : m_hInstance(nullptr)
    , m_className(L"HspppWindowClass")  // std::wstring として初期化
    , m_classRegistered(false)
{
    m_hInstance = GetModuleHandle(nullptr);
}

WindowManager::~WindowManager() {
    unregisterWindowClass();
}

WindowManager& WindowManager::getInstance() {
    // C++11以降、静的ローカル変数の初期化はスレッドセーフ
    static WindowManager instance;
    return instance;
}

bool WindowManager::registerWindowClass() {
    if (m_classRegistered) {
        return true;
    }

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // Direct2Dで描画するため背景ブラシは不要
    wc.lpszClassName = m_className.c_str();  // wstring から c_str() で取得

    if (!RegisterClassExW(&wc)) {
        return false;
    }

    m_classRegistered = true;
    return true;
}

void WindowManager::unregisterWindowClass() {
    if (m_classRegistered) {
        UnregisterClassW(m_className.c_str(), m_hInstance);  // wstring から c_str() で取得
        m_classRegistered = false;
    }
}

LRESULT CALLBACK WindowManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Win32 API の制約により、GWLP_USERDATA から HspWindow を取得する際は
    // 生ポインタを使用せざるを得ない。スコープを最小化して安全性を確保。

    // oncmd: 登録されたメッセージを処理
    int oncmdReturnValue = 0;
    if (triggerOnCmd(uMsg, wParam, lParam, oncmdReturnValue)) {
        return oncmdReturnValue;
    }

    switch (uMsg) {
    case WM_CREATE:
    {
        // ウィンドウ作成時：HspWindow ポインタを GWLP_USERDATA に保存
        const auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        if (pCreate && pCreate->lpCreateParams) {
            SetWindowLongPtr(hwnd, GWLP_USERDATA,
                           reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
    {
        // onexit: 終了割り込みを確認
        // ウィンドウIDは今のところ0固定（将来的にはウィンドウ管理が必要）
        if (triggerOnExit(0, 0)) {
            // 割り込みハンドラが設定されている場合は終了をブロック
            // ウィンドウは閉じず、ハンドラ内でend()が呼ばれるまで待機
            return 0;
        }
        DestroyWindow(hwnd);
        return 0;
    }

    case WM_PAINT:
    {
        // ポインタのスコープを最小化：このブロック内でのみ有効
        const auto pWindow = reinterpret_cast<HspWindow*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA));

        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);

        // 厳格な null チェック
        if (pWindow != nullptr) {
            pWindow->onPaint();
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    // onclick: マウスボタン押下
    case WM_LBUTTONDOWN:
        triggerOnClick(0, wParam, lParam);  // 左ボタン = 0
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    case WM_RBUTTONDOWN:
        triggerOnClick(1, wParam, lParam);  // 右ボタン = 1
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    case WM_MBUTTONDOWN:
        triggerOnClick(2, wParam, lParam);  // 中ボタン = 2
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    // onkey: キー入力
    // WM_KEYDOWN/WM_SYSKEYDOWN: 特殊キー（矢印、Ctrl、Alt等）と通常キーの仮想キーコード
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        triggerOnKey(static_cast<int>(wParam), wParam, lParam);
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    // WM_CHAR: 文字入力（後方互換性のため残す）
    case WM_CHAR:
        // WM_KEYDOWNで既に処理されているため、ここでは何もしない
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    // Windowsシャットダウン時
    case WM_QUERYENDSESSION:
        // onexit で処理される可能性がある
        if (triggerOnExit(0, 1)) {
            // 割り込みハンドラが設定されている場合はシャットダウンを遅延
            return TRUE;  // 終了を許可するが、処理を実行
        }
        return TRUE;

    case WM_ENDSESSION:
        if (wParam) {
            // セッションが実際に終了する場合
            PostQuitMessage(0);
        }
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

} // namespace internal
} // namespace hsppp
