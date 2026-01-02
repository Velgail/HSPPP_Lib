// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp_interrupt.ixx
// 割り込みモジュール: onclick, onerror, stop 等の割り込み/エラー処理

export module hsppp:interrupt;

import :types;

import <string>;
import <string_view>;
import <stdexcept>;
import <format>;
import <source_location>;
import <exception>;
import <memory>;

export namespace hsppp {

    // ============================================================
    // エラーコード定義
    // ============================================================

    inline constexpr int ERR_NONE              = 0;
    inline constexpr int ERR_SYNTAX            = 1;
    inline constexpr int ERR_ILLEGAL_FUNCTION  = 2;
    inline constexpr int ERR_LABEL_REQUIRED    = 3;
    inline constexpr int ERR_OUT_OF_MEMORY     = 4;
    inline constexpr int ERR_TYPE_MISMATCH     = 5;
    inline constexpr int ERR_OUT_OF_ARRAY      = 6;
    inline constexpr int ERR_OUT_OF_RANGE      = 7;
    inline constexpr int ERR_DIVIDE_BY_ZERO    = 8;
    inline constexpr int ERR_BUFFER_OVERFLOW   = 9;
    inline constexpr int ERR_UNSUPPORTED       = 10;
    inline constexpr int ERR_EXPRESSION        = 11;
    inline constexpr int ERR_FILE_IO           = 12;
    inline constexpr int ERR_WINDOW_INIT       = 13;
    inline constexpr int ERR_INVALID_HANDLE    = 14;
    inline constexpr int ERR_EXTERNAL_EXECUTE  = 15;
    inline constexpr int ERR_SYSTEM_ERROR      = 16;
    inline constexpr int ERR_INTERNAL          = 17;

    // ============================================================
    // HspErrorBase - エラー例外基底クラス
    // ============================================================

    /// @brief HSPエラー例外の基底クラス
    class HspErrorBase : public std::runtime_error {
    protected:
        int m_errorCode;
        int m_lineNumber;
        std::string m_fileName;
        std::string m_functionName;
        std::string m_message;
        std::exception_ptr m_originalException;

        /// @brief エラー例外を構築（派生クラス用）
        HspErrorBase(const char* prefix,
                    int errorCode,
                    std::string_view message,
                    const std::source_location& location)
            : std::runtime_error(std::format("[{} {}] {}", prefix, errorCode, message))
            , m_errorCode(errorCode)
            , m_lineNumber(static_cast<int>(location.line()))
            , m_fileName(location.file_name())
            , m_functionName(location.function_name())
            , m_message(message)
            , m_originalException(nullptr)
        {}

        /// @brief std::exceptionを抱え込んでエラー例外を構築（派生クラス用）
        HspErrorBase(const char* prefix,
                    int errorCode,
                    std::string_view message,
                    std::exception_ptr originalException,
                    const std::source_location& location)
            : std::runtime_error(std::format("[{} {}] {}", prefix, errorCode, message))
            , m_errorCode(errorCode)
            , m_lineNumber(static_cast<int>(location.line()))
            , m_fileName(location.file_name())
            , m_functionName(location.function_name())
            , m_message(message)
            , m_originalException(std::move(originalException))
        {}

        /// @brief std::exceptionから直接エラー例外を構築（派生クラス用）
        HspErrorBase(const char* prefix,
                    int errorCode,
                    const std::exception& originalException,
                    const std::source_location& location)
            : std::runtime_error(std::format("[{} {}] {}", prefix, errorCode, originalException.what()))
            , m_errorCode(errorCode)
            , m_lineNumber(static_cast<int>(location.line()))
            , m_fileName(location.file_name())
            , m_functionName(location.function_name())
            , m_message(originalException.what())
            , m_originalException(std::current_exception())
        {}

    public:
        virtual ~HspErrorBase() = default;

        /// @brief エラーコードを取得
        [[nodiscard]] int error_code() const noexcept { return m_errorCode; }

        /// @brief 行番号を取得
        [[nodiscard]] int line_number() const noexcept { return m_lineNumber; }

        /// @brief ファイル名を取得
        [[nodiscard]] const std::string& file_name() const noexcept { return m_fileName; }

        /// @brief 関数名を取得
        [[nodiscard]] const std::string& function_name() const noexcept { return m_functionName; }

        /// @brief 元のエラーメッセージを取得
        [[nodiscard]] const std::string& message() const noexcept { return m_message; }

        /// @brief 元の例外を取得
        [[nodiscard]] std::exception_ptr original_exception() const noexcept { return m_originalException; }

        /// @brief 元の例外が存在するかチェック
        [[nodiscard]] bool has_original_exception() const noexcept { return m_originalException != nullptr; }

        /// @brief 元の例外を再スローする
        void rethrow_original() const {
            if (m_originalException) {
                std::rethrow_exception(m_originalException);
            }
        }

        /// @brief 致命的エラーかどうか
        [[nodiscard]] virtual bool is_fatal() const noexcept = 0;
    };

    // ============================================================
    // HspError - 致命的エラー例外クラス
    // ============================================================

    /// @brief HSP致命的エラー例外
    class HspError : public HspErrorBase {
    public:
        HspError(int errorCode,
                std::string_view message,
                const std::source_location& location = std::source_location::current())
            : HspErrorBase("HSP Error", errorCode, message, location)
        {}

        HspError(int errorCode,
                std::string_view message,
                std::exception_ptr originalException,
                const std::source_location& location = std::source_location::current())
            : HspErrorBase("HSP Error", errorCode, message, std::move(originalException), location)
        {}

        HspError(int errorCode,
                const std::exception& originalException,
                const std::source_location& location = std::source_location::current())
            : HspErrorBase("HSP Error", errorCode, originalException, location)
        {}

        [[nodiscard]] bool is_fatal() const noexcept override { return true; }
    };

    // ============================================================
    // HspWeakError - 復帰可能エラー例外クラス
    // ============================================================

    /// @brief HSP復帰可能エラー例外
    class HspWeakError : public HspErrorBase {
    public:
        HspWeakError(int errorCode,
                    std::string_view message,
                    const std::source_location& location = std::source_location::current())
            : HspErrorBase("HSP Warning", errorCode, message, location)
        {}

        HspWeakError(int errorCode,
                    std::string_view message,
                    std::exception_ptr originalException,
                    const std::source_location& location = std::source_location::current())
            : HspErrorBase("HSP Warning", errorCode, message, std::move(originalException), location)
        {}

        HspWeakError(int errorCode,
                    const std::exception& originalException,
                    const std::source_location& location = std::source_location::current())
            : HspErrorBase("HSP Warning", errorCode, originalException, location)
        {}

        [[nodiscard]] bool is_fatal() const noexcept override { return false; }
    };

    // ============================================================
    // 割り込み情報
    // ============================================================

    /// @brief 割り込み発生時のパラメータ
    struct InterruptParams {
        int iparam;
        int wparam;
        int lparam;
    };

    /// @brief 現在の割り込みパラメータを取得
    const InterruptParams& getInterruptParams() noexcept;

    /// @brief システム変数 iparam を取得
    int iparam(const std::source_location& location = std::source_location::current()) noexcept;

    /// @brief システム変数 wparam を取得
    int wparam(const std::source_location& location = std::source_location::current()) noexcept;

    /// @brief システム変数 lparam を取得
    int lparam(const std::source_location& location = std::source_location::current()) noexcept;

    // ============================================================
    // sysval互換（Windowsハンドル系）
    // ============================================================

    /// @brief 現在選択されているウィンドウのハンドル
    int64_t hwnd(const std::source_location& location = std::source_location::current());

    /// @brief 現在のデバイスコンテキスト
    int64_t hdc(const std::source_location& location = std::source_location::current());

    /// @brief 現在のインスタンスハンドル
    int64_t hinstance(const std::source_location& location = std::source_location::current());

    // ============================================================
    // sendmsg
    // ============================================================

    /// @brief 指定したウィンドウにメッセージを送信
    int64_t sendmsg(int64_t hwndValue, int msg, int64_t wparam = 0, int64_t lparam = 0, const std::source_location& location = std::source_location::current());

    /// @brief lParamに文字列ポインタを渡す簡易版
    int64_t sendmsg(int64_t hwndValue, int msg, int64_t wparam, std::string_view lparamText, const std::source_location& location = std::source_location::current());

    // ============================================================
    // 割り込みハンドラ設定
    // ============================================================

    /// @brief エラー発生時の割り込みを設定
    void onerror(ErrorHandler handler, const std::source_location& location = std::source_location::current());

    /// @brief onerror割り込みの一時停止/再開
    void onerror(int enable, const std::source_location& location = std::source_location::current()) noexcept;

    /// @brief マウスクリック時の割り込みを設定
    void onclick(InterruptHandler handler, const std::source_location& location = std::source_location::current());

    /// @brief onclick割り込みの一時停止/再開
    void onclick(int enable, const std::source_location& location = std::source_location::current()) noexcept;

    /// @brief Windowsメッセージ受信時の割り込みを設定
    void oncmd(InterruptHandler handler, int messageId, const std::source_location& location = std::source_location::current());

    /// @brief 指定メッセージIDの割り込みの一時停止/再開
    void oncmd(int enable, int messageId, const std::source_location& location = std::source_location::current());

    /// @brief oncmd割り込み全体の一時停止/再開
    void oncmd(int enable, const std::source_location& location = std::source_location::current()) noexcept;

    /// @brief 終了ボタン押下時の割り込みを設定
    void onexit(InterruptHandler handler, const std::source_location& location = std::source_location::current());

    /// @brief onexit割り込みの一時停止/再開
    void onexit(int enable, const std::source_location& location = std::source_location::current()) noexcept;

    /// @brief キー入力時の割り込みを設定
    void onkey(InterruptHandler handler, const std::source_location& location = std::source_location::current());

    /// @brief onkey割り込みの一時停止/再開
    void onkey(int enable, const std::source_location& location = std::source_location::current()) noexcept;

    /// @brief プログラムを一時停止し、割り込みを待機
    void stop(const std::source_location& location = std::source_location::current());

    // ============================================================
    // System / Internal
    // ============================================================

    namespace internal {
        /// @brief ライブラリの初期化処理
        void init_system(const std::source_location& location = std::source_location::current());

        /// @brief ライブラリの終了処理
        void close_system(const std::source_location& location = std::source_location::current());

        /// @brief HspErrorBase派生例外を処理
        void handleHspError(const HspErrorBase& error, const std::source_location& location = std::source_location::current());
    }

} // namespace hsppp

// グローバル名前空間にユーザー定義関数が存在することを期待する
extern void hspMain();
