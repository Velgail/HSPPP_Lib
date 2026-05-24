---
layout: default
title: Repository API
---

# Repository API

永続データを型安全に管理するリポジトリパターンの実装です。

## 概要

`Repository<T>` と `GameServices` は、グローバル変数の代わりに使用する永続データ管理システムです。

### 主な特徴

- ✅ **型安全**: 各データ型ごとに独立したストレージ
- ✅ **グローバルアクセス**: どこからでもアクセス可能
- ✅ **シングルトン**: 型ごとに1つのインスタンス
- ✅ **簡潔な構文**: `services().data<T>()`

---

## Repository<T>

### 基本的な使い方

```cpp
import hsppp;
using namespace hsppp;

struct PlayerData {
    int hp = 100;
    int level = 1;
    std::string name = "Player";
};

void hspMain() {
    // データにアクセス
    auto& player = services().data<PlayerData>();
    
    player.hp -= 10;
    player.level = 5;
    
    logmes(strf("HP: %d, Level: %d", player.hp, player.level));
}
```

### メソッド

#### `static Repository<T>& instance()`

シングルトンインスタンスを取得します。

```cpp
auto& repo = Repository<PlayerData>::instance();
```

通常は `services().repo<T>()` または `services().data<T>()` を使用します。

#### `T& get()` / `const T& get() const`

データの参照を取得します。

```cpp
auto& data = repo.get();
data.hp = 100;
```

#### `void set(const T& value)` / `void set(T&& value)`

データを設定します。

```cpp
PlayerData new_data;
new_data.hp = 50;
repo.set(new_data);

// ムーブも可能
repo.set(PlayerData{});
```

#### `void reset()`

データをデフォルト値にリセットします。

```cpp
repo.reset();  // PlayerData{} と同じ
```

---

## GameServices

### 基本的な使い方

```cpp
// データに直接アクセス
auto& player = services().data<PlayerData>();

// リポジトリにアクセス
auto& repo = services().repo<PlayerData>();
```

### メソッド

#### `static GameServices& instance()`

シングルトンインスタンスを取得します。

```cpp
auto& svc = GameServices::instance();
```

通常は `services()` ショートカットを使用します。

#### `template<typename T> Repository<T>& repo()`

指定した型のリポジトリを取得します。

```cpp
auto& repo = services().repo<PlayerData>();
```

#### `template<typename T> T& data()`

リポジトリのデータに直接アクセスします。

```cpp
auto& player = services().data<PlayerData>();
player.hp -= 10;
```

#### `template<typename T, typename Tag = GlobalTag> void register_repository()`

リポジトリを登録します（`reset_all()` で一括リセット可能にする）。

```cpp
services().register_repository<PlayerData>();  // 既定は GlobalTag
services().register_repository<GameProgress>();

// タグを指定して登録（任意の型でOK）
services().register_repository<PlayerData, StateTag>();

// ユーザー定義タグも使用可能
struct SaveSlot1Tag : RepositoryTagBase {};
services().register_repository<PlayerData, SaveSlot1Tag>();
```

#### `void reset_all()`

登録されたすべてのリポジトリをリセットします。

```cpp
// ゲーム開始時に全データをリセット
services().reset_all();
```

---

## services() ヘルパー関数

`GameServices::instance()` のショートカットです。

```cpp
// この2つは同じ
auto& data1 = GameServices::instance().data<PlayerData>();
auto& data2 = services().data<PlayerData>();
```

---

## 実践例

### 複数のデータ型を管理

```cpp
struct PlayerProfile {
    std::string name = "Player";
    int high_score = 0;
};

struct GameSettings {
    int difficulty = 1;
    float volume = 0.8f;
};

void hspMain() {
    // リポジトリを登録
    services().register_repository<PlayerProfile, StateTag>();
    services().register_repository<GameSettings, GlobalTag>();
    
    // データにアクセス
    auto& profile = services().data<PlayerProfile>();
    auto& settings = services().data<GameSettings>();
    
    profile.high_score = 1000;
    settings.volume = 0.5f;
    
    // 全リセット
    services().reset_all();
    
    logmes(strf("High Score: %d", profile.high_score));  // 0
}
```

### StateMachine との組み合わせ

```cpp
enum class Scene { Title, Game, Result };

void hspMain() {
    StateMachine<Scene> sm;
    
    sm.state(Scene::Game)
      .on_update([](auto& sm) {
          // ✅ キャプチャ不要でアクセス
          auto& profile = services().data<PlayerProfile>();
          
          while (!sm.is_transitioning()) {
              profile.high_score += 10;
              
              mes(strf("Score: %d", profile.high_score));
              await(16);
          }
      });
    
    sm.start(Scene::Title);
}
```

---

## ベストプラクティス

### ✅ 推奨

1. **構造体でデータをグループ化**
   ```cpp
   struct PlayerData {
       int hp;
       int mp;
       int level;
   };
   ```

2. **デフォルト値を設定**
   ```cpp
   struct GameSettings {
       int difficulty = 1;
       float volume = 0.8f;
   };
   ```

3. **services() を使用**
   ```cpp
   auto& data = services().data<PlayerData>();  // ✅
   auto& data = Repository<PlayerData>::instance().get();  // ❌ 冗長
   ```

### ⚠️ 注意点

1. **グローバル変数と同等**: テスタビリティは低い
2. **型ごとに1つ**: 同じ型で複数のインスタンスは作れない
3. **スレッド安全性なし**: マルチスレッドでは使用不可

---

## 関連項目

- [StateMachine API](statemachine.html) - ステートマシンとの連携
- [Data Sharing Guide](../guides/data-sharing.html) - データ共有のベストプラクティス
