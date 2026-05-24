---
layout: default
title: Data Sharing Guide
---

# データ共有ガイド

StateMachine でデータを共有する2つのパターンと使い分け方法。

---

## 2つのパターン

### 1. シンプルパターン（ローカル変数）

`on_update` 内でループを書く場合、普通のローカル変数で十分です。

```cpp
enum class Scene { Title, Game };

void hspMain() {
    StateMachine<Scene> sm;
    
    sm.state(Scene::Game)
      .on_update([](auto& sm) {
          // ✅ ローカル変数でOK
          int score = 0;
          int lives = 3;
          
          while (!sm.is_transitioning()) {
              score += 10;
              
              if (stick(256)) {
                  lives--;
                  if (lives == 0) {
                      sm.jump(Scene::Title);
                  }
              }
              
              mes(strf("Score: %d, Lives: %d", score, lives));
              await(16);
          }
      });
    
    sm.start(Scene::Title);
}
```

**メリット**:
- ✅ シンプル
- ✅ 追加コード不要
- ✅ スコープが明確

**デメリット**:
- ❌ `on_enter` と `on_exit` では使えない

---

### 2. 構造化パターン（state<T>()）

`on_enter`, `on_update`, `on_exit` でデータを共有したい場合に使います。

```cpp
struct GameSceneData {
    int score = 0;
    int lives = 3;
    int stage = 1;
};

void hspMain() {
    StateMachine<Scene> sm;
    
    sm.state<GameSceneData>(Scene::Game)
      .on_enter([](auto& sm, GameSceneData& data) {
          // ✅ 初期化
          data.score = 0;
          data.lives = 3;
          data.stage = 1;
      })
      .on_update([](auto& sm, GameSceneData& data) {
          while (!sm.is_transitioning()) {
              data.score += 10;
              
              if (stick(256)) {
                  data.lives--;
                  if (data.lives == 0) {
                      sm.jump(Scene::Title);
                  }
              }
              
              mes(strf("Score: %d, Lives: %d", data.score, data.lives));
              await(16);
          }
      })
      .on_exit([](auto& sm, GameSceneData& data) {
          // ✅ 終了処理（ハイスコア保存など）
          auto& profile = services().data<PlayerProfile>();
          if (data.score > profile.high_score) {
              profile.high_score = data.score;
          }
      });
    
    sm.start(Scene::Title);
}
```

**メリット**:
- ✅ `on_enter` / `on_exit` でも使える
- ✅ データがまとまっている
- ✅ 自動的にクリーンアップされる

**デメリット**:
- ❌ 構造体の定義が必要

---

## 永続データ: Repository<T>

シーンをまたいで保持したいデータには `Repository<T>` を使います。

```cpp
struct PlayerProfile {
    std::string name = "Player";
    int high_score = 0;
};

void hspMain() {
    StateMachine<Scene> sm;
    
    sm.state(Scene::Game)
      .on_update([](auto& sm) {
          // ✅ どのシーンからでもアクセス可能
          auto& profile = services().data<PlayerProfile>();
          
          int score = 0;
          while (!sm.is_transitioning()) {
              score += 10;
              mes(strf("Score: %d, High: %d", score, profile.high_score));
              await(16);
          }
          
          // ハイスコア更新
          if (score > profile.high_score) {
              profile.high_score = score;
          }
      });
    
    sm.state(Scene::Result)
      .on_update([](auto& sm) {
          // ✅ 別のシーンでも同じデータにアクセス
          auto& profile = services().data<PlayerProfile>();
          
          mes(strf("High Score: %d", profile.high_score));
          mes("Press SPACE");
          
          while (!sm.is_transitioning()) {
              if (stick(16)) sm.jump(Scene::Title);
              await(16);
          }
      });
    
    sm.start(Scene::Title);
}
```

---

## グローバル変数との違い

### Repository<T> vs グローバル変数

`Repository<T>` は「型安全なグローバル変数」ですが、いくつかの利点があります：

| 特徴 | グローバル変数 | Repository<T> |
|------|--------------|--------------|
| **型安全性** | ❌ 名前の衝突が起きる | ✅ 型ごとに独立 |
| **名前空間** | ❌ グローバル名前空間を汚染 | ✅ 型名で管理 |
| **リセット** | ❌ 手動で初期化が必要 | ✅ reset_all() で一括初期化 |
| **可視性** | ❌ どこからでも暗黙的にアクセス | ✅ services().data<T>() で明示的 |
| **テスト** | ❌ 状態のクリアが面倒 | ✅ register/reset でクリア可能 |

**例：グローバル変数の問題**

```cpp
// ❌ グローバル変数：名前の衝突が起きる
int score = 0;          // ゲームのスコア？
int score_multiplier;   // これも score？
int high_score;         // 似た名前が増える...

void game_scene() {
    score += 10;  // どのスコアかわかりにくい
}
```

**Repository<T> の解決策**

```cpp
// ✅ Repository: 型でグループ化
struct GameState {
    int score = 0;
    int multiplier = 1;
};

struct PlayerProfile {
    int high_score = 0;
};

void game_scene() {
    auto& game = services().data<GameState>();
    game.score += 10 * game.multiplier;  // 明確
    
    auto& profile = services().data<PlayerProfile>();
    if (game.score > profile.high_score) {
        profile.high_score = game.score;
    }
}
```

### 注意点：それでもグローバルステート

**Repository<T> は万能ではありません**：

1. **結局グローバル**: 内部的にはシングルトンなので、グローバル変数と同じ問題を抱える
2. **テスタビリティ**: 依存性注入 (DI) を使ったほうがテストしやすい
3. **スレッドセーフではない**: マルチスレッドでは使用不可
4. **暗黙的な依存関係**: どの関数がどのデータを使うか、コードから見えにくい

**推奨される使い方**：

```cpp
// ✅ ゲームのトップレベルで使う（HSPのグローバル変数の代わり）
void hspMain() {
    auto& settings = services().data<GameSettings>();
    settings.volume = 0.8f;
    
    StateMachine<Scene> sm;
    // ...
}

// ⚠️ 深い関数から使うのは避ける（依存関係が見えにくくなる）
void deep_function() {
    auto& settings = services().data<GameSettings>();  // ここで突然出てくると混乱
    // ...
}
```

**いつグローバル変数を使うべきか**：

| ケース | グローバル変数 | Repository<T> |
|--------|--------------|--------------|
| **HSPから移植** | ❌ | ✅ Repository で置き換え |
| **設定データ** | ❌ | ✅ Repository 推奨 |
| **セーブデータ** | ❌ | ✅ Repository 推奨 |
| **一時的なフラグ** | ✅ ローカル変数で | ❌ 不要 |
| **大規模プロジェクト** | ❌ | ⚠️ DI を検討 |

---

## 使い分けチャート

```
データを共有したい？
│
├─ シーンをまたぐ？
│  └─ YES → Repository<T>
│
└─ 同じシーン内だけ？
   │
   ├─ on_enter / on_exit で使う？
   │  └─ YES → state<T>()
   │
   └─ on_update 内だけ？
      └─ YES → ローカル変数
```

---

## パターン別の実例

### 例1: タイトル画面（シンプル）

```cpp
sm.state(Scene::Title)
  .on_update([](auto& sm) {
      int selected = 0;  // ✅ ローカル変数で十分
      
      while (!sm.is_transitioning()) {
          mes("1. Start Game");
          mes("2. Options");
          
          if (stick(128)) selected--;
          if (stick(256)) selected++;
          selected = (selected + 2) % 2;
          
          if (stick(16)) {
              sm.jump(selected == 0 ? Scene::Game : Scene::Options);
          }
          
          await(16);
      }
  });
```

### 例2: ゲームシーン（構造化）

```cpp
struct GameSceneData {
    int score = 0;
    int stage = 1;
    bool boss_defeated = false;
};

sm.state<GameSceneData>(Scene::Game)
  .on_enter([](auto& sm, GameSceneData& data) {
      // ステージ開始時の初期化
      data.score = 0;
      data.boss_defeated = false;
  })
  .on_update([](auto& sm, GameSceneData& data) {
      while (!sm.is_transitioning()) {
          // ゲームロジック
          data.score += 10;
          
          // ボス撃破判定
          if (/* boss HP == 0 */) {
              data.boss_defeated = true;
              data.stage++;
          }
          
          await(16);
      }
  })
  .on_exit([](auto& sm, GameSceneData& data) {
      // スコアを保存
      auto& profile = services().data<PlayerProfile>();
      if (data.score > profile.high_score) {
          profile.high_score = data.score;
      }
  });
```

### 例3: プレイヤープロファイル（永続）

```cpp
struct PlayerProfile {
    std::string name = "Player";
    int level = 1;
    int exp = 0;
    int gold = 100;
};

void hspMain() {
    // ✅ 登録してリセット可能にする
    services().register_repository<PlayerProfile>();
    
    StateMachine<Scene> sm;
    
    sm.state(Scene::Game)
      .on_update([](auto& sm) {
          auto& profile = services().data<PlayerProfile>();
          
          while (!sm.is_transitioning()) {
              profile.exp += 10;
              if (profile.exp >= 100) {
                  profile.level++;
                  profile.exp = 0;
              }
              await(16);
          }
      });
    
    sm.state(Scene::Shop)
      .on_update([](auto& sm) {
          auto& profile = services().data<PlayerProfile>();
          
          // ✅ 同じデータにアクセス
          mes(strf("Gold: %d", profile.gold));
          await(16);
      });
    
    sm.start(Scene::Title);
}
```

---

## サブステートマシン: tick()

メインステートマシン内でサブステートマシンを使う場合、`tick()` を使います。

```cpp
enum class BattlePhase { Start, PlayerTurn, EnemyTurn, End };

struct BattleData {
    int turn = 0;
    int player_hp = 100;
    int enemy_hp = 50;
};

sm.state(Scene::Battle)
  .on_update([](auto& sm) {
      // ✅ サブステートマシンを作成
      StateMachine<BattlePhase> battle;
      
      battle.state<BattleData>(BattlePhase::Start)
        .on_enter([](auto& sm, BattleData& data) {
            data.turn = 0;
            data.player_hp = 100;
            data.enemy_hp = 50;
        })
        .on_update([](auto& sm, BattleData& data) {
            int frame = 0;
            while (!sm.is_transitioning()) {
                mes("Battle Start!");
                if (++frame >= 60) sm.jump(BattlePhase::PlayerTurn);
                await(16);
            }
        });
      
      battle.state<BattleData>(BattlePhase::PlayerTurn)
        .on_update([](auto& sm, BattleData& data) {
            while (!sm.is_transitioning()) {
                mes(strf("Turn %d: Your Turn", data.turn));
                if (stick(16)) {  // SPACE
                    data.enemy_hp -= 20;
                    sm.jump(BattlePhase::EnemyTurn);
                }
                await(16);
            }
        });
      
      battle.state<BattleData>(BattlePhase::EnemyTurn)
        .on_update([](auto& sm, BattleData& data) {
            int frame = 0;
            while (!sm.is_transitioning()) {
                mes("Enemy Turn");
                if (++frame >= 60) {
                    data.player_hp -= 10;
                    data.turn++;
                    sm.jump(BattlePhase::PlayerTurn);
                }
                await(16);
            }
        });
      
      battle.start(BattlePhase::Start);
      
      // ✅ tick() で1フレームずつ更新（ブロックしない）
      while (!sm.is_transitioning()) {
          battle.tick();
          
          // バトル終了チェック
          if (!battle.is_running()) {
              sm.jump(Scene::Result);
          }
          
          await(16);
      }
  });
```

**ポイント**:
- `tick()` は1フレームだけ実行してすぐ戻る
- `run()` と違ってブロックしない
- メインループで `battle.tick()` を呼ぶ
- `is_running()` でサブステートが終了したか確認

---

## is_transitioning() の使い方

`is_running()` は「ステートマシンが動いているか」ですが、  
`is_transitioning()` は「`jump()` が呼ばれてステート遷移中か」を示します。

```cpp
sm.state(Scene::Game)
  .on_update([](auto& sm) {
      while (!sm.is_transitioning()) {  // ✅ 遷移が呼ばれるまでループ
          // ゲームロジック
          
          if (stick(256)) {  // ESC
              sm.jump(Scene::Title);  // ← これが呼ばれると is_transitioning() == true
          }
          
          await(16);
      }
  });
```

**使い分け**:
- `is_running()`: ステートマシン全体が動いているか（メインループ用）
- `is_transitioning()`: ステート遷移が始まったか（`on_update` 内のループ用）

---

## まとめ

| パターン | 用途 | ライフサイクル |
|---------|------|--------------|
| **ローカル変数** | `on_update` 内だけで使う | ループの間だけ |
| **state<T>()** | `on_enter/update/exit` で共有 | ステートの間だけ |
| **Repository<T>** | シーンをまたいで永続 | アプリ全体 |

**推奨フロー**:
1. まずローカル変数で書く
2. `on_enter`/`on_exit` が必要なら `state<T>()` に変更
3. シーンをまたぐデータは `Repository<T>` に移動

---

## 関連項目

- [Repository API](../api/repository.html) - Repository<T> の詳細
- [StateMachine API](../api/statemachine.html) - tick(), is_transitioning() の詳細
