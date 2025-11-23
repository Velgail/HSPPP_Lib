// HspppLib/src/core/Window.cpp
// ウィンドウ管理とメッセージループの実装

#include "Internal.h"

namespace hsppp {
namespace internal {

// ========== WindowManager 実装 ==========

WindowManager::WindowManager()
    : m_hInstance(nullptr)
    , m_className(L"HspppWindowClass")
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
    wc.lpszClassName = m_className;

    if (!RegisterClassExW(&wc)) {
        return false;
    }

    m_classRegistered = true;
    return true;
}

void WindowManager::unregisterWindowClass() {
    if (m_classRegistered) {
        UnregisterClassW(m_className, m_hInstance);
        m_classRegistered = false;
    }
}

LRESULT CALLBACK WindowManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // ウィンドウ作成時に HspWindow ポインタを保存
    HspWindow* pWindow = nullptr;

    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pWindow = reinterpret_cast<HspWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
    }
    else {
        pWindow = reinterpret_cast<HspWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

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
        BeginPaint(hwnd, &ps);

        // HspWindow インスタンスを取得し、オフスクリーンバッファを画面にコピー
        if (pWindow) {
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
