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
    
    /// @brief 初期ステートを設定してメインループを実行
    /// 
    /// jump() と run() を一度に行う便利メソッド。
    /// 
    /// @param initial_state 開始するステート
    /// 
    /// @code
    /// sm.start(Screen::Title);  // jump + run の代わり
    /// @endcode
    void start(StateType initial_state);
    
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

// ═══════════════════════════════════════════════════════════════════
// テンプレート実装（モジュールインターフェース内に統合）
// ═══════════════════════════════════════════════════════════════════

namespace hsppp {

// ═══════════════════════════════════════════════════════════════════
// StateBuilder 実装
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
StateBuilder<StateType>::StateBuilder(StateMachine<StateType>& sm, StateType state)
    : sm_(sm), state_(state)
{
    // states_ に登録されていなければ作成
    if (sm_.states_.find(state) == sm_.states_.end()) {
        sm_.states_[state] = typename StateMachine<StateType>::StateData{};
    }
}

template<typename StateType>
StateBuilder<StateType>& StateBuilder<StateType>::on_enter(
    typename StateMachine<StateType>::EnterCallback callback)
{
    sm_.states_[state_].on_enter = std::move(callback);
    return *this;
}

template<typename StateType>
StateBuilder<StateType>& StateBuilder<StateType>::on_update(
    typename StateMachine<StateType>::UpdateCallback callback)
{
    sm_.states_[state_].on_update = std::move(callback);
    return *this;
}

template<typename StateType>
StateBuilder<StateType>& StateBuilder<StateType>::on_exit(
    typename StateMachine<StateType>::ExitCallback callback)
{
    sm_.states_[state_].on_exit = std::move(callback);
    return *this;
}

// ═══════════════════════════════════════════════════════════════════
// StateMachine 実装 - ステート定義
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
StateBuilder<StateType> StateMachine<StateType>::state(StateType state_enum)
{
    return StateBuilder<StateType>(*this, state_enum);
}

// ═══════════════════════════════════════════════════════════════════
// StateMachine 実装 - 状態遷移制御
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::jump(StateType target_state)
{
    next_state_ = target_state;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::defer_jump(StateType target_state)
{
    // jumpと同じ（将来的な拡張用に分離）
    next_state_ = target_state;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::allow_transition(StateType from, StateType to)
{
    allowed_transitions_.insert(std::make_pair(from, to));
    
    // 遷移グラフに記録（デバッグ用）
    transition_graph_.insert(std::make_pair(from, to));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::deny_transition(StateType from, StateType to)
{
    denied_transitions_.insert(std::make_pair(from, to));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::set_unrestricted_transitions(bool enabled)
{
    unrestricted_transitions_ = enabled;
}

// ═══════════════════════════════════════════════════════════════════
// StateMachine 実装 - メインループ制御
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::run()
{
    // RAIIによるコンテキスト管理
    // 例外発生時も確実に復元される
    StateMachineScope scope(this);
    
    if (!running_) {
        return;
    }
    
    // 初回呼び出し時のチェック
    if (first_run_) {
        first_run_ = false;
        
        // jump()が呼ばれていない場合はエラー
        if (!next_state_.has_value() && !current_state_.has_value()) {
            debug_log("Warning: run() called without initial state. Call jump() first.");
            return;
        }
    }
    
    // ✅ HSP的構造: quit() されるまで内部でループ
    // 各ステートの on_update 内で await/stop するため、
    // 外側にループは不要
    while (running_) {
        // タイマー更新
        update_timer();
        
        // 遷移処理
        if (next_state_.has_value()) {
            StateType new_state = next_state_.value();
            next_state_.reset();
            
            // 遷移チェック（current_state_がある場合のみ）
            if (current_state_.has_value()) {
                if (!check_transition_allowed(current_state_.value(), new_state)) {
                    debug_log(std::format("Transition denied: {} -> {}",
                        state_to_string(current_state_.value()),
                        state_to_string(new_state)));
                    // 遷移を拒否するが、ループは継続
                }
                else {
                    perform_transition(new_state);
                }
            }
            else {
                // 初回遷移
                perform_transition(new_state);
            }
        }
        
        // 現在のステートがなければ終了
        if (!current_state_.has_value()) {
            return;
        }
        
        // 現在のステートのデータを取得
        auto it = states_.find(current_state_.value());
        if (it == states_.end()) {
            // 未定義のステート
            debug_log(std::format("Warning: State {} has no definition",
                state_to_string(current_state_.value())));
            continue;
        }
        
        auto& state_data = it->second;
        
        // on_enter 実行（初回のみ）
        if (!state_data.entered) {
            state_data.entered = true;
            debug_log(std::format("Enter state: {}", state_to_string(current_state_.value())));
            
            if (state_data.on_enter) {
                state_data.on_enter();
            }
        }
        
        // on_update 実行
        // ✅ ここで await/stop によりブロックされる
        if (state_data.on_update) {
            state_data.on_update(*this);
        }
        
        // フレームカウンタ更新
        global_frame_count_++;
        state_frame_count_++;
    }
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::quit()
{
    running_ = false;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::start(StateType initial_state)
{
    jump(initial_state);
    run();
}

// ═══════════════════════════════════════════════════════════════════
// StateMachine 実装 - 状態クエリ
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
StateType StateMachine<StateType>::current_state() const noexcept
{
    // current_state_が設定されていない場合はデフォルト値を返す
    return current_state_.value_or(static_cast<StateType>(0));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
StateType StateMachine<StateType>::previous_state() const noexcept
{
    return previous_state_.value_or(static_cast<StateType>(0));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
std::string StateMachine<StateType>::current_state_name() const
{
    if (current_state_.has_value()) {
        return state_to_string(current_state_.value());
    }
    return "(none)";
}

template<typename StateType>
    requires std::is_enum_v<StateType>
int StateMachine<StateType>::frame_count() const noexcept
{
    return global_frame_count_;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
int StateMachine<StateType>::state_frame_count() const noexcept
{
    return state_frame_count_;
}

// ═══════════════════════════════════════════════════════════════════
// StateMachine 実装 - 履歴機能
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::enable_history(int max_size)
{
    max_history_size_ = max_size;
    
    // 既存履歴がサイズを超えていれば縮小
    while (static_cast<int>(history_.size()) > max_history_size_ && !history_.empty()) {
        history_.pop_front();
    }
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::back()
{
    if (history_.empty()) {
        debug_log("Warning: back() called but history is empty");
        return;
    }
    
    StateType prev = history_.back();
    history_.pop_back();
    
    // 履歴に追加せずに遷移
    next_state_ = prev;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::clear_history()
{
    history_.clear();
}

// ═══════════════════════════════════════════════════════════════════
// StateMachine 実装 - タイマー機能
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::set_timer(StateType target_state, int milliseconds)
{
    timer_target_ = target_state;
    timer_duration_ms_ = milliseconds;
    timer_start_ = std::chrono::steady_clock::now();
    
    debug_log(std::format("Timer set: {} -> {} in {}ms",
        current_state_name(),
        state_to_string(target_state),
        milliseconds));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::cancel_timer()
{
    if (timer_target_.has_value()) {
        debug_log("Timer cancelled");
    }
    timer_target_.reset();
    timer_duration_ms_ = 0;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::update_timer()
{
    if (!timer_target_.has_value()) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - timer_start_).count();
    
    if (elapsed >= timer_duration_ms_) {
        StateType target = timer_target_.value();
        timer_target_.reset();
        timer_duration_ms_ = 0;
        
        debug_log(std::format("Timer fired: -> {}", state_to_string(target)));
        jump(target);
    }
}

// ═══════════════════════════════════════════════════════════════════
// StateMachine 実装 - デバッグ支援
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::enable_debug_log(bool enabled)
{
    debug_enabled_ = enabled;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::export_graph(const std::string& filename)
{
    std::ofstream ofs(filename);
    if (!ofs) {
        debug_log(std::format("Failed to open file: {}", filename));
        return;
    }
    
    ofs << "digraph StateMachine {\n";
    ofs << "    rankdir=LR;\n";
    ofs << "    node [shape=box, style=rounded];\n";
    
    // 登録されたステートをノードとして出力
    for (const auto& [state, _] : states_) {
        ofs << std::format("    \"{}\";\n", state_to_string(state));
    }
    
    // 遷移をエッジとして出力
    for (const auto& [from, to] : transition_graph_) {
        ofs << std::format("    \"{}\" -> \"{}\";\n",
            state_to_string(from), state_to_string(to));
    }
    
    ofs << "}\n";
    
    debug_log(std::format("Graph exported to: {}", filename));
}

// ═══════════════════════════════════════════════════════════════════
// StateMachine 実装 - StateMachineBase インターフェース
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
bool StateMachine<StateType>::should_transition() const noexcept
{
    return next_state_.has_value() || !running_;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
bool StateMachine<StateType>::is_running() const noexcept
{
    return running_;
}

// ═══════════════════════════════════════════════════════════════════
// StateMachine 実装 - 内部ヘルパー
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
bool StateMachine<StateType>::check_transition_allowed(StateType from, StateType to) const
{
    // All-to-All モードなら常に許可
    if (unrestricted_transitions_) {
        return true;
    }
    
    auto key = std::make_pair(from, to);
    
    // 明示的に拒否されていればNG
    if (denied_transitions_.contains(key)) {
        return false;
    }
    
    // 明示的に許可されていればOK
    if (allowed_transitions_.contains(key)) {
        return true;
    }
    
    // 制約モードでルール未定義ならNG
    return false;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::set_state_name(StateType state, std::string_view name)
{
    int key = static_cast<int>(state);
    state_names_[key] = std::string(name);
    debug_log(std::format("Registered state name: {} = \"{}\"", key, name));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
std::string StateMachine<StateType>::state_to_string(StateType state) const
{
    int key = static_cast<int>(state);
    
    // 登録済みの名前があればそれを使用
    auto it = state_names_.find(key);
    if (it != state_names_.end()) {
        return it->second;
    }
    
    // 登録されていない場合は数値で表示
    return std::format("State({})", key);
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::debug_log(const std::string& message)
{
    if (debug_enabled_) {
        // Windows APIを使用してデバッグ出力
        std::string output = "[StateMachine] " + message + "\n";
        
        // UTF-8からUTF-16へ変換
        int wide_len = MultiByteToWideChar(CP_UTF8, 0, output.c_str(), -1, nullptr, 0);
        if (wide_len > 0) {
            std::wstring wide_output(wide_len, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, output.c_str(), -1, wide_output.data(), wide_len);
            OutputDebugStringW(wide_output.c_str());
        }
    }
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateMachine<StateType>::perform_transition(StateType new_state)
{
    // 現在のステートがあればon_exitを呼び出し
    if (current_state_.has_value()) {
        auto it = states_.find(current_state_.value());
        if (it != states_.end() && it->second.on_exit) {
            it->second.on_exit();
        }
        
        // 履歴に追加
        if (max_history_size_ > 0) {
            history_.push_back(current_state_.value());
            while (static_cast<int>(history_.size()) > max_history_size_) {
                history_.pop_front();
            }
        }
        
        // 遷移グラフに記録
        transition_graph_.insert(std::make_pair(current_state_.value(), new_state));
        
        debug_log(std::format("Transition: {} -> {} (frame: {})",
            state_to_string(current_state_.value()),
            state_to_string(new_state),
            global_frame_count_));
    }
    
    // ステート更新
    previous_state_ = current_state_;
    current_state_ = new_state;
    state_frame_count_ = 0;
    
    // 新しいステートのenteredフラグをリセット
    auto it = states_.find(new_state);
    if (it != states_.end()) {
        it->second.entered = false;
    }
    
    // タイマーをキャンセル（遷移時にリセット）
    cancel_timer();
}

}  // namespace hsppp
