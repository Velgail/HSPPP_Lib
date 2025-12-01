// HspppLib/src/core/hsppp_input.inl
// 入力系関数の実装（stick, getkey, mouse, mousex, mousey, mousew, wait）
// hsppp.cpp から #include されることを想定

namespace {
    // stick用の前回キー状態
    DWORD g_prevKeyState = 0;
}

namespace hsppp {

    // ============================================================
    // getkey - キー入力チェック（HSP互換）
    // ============================================================
    int getkey(int keycode) {
        // GetAsyncKeyState で指定キーの状態を取得
        SHORT state = GetAsyncKeyState(keycode);
        // 最上位ビットが1なら押されている
        return (state & 0x8000) ? 1 : 0;
    }

    // ============================================================
    // stick - キー入力情報取得（HSP互換）
    // ============================================================
    int stick(OptInt nonTrigger, OptInt checkActive) {
        using namespace internal;

        int p2 = nonTrigger.value_or(0);
        int p3 = checkActive.value_or(1);

        // p3=1 の場合、HSPウィンドウがアクティブでなければ0を返す
        if (p3 == 1) {
            HWND hwndActive = GetForegroundWindow();
            bool isHspWindowActive = false;
            for (const auto& pair : g_surfaces) {
                auto pWin = std::dynamic_pointer_cast<HspWindow>(pair.second);
                if (pWin && pWin->getHwnd() == hwndActive) {
                    isHspWindowActive = true;
                    break;
                }
            }
            if (!isHspWindowActive) {
                g_prevKeyState = 0;
                return 0;
            }
        }

        // 現在のキー状態を取得
        DWORD currentState = 0;

        // カーソルキー左(←) - VK_LEFT = 37
        if (GetAsyncKeyState(VK_LEFT) & 0x8000)   currentState |= 1;
        // カーソルキー上(↑) - VK_UP = 38
        if (GetAsyncKeyState(VK_UP) & 0x8000)     currentState |= 2;
        // カーソルキー右(→) - VK_RIGHT = 39
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000)  currentState |= 4;
        // カーソルキー下(↓) - VK_DOWN = 40
        if (GetAsyncKeyState(VK_DOWN) & 0x8000)   currentState |= 8;
        // スペースキー - VK_SPACE = 32
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)  currentState |= 16;
        // Enterキー - VK_RETURN = 13
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) currentState |= 32;
        // Ctrlキー - VK_CONTROL = 17
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000) currentState |= 64;
        // ESCキー - VK_ESCAPE = 27
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) currentState |= 128;
        // マウスの左ボタン - VK_LBUTTON = 1
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) currentState |= 256;
        // マウスの右ボタン - VK_RBUTTON = 2
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) currentState |= 512;
        // TABキー - VK_TAB = 9
        if (GetAsyncKeyState(VK_TAB) & 0x8000)    currentState |= 1024;
        // [Z]キー - 'Z' = 90
        if (GetAsyncKeyState('Z') & 0x8000)       currentState |= 2048;
        // [X]キー - 'X' = 88
        if (GetAsyncKeyState('X') & 0x8000)       currentState |= 4096;
        // [C]キー - 'C' = 67
        if (GetAsyncKeyState('C') & 0x8000)       currentState |= 8192;
        // [A]キー - 'A' = 65
        if (GetAsyncKeyState('A') & 0x8000)       currentState |= 16384;
        // [W]キー - 'W' = 87
        if (GetAsyncKeyState('W') & 0x8000)       currentState |= 32768;
        // [D]キー - 'D' = 68
        if (GetAsyncKeyState('D') & 0x8000)       currentState |= 65536;
        // [S]キー - 'S' = 83
        if (GetAsyncKeyState('S') & 0x8000)       currentState |= 131072;

        // トリガー処理
        // p2で指定されたキーは非トリガー（押しっぱなしでも検出）
        // それ以外はトリガー（押した瞬間のみ検出）
        DWORD triggerMask = ~static_cast<DWORD>(p2);  // トリガー対象マスク
        DWORD newlyPressed = currentState & ~g_prevKeyState;  // 新しく押されたキー
        DWORD nonTriggerState = currentState & p2;  // 非トリガーキーの状態
        DWORD triggerState = newlyPressed & triggerMask;  // トリガーキーの新規押下

        // 前回の状態を更新
        g_prevKeyState = currentState;

        return static_cast<int>(triggerState | nonTriggerState);
    }

    // ============================================================
    // mouse - マウスカーソル座標設定（HSP互換）
    // ============================================================
    void mouse(OptInt x, OptInt y, OptInt mode) {
        int p1 = x.value_or(0);
        int p2 = y.value_or(0);
        int p3 = mode.value_or(0);

        // 現在の座標を取得（省略時用）
        POINT pt;
        GetCursorPos(&pt);
        if (x.is_default()) p1 = pt.x;
        if (y.is_default()) p2 = pt.y;

        switch (p3) {
        case 0:
            // p1またはp2がマイナスの時にマウスカーソルを非表示
            // それ以外は移動して表示
            if (p1 < 0 || p2 < 0) {
                ShowCursor(FALSE);
            } else {
                SetCursorPos(p1, p2);
                ShowCursor(TRUE);
            }
            break;
        case -1:
            // 移動して非表示
            SetCursorPos(p1, p2);
            ShowCursor(FALSE);
            break;
        case 1:
            // 移動のみ（表示状態は維持）
            SetCursorPos(p1, p2);
            break;
        case 2:
            // 移動して表示
            SetCursorPos(p1, p2);
            ShowCursor(TRUE);
            break;
        }
    }

    // ============================================================
    // mousex - マウスカーソルのX座標（HSP互換）
    // ============================================================
    int mousex() {
        using namespace internal;

        auto currentSurface = getCurrentSurface();
        auto pWindow = currentSurface ? std::dynamic_pointer_cast<HspWindow>(currentSurface) : nullptr;

        POINT pt;
        GetCursorPos(&pt);

        // ウィンドウがあればクライアント座標に変換
        if (pWindow && pWindow->getHwnd()) {
            ScreenToClient(pWindow->getHwnd(), &pt);
        }

        return pt.x;
    }

    // ============================================================
    // mousey - マウスカーソルのY座標（HSP互換）
    // ============================================================
    int mousey() {
        using namespace internal;

        auto currentSurface = getCurrentSurface();
        auto pWindow = currentSurface ? std::dynamic_pointer_cast<HspWindow>(currentSurface) : nullptr;

        POINT pt;
        GetCursorPos(&pt);

        // ウィンドウがあればクライアント座標に変換
        if (pWindow && pWindow->getHwnd()) {
            ScreenToClient(pWindow->getHwnd(), &pt);
        }

        return pt.y;
    }

    // ============================================================
    // mousew - マウスカーソルのホイール値（HSP互換）
    // ============================================================
    int mousew() {
        // ホイール値はウィンドウメッセージで管理する必要がある
        // 現在の実装では未サポート（0を返す）
        // TODO: WM_MOUSEWHEELをWindowProcでキャプチャして値を保持
        return 0;
    }

    // ============================================================
    // wait - 実行を一定時間中断する（HSP互換）
    // ============================================================
    void wait(OptInt time) {
        int p1 = time.value_or(100);

        // 10ms単位なのでミリ秒に変換
        int waitMs = p1 * 10;

        MSG msg;
        DWORD startTime = GetTickCount();
        DWORD endTime = startTime + waitMs;

        // 待機中もメッセージを処理
        while (GetTickCount() < endTime) {
            // ペンディング中の割り込みを処理
            if (processPendingInterrupt()) {
                // 割り込みハンドラが呼ばれた
            }

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

} // namespace hsppp
