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
// HSPの *label / goto を型安全に再現する後継ステートマシンライブラリ。
// 本体クラスは StateGraph<T>、旧名 StateMachine<T> は alias として残存。
//
// 使用例:
//   enum class Screen { Title, Game, Result };
//   StateGraph<Screen> sm;
//   sm.state(Screen::Title).on_update([&](auto& sm) {
//       if (getkey(' ')) sm.jump(Screen::Game);
//   });
//   sm.start(Screen::Title);   // = jump + run()   ※ run() は dispatch only（内部で await を呼ばない / design-TICKET-008 §7.1）
//
// 設計根拠:
//   .github/agents/sprints/current/artifacts/design-TICKET-002.md
//   §6, §7.1, §8.1, §12.1, §13(L1,L2,L6,L7), §15(F2,F6,C4,C5), §17(Risk-1,3,4)

module;

#define NOMINMAX
#include <windows.h>

export module hsppp:statemachine;

import :types;
import :interrupt;   // HspError / ERR_INTERNAL

import <functional>;
import <map>;
import <set>;
import <deque>;
import <optional>;
import <string>;
import <string_view>;
import <type_traits>;
import <format>;
import <chrono>;
import <fstream>;
import <memory>;
import <unordered_map>;
import <limits>;

export namespace hsppp {

// ═══════════════════════════════════════════════════════════════════
// 前方宣言
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
class StateGraph;

template<typename StateType>
class StateBuilder;

template<typename StateType, typename LocalDataType>
class StateBuilderWithLocal;

// ═══════════════════════════════════════════════════════════════════
// StateMachineBase - 非テンプレート基底クラス
// ═══════════════════════════════════════════════════════════════════

/// @brief StateGraph の非テンプレート基底クラス
///
/// グローバル関数 (await/stop/vwait) が StateGraph コンテキストを
/// 検出するために使用されるほか、サブ SM をベース型経由で
/// `step()` / `tick()` 呼出するためにも利用される
/// （design-TICKET-008.md §7.3 / Q-2 PM 判断）。
class StateMachineBase {
public:
    virtual ~StateMachineBase() = default;

    /// @brief 遷移が予約されているかチェック
    [[nodiscard]] virtual bool should_transition() const = 0;

    /// @brief ステートマシンが実行中かチェック
    [[nodiscard]] virtual bool is_running() const = 0;

    /// @brief ステート遷移が要求されているかチェック
    [[nodiscard]] virtual bool is_transitioning() const = 0;

    /// @brief 1 ステップ分だけ更新（親 on_update 内からサブ SM を駆動する用途）
    /// @note design-TICKET-008.md §7.3: 親 on_update 内で `child.step()` を
    ///       明示呼出する規約。`step()` は本メソッド `tick()` への inline 委譲。
    virtual void tick() = 0;
};

// ═══════════════════════════════════════════════════════════════════
// StateMachineContext - RAIIコンテキスト管理（内部用）
// ═══════════════════════════════════════════════════════════════════

namespace detail {
    /// @brief 現在のStateMachineコンテキスト（スレッドローカル）
    inline thread_local StateMachineBase* current_statemachine = nullptr;

    /// @brief 現在のStateMachineを取得
    inline StateMachineBase* get_current_statemachine() noexcept {
        return current_statemachine;
    }
}

/// @brief RAIIによる StateGraph コンテキスト管理
///
/// StateGraph::run() / tick() 内でスコープガードとして使用し、
/// 例外発生時も確実にコンテキストを復元します。
class StateMachineScope {
public:
    explicit StateMachineScope(StateMachineBase* sm) noexcept
        : prev_(detail::current_statemachine)
    {
        detail::current_statemachine = sm;
    }

    ~StateMachineScope() noexcept {
        detail::current_statemachine = prev_;
    }

    StateMachineScope(const StateMachineScope&) = delete;
    StateMachineScope& operator=(const StateMachineScope&) = delete;
    StateMachineScope(StateMachineScope&&) = delete;
    StateMachineScope& operator=(StateMachineScope&&) = delete;

private:
    StateMachineBase* prev_;
};

// ═══════════════════════════════════════════════════════════════════
// StateGraph クラステンプレート（design §7.1）
// ═══════════════════════════════════════════════════════════════════

/// @brief 型安全な後継ステートマシン
///
/// HSP の *label / goto を enum class ベースで再現する後継 API。
/// 旧名 StateMachine<T> は本ファイル末尾で alias として残存
/// （design-TICKET-002.md §12.4 互換戦略）。
///
/// @tparam StateType ステートを表す enum class 型
template<typename StateType>
    requires std::is_enum_v<StateType>
class StateGraph : public StateMachineBase {
public:
    // ====================================================
    // 型定義
    // ====================================================

    using EnterCallback  = std::function<void()>;
    /// @brief on_update コールバック型
    /// @note design-TICKET-008.md §7.2: on_update 1 回の呼出はユーザーが
    ///       そのステートに与えた `repeat`-`loop` の 1 iteration を意味する。
    ///       return すれば同ステートの on_update が再呼出される。
    ///       `jump()` してから return すれば次ステートへ遷移する。
    ///       on_update 内で `await` / `stop` / `vwait` / 任意ループを書くことは正規。
    using UpdateCallback = std::function<void(StateGraph&)>;
    using ExitCallback   = std::function<void()>;

    // ====================================================
    // コンストラクタ / デストラクタ
    // ====================================================

    StateGraph() = default;
    ~StateGraph() override = default;

    // コピー禁止（コールバック内で this を参照するため）
    StateGraph(const StateGraph&) = delete;
    StateGraph& operator=(const StateGraph&) = delete;

    // ムーブ可能
    StateGraph(StateGraph&&) = default;
    StateGraph& operator=(StateGraph&&) = default;

    // ====================================================
    // ステート定義（Builder パターン）
    // ====================================================

    /// @brief ステートを定義
    StateBuilder<StateType> state(StateType state_enum);

    /// @brief ローカルデータ付きステートを定義（旧 API 互換薄ラッパ）
    /// @note design §13 L3-b の薄ラッパ位置付け。新規コードでは
    ///       hsppp_state_vars.ixx の StateScope を使用すること。
    template<typename LocalDataType, typename... Args>
    StateBuilderWithLocal<StateType, LocalDataType> state(StateType state_enum, Args&&... args);

    // ====================================================
    // 状態遷移制御
    // ====================================================

    /// @brief 状態遷移（HSP goto 相当）
    void jump(StateType target_state);

    /// @brief 状態遷移を予約（jump と同等／互換用）
    [[deprecated("use jump (design §7.1 / §18 Q-3)")]]
    void defer_jump(StateType target_state);

    /// @brief 遷移ルールを追加（厳格モード用）
    void allow_transition(StateType from, StateType to);

    /// @brief 遷移を禁止
    void deny_transition(StateType from, StateType to);

    /// @brief All-to-All遷移の有効/無効設定
    void set_unrestricted_transitions(bool enabled);

    // ====================================================
    // メインループ制御
    // ====================================================

    /// @brief メインループを実行（dispatch only）
    ///
    /// design-TICKET-008.md §7.1 / §12.1: 内部は `while (running_) { tick(); }` のみで、
    /// `await` / `Sleep` / `vwait` を一切呼ばない。フレームペーシングは
    /// ユーザーが on_update 内で `await(ms)` 等を明示的に書く契約。
    /// @throws HspError jump() も current_state も無いまま呼ばれた場合（design §12.1 / §7.1）。
    void run();

    /// @brief 初期ステートを設定してメインループを実行
    void start(StateType initial_state);

    /// @brief 1 ステップ分だけ更新（手動駆動・サブ SM 駆動用）
    /// @note design-TICKET-008.md §7.3: 親 on_update 内から
    ///       `child.step()` を明示呼出することでサブ SM を進行させる。
    ///       `step()` は本メソッドへの inline 委譲（同義）。
    void tick() override;

    /// @brief `tick()` の推奨 alias（design-TICKET-008.md §12.1 / §7.3）
    /// @note 親 on_update 内でサブ SM を駆動する用途では `step()` を推奨。
    ///       `tick()` は後方互換のため維持。
    void step();

    /// @brief メインループを終了
    void quit();

    // ====================================================
    // 状態クエリ（design §7.1: optional 化）
    // ====================================================

    /// @brief 現在のステートを取得
    /// @return まだ jump() されていない場合は std::nullopt
    [[nodiscard]] std::optional<StateType> current_state() const noexcept;

    /// @brief 前回のステートを取得
    [[nodiscard]] std::optional<StateType> previous_state() const noexcept;

    /// @brief ステート名を取得（デバッグ用）
    [[nodiscard]] std::string_view current_state_name() const;

    /// @brief グローバルフレームカウンタ（HSP cnt 相当）
    [[nodiscard]] int frame_count() const noexcept;

    /// @brief 現在のステートに滞在しているフレーム数
    [[deprecated("use state_elapsed_ms() (design §7.1 / §18 Q-3)")]]
    [[nodiscard]] int state_frame_count() const noexcept;

    /// @brief 現在のステートに滞在している経過時間 (ms)
    [[nodiscard]] int state_elapsed_ms() const noexcept;

    // ====================================================
    // 履歴機能
    // ====================================================

    /// @brief 履歴記録を有効化
    void enable_history(int max_size = 10);

    /// @brief 前のステートに戻る
    /// @return 戻る履歴がある場合 true、空の場合 false
    bool back();

    /// @brief 履歴をクリア
    void clear_history();

    // ====================================================
    // タイマー機能（ms 統一・design §7.1）
    // ====================================================

    /// @brief タイマーを設定（指定ミリ秒後に自動遷移）
    void set_timer(StateType target_state, int milliseconds);

    /// @brief タイマーをキャンセル
    void cancel_timer();

    /// @brief タイマーを一時停止（経過時間は引き継がれる）
    /// @details paused 状態のタイマーは perform_transition による自動 cancel から
    ///          保護される。Pause UI のように state を跨いで継続したいケースで使用する。
    ///          ユーザー明示の cancel_timer() は paused でも無条件で wipe する。
    ///          (design-TICKET-010 §5.1 / §7)
    void pause_timer();

    /// @brief タイマーを再開
    /// @details pause_timer() で保持された timer を再開する。state を跨いで保持
    ///          された場合は、復帰先 state の on_enter で呼ぶのが推奨。
    ///          (design-TICKET-010 §5.2 / §7)
    void resume_timer();

    // ====================================================
    // デバッグ支援
    // ====================================================

    /// @brief デバッグログを有効化
    void enable_debug_log(bool enabled = true);

    /// @brief 状態遷移グラフをGraphviz dot形式で出力
    void export_graph(const std::string& filename);

    /// @brief ステート名を登録（デバッグ用）
    void set_state_name(StateType state, std::string_view name);

    // ====================================================
    // StateMachineBase インターフェース実装
    // ====================================================

    [[nodiscard]] bool should_transition() const override;
    [[nodiscard]] bool is_running() const override;
    [[nodiscard]] bool is_transitioning() const override;

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

    struct TimerState {
        std::optional<StateType> target;
        int target_ms = 0;
        std::chrono::steady_clock::time_point start{};
        long long accumulated_ms = 0;
        bool paused = false;

        [[nodiscard]] bool active() const noexcept { return target.has_value(); }
    };

    std::map<StateType, StateData> states_;
    std::optional<StateType> current_state_;
    std::optional<StateType> previous_state_;
    std::optional<StateType> next_state_;

    // ローカルデータストレージ（型消去・旧 API 互換）
    std::map<StateType, std::shared_ptr<void>> local_data_storage_;

    int global_frame_count_ = 0;
    int state_frame_count_ = 0;
    std::chrono::steady_clock::time_point state_enter_time_{};
    bool running_ = true;
    bool unrestricted_transitions_ = true;
    bool first_run_ = true;

    // 遷移ルール（上級者向け）
    std::set<std::pair<StateType, StateType>> allowed_transitions_;
    std::set<std::pair<StateType, StateType>> denied_transitions_;

    // 履歴機能
    std::deque<StateType> history_;
    int max_history_size_ = 0;

    // タイマー機能（design §7.1: pause 引き継ぎ対応）
    TimerState timer_{};

    // デバッグ機能
    bool debug_enabled_ = false;
    std::set<std::pair<StateType, StateType>> transition_graph_;
    std::unordered_map<int, std::string> state_names_;

    mutable std::string current_state_name_cache_;  // string_view 戻り値の生存延長用

    // ====================================================
    // 内部ヘルパー
    // ====================================================

    [[nodiscard]] bool check_transition_allowed(StateType from, StateType to) const;
    [[nodiscard]] std::string state_to_string(StateType state) const;
    void debug_log(const std::string& message) const;
    void perform_transition(StateType new_state);
    void update_timer();
    void process_pending_transition();
    void step_once();   // 1 フレーム分の本体処理（tick / run 共通）

    friend class StateBuilder<StateType>;
    template<typename, typename> friend class StateBuilderWithLocal;
};

// ═══════════════════════════════════════════════════════════════════
// StateBuilder クラステンプレート
// ═══════════════════════════════════════════════════════════════════

/// @brief ステート定義用ビルダークラス
template<typename StateType>
class StateBuilder {
public:
    StateBuilder& on_enter(typename StateGraph<StateType>::EnterCallback callback);
    StateBuilder& on_update(typename StateGraph<StateType>::UpdateCallback callback);
    StateBuilder& on_exit(typename StateGraph<StateType>::ExitCallback callback);

private:
    friend class StateGraph<StateType>;

    StateBuilder(StateGraph<StateType>& sm, StateType state);

    StateGraph<StateType>& sm_;
    StateType state_;
};

// ═══════════════════════════════════════════════════════════════════
// StateBuilderWithLocal - ローカルデータ付きステートビルダー
// ═══════════════════════════════════════════════════════════════════

/// @brief ローカルデータ付きステート定義ビルダー（旧 API 互換薄ラッパ）
template<typename StateType, typename LocalDataType>
class StateBuilderWithLocal {
public:
    using EnterCallbackWithLocal  = std::function<void(LocalDataType&)>;
    using UpdateCallbackWithLocal = std::function<void(StateGraph<StateType>&, LocalDataType&)>;
    using ExitCallbackWithLocal   = std::function<void(LocalDataType&)>;

    StateBuilderWithLocal& on_enter(EnterCallbackWithLocal callback);
    StateBuilderWithLocal& on_update(UpdateCallbackWithLocal callback);
    StateBuilderWithLocal& on_exit(ExitCallbackWithLocal callback);

private:
    friend class StateGraph<StateType>;

    StateBuilderWithLocal(StateGraph<StateType>& sm, StateType state, std::shared_ptr<LocalDataType> data);

    StateGraph<StateType>& sm_;
    StateType state_;
    std::shared_ptr<LocalDataType> local_data_;
};

// ═══════════════════════════════════════════════════════════════════
// 互換 alias（design §12.4 / §15 C5）
// 本 Sprint 中は無印、次 Sprint で [[deprecated]] 付与（design §18 Q-2）
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
using StateMachine = StateGraph<StateType>;

}  // namespace hsppp

// ═══════════════════════════════════════════════════════════════════
// テンプレート実装（モジュールインターフェース内に統合）
// ═══════════════════════════════════════════════════════════════════

namespace hsppp {

// ═══════════════════════════════════════════════════════════════════
// StateBuilder 実装
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
StateBuilder<StateType>::StateBuilder(StateGraph<StateType>& sm, StateType state)
    : sm_(sm), state_(state)
{
    if (sm_.states_.find(state) == sm_.states_.end()) {
        sm_.states_[state] = typename StateGraph<StateType>::StateData{};
    }
}

template<typename StateType>
StateBuilder<StateType>& StateBuilder<StateType>::on_enter(
    typename StateGraph<StateType>::EnterCallback callback)
{
    sm_.states_[state_].on_enter = std::move(callback);
    return *this;
}

template<typename StateType>
StateBuilder<StateType>& StateBuilder<StateType>::on_update(
    typename StateGraph<StateType>::UpdateCallback callback)
{
    sm_.states_[state_].on_update = std::move(callback);
    return *this;
}

template<typename StateType>
StateBuilder<StateType>& StateBuilder<StateType>::on_exit(
    typename StateGraph<StateType>::ExitCallback callback)
{
    sm_.states_[state_].on_exit = std::move(callback);
    return *this;
}

// ═══════════════════════════════════════════════════════════════════
// StateBuilderWithLocal 実装
// ═══════════════════════════════════════════════════════════════════

template<typename StateType, typename LocalDataType>
StateBuilderWithLocal<StateType, LocalDataType>::StateBuilderWithLocal(
    StateGraph<StateType>& sm, StateType state, std::shared_ptr<LocalDataType> data)
    : sm_(sm), state_(state), local_data_(data)
{
    if (sm_.states_.find(state) == sm_.states_.end()) {
        sm_.states_[state] = typename StateGraph<StateType>::StateData{};
    }
}

template<typename StateType, typename LocalDataType>
StateBuilderWithLocal<StateType, LocalDataType>&
StateBuilderWithLocal<StateType, LocalDataType>::on_enter(EnterCallbackWithLocal callback)
{
    auto data_ptr = local_data_;
    sm_.states_[state_].on_enter = [data_ptr, callback]() {
        callback(*data_ptr);
    };
    return *this;
}

template<typename StateType, typename LocalDataType>
StateBuilderWithLocal<StateType, LocalDataType>&
StateBuilderWithLocal<StateType, LocalDataType>::on_update(UpdateCallbackWithLocal callback)
{
    auto data_ptr = local_data_;
    sm_.states_[state_].on_update = [data_ptr, callback](StateGraph<StateType>& sm) {
        callback(sm, *data_ptr);
    };
    return *this;
}

template<typename StateType, typename LocalDataType>
StateBuilderWithLocal<StateType, LocalDataType>&
StateBuilderWithLocal<StateType, LocalDataType>::on_exit(ExitCallbackWithLocal callback)
{
    auto data_ptr = local_data_;
    sm_.states_[state_].on_exit = [data_ptr, callback]() {
        callback(*data_ptr);
    };
    return *this;
}

// ═══════════════════════════════════════════════════════════════════
// StateGraph 実装 - ステート定義
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
StateBuilder<StateType> StateGraph<StateType>::state(StateType state_enum)
{
    return StateBuilder<StateType>(*this, state_enum);
}

template<typename StateType>
    requires std::is_enum_v<StateType>
template<typename LocalDataType, typename... Args>
StateBuilderWithLocal<StateType, LocalDataType>
StateGraph<StateType>::state(StateType state_enum, Args&&... args)
{
    auto data = std::make_shared<LocalDataType>(std::forward<Args>(args)...);
    local_data_storage_[state_enum] = data;
    return StateBuilderWithLocal<StateType, LocalDataType>(*this, state_enum, data);
}

// ═══════════════════════════════════════════════════════════════════
// StateGraph 実装 - 状態遷移制御
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::jump(StateType target_state)
{
    next_state_ = target_state;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::defer_jump(StateType target_state)
{
    next_state_ = target_state;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::allow_transition(StateType from, StateType to)
{
    allowed_transitions_.insert(std::make_pair(from, to));
    transition_graph_.insert(std::make_pair(from, to));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::deny_transition(StateType from, StateType to)
{
    denied_transitions_.insert(std::make_pair(from, to));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::set_unrestricted_transitions(bool enabled)
{
    unrestricted_transitions_ = enabled;
}

// ═══════════════════════════════════════════════════════════════════
// StateGraph 実装 - メインループ制御
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::process_pending_transition()
{
    if (!next_state_.has_value()) {
        return;
    }

    StateType new_state = next_state_.value();
    next_state_.reset();

    if (current_state_.has_value()) {
        if (!check_transition_allowed(current_state_.value(), new_state)) {
            // design §12.1 L7: Warning を HspError に格上げ
            throw HspError(ERR_INTERNAL,
                std::format("Transition denied: {} -> {}",
                    state_to_string(current_state_.value()),
                    state_to_string(new_state)));
        }
        perform_transition(new_state);
    }
    else {
        // 初回遷移
        perform_transition(new_state);
    }
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::step_once()
{
    if (!running_) {
        return;
    }

    // タイマー更新（pause 中は経過進めない）
    update_timer();

    // 遷移処理
    process_pending_transition();

    // 現在のステートがなければ何もしない
    if (!current_state_.has_value()) {
        return;
    }

    // 現在のステートのデータを取得
    auto it = states_.find(current_state_.value());
    if (it == states_.end()) {
        // design §12.1 L7: 未定義ステートは HspError 化
        throw HspError(ERR_INTERNAL,
            std::format("State {} has no definition",
                state_to_string(current_state_.value())));
    }

    auto& state_data = it->second;

    // on_enter 実行（初回のみ）
    if (!state_data.entered) {
        state_data.entered = true;
        state_enter_time_ = std::chrono::steady_clock::now();
        state_frame_count_ = 0;
        debug_log(std::format("Enter state: {}", state_to_string(current_state_.value())));
        if (state_data.on_enter) {
            state_data.on_enter();
        }
    }

    // on_update 実行（design-TICKET-008.md §7.2: named repeat-loop の 1 iteration）
    if (state_data.on_update) {
        state_data.on_update(*this);
    }

    // フレームカウンタ更新
    global_frame_count_++;
    state_frame_count_++;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::run()
{
    // RAII によるコンテキスト管理
    StateMachineScope scope(this);

    if (!running_) {
        return;
    }

    // 初回呼び出し時のチェック（design §12.1 L7: HspError 化）
    if (first_run_) {
        first_run_ = false;
        if (!next_state_.has_value() && !current_state_.has_value()) {
            throw HspError(ERR_INTERNAL,
                "run() called without initial state. Call jump() or start() first.");
        }
    }

    // design-TICKET-008.md §7.1 / §12.1: dispatch only。
    // フレームペーシング（await / stop / vwait）はユーザーが on_update 内で明示。
    while (running_) {
        step_once();
    }
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::start(StateType initial_state)
{
    jump(initial_state);
    run();
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::tick()
{
    // RAII によるコンテキスト管理（サブ SM 駆動経由でも有効に）
    StateMachineScope scope(this);
    step_once();
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::step()
{
    // design-TICKET-008.md §12.1: step() は tick() への委譲（推奨 alias）
    tick();
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::quit()
{
    running_ = false;
}

// ═══════════════════════════════════════════════════════════════════
// StateGraph 実装 - 状態クエリ
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
std::optional<StateType> StateGraph<StateType>::current_state() const noexcept
{
    return current_state_;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
std::optional<StateType> StateGraph<StateType>::previous_state() const noexcept
{
    return previous_state_;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
std::string_view StateGraph<StateType>::current_state_name() const
{
    if (current_state_.has_value()) {
        current_state_name_cache_ = state_to_string(current_state_.value());
    }
    else {
        current_state_name_cache_ = "(none)";
    }
    return current_state_name_cache_;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
int StateGraph<StateType>::frame_count() const noexcept
{
    return global_frame_count_;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
int StateGraph<StateType>::state_frame_count() const noexcept
{
    return state_frame_count_;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
int StateGraph<StateType>::state_elapsed_ms() const noexcept
{
    if (!current_state_.has_value()) {
        return 0;
    }
    using namespace std::chrono;
    auto now = steady_clock::now();
    auto elapsed = duration_cast<milliseconds>(now - state_enter_time_).count();
    if (elapsed < 0) elapsed = 0;
    if (elapsed > static_cast<long long>(std::numeric_limits<int>::max())) {
        elapsed = std::numeric_limits<int>::max();
    }
    return static_cast<int>(elapsed);
}

// ═══════════════════════════════════════════════════════════════════
// StateGraph 実装 - 履歴機能
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::enable_history(int max_size)
{
    max_history_size_ = max_size;
    while (static_cast<int>(history_.size()) > max_history_size_ && !history_.empty()) {
        history_.pop_front();
    }
}

template<typename StateType>
    requires std::is_enum_v<StateType>
bool StateGraph<StateType>::back()
{
    if (history_.empty()) {
        return false;
    }
    StateType prev = history_.back();
    history_.pop_back();
    next_state_ = prev;
    return true;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::clear_history()
{
    history_.clear();
}

// ═══════════════════════════════════════════════════════════════════
// StateGraph 実装 - タイマー機能（design §7.1: pause 引き継ぎ）
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::set_timer(StateType target_state, int milliseconds)
{
    timer_.target = target_state;
    timer_.target_ms = milliseconds;
    timer_.start = std::chrono::steady_clock::now();
    timer_.accumulated_ms = 0;
    timer_.paused = false;

    debug_log(std::format("Timer set: {} -> {} in {}ms",
        std::string(current_state_name()),
        state_to_string(target_state),
        milliseconds));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::cancel_timer()
{
    if (timer_.active()) {
        debug_log("Timer cancelled");
    }
    timer_ = TimerState{};
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::pause_timer()
{
    if (!timer_.active() || timer_.paused) {
        return;
    }
    using namespace std::chrono;
    auto now = steady_clock::now();
    timer_.accumulated_ms += duration_cast<milliseconds>(now - timer_.start).count();
    timer_.paused = true;
    debug_log("Timer paused");
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::resume_timer()
{
    if (!timer_.active() || !timer_.paused) {
        return;
    }
    timer_.start = std::chrono::steady_clock::now();
    timer_.paused = false;
    debug_log("Timer resumed");
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::update_timer()
{
    if (!timer_.active() || timer_.paused) {
        return;
    }
    using namespace std::chrono;
    auto now = steady_clock::now();
    auto elapsed = timer_.accumulated_ms + duration_cast<milliseconds>(now - timer_.start).count();
    if (elapsed >= timer_.target_ms) {
        StateType target = timer_.target.value();
        timer_ = TimerState{};
        debug_log(std::format("Timer fired: -> {}", state_to_string(target)));
        jump(target);
    }
}

// ═══════════════════════════════════════════════════════════════════
// StateGraph 実装 - デバッグ支援
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::enable_debug_log(bool enabled)
{
    debug_enabled_ = enabled;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::export_graph(const std::string& filename)
{
    std::ofstream ofs(filename);
    if (!ofs) {
        // design §12.1 L7: 失敗系は HspError 化
        throw HspError(ERR_FILE_IO,
            std::format("Failed to open file: {}", filename));
    }

    ofs << "digraph StateMachine {\n";
    ofs << "    rankdir=LR;\n";
    ofs << "    node [shape=box, style=rounded];\n";

    for (const auto& [state, _] : states_) {
        ofs << std::format("    \"{}\";\n", state_to_string(state));
    }
    for (const auto& [from, to] : transition_graph_) {
        ofs << std::format("    \"{}\" -> \"{}\";\n",
            state_to_string(from), state_to_string(to));
    }
    ofs << "}\n";

    debug_log(std::format("Graph exported to: {}", filename));
}

// ═══════════════════════════════════════════════════════════════════
// StateGraph 実装 - StateMachineBase インターフェース
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
bool StateGraph<StateType>::should_transition() const
{
    return next_state_.has_value() || !running_;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
bool StateGraph<StateType>::is_running() const
{
    return running_;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
bool StateGraph<StateType>::is_transitioning() const
{
    return next_state_.has_value();
}

// ═══════════════════════════════════════════════════════════════════
// StateGraph 実装 - 内部ヘルパー
// ═══════════════════════════════════════════════════════════════════

template<typename StateType>
    requires std::is_enum_v<StateType>
bool StateGraph<StateType>::check_transition_allowed(StateType from, StateType to) const
{
    if (unrestricted_transitions_) {
        return true;
    }
    auto key = std::make_pair(from, to);
    if (denied_transitions_.contains(key)) {
        return false;
    }
    if (allowed_transitions_.contains(key)) {
        return true;
    }
    return false;
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::set_state_name(StateType state, std::string_view name)
{
    int key = static_cast<int>(state);
    state_names_[key] = std::string(name);
    debug_log(std::format("Registered state name: {} = \"{}\"", key, name));
}

template<typename StateType>
    requires std::is_enum_v<StateType>
std::string StateGraph<StateType>::state_to_string(StateType state) const
{
    int key = static_cast<int>(state);
    auto it = state_names_.find(key);
    if (it != state_names_.end()) {
        return it->second;
    }
    return std::format("State({})", key);
}

template<typename StateType>
    requires std::is_enum_v<StateType>
void StateGraph<StateType>::debug_log(const std::string& message) const
{
    if (debug_enabled_) {
        std::string output = "[StateGraph] " + message + "\n";
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
void StateGraph<StateType>::perform_transition(StateType new_state)
{
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

    previous_state_ = current_state_;
    current_state_ = new_state;
    state_frame_count_ = 0;
    state_enter_time_ = std::chrono::steady_clock::now();

    auto it = states_.find(new_state);
    if (it != states_.end()) {
        it->second.entered = false;
    }

    // 遷移完了時にタイマーをキャンセル。
    // ただし pause_timer() で明示的に「state を跨いで保持」する
    // 意思表示がされている場合は保持する（design-TICKET-010 §5.1）。
    // ユーザー明示の cancel_timer() は paused でも無条件 wipe する
    // ためのエスケープハッチとして従来通り機能する（§5.3）。
    if (!timer_.paused) {
        cancel_timer();
    }
}

}  // namespace hsppp
