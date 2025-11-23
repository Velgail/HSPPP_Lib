// HspppLib/src/core/Window.cpp
// ウィンドウ管理とメッセージループの実装

#include "Internal.h"

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
        DestroyWindow(hwnd);
        return 0;

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

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

} // namespace internal
} // namespace hsppp
