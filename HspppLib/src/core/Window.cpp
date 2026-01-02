// HspppLib/src/core/Window.cpp
// ウィンドウ管理とメッセージループの実装

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

    // パフォーマンス最適化: HspWindowポインタから直接ウィンドウIDを取得
    auto pWindow = reinterpret_cast<HspWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    const int windowId = pWindow ? pWindow->getWindowId() : getWindowIdFromHwnd(hwnd);

    // oncmd: 登録されたメッセージを処理
    int oncmdReturnValue = 0;
    if (triggerOnCmd(windowId, uMsg, wParam, lParam, oncmdReturnValue)) {
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
        if (triggerOnExit(windowId, 0)) {
            // 割り込みハンドラが設定されている場合は終了をブロック
            // ウィンドウは閉じず、ハンドラ内でend()が呼ばれるまで待機
            return 0;
        }
        // onexitが未定義の場合は、ウィンドウを閉じてアプリケーションを終了
        DestroyWindow(hwnd);
        hsppp::end(0);
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);

        // 厳格な null チェック（pWindowは既に取得済み）
        if (pWindow != nullptr) {
            pWindow->onPaint();
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    // onclick: マウスボタン押下
    case WM_LBUTTONDOWN:
        triggerOnClick(windowId, 0, wParam, lParam);  // 左ボタン = 0
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    case WM_RBUTTONDOWN:
        triggerOnClick(windowId, 1, wParam, lParam);  // 右ボタン = 1
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    case WM_MBUTTONDOWN:
        triggerOnClick(windowId, 2, wParam, lParam);  // 中ボタン = 2
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    // onkey: キー入力
    // WM_KEYDOWN/WM_SYSKEYDOWN: 特殊キー（矢印、Ctrl、Alt等）と通常キーの仮想キーコード
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        triggerOnKey(windowId, static_cast<int>(wParam), wParam, lParam);
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    // WM_CHAR: 文字入力（後方互換性のため残す）
    case WM_CHAR:
        // WM_KEYDOWNで既に処理されているため、ここでは何もしない
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    // マウスホイール
    case WM_MOUSEWHEEL:
    {
        // WHEEL_DELTAは120が1ノッチに相当
        int delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        setMouseWheelDelta(delta);
        return 0;
    }

    // Windowsシャットダウン時
    case WM_QUERYENDSESSION:
        // onexit で処理される可能性がある
        if (triggerOnExit(windowId, 1)) {
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

    // ウィンドウサイズ変更
    case WM_SIZE:
    {
        if (wParam != SIZE_MINIMIZED) {
            if (pWindow != nullptr) {
                int newWidth = LOWORD(lParam);
                int newHeight = HIWORD(lParam);
                pWindow->onSize(newWidth, newHeight);
            }
        }
        return 0;
    }

    // ウィンドウの最大/最小サイズ制限（HSP仕様：バッファサイズがリミッター）
    case WM_GETMINMAXINFO:
    {
        if (pWindow != nullptr) {
            auto pMinMax = reinterpret_cast<MINMAXINFO*>(lParam);
            
            // バッファサイズ（m_width, m_height）を最大クライアントサイズとする
            int maxClientW = pWindow->getWidth();
            int maxClientH = pWindow->getHeight();
            
            // クライアントサイズからウィンドウサイズを計算
            DWORD style = static_cast<DWORD>(GetWindowLongPtr(hwnd, GWL_STYLE));
            DWORD exStyle = static_cast<DWORD>(GetWindowLongPtr(hwnd, GWL_EXSTYLE));
            RECT rect = { 0, 0, maxClientW, maxClientH };
            AdjustWindowRectEx(&rect, style, FALSE, exStyle);
            
            pMinMax->ptMaxTrackSize.x = rect.right - rect.left;
            pMinMax->ptMaxTrackSize.y = rect.bottom - rect.top;
        }
        return 0;
    }

    // GUIコントロールからの通知（ボタンクリック、コンボボックス変更等）
    case WM_COMMAND:
    {
        WORD notifyCode = HIWORD(wParam);
        WORD controlId = LOWORD(wParam);
        HWND hwndControl = reinterpret_cast<HWND>(lParam);
        
        // ObjectManagerからオブジェクトを検索してコールバックを実行
        auto& objMgr = ObjectManager::getInstance();
        int objectId = objMgr.findObjectByHwnd(hwndControl);
        if (objectId >= 0) {
            ObjectInfo* pInfo = objMgr.getObject(objectId);
            if (pInfo) {
                // ボタンクリック
                if (pInfo->type == ObjectType::Button && notifyCode == BN_CLICKED) {
                    if (pInfo->callback) {
                        pInfo->callback();
                    }
                }
                // チェックボックス状態変更
                else if (pInfo->type == ObjectType::Chkbox && notifyCode == BN_CLICKED) {
                    int* pVar = pInfo->getStateVar();
                    if (pVar) {
                        LRESULT state = SendMessageW(hwndControl, BM_GETCHECK, 0, 0);
                        *pVar = (state == BST_CHECKED) ? 1 : 0;
                    }
                }
                // コンボボックス選択変更
                else if (pInfo->type == ObjectType::Combox && notifyCode == CBN_SELCHANGE) {
                    int* pVar = pInfo->getStateVar();
                    if (pVar) {
                        LRESULT sel = SendMessageW(hwndControl, CB_GETCURSEL, 0, 0);
                        *pVar = static_cast<int>(sel);
                    }
                }
                // リストボックス選択変更
                else if (pInfo->type == ObjectType::Listbox && notifyCode == LBN_SELCHANGE) {
                    int* pVar = pInfo->getStateVar();
                    if (pVar) {
                        LRESULT sel = SendMessageW(hwndControl, LB_GETCURSEL, 0, 0);
                        *pVar = static_cast<int>(sel);
                    }
                }
                // Input/Mesbox のテキスト変更（即時同期）
                else if ((pInfo->type == ObjectType::Input || pInfo->type == ObjectType::Mesbox) && notifyCode == EN_CHANGE) {
                    objMgr.syncSingleInputControl(hwndControl);
                }
            }
        }
        return 0;
    }

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

} // namespace internal
} // namespace hsppp
