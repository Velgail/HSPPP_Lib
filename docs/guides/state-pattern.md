---
layout: default
title: ステートパターンガイド
---

# ステートパターン 使い方ガイド

HSP の `*label` / `goto` を使った状態遷移を、安全かつ効率的に実装する方法を解説します。

---

## なぜステートマシンが必要？

### HSPでよくあるパターン

```hsp
; HSP: goto によるループ
*title
    redraw 0
    color 255, 255, 255 : cls
    mes "Title Screen"
    redraw 1
    
    stick key
    if key & 32 : goto *game  ; Space
    
    await 16
    goto *title

*game
    redraw 0
    color 0, 0, 0 : cls
    mes "Playing..."
    redraw 1
    
    stick key
    if key & 128 : goto *result  ; ESC
    
    await 16
    goto *game
```

### 問題点

1. **GUIオブジェクトの管理が難しい**
   - ボタンを毎回作り直すと重い
   - `clrobj` のタイミングが難しい

2. **状態遷移が複雑になると破綻**
   - どこからどこに遷移できるか分からない
   - スパゲッティコード化しやすい

3. **C++では goto は非推奨**
   - モダンなコードスタイルに合わない

---

## HSPPP ステートマシンの解決策

### 基本的な構造

```cpp
enum class Screen {
    Title,
    Game,
    Result
};

auto sm = StateMachine<Screen>();

sm.state(Screen::Title)
  .on_enter([]() {
      // ✅ 初回のみ: 重い処理
  })
  .on_update([&](auto& sm) {
      // ✅ 毎フレーム: 軽い処理
      await(16);
  })
  .on_exit([]() {
      // ✅ 離脱時: 後始末
  });

sm.jump(Screen::Title);
sm.run();
```

---

## パターン1: 基本的なゲームフロー

### 最小限の実装

```cpp
import hsppp;
using namespace hsppp;

enum class GameScreen {
    Title,
    Game,
    Result
};

void hspMain() {
    auto sm = StateMachine<GameScreen>();
    int score = 0;
    
    // ═══════════════════════════════════════════
    // タイトル画面
    // ═══════════════════════════════════════════
    sm.state(GameScreen::Title)
      .on_update([&](auto& sm) {
          color(0, 0, 0);
          boxf();
          color(255, 255, 255);
          pos(200, 200);
          mes("Press SPACE to Start");
          
          if (getkey(' ')) {
              sm.jump(GameScreen::Game);
          }
          if (getkey('Q')) {
              sm.quit();
          }
          await(16);
      });
    
    // ═══════════════════════════════════════════
    // ゲーム画面
    // ═══════════════════════════════════════════
    sm.state(GameScreen::Game)
      .on_enter([&]() {
          score = 0;  // 初期化
      })
      .on_update([&](auto& sm) {
          color(0, 0, 0);
          boxf();
          mes("Score: " + std::to_string(score));
          
          // ゲームロジック
          score += rnd(10);
          
          if (getkey(VK_ESCAPE)) {
              sm.jump(GameScreen::Result);
          }
          await(16);
      });
    
    // ═══════════════════════════════════════════
    // リザルト画面
    // ═══════════════════════════════════════════
    sm.state(GameScreen::Result)
      .on_update([&](auto& sm) {
          color(0, 0, 0);
          boxf();
          mes("Final Score: " + std::to_string(score));
          mes("R: Retry  T: Title  Q: Quit");
          
          if (getkey('R')) sm.jump(GameScreen::Game);
          if (getkey('T')) sm.jump(GameScreen::Title);
          if (getkey('Q')) sm.quit();
          await(16);
      });
    
    sm.start(GameScreen::Title, 60);
}
```

---

## パターン2: GUIオブジェクトの効率的管理

### 問題: 毎フレーム作り直すと重い

```cpp
// ❌ 悪い例
sm.state(Screen::Title)
  .on_update([](auto& sm) {
      clrobj();  // 毎フレーム全削除
      button("Start", []() { /* ... */ });  // 毎フレーム作り直し
      button("Quit", []() { /* ... */ });
      // → 重い、非効率
  });
```

### 解決: on_enter / on_exit で分離

```cpp
// ✅ 良い例
sm.state(Screen::Title)
  .on_enter([]() {
      // ✅ 初回のみ実行: GUI作成
      button("Start", []() { /* ... */ });
      button("Quit", []() { /* ... */ });
      button("Options", []() { /* ... */ });
  })
  .on_update([&](auto& sm) {
      // ✅ 毎フレーム: 軽い処理のみ
      // 画面遷移チェックなど
      if (getkey(' ')) {
          sm.jump(Screen::Game);
      }
  })
  .on_exit([]() {
      // ✅ 離脱時: GUIクリア
      clrobj();
  });
```

### 完全な例

```cpp
enum class Screen {
    Menu,
    Settings,
    Credits
};

void hspMain() {
    auto sm = StateMachine<Screen>();
    
    // 設定値（ステート間で共有）
    auto volume = std::make_shared<int>(50);
    auto fullscreen = std::make_shared<int>(0);
    
    // ═══════════════════════════════════════════
    // メインメニュー
    // ═══════════════════════════════════════════
    sm.state(Screen::Menu)
      .on_enter([&]() {
          color(0, 0, 0);
          boxf();
          pos(200, 100);
          mes("Main Menu");
          
          // GUIオブジェクト作成（1回のみ）
          button("Settings", [&]() {
              sm.jump(Screen::Settings);
              return 0;
          });
          button("Credits", [&]() {
              sm.jump(Screen::Credits);
              return 0;
          });
          button("Quit", []() {
              end(0);
              return 0;
          });
      })
      .on_update([&](auto& sm) {
          // キーボードショートカット
          if (getkey('1')) sm.jump(Screen::Settings);
          if (getkey('2')) sm.jump(Screen::Credits);
          if (getkey('Q')) sm.quit();
      })
      .on_exit([]() {
          clrobj();  // GUIクリア
      });
    
    // ═══════════════════════════════════════════
    // 設定画面
    // ═══════════════════════════════════════════
    sm.state(Screen::Settings)
      .on_enter([&]() {
          color(0, 0, 0);
          boxf();
          mes("Settings");
          
          // スライダーやチェックボックス作成
          pos(50, 100);
          mes("Volume:");
          // slider等を配置...
      })
      .on_update([&](auto& sm) {
          // 設定値の表示
          pos(50, 150);
          mes("Current Volume: " + std::to_string(*volume));
          
          if (getkey(VK_ESCAPE)) {
              sm.back();  // 前の画面に戻る
          }
      })
      .on_exit([]() {
          clrobj();
      });
    
    sm.enable_history(5);  // 履歴機能を有効化
    
    sm.jump(Screen::Menu);
    sm.run();
}
```

---

## パターン3: ポーズ機能の実装

```cpp
enum class GameState {
    Playing,
    Paused,
    GameOver
};

void hspMain() {
    auto sm = StateMachine<GameState>();
    
    int score = 0;
    int player_x = 320;
    int player_y = 240;
    
    // ═══════════════════════════════════════════
    // プレイ中
    // ═══════════════════════════════════════════
    sm.state(GameState::Playing)
      .on_update([&](auto& sm) {
          // ゲームロジック
          color(0, 0, 0);
          boxf();
          circle(player_x - 10, player_y - 10, player_x + 10, player_y + 10);
          mes("Score: " + std::to_string(score));
          
          // 入力処理
          if (getkey(VK_LEFT)) player_x -= 5;
          if (getkey(VK_RIGHT)) player_x += 5;
          
          // ポーズ
          if (getkey('P')) {
              sm.jump(GameState::Paused);
          }
          
          score++;
      });
    
    // ═══════════════════════════════════════════
    // ポーズ中
    // ═══════════════════════════════════════════
    sm.state(GameState::Paused)
      .on_enter([]() {
          // 半透明オーバーレイ表示など
      })
      .on_update([&](auto& sm) {
          // ゲーム画面はそのまま
          color(0, 0, 0);
          boxf(0, 0, 640, 480);
          
          color(255, 255, 255);
          pos(250, 200);
          mes("PAUSED");
          pos(200, 250);
          mes("Press P to Resume");
          
          if (getkey('P')) {
              sm.jump(GameState::Playing);
          }
          await(16);
      });
    
    sm.jump(GameState::Playing);
    sm.run();
}
```

---

## パターン4: 遷移制約（上級者向け）

### いつ使う？

- 不正な画面遷移を防ぎたい
- ゲームフローを厳密に制御したい
- バグを早期発見したい

### 実装例

```cpp
enum class GameFlow {
    Splash,      // スプラッシュ画面
    Title,       // タイトル
    CharSelect,  // キャラ選択
    Game,        // ゲーム本編
    Result       // リザルト
};

void hspMain() {
    auto sm = StateMachine<GameFlow>();
    
    // 厳格モードを有効化
    sm.set_unrestricted_transitions(false);
    
    // 許可する遷移のみ定義
    sm.allow_transition(GameFlow::Splash, GameFlow::Title);
    sm.allow_transition(GameFlow::Title, GameFlow::CharSelect);
    sm.allow_transition(GameFlow::CharSelect, GameFlow::Game);
    sm.allow_transition(GameFlow::Game, GameFlow::Result);
    sm.allow_transition(GameFlow::Result, GameFlow::Title);
    sm.allow_transition(GameFlow::Result, GameFlow::CharSelect);
    
    // この場合、以下の遷移は実行時エラー
    // - Title → Game (CharSelectを経由する必要がある)
    // - Game → Title (Resultを経由する必要がある)
    
    sm.state(GameFlow::Splash)
      .on_enter([&]() {
          sm.set_timer(GameFlow::Title, 3000);  // 3秒後に自動遷移
      });
    
    sm.state(GameFlow::Title)
      .on_update([&](auto& sm) {
          if (getkey(' ')) {
              sm.jump(GameFlow::CharSelect);  // ✅ OK
              // sm.jump(GameFlow::Game);  // ❌ 実行時エラー
          }
          await(16);
      });
    
    sm.jump(GameFlow::Splash);
    sm.run();
}
```

---

## パターン5: 階層的な状態管理

### サブステートの実装（応用編）

```cpp
enum class MainState {
    Frontend,  // フロントエンド（タイトルなど）
    InGame     // ゲーム本編
};

enum class FrontendState {
    Title,
    CharSelect,
    Options
};

enum class InGameState {
    Playing,
    Paused,
    Inventory
};

void hspMain() {
    auto main_sm = StateMachine<MainState>();
    auto frontend_sm = StateMachine<FrontendState>();
    auto ingame_sm = StateMachine<InGameState>();
    
    main_sm.state(MainState::Frontend)
      .on_update([&](auto& sm) {
          // フロントエンド側のステートマシンを更新
          // (実装は複雑になるため、シンプルな設計を推奨)
      });
    
    // ※この例は高度すぎるため、通常は1つのステートマシンで十分です
}
```

---

## パターン6: 複数ファイルに分割したシーン管理

大規模プロジェクトでは、各ステートを別ファイルに分割することで保守性が向上します。

### ファイル構成例

```
MyProject/
├── main.cpp              // エントリーポイント
├── Scenes/
│   ├── SceneCommon.h     // 共通定義
│   ├── TitleScene.cpp    // タイトルシーン
│   ├── GameScene.cpp     // ゲームシーン
│   └── ResultScene.cpp   // リザルトシーン
```

### SceneCommon.h - 共通定義

```cpp
// Scenes/SceneCommon.h
#pragma once
import hsppp;

// シーン列挙型
enum class Scene {
    Title,
    Game,
    Result
};

// ステートマシン型の別名
using SceneManager = hsppp::StateMachine<Scene>;

// 各シーンの登録関数（前方宣言）
void registerTitleScene(SceneManager& sm);
void registerGameScene(SceneManager& sm);
void registerResultScene(SceneManager& sm);
```

### TitleScene.cpp - タイトルシーン

```cpp
// Scenes/TitleScene.cpp
#include "SceneCommon.h"
import <string>;
using namespace hsppp;

void registerTitleScene(SceneManager& sm) {
    sm.state(Scene::Title)
      .on_enter([]() {
          // タイトル画面の初期化
          color(0, 0, 0);
          boxf();
          
          button("Start Game", []() {
              // ボタンから直接遷移はできないため、
              // on_update でフラグチェック
          });
          button("Quit", []() {
              end();
          });
      })
      .on_update([&](auto& sm) {
          // 描画更新
          color(255, 255, 255);
          pos(200, 100);
          mes("My Game Title");
          
          // キーボード入力でもシーン遷移可能
          if (getkey(' ')) {
              sm.jump(Scene::Game);
          }
          
          await(16);
      })
      .on_exit([]() {
          // GUIオブジェクトをクリア
          clrobj();
      });
}
```

### GameScene.cpp - ゲームシーン

```cpp
// Scenes/GameScene.cpp
#include "SceneCommon.h"
import <format>;
using namespace hsppp;

void registerGameScene(SceneManager& sm) {
    // ゲーム固有の状態（スタティックで保持）
    static int score = 0;
    static int player_x = 320;
    static int player_y = 240;
    
    sm.state(Scene::Game)
      .on_enter([]() {
          // ゲーム開始時の初期化
          score = 0;
          player_x = 320;
          player_y = 240;
      })
      .on_update([&](auto& sm) {
          // 入力処理
          if (getkey(VK_LEFT))  player_x -= 5;
          if (getkey(VK_RIGHT)) player_x += 5;
          if (getkey(VK_UP))    player_y -= 5;
          if (getkey(VK_DOWN))  player_y += 5;
          
          // ゲームロジック
          score += rnd(10);
          
          // 描画
          color(0, 0, 0);
          boxf();
          color(255, 255, 255);
          pos(10, 10);
          mes(std::format("Score: {}", score));
          
          color(255, 0, 0);
          boxf(player_x - 10, player_y - 10, player_x + 10, player_y + 10);
          
          // リザルトへ遷移
          if (getkey(VK_ESCAPE)) {
              sm.jump(Scene::Result);
          }
          
          await(16);
      })
      .on_exit([]() {
          // ゲーム終了処理
      });
}
```

### ResultScene.cpp - リザルトシーン

```cpp
// Scenes/ResultScene.cpp
#include "SceneCommon.h"
import <format>;
using namespace hsppp;

void registerResultScene(SceneManager& sm) {
    sm.state(Scene::Result)
      .on_enter([]() {
          color(0, 0, 0);
          boxf();
          
          button("Retry", []() {
              // on_updateでフラグチェック
          });
          button("Title", []() {
              // on_updateでフラグチェック
          });
      })
      .on_update([&](auto& sm) {
          color(255, 255, 255);
          pos(200, 200);
          mes("Game Over");
          
          // キーボード入力
          if (getkey('R')) sm.jump(Scene::Game);
          if (getkey('T')) sm.jump(Scene::Title);
          if (getkey('Q')) sm.quit();
          
          await(16);
      })
      .on_exit([]() {
          clrobj();
      });
}
```

### main.cpp - エントリーポイント

```cpp
// main.cpp
import hsppp;
#include "Scenes/SceneCommon.h"
using namespace hsppp;

void hspMain() {
    auto sm = SceneManager();
    
    // 各シーンを登録
    registerTitleScene(sm);
    registerGameScene(sm);
    registerResultScene(sm);
    
    // 初期シーンから開始
    sm.start(Scene::Title);
}
```

### 分割のメリット

1. **保守性向上**
   - 各シーンが独立したファイルで管理
   - 変更の影響範囲が明確

2. **チーム開発に有利**
   - 複数人で異なるシーンを同時編集可能
   - コンフリクトが起きにくい

3. **コード再利用**
   - 共通処理をヘルパー関数として分離しやすい

4. **コンパイル時間短縮**
   - 変更したファイルのみ再コンパイル

### 共有データの扱い方

シーン間で共有するデータ（スコア、プレイヤー情報など）は以下の方法で管理：

**方法1: グローバル変数（シンプル）**

```cpp
// SceneCommon.h
inline int g_score = 0;
inline std::string g_player_name;
```

**方法2: 構造体でまとめる（推奨）**

```cpp
// SceneCommon.h
struct GameData {
    int score = 0;
    int high_score = 0;
    std::string player_name;
    int player_level = 1;
};

inline GameData g_gameData;
```

**方法3: ラムダキャプチャ（main.cpp内）**

```cpp
// main.cpp
void hspMain() {
    auto sm = SceneManager();
    
    // 共有データ
    GameData data;
    
    // データを各シーンに渡す
    sm.state(Scene::Game)
      .on_update([&data, &sm]() {
          data.score += 10;
      });
}
```

---

## ベストプラクティス

### ✅ DO: やるべきこと

1. **enum class でステートを定義**
   ```cpp
   enum class Screen { Title, Game, Result };
   ```

2. **GUIは on_enter で作成、on_exit で破棄**
   ```cpp
   .on_enter([]() { button(...); })
   .on_exit([]() { clrobj(); })
   ```

3. **重い処理は on_enter、軽い処理は on_update**
   ```cpp
   .on_enter([]() { load_resources(); })  // 重い
   .on_update([]() { check_input(); })    // 軽い
   ```

4. **ステート間の共有データはラムダキャプチャ**
   ```cpp
   int score = 0;
   sm.state(Screen::Game)
     .on_update([&]() { score++; });  // [&] でキャプチャ
   ```

---

### ❌ DON'T: やってはいけないこと

1. **文字列でステートを指定しない**
   ```cpp
   // ❌ 型安全性がない
   // sm.jump("game");
   
   // ✅ enum class を使う
   sm.jump(Screen::Game);
   ```

2. **on_update で GUI を作り直さない**
   ```cpp
   // ❌ 毎フレーム作り直し
   .on_update([]() { button(...); })
   
   // ✅ on_enter で作成
   .on_enter([]() { button(...); })
   ```

3. **goto を使わない**
   ```cpp
   // ❌ C++ では goto は非推奨
   // goto label;
   
   // ✅ ステートマシンを使う
   sm.jump(Screen::Next);
   ```

---

## デバッグ Tips

### ログ出力を有効化

```cpp
sm.enable_debug_log(true);
```

**出力例:**
```
[StateMachine] Enter state: Title
[StateMachine] Transition: Title -> Game (frame: 120)
[StateMachine] Enter state: Game
[StateMachine] Transition: Game -> Result (frame: 1850)
```

---

### 状態遷移グラフの可視化

```cpp
sm.export_graph("state_graph.dot");
```

**Graphviz で画像化:**
```bash
dot -Tpng state_graph.dot -o graph.png
```

---

## よくあるエラーと対処法

### エラー1: "Invalid state transition"

**原因:** 遷移制約モードで許可されていない遷移を実行した

**対処:**
```cpp
// 遷移を許可する
sm.allow_transition(Screen::Title, Screen::Game);

// または制約を無効化
sm.set_unrestricted_transitions(true);
```

---

### エラー2: コンパイルエラー "no member named 'Gamee'"

**原因:** 存在しないステートを指定した

```cpp
enum class Screen { Title, Game };

sm.jump(Screen::Gamee);  // ❌ コンパイルエラー
```

**対処:** タイポを修正する（IDE補完を活用）

---

### エラー3: ボタンが反応しない

**原因:** on_update で毎フレーム `clrobj()` している

```cpp
// ❌ 悪い例
.on_update([]() {
    clrobj();  // GUIが削除される
    button(...);  // 毎フレーム作り直し
})
```

**対処:** on_enter / on_exit で分離

```cpp
// ✅ 良い例
.on_enter([]() { button(...); })
.on_exit([]() { clrobj(); })
```

---

## まとめ

| 項目 | HSP goto | HSPPP StateMachine |
|------|----------|-------------------|
| 安全性 | コンパイル時チェック | コンパイル時チェック ✅ |
| GUI管理 | 手動 | 自動化（on_enter/exit） |
| 可読性 | goto 多用で複雑化 | 構造化されて明確 |
| デバッグ | 難しい | ログ・グラフ出力 |

HSPPP ステートマシンは、HSPの安全性を継承しつつ、GUIライフタイム管理やデバッグ支援を提供します。

---

## 関連項目

- [StateMachine API リファレンス](../api/statemachine.md) - 全メソッドの詳細説明
- [HSP goto 移行ガイド](hsp-goto-migration.md) - HSPからの具体的な移行例
