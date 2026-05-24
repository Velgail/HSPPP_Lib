// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp_repository.ixx
// ═══════════════════════════════════════════════════════════════════
// HSPPP Repository - モジュールインターフェース
// ═══════════════════════════════════════════════════════════════════
//
// グローバル変数の代わりに使用する型安全な永続データストレージ。
// シングルトンベースで、型ごとに独立したストレージを提供します。
//
// 使用例:
//   struct PlayerData { int score = 0; };
//   auto& data = services().data<PlayerData>();
//   data.score += 10;

export module hsppp:repository;

import :types;
import <memory>;
import <unordered_map>;
import <typeindex>;
import <functional>;

export namespace hsppp {

/// @brief リポジトリ保存タグの基底
struct RepositoryTagBase {};

/// @brief グローバル設定向けの既定タグ
struct GlobalTag : RepositoryTagBase {};

/// @brief セーブデータ向けの既定タグ
struct StateTag : RepositoryTagBase {};

/// @brief 型安全なリポジトリ（データストア）
/// 
/// シングルトンパターンで永続データを管理します。
/// 型ごとに独立したストレージを持ちます。
/// 
/// @tparam T 管理するデータ型（POD推奨）
template<typename T>
class Repository {
public:
    /// @brief シングルトンインスタンスを取得
    static Repository<T>& instance() {
        static Repository<T> inst;
        return inst;
    }
    
    /// @brief データの参照を取得
    T& get() { return data_; }
    const T& get() const { return data_; }
    
    /// @brief データを設定（コピー）
    void set(const T& value) { data_ = value; }
    
    /// @brief データを設定（ムーブ）
    void set(T&& value) { data_ = std::move(value); }
    
    /// @brief データをデフォルト構築された状態にリセット
    void reset() { data_ = T{}; }
    
private:
    Repository() = default;
    ~Repository() = default;
    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;
    
    T data_{};
};

/// @brief ゲームサービス統合クラス
/// 
/// 永続データへのアクセスポイントを提供します。
/// グローバル変数の代わりに使用してください。
class GameServices {
public:
    /// @brief シングルトンインスタンスを取得
    static GameServices& instance() {
        static GameServices inst;
        return inst;
    }
    
    /// @brief 型によるリポジトリアクセス
    template<typename T>
    Repository<T>& repo() {
        return Repository<T>::instance();
    }
    
    /// @brief リポジトリデータへの直接アクセス
    template<typename T>
    T& data() {
        return Repository<T>::instance().get();
    }
    
    /// @brief リポジトリをリセット追跡用に登録
    /// 
    /// reset_all() に含めたいデータ型ごとに1回呼び出してください。
    /// @tparam Tag 分類用タグ（任意）。既定は GlobalTag。
    template<typename T, typename Tag = GlobalTag>
    void register_repository() {
        auto type_id = std::type_index(typeid(T));
        if (registrations_.find(type_id) == registrations_.end()) {
            registrations_[type_id] = RepositoryRegistration{
                []() { Repository<T>::instance().reset(); },
                std::type_index(typeid(Tag))
            };
        }
    }
    
    /// @brief 登録されたすべてのリポジトリをリセット
    /// 
    /// ゲーム開始時に呼び出してすべてのデータを初期化します。
    /// register_repository<T>() で登録されたリポジトリのみリセットされます。
    void reset_all() {
        for (auto& [type, registration] : registrations_) {
            registration.reset();
        }
    }
    
private:
    GameServices() = default;
    ~GameServices() = default;
    GameServices(const GameServices&) = delete;
    GameServices& operator=(const GameServices&) = delete;
    
    struct RepositoryRegistration {
        std::function<void()> reset;
        std::type_index tag;
    };

    std::unordered_map<std::type_index, RepositoryRegistration> registrations_;
};

/// @brief GameServices へアクセスする便利なショートカット
inline GameServices& services() {
    return GameServices::instance();
}

}  // namespace hsppp
