// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp_state_vars.ixx
// ═══════════════════════════════════════════════════════════════════
// HSPPP ステート別変数パッケージ - モジュールインターフェース
// ═══════════════════════════════════════════════════════════════════
//
// StateGraph<T> に紐付くステート別ローカルデータを (TState × 型 L) の
// キーで一意管理する。Repository<T>（プロセス寿命グローバル変数置換）と
// は責務を分離する（design §6, §12.2）。
//
// ライフサイクル契約（design §7.2）:
//   - 既定: ステート離脱後も破棄しない（永続）。release() 明示破棄のみ。
//   - bind は同じキーに対し冪等（既存返却）。
//   - snapshot は Serializable<L> 概念を満たす型のみ対象。
//     非対応型は enumerate() で serializable=false が立つ。
//
// 設計根拠:
//   §6, §7.2, §8.2, §12.2, §13(L3-b, L4-b), §15(F3, F5, C4), §17(Risk-2, 6)

export module hsppp:state_vars;

import :interrupt;     // HspError / ERR_*
import :savedata;      // SaveWriter / SaveReader / Serializable
import :statemachine;  // StateGraph<T> （前方参照用）

import <any>;
import <concepts>;
import <cstddef>;
import <cstdint>;
import <format>;
import <functional>;
import <optional>;
import <span>;
import <stdexcept>;
import <string>;
import <string_view>;
import <type_traits>;
import <typeindex>;
import <typeinfo>;
import <unordered_map>;
import <utility>;
import <vector>;

export namespace hsppp {

template <typename TState>
    requires std::is_enum_v<TState>
class StateScope;

// ═══════════════════════════════════════════════════════════════════
// StateVarEntry（design §7.2 / §10 未決の確定形）
// ═══════════════════════════════════════════════════════════════════

/// @brief enumerate() が返す要素型
///
/// design §10 で「実装着手前に Architect 確定 or Developer 判断委譲」と
/// された未決事項。Developer 判断として以下フィールドを採用する。
struct StateVarEntry {
    std::string   key;            ///< snapshot 内で使われるキー（"{state_index}:{type_tag}" など）
    std::string   type_tag;       ///< L::type_tag()。非 Serializable 型では空文字
    bool          serializable;   ///< Serializable<L> を満たすか
    std::int64_t  state_index;    ///< static_cast<underlying>(TState) の値
};

/// @brief StateScope の参照専用 view
template <typename TState>
    requires std::is_enum_v<TState>
class StateScopeReadView {
public:
    explicit StateScopeReadView(const StateScope<TState>& scope) noexcept;

    template <typename L>
    [[nodiscard]] const L& get(TState s) const;

    template <typename L>
    [[nodiscard]] const L* try_get(TState s) const noexcept;

    template <typename L>
    [[nodiscard]] bool contains(TState s) const noexcept;

    [[nodiscard]] std::vector<StateVarEntry> enumerate() const;

private:
    const StateScope<TState>* scope_ = nullptr;
};

// ═══════════════════════════════════════════════════════════════════
// StateScope<TState>（design §7.2 / §8.2）
// ═══════════════════════════════════════════════════════════════════

/// @brief ステート別変数の登録・取得・寿命管理・スナップショット
///
/// StateGraph<TState> に対し弱参照を保持するのみで、StateMachine 本体は
/// StateScope を知らない（一方向依存：design §6）。
///
/// 内部ストレージは (TState, std::type_index) → Slot の unordered_map。
/// 同一キーで bind を再呼出した場合は **冪等**（既存スロットを返却）。
template <typename TState>
    requires std::is_enum_v<TState>
class StateScope {
public:
    using Underlying = std::underlying_type_t<TState>;

    explicit StateScope(StateGraph<TState>& sm) noexcept
        : sm_(&sm)
    {}

    StateScope(const StateScope&) = delete;
    StateScope& operator=(const StateScope&) = delete;
    StateScope(StateScope&&) noexcept = default;
    StateScope& operator=(StateScope&&) noexcept = default;

    /// @brief 紐付け対象の StateGraph
    [[nodiscard]] StateGraph<TState>& state_graph() noexcept { return *sm_; }
    [[nodiscard]] const StateGraph<TState>& state_graph() const noexcept { return *sm_; }

    /// @brief 参照専用 view を取得
    [[nodiscard]] StateScopeReadView<TState> read_view() const noexcept;

    // ====================================================
    // bind / get / try_get
    // ====================================================

    /// @brief ステート別変数を登録（既存があれば冪等に返却）
    /// @tparam L 変数型。Serializable<L> 満足時のみ snapshot 対象。
    template <typename L, typename... Args>
    L& bind(TState s, Args&&... args)
    {
        const Key key{ s, std::type_index(typeid(L)) };
        if (auto it = storage_.find(key); it != storage_.end()) {
            // 冪等: 既存値を返す（args は無視。design §7.2「同キー再呼出は既存返却」）
            return std::any_cast<L&>(it->second.value);
        }
        Slot slot;
        slot.value.emplace<L>(std::forward<Args>(args)...);
        slot.type_index = std::type_index(typeid(L));
        slot.state_index = static_cast<std::int64_t>(static_cast<Underlying>(s));

        if constexpr (Serializable<L>) {
            slot.type_tag      = std::string(L::type_tag());
            slot.serializable  = true;
            slot.serialize     = [](const std::any& a, std::vector<std::byte>& out) {
                L::serialize(std::any_cast<const L&>(a), out);
            };
            slot.deserialize   = [](std::span<const std::byte> in, std::any& a) {
                L dst{};
                (void)L::deserialize(in, dst);
                a.emplace<L>(std::move(dst));
            };
        }
        else {
            slot.serializable = false;
        }

        auto [it, inserted] = storage_.emplace(key, std::move(slot));
        (void)inserted;
        return std::any_cast<L&>(it->second.value);
    }

    /// @brief 登録済みステート別変数を取得
    /// @throws std::out_of_range 未登録または型不一致
    template <typename L>
    [[nodiscard]] L& get(TState s)
    {
        auto it = storage_.find(Key{ s, std::type_index(typeid(L)) });
        if (it == storage_.end()) {
            throw std::out_of_range(
                std::format("StateScope::get: no variable for state_index={}, type='{}'",
                            static_cast<std::int64_t>(static_cast<Underlying>(s)),
                            typeid(L).name()));
        }
        L* p = std::any_cast<L>(&it->second.value);
        if (!p) {
            throw std::out_of_range(
                std::format("StateScope::get: type mismatch for state_index={}, requested='{}'",
                            static_cast<std::int64_t>(static_cast<Underlying>(s)),
                            typeid(L).name()));
        }
        return *p;
    }

    /// @brief 登録済みステート別変数を const 参照で取得
    /// @throws std::out_of_range 未登録または型不一致
    template <typename L>
    [[nodiscard]] const L& get(TState s) const
    {
        const L* p = try_get<L>(s);
        if (!p) {
            throw std::out_of_range(
                std::format("StateScope::get: no const variable for state_index={}, type='{}'",
                            static_cast<std::int64_t>(static_cast<Underlying>(s)),
                            typeid(L).name()));
        }
        return *p;
    }

    /// @brief 登録済みステート別変数の取得試行（無ければ nullptr）
    template <typename L>
    [[nodiscard]] L* try_get(TState s) noexcept
    {
        auto it = storage_.find(Key{ s, std::type_index(typeid(L)) });
        if (it == storage_.end()) return nullptr;
        return std::any_cast<L>(&it->second.value);
    }

    template <typename L>
    [[nodiscard]] const L* try_get(TState s) const noexcept
    {
        auto it = storage_.find(Key{ s, std::type_index(typeid(L)) });
        if (it == storage_.end()) return nullptr;
        return std::any_cast<const L>(&it->second.value);
    }

    // ====================================================
    // 寿命管理
    // ====================================================

    /// @brief 単一型のステート別変数を破棄
    template <typename L>
    void release(TState s) noexcept
    {
        storage_.erase(Key{ s, std::type_index(typeid(L)) });
    }

    /// @brief 指定ステートに紐付く全変数を破棄
    void release_all_for(TState s) noexcept
    {
        const auto idx = static_cast<std::int64_t>(static_cast<Underlying>(s));
        for (auto it = storage_.begin(); it != storage_.end(); ) {
            if (it->second.state_index == idx) {
                it = storage_.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    /// @brief 全変数を破棄
    void release_all() noexcept
    {
        storage_.clear();
    }

    /// @brief 登録済みエントリ数
    [[nodiscard]] std::size_t size() const noexcept { return storage_.size(); }

    /// @brief 指定キーの存在チェック
    template <typename L>
    [[nodiscard]] bool contains(TState s) const noexcept
    {
        return storage_.find(Key{ s, std::type_index(typeid(L)) }) != storage_.end();
    }

    // ====================================================
    // enumerate / snapshot / restore（design §7.2 / §8.3）
    // ====================================================

    /// @brief 登録済みエントリの一覧を返す
    [[nodiscard]] std::vector<StateVarEntry> enumerate() const
    {
        std::vector<StateVarEntry> out;
        out.reserve(storage_.size());
        for (const auto& [key, slot] : storage_) {
            (void)key;
            out.push_back(StateVarEntry{
                make_block_key_(slot.state_index, slot.serializable ? slot.type_tag
                                                                    : type_index_fallback_tag_(slot.type_index)),
                slot.serializable ? slot.type_tag : std::string{},
                slot.serializable,
                slot.state_index
            });
        }
        return out;
    }

    /// @brief 現在の全 Serializable 変数をバイナリにスナップショット
    ///
    /// 非 Serializable 型は黙ってスキップされる（enumerate() で確認可能）。
    /// design §17 Risk-2: snapshot から漏れた事実は呼び出し側で enumerate を見て検知する。
    [[nodiscard]] std::vector<std::byte> snapshot() const
    {
        SaveWriter writer;
        for (const auto& [key, slot] : storage_) {
            (void)key;
            if (!slot.serializable) continue;
            std::vector<std::byte> payload;
            slot.serialize(slot.value, payload);
            writer.write_raw(make_block_key_(slot.state_index, slot.type_tag),
                             slot.type_tag,
                             payload);
        }
        return writer.finalize();
    }

    /// @brief snapshot() で得たバイト列から登録済み変数を復元
    ///
    /// 復元の前提:
    ///   - 復元対象キーは事前に bind() 済みであること（型情報を持たないため）。
    ///   - データが対象キーを欠く場合は当該変数は保持されたまま（部分復元を許容）。
    ///   - type_tag mismatch は HspError throw。
    ///
    /// @throws HspError magic / version / type_tag mismatch
    void restore(std::span<const std::byte> data)
    {
        SaveReader reader(data);
        for (auto& [key, slot] : storage_) {
            (void)key;
            if (!slot.serializable) continue;
            const auto block_key = make_block_key_(slot.state_index, slot.type_tag);
            auto raw = reader.find_raw(block_key);
            if (!raw) continue;  // 部分復元を許容
            if (raw->type_tag != std::string_view(slot.type_tag)) {
                throw HspError(ERR_TYPE_MISMATCH,
                               std::format("StateScope::restore: type_tag mismatch for key '{}': stored='{}', expected='{}'",
                                           block_key, raw->type_tag, slot.type_tag));
            }
            slot.deserialize(raw->payload, slot.value);
        }
    }

private:
    using Key = std::pair<TState, std::type_index>;

    struct KeyHash {
        std::size_t operator()(const Key& k) const noexcept
        {
            const auto a = static_cast<std::size_t>(
                std::hash<Underlying>{}(static_cast<Underlying>(k.first)));
            const auto b = k.second.hash_code();
            if constexpr (sizeof(std::size_t) >= 8) {
                return a ^ (b + static_cast<std::size_t>(0x9e3779b97f4a7c15ULL) + (a << 6) + (a >> 2));
            }
            else {
                return a ^ (b + static_cast<std::size_t>(0x9e3779b9UL) + (a << 6) + (a >> 2));
            }
        }
    };

    struct Slot {
        std::any                                                                          value;
        std::type_index                                                                   type_index = std::type_index(typeid(void));
        std::string                                                                       type_tag;       // 非 Serializable 時は空
        bool                                                                              serializable = false;
        std::int64_t                                                                      state_index  = 0;
        std::function<void(const std::any&, std::vector<std::byte>&)>                     serialize;     // 非 Serializable 時は空
        std::function<void(std::span<const std::byte>, std::any&)>                        deserialize;   // 同上
    };

    // ※ Key は (TState, std::type_index(typeid(L))) で構築する。
    //    各 templated API 内で typeid(L) を直接渡して Key{...} を組み立てる。

    static std::string make_block_key_(std::int64_t state_index, std::string_view type_tag)
    {
        return std::format("sv:{}:{}", state_index, type_tag);
    }

    static std::string type_index_fallback_tag_(const std::type_index& ti)
    {
        // 非 Serializable 型でも enumerate 上の key が重複しないようにする
        return std::format("<non-serializable:{}>", ti.name());
    }

    StateGraph<TState>*                          sm_ = nullptr;
    std::unordered_map<Key, Slot, KeyHash>       storage_;
};

template <typename TState>
    requires std::is_enum_v<TState>
StateScopeReadView<TState> StateScope<TState>::read_view() const noexcept
{
    return StateScopeReadView<TState>(*this);
}

template <typename TState>
    requires std::is_enum_v<TState>
StateScopeReadView<TState>::StateScopeReadView(const StateScope<TState>& scope) noexcept
    : scope_(&scope)
{}

template <typename TState>
    requires std::is_enum_v<TState>
template <typename L>
const L& StateScopeReadView<TState>::get(TState s) const
{
    return scope_->template get<L>(s);
}

template <typename TState>
    requires std::is_enum_v<TState>
template <typename L>
const L* StateScopeReadView<TState>::try_get(TState s) const noexcept
{
    return scope_->template try_get<L>(s);
}

template <typename TState>
    requires std::is_enum_v<TState>
template <typename L>
bool StateScopeReadView<TState>::contains(TState s) const noexcept
{
    return scope_->template contains<L>(s);
}

template <typename TState>
    requires std::is_enum_v<TState>
std::vector<StateVarEntry> StateScopeReadView<TState>::enumerate() const
{
    return scope_->enumerate();
}

// ═══════════════════════════════════════════════════════════════════
// state_vars 利便関数（design §7.2 末尾 / HSPPP 流儀 小文字グローバル）
// ═══════════════════════════════════════════════════════════════════

/// @brief StateScope に変数を登録するショートカット
template <typename L, typename TState, typename... Args>
    requires std::is_enum_v<TState>
L& state_vars(StateScope<TState>& scope, TState s, Args&&... args)
{
    return scope.template bind<L>(s, std::forward<Args>(args)...);
}

}  // namespace hsppp
