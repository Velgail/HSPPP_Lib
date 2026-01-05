// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp_statemachine.ixx
// ═══════════════════════════════════════════════════════════════════
// HSPPP ステートマシン - モジュールインターフェース
// ═══════════════════════════════════════════════════════════════════
//
// HSPの *label / goto を型安全に再現するステートマシンライブラリ。
// enum class による コンパイル時チェック で、存在しないステートへの
// 遷移を防止します（HSPと同等の安全性）。
//
// 使用例:
//   enum class Screen { Title, Game, Result };
//   StateMachine<Screen> sm;
//   sm.state(Screen::Title).on_update([&](auto& sm) {
//       if (getkey(' ')) sm.jump(Screen::Game);
//   });
//   sm.jump(Screen::Title);
//   while (sm.run()) { await(16); }

module;

#define NOMINMAX
#include <windows.h>

export module hsppp:statemachine;

import :types;

import <functional>;
import <map>;
import <set>;
import <deque>;
import <optional>;
import <string>;
import <type_traits>;
import <format>;
import <chrono>;
import <fstream>;

export namespace hsppp {

// ═══════════════════════════════════════════════════════════════════
// 前方宣言
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
class StateMachine;

template<typename StateType>
class StateBuilder;

// ═══════════════════════════════════════════════════════════════════
// StateMachineBase - 非テンプレート基底クラス
// ═══════════════════════════════════════════════════════════════════

/// @brief StateMachineの非テンプレート基底クラス
/// 
/// グローバル関数(await/stop/vwait)がStateMachineコンテキストを
/// 検出するために使用されます。
class StateMachineBase {
public:
    virtual ~StateMachineBase() = default;
    
    /// @brief 遷移が予約されているかチェック
    /// @return 遷移が予約されている場合 true
    [[nodiscard]] virtual bool should_transition() const = 0;
    
    /// @brief ステートマシンが実行中かチェック
    /// @return 実行中の場合 true
    [[nodiscard]] virtual bool is_running() const = 0;
};

// ═══════════════════════════════════════════════════════════════════
// StateMachineContext - RAIIコンテキスト管理（内部用）
// ═══════════════════════════════════════════════════════════════════

namespace detail {
    /// @brief 現在のStateMachineコンテキスト（スレッドローカル）
    inline thread_local StateMachineBase* current_statemachine = nullptr;
    
    /// @brief 現在のStateMachineを取得
    /// @return 現在のStateMachine、なければnullptr
    inline StateMachineBase* get_current_statemachine() noexcept {
        return current_statemachine;
    }
}

/// @brief RAIIによるStateMachineコンテキスト管理
/// 
/// StateMachine::run()内でスコープガードとして使用し、
/// 例外発生時も確実にコンテキストを復元します。
class StateMachineScope {
public:
    /// @brief コンストラクタ: コンテキストを設定
    /// @param sm 設定するStateMachine
    explicit StateMachineScope(StateMachineBase* sm) noexcept
        : prev_(detail::current_statemachine)
    {
        detail::current_statemachine = sm;
    }
    
    /// @brief デストラクタ: コンテキストを復元
    ~StateMachineScope() noexcept {
        detail::current_statemachine = prev_;
    }
    
    // コピー・移動禁止（スタック上でのみ使用）
    StateMachineScope(const StateMachineScope&) = delete;
    StateMachineScope& operator=(const StateMachineScope&) = delete;
    StateMachineScope(StateMachineScope&&) = delete;
    StateMachineScope& operator=(StateMachineScope&&) = delete;
    
private:
    StateMachineBase* prev_;  ///< 以前のコンテキスト（ネスト対応）
};

// ═══════════════════════════════════════════════════════════════════
// StateMachine クラステンプレート
// ═══════════════════════════════════════════════════════════════════

/// @brief 型安全なステートマシン
/// 
/// HSPの *label / goto を enum class ベースで再現します。
/// コンパイル時に存在しないステートへの遷移を検出できます。
/// 
/// @tparam StateType ステートを表す enum class 型
/// 
/// @code
/// enum class Screen { Title, Game, Result };
/// StateMachine<Screen> sm;
/// sm.state(Screen::Title)
///   .on_enter([]() { button("Start", ...); })
///   .on_update([&](auto& sm) { if (getkey(' ')) sm.jump(Screen::Game); })
///   .on_exit([]() { clrobj(); });
/// sm.jump(Screen::Title);
/// while (sm.run()) { await(16); }
/// @endcode
template<typename StateType>
    requires std::is_enum_v<StateType>
class StateMachine : public StateMachineBase {
public:
    // ====================================================
    // 型定義
    // ====================================================
    
    using EnterCallback = std::function<void()>;
    using UpdateCallback = std::function<void(StateMachine&)>;
    using ExitCallback = std::function<void()>;
    
    // ====================================================
    // コンストラクタ / デストラクタ
    // ====================================================
    
    /// @brief デフォルトコンストラクタ
    StateMachine() = default;
    
    /// @brief デストラクタ
    ~StateMachine() override = default;
    
    // コピー禁止（コールバック内でthisを参照するため）
    StateMachine(const StateMachine&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;
    
    // ムーブ可能
    StateMachine(StateMachine&&) = default;
    StateMachine& operator=(StateMachine&&) = default;
    
    // ====================================================
    // ステート定義（Builder パターン）
    // ====================================================
    
    /// @brief ステートを定義
    /// 
    /// メソッドチェーンで on_enter / on_update / on_exit を設定します。
    /// 
    /// @param state_enum ステート値
    /// @return ビルダーオブジェクト
    /// 
    /// @code
    /// sm.state(Screen::Title)
    ///   .on_enter([]() { /* 初期化 */ })
    ///   .on_update([](auto& sm) { /* 毎フレーム */ })
    ///   .on_exit([]() { /* 後処理 */ });
    /// @endcode
    StateBuilder<StateType> state(StateType state_enum);
    
    // ====================================================
    // 状態遷移制御
    // ====================================================
    
    /// @brief 状態遷移（HSP goto 相当）
    /// 
    /// 指定したステートに遷移を予約します。
    /// 遷移は次の run() 呼び出し時に実行されます。
    /// 
    /// @param target_state 遷移先のステート
    void jump(StateType target_state);
    
    /// @brief 状態遷移を予約（jumpと同じ、将来拡張用）
    /// @param target_state 遷移先のステート
    void defer_jump(StateType target_state);
    
    /// @brief 遷移ルールを追加（厳格モード用）
    /// 
    /// set_unrestricted_transitions(false) 時に使用します。
    /// 
    /// @param from 遷移元ステート
    /// @param to 遷移先ステート
    void allow_transition(StateType from, StateType to);
    
    /// @brief 遷移を禁止
    /// @param from 遷移元ステート
    /// @param to 遷移先ステート
    void deny_transition(StateType from, StateType to);
    
    /// @brief All-to-All遷移の有効/無効設定
    /// 
    /// @param enabled true で制約なし（デフォルト）、false で厳格チェック
    void set_unrestricted_transitions(bool enabled);
    
    // ====================================================
    // メインループ制御
    // ====================================================
    
    /// @brief メインループを実行
    /// 
    /// 各ステートの on_update 内で await/stop するため、
    /// quit() が呼ばれるまでこの関数はブロックします。
    /// 
    /// @code
    /// sm.jump(Screen::Title);
    /// sm.run();  // quit() まで戻ってこない
    /// @endcode
    void run();
    
    /// @brief メインループを終了
    void quit();
    
    // ====================================================
    // 状態クエリ
    // ====================================================
    
    /// @brief 現在のステートを取得
    [[nodiscard]] StateType current_state() const noexcept;
    
    /// @brief 前回のステートを取得
    [[nodiscard]] StateType previous_state() const noexcept;
    
    /// @brief ステート名を取得（デバッグ用）
    /// @return ステートの数値表現
    [[nodiscard]] std::string current_state_name() const;
    
    /// @brief グローバルフレームカウンタ（HSP cnt 相当）
    /// @return 起動からの総フレーム数
    [[nodiscard]] int frame_count() const noexcept;
    
    /// @brief 現在のステートに滞在しているフレーム数
    [[nodiscard]] int state_frame_count() const noexcept;
    
    // ====================================================
    // 履歴機能
    // ====================================================
    
    /// @brief 履歴記録を有効化
    /// @param max_size 最大履歴サイズ（デフォルト: 10）
    void enable_history(int max_size = 10);
    
    /// @brief 前のステートに戻る
    void back();
    
    /// @brief 履歴をクリア
    void clear_history();
    
    // ====================================================
    // タイマー機能
    // ====================================================
    
    /// @brief タイマーを設定（指定ミリ秒後に自動遷移）
    /// @param target_state 遷移先ステート
    /// @param milliseconds ミリ秒
    void set_timer(StateType target_state, int milliseconds);
    
    /// @brief タイマーをキャンセル
    void cancel_timer();
    
    // ====================================================
    // デバッグ支援
    // ====================================================
    
    /// @brief デバッグログを有効化
    /// @param enabled true でログ出力
    void enable_debug_log(bool enabled = true);
    
    /// @brief 状態遷移グラフをGraphviz dot形式で出力
    /// @param filename 出力ファイル名
    void export_graph(const std::string& filename);
    
    /// @brief ステート名を登録（デバッグ用）
    /// @param state ステート値
    /// @param name ステート名
    void set_state_name(StateType state, std::string_view name);
    
    // ====================================================
    // StateMachineBase インターフェース実装
    // ====================================================
    
    /// @brief 遷移が予約されているかチェック
    [[nodiscard]] bool should_transition() const noexcept override;
    
    /// @brief ステートマシンが実行中かチェック
    [[nodiscard]] bool is_running() const noexcept override;
    
private:
    // ====================================================
    // 内部データ構造
    // ====================================================
    
    struct StateData {
        EnterCallback on_enter;
        UpdateCallback on_update;
        ExitCallback on_exit;
        bool entered = false;  // on_enter実行済みフラグ
    };
    
    std::map<StateType, StateData> states_;
    std::optional<StateType> current_state_;
    std::optional<StateType> previous_state_;
    std::optional<StateType> next_state_;
    
    int global_frame_count_ = 0;
    int state_frame_count_ = 0;
    bool running_ = true;
    bool unrestricted_transitions_ = true;
    bool first_run_ = true;  // 初回run()フラグ
    
    // 遷移ルール（上級者向け）
    std::set<std::pair<StateType, StateType>> allowed_transitions_;
    std::set<std::pair<StateType, StateType>> denied_transitions_;
    
    // 履歴機能
    std::deque<StateType> history_;
    int max_history_size_ = 0;
    
    // タイマー機能
    std::optional<StateType> timer_target_;
    std::chrono::steady_clock::time_point timer_start_;
    int timer_duration_ms_ = 0;
    
    // デバッグ機能
    bool debug_enabled_ = false;
    std::set<std::pair<StateType, StateType>> transition_graph_;
    std::unordered_map<int, std::string> state_names_;  // enum値 -> 名前マップ
    
    // ====================================================
    // 内部ヘルパー
    // ====================================================
    
    /// @brief 遷移可否をチェック
    [[nodiscard]] bool check_transition_allowed(StateType from, StateType to) const;
    
    /// @brief enum → string 変換（デバッグ用）
    [[nodiscard]] std::string state_to_string(StateType state) const;
    
    /// @brief デバッグログ出力
    void debug_log(const std::string& message);
    
    /// @brief ステート遷移実行
    void perform_transition(StateType new_state);
    
    /// @brief タイマー更新
    void update_timer();
    
    friend class StateBuilder<StateType>;
};

// ═══════════════════════════════════════════════════════════════════
// StateBuilder クラステンプレート
// ═══════════════════════════════════════════════════════════════════

/// @brief ステート定義用ビルダークラス
/// 
/// StateMachine::state() から返され、メソッドチェーンで
/// コールバックを設定します。
/// 
/// @tparam StateType ステートを表す enum class 型
template<typename StateType>
class StateBuilder {
public:
    /// @brief 初回実行時のコールバックを設定
    /// 
    /// ステートに入るたびに1回だけ実行されます。
    /// GUIオブジェクトの作成、リソースロードなどに使用します。
    /// 
    /// @param callback 初回実行関数
    /// @return 自身の参照（メソッドチェーン用）
    StateBuilder& on_enter(typename StateMachine<StateType>::EnterCallback callback);
    
    /// @brief 毎フレーム実行のコールバックを設定
    /// 
    /// run() が呼ばれるたびに実行されます。
    /// 入力チェック、描画など軽い処理に使用します。
    /// 
    /// @param callback 毎フレーム実行関数
    /// @return 自身の参照（メソッドチェーン用）
    StateBuilder& on_update(typename StateMachine<StateType>::UpdateCallback callback);
    
    /// @brief 離脱時のコールバックを設定
    /// 
    /// ステートから出るときに1回だけ実行されます。
    /// GUIオブジェクトの削除、リソース解放に使用します。
    /// 
    /// @param callback 離脱時実行関数
    /// @return 自身の参照（メソッドチェーン用）
    StateBuilder& on_exit(typename StateMachine<StateType>::ExitCallback callback);
    
private:
    friend class StateMachine<StateType>;
    
    /// @brief コンストラクタ（StateMachine からのみ呼び出し可能）
    StateBuilder(StateMachine<StateType>& sm, StateType state);
    
    StateMachine<StateType>& sm_;
    StateType state_;
};

}  // namespace hsppp

// 実装のインクルード
#include "../src/core/hsppp_statemachine.inl"
