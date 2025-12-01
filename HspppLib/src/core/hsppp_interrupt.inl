// HspppLib/src/core/hsppp_interrupt.inl
// 割り込みハンドラ関数の実装（onclick, oncmd, onerror, onexit, onkey）
// hsppp.cpp から #include されることを想定

namespace {
    using namespace hsppp;
    using namespace hsppp::internal;

    // ============================================================
    // 割り込みハンドラの内部管理構造
    // ============================================================

    // 割り込みハンドラ情報
    struct InterruptHandlerInfo {
        InterruptHandler handler = nullptr;  // コールバック関数ポインタ
        bool enabled = true;                 // 有効/無効
    };

    // グローバル割り込みハンドラ
    InterruptHandlerInfo g_onclickHandler;
    InterruptHandlerInfo g_onerrorHandler;
    InterruptHandlerInfo g_onexitHandler;
    InterruptHandlerInfo g_onkeyHandler;

    // oncmd: メッセージID別のハンドラ
    std::map<int, InterruptHandlerInfo> g_oncmdHandlers;
    bool g_oncmdGlobalEnabled = true;

    // 割り込みパラメータ（システム変数相当）
    InterruptParams g_interruptParams = {0, 0, 0};

    // 割り込みが発生しているかのフラグ
    bool g_interruptPending = false;

    // ペンディング中の割り込み種類
    enum class PendingInterruptType {
        None,
        OnClick,
        OnCmd,
        OnError,
        OnExit,
        OnKey
    };
    PendingInterruptType g_pendingType = PendingInterruptType::None;
    int g_pendingMessageId = 0;  // oncmd用

    // 割り込みを処理する内部関数
    bool processPendingInterrupt() {
        if (!g_interruptPending) return false;

        g_interruptPending = false;
        InterruptHandlerInfo* handlerInfo = nullptr;

        switch (g_pendingType) {
        case PendingInterruptType::OnClick:
            handlerInfo = &g_onclickHandler;
            break;
        case PendingInterruptType::OnCmd:
            {
                auto it = g_oncmdHandlers.find(g_pendingMessageId);
                if (it != g_oncmdHandlers.end()) {
                    handlerInfo = &it->second;
                }
            }
            break;
        case PendingInterruptType::OnError:
            handlerInfo = &g_onerrorHandler;
            break;
        case PendingInterruptType::OnExit:
            handlerInfo = &g_onexitHandler;
            break;
        case PendingInterruptType::OnKey:
            handlerInfo = &g_onkeyHandler;
            break;
        default:
            break;
        }

        g_pendingType = PendingInterruptType::None;

        if (!handlerInfo || !handlerInfo->enabled || !handlerInfo->handler) {
            return false;
        }

        handlerInfo->handler();
        return true;
    }

    // 割り込みをペンディングにセット
    void setPendingInterrupt(PendingInterruptType type, int ip, int wp, int lp, int msgId = 0) {
        g_interruptParams.iparam = ip;
        g_interruptParams.wparam = wp;
        g_interruptParams.lparam = lp;
        g_pendingType = type;
        g_pendingMessageId = msgId;
        g_interruptPending = true;
    }

} // anonymous namespace

namespace hsppp {

    // ============================================================
    // 割り込みパラメータ取得関数
    // ============================================================

    const InterruptParams& getInterruptParams() {
        return g_interruptParams;
    }

    int iparam() {
        return g_interruptParams.iparam;
    }

    int wparam() {
        return g_interruptParams.wparam;
    }

    int lparam() {
        return g_interruptParams.lparam;
    }

    // ============================================================
    // stop - プログラム実行を一時停止
    // ============================================================

    void stop() {
        MSG msg;

        // 割り込みが発生するまでメッセージループ
        while (!g_shouldQuit) {
            // ペンディング中の割り込みを処理
            if (processPendingInterrupt()) {
                return;  // 割り込みハンドラが呼ばれたら戻る
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

    // ============================================================
    // onclick - クリック割り込み実行指定
    // ============================================================

    void onclick(InterruptHandler handler) {
        g_onclickHandler.handler = handler;
        g_onclickHandler.enabled = (handler != nullptr);
    }

    void onclick(int enable) {
        g_onclickHandler.enabled = (enable != 0);
    }

    // ============================================================
    // oncmd - Windowsメッセージ割り込み実行指定
    // ============================================================

    void oncmd(InterruptHandler handler, int messageId) {
        auto& info = g_oncmdHandlers[messageId];
        info.handler = handler;
        info.enabled = (handler != nullptr);
    }

    void oncmd(int enable, int messageId) {
        auto it = g_oncmdHandlers.find(messageId);
        if (it != g_oncmdHandlers.end()) {
            it->second.enabled = (enable != 0);
        }
    }

    void oncmd(int enable) {
        g_oncmdGlobalEnabled = (enable != 0);
    }

    // ============================================================
    // onerror - エラー発生時にジャンプ
    // ============================================================

    void onerror(InterruptHandler handler) {
        g_onerrorHandler.handler = handler;
        g_onerrorHandler.enabled = (handler != nullptr);
    }

    void onerror(int enable) {
        g_onerrorHandler.enabled = (enable != 0);
    }

    // ============================================================
    // onexit - 終了時にジャンプ
    // ============================================================

    void onexit(InterruptHandler handler) {
        g_onexitHandler.handler = handler;
        g_onexitHandler.enabled = (handler != nullptr);
    }

    void onexit(int enable) {
        g_onexitHandler.enabled = (enable != 0);
    }

    // ============================================================
    // onkey - キー割り込み実行指定
    // ============================================================

    void onkey(InterruptHandler handler) {
        g_onkeyHandler.handler = handler;
        g_onkeyHandler.enabled = (handler != nullptr);
    }

    void onkey(int enable) {
        g_onkeyHandler.enabled = (enable != 0);
    }

    // ============================================================
    // Screen メンバ関数版
    // ============================================================

    Screen& Screen::onclick(InterruptHandler handler) {
        hsppp::onclick(handler);
        return *this;
    }

    Screen& Screen::oncmd(InterruptHandler handler, int messageId) {
        hsppp::oncmd(handler, messageId);
        return *this;
    }

    Screen& Screen::onkey(InterruptHandler handler) {
        hsppp::onkey(handler);
        return *this;
    }

} // namespace hsppp

// ============================================================
// 内部用: WindowProcから呼び出される割り込みトリガー関数
// ============================================================
namespace hsppp::internal {

    // クリック割り込みをトリガー
    void triggerOnClick(int buttonId, WPARAM wp, LPARAM lp) {
        if (g_onclickHandler.enabled && g_onclickHandler.handler) {
            setPendingInterrupt(PendingInterruptType::OnClick, 
                               buttonId, 
                               static_cast<int>(wp), 
                               static_cast<int>(lp));
        }
    }

    // キー割り込みをトリガー
    void triggerOnKey(int charCode, WPARAM wp, LPARAM lp) {
        if (g_onkeyHandler.enabled && g_onkeyHandler.handler) {
            setPendingInterrupt(PendingInterruptType::OnKey, 
                               charCode, 
                               static_cast<int>(wp), 
                               static_cast<int>(lp));
        }
    }

    // Windowsメッセージ割り込みをトリガー
    bool triggerOnCmd(int messageId, WPARAM wp, LPARAM lp, int& returnValue) {
        if (!g_oncmdGlobalEnabled) return false;

        auto it = g_oncmdHandlers.find(messageId);
        if (it == g_oncmdHandlers.end()) return false;

        auto& info = it->second;
        if (!info.enabled || !info.handler) return false;

        // 即座に呼び出して戻り値を取得
        g_interruptParams.iparam = messageId;
        g_interruptParams.wparam = static_cast<int>(wp);
        g_interruptParams.lparam = static_cast<int>(lp);
        returnValue = info.handler();
        return (returnValue != -1);  // -1以外ならカスタム戻り値を使用
    }

    // 終了割り込みをトリガー
    bool triggerOnExit(int windowId, int reason) {
        if (!g_onexitHandler.enabled || !g_onexitHandler.handler) {
            return false;
        }

        // onexit が設定されている場合は終了をブロック
        setPendingInterrupt(PendingInterruptType::OnExit, 
                           reason,  // 0=ユーザー終了, 1=シャットダウン
                           windowId, 
                           0);
        return true;  // 終了をブロック
    }

    // エラー割り込みをトリガー
    void triggerOnError(int errorCode, int lineNumber) {
        if (g_onerrorHandler.enabled && g_onerrorHandler.handler) {
            g_interruptParams.wparam = errorCode;
            g_interruptParams.lparam = lineNumber;
            g_interruptParams.iparam = 0;
            g_onerrorHandler.handler();
        }
    }

} // namespace hsppp::internal
