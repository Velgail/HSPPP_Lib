// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/src/core/hsppp_statemachine.inl
// ═══════════════════════════════════════════════════════════════════
// HSPPP ステートマシン - テンプレート実装
// ═══════════════════════════════════════════════════════════════════
//
// 注意: このファイルは hsppp_statemachine.ixx から #include されます。
//       直接コンパイルしないでください。

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
        OutputDebugStringA(output.c_str());
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
