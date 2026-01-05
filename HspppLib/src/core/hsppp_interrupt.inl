// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

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
        InterruptHandler handler;  // コールバック関数（ラムダ式対応）
        bool enabled = true;       // 有効/無効
    };

    // エラーハンドラ情報（HspErrorを受け取る）
    struct ErrorHandlerInfo {
        ErrorHandler handler;  // エラーハンドラ（ラムダ式対応）
        bool enabled = true;   // 有効/無効
    };

    // グローバル割り込みハンドラ
    InterruptHandlerInfo g_onclickHandler;
    ErrorHandlerInfo g_onerrorHandler;  // onerrorだけ型が異なる
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
    // 注意: OnErrorはWinMain.cppのtry-catchで即座に処理されるためペンディングしない
    enum class PendingInterruptType {
        None,
        OnClick,
        OnCmd,
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

    const InterruptParams& getInterruptParams() noexcept {
        return g_interruptParams;
    }

    int iparam(const std::source_location& location) noexcept {
        return g_interruptParams.iparam;
    }

    int wparam(const std::source_location& location) noexcept {
        return g_interruptParams.wparam;
    }

    int lparam(const std::source_location& location) noexcept {
        return g_interruptParams.lparam;
    }

    // ============================================================
    // stop - プログラム実行を一時停止
    // ============================================================
    // StateMachine コンテキスト内では遷移予約を検出して早期リターン

    void stop(const std::source_location& location) {
        safe_call(location, [&] {
            MSG msg;
            
            // StateMachine コンテキストを取得（遷移チェック用）
            auto* sm_context = detail::get_current_statemachine();

            // 割り込みが発生するまでメッセージループ
            while (!g_shouldQuit) {
                // StateMachine コンテキストがある場合、遷移予約をチェック
                if (sm_context) {
                    if (sm_context->should_transition() || !sm_context->is_running()) {
                        // 遷移が予約されている、またはステートマシン終了
                        return;
                    }
                }
                
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
        });
    }

    // ============================================================
    // onclick - クリック割り込み実行指定
    // ============================================================

    void onclick(InterruptHandler handler, const std::source_location& location) {
        safe_call(location, [&] {
            g_onclickHandler.handler = handler;
            g_onclickHandler.enabled = (handler != nullptr);
        });
    }

    void onclick(int enable, const std::source_location& location) noexcept {
        g_onclickHandler.enabled = (enable != 0);
    }

    // ============================================================
    // oncmd - Windowsメッセージ割り込み実行指定
    // ============================================================

    void oncmd(InterruptHandler handler, int messageId, const std::source_location& location) {
        safe_call(location, [&] {
            auto& info = g_oncmdHandlers[messageId];
            info.handler = handler;
            info.enabled = (handler != nullptr);
        });
    }

    void oncmd(int enable, int messageId, const std::source_location& location) {
        safe_call(location, [&] {
            auto it = g_oncmdHandlers.find(messageId);
            if (it != g_oncmdHandlers.end()) {
                it->second.enabled = (enable != 0);
            }
        });
    }

    void oncmd(int enable, const std::source_location& location) noexcept {
        g_oncmdGlobalEnabled = (enable != 0);
    }

    // ============================================================
    // onerror - エラー発生時にジャンプ
    // ============================================================

    void onerror(ErrorHandler handler, const std::source_location& location) {
        safe_call(location, [&] {
            g_onerrorHandler.handler = handler;
            g_onerrorHandler.enabled = (handler != nullptr);
        });
    }

    void onerror(int enable, const std::source_location& location) noexcept {
        g_onerrorHandler.enabled = (enable != 0);
    }

    // ============================================================
    // onexit - 終了時にジャンプ
    // ============================================================

    void onexit(InterruptHandler handler, const std::source_location& location) {
        safe_call(location, [&] {
            g_onexitHandler.handler = handler;
            g_onexitHandler.enabled = (handler != nullptr);
        });
    }

    void onexit(int enable, const std::source_location& location) noexcept {
        g_onexitHandler.enabled = (enable != 0);
    }

    // ============================================================
    // onkey - キー割り込み実行指定
    // ============================================================

    void onkey(InterruptHandler handler, const std::source_location& location) {
        safe_call(location, [&] {
            g_onkeyHandler.handler = handler;
            g_onkeyHandler.enabled = (handler != nullptr);
        });
    }

    void onkey(int enable, const std::source_location& location) noexcept {
        g_onkeyHandler.enabled = (enable != 0);
    }

    // ============================================================
    // Screen メンバ関数版
    // ============================================================

    Screen& Screen::onclick(InterruptHandler handler, const std::source_location& location) {
        safe_call(location, [&] {
            hsppp::onclick(handler);
        });
        return *this;
    }

    Screen& Screen::oncmd(InterruptHandler handler, int messageId, const std::source_location& location) {
        safe_call(location, [&] {
            hsppp::oncmd(handler, messageId);
        });
        return *this;
    }

    Screen& Screen::onkey(InterruptHandler handler, const std::source_location& location) {
        safe_call(location, [&] {
            hsppp::onkey(handler);
        });
        return *this;
    }

} // namespace hsppp

// ============================================================
// 内部用: WindowProcから呼び出される割り込みトリガー関数
// ============================================================
namespace hsppp::internal {

    // クリック割り込みをトリガー
    void triggerOnClick(int windowId, int buttonId, WPARAM wp, LPARAM lp) {
        if (g_onclickHandler.enabled && g_onclickHandler.handler) {
            setPendingInterrupt(PendingInterruptType::OnClick, 
                               buttonId,    // iparam: ボタンID
                               windowId,    // wparam: ウィンドウID
                               static_cast<int>(lp));
        }
    }

    // キー割り込みをトリガー
    void triggerOnKey(int windowId, int charCode, WPARAM wp, LPARAM lp) {
        if (g_onkeyHandler.enabled && g_onkeyHandler.handler) {
            setPendingInterrupt(PendingInterruptType::OnKey, 
                               charCode,    // iparam: キーコード
                               windowId,    // wparam: ウィンドウID
                               static_cast<int>(lp));
        }
    }

    // Windowsメッセージ割り込みをトリガー
    bool triggerOnCmd(int windowId, int messageId, WPARAM wp, LPARAM lp, int& returnValue) {
        if (!g_oncmdGlobalEnabled) return false;

        auto it = g_oncmdHandlers.find(messageId);
        if (it == g_oncmdHandlers.end()) return false;

        auto& info = it->second;
        if (!info.enabled || !info.handler) return false;

        // 即座に呼び出し
        g_interruptParams.iparam = messageId;
        g_interruptParams.wparam = windowId;  // ウィンドウID
        g_interruptParams.lparam = static_cast<int>(lp);
        info.handler();
        returnValue = 0;  // デフォルト値
        return false;  // Windowsデフォルト処理を実行
    }

    // 終了割り込みをトリガー
    bool triggerOnExit(int windowId, int reason) {
        if (!g_onexitHandler.enabled || !g_onexitHandler.handler) {
            return false;
        }

        // onexit が設定されている場合は終了をブロック
        // HSP仕様: iparam=終了理由 (0=ユーザー終了, 1=シャットダウン)
        //          wparam=終了通知を受けたウィンドウID
        setPendingInterrupt(PendingInterruptType::OnExit, 
                           reason,    // iparam: 0=ユーザー終了, 1=シャットダウン
                           windowId,  // wparam: ウィンドウID
                           0);
        return true;  // 終了をブロック
    }

    // エラー割り込みは例外処理で直接handleHspErrorによって処理されるため、
    // triggerOnError関数は使用されません（後方互換性のため残していますが、実際には呼び出されない）

    // HspErrorBase派生例外を処理（メインループから呼び出し）
    // HspError（致命的）とHspWeakError（復帰可能）の両方を同じように表示して終了
    void handleHspError(const hsppp::HspErrorBase& error, const std::source_location& location) {
        if (g_onerrorHandler.enabled && g_onerrorHandler.handler) {
            // onerrorハンドラが設定されている場合
            g_onerrorHandler.handler(error);

            // HSP仕様: onerror ハンドラは gosub 相当で、自動的に end で終了
            hsppp::end(1);
        }
        else {
            // ハンドラが設定されていない場合は、HSP互換の簡潔なエラー表示
            // 形式: "#Error <エラー番号> in line <行番号> (<ファイル名>)"
            //       "-->エラーメッセージ"
            // HspWeakErrorの場合は "#Warning" と表示

            // ファイル名を取得（パスの最後の部分のみ）
            std::string fileName = error.file_name();
            size_t lastSlash = fileName.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                fileName = fileName.substr(lastSlash + 1);
            }

            const char* prefix = error.is_fatal() ? "#Error" : "#Warning";
            std::string errorMsg = std::format(
                "{} {} in line {} ({})\n-->{}",
                prefix,
                error.error_code(),
                error.line_number(),
                fileName,
                error.what()
            );

            std::wstring wideMsg = Utf8ToWide(errorMsg);
            MessageBoxW(nullptr, wideMsg.c_str(), L"Error", MB_OK | MB_ICONWARNING);
            hsppp::end(1);
        }
    }

} // namespace hsppp::internal
