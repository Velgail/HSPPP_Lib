---
layout: default
title: HSP goto 移行ガイド
---

# HSP goto から HSPPP StateMachine への移行ガイド

HSP の `*label` / `goto` パターンを HSPPP ステートマシンに移行する方法を解説します。

---

## 基本的な対応表

| HSP | HSPPP |
|-----|-------|
| `*label` | `sm.state(State::Label)` |
| `goto *label` | `sm.jump(State::Label)` |
| `await 16` | ユーザーがループ内で `await()` を記述 |
| `stop` | `sm.quit()` |

---

## パターン1: シンプルなタイトル→ゲームループ

### HSP

```hsp
; タイトル画面
*title
    redraw 0
    color 255, 255, 255 : cls
    pos 200, 200
    mes "Press SPACE to Start"
    redraw 1
    
    stick key, 15
    if key & 32 : goto *game  ; Space
    
    await 16
    goto *title

; ゲーム画面
*game
    redraw 0
    color 0, 0, 0 : cls
    mes "Playing..."
    redraw 1
    
    stick key, 15
    if key & 128 : goto *result  ; ESC
    
    await 16
    goto *game

; リザルト画面
*result
    redraw 0
    color 255, 255, 255 : cls
    mes "Game Over"
    redraw 1
    
    stick key, 15
    if key & 32 : goto *game  ; Space: Retry
    
    await 16
    goto *result
```

### HSPPP

```cpp
import hsppp;
using namespace hsppp;

// ステップ1: ラベルを enum class に変換
enum class Screen {
    Title,   // *title
    Game,    // *game
    Result   // *result
};

void hspMain() {
    auto sm = StateMachine<Screen>();
    
    // ステップ2: 各ラベルを state() に変換
    
    // *title
    sm.state(Screen::Title)
      .on_update([&](auto& sm) {
          redraw(0);
          color(0, 0, 0);
          boxf();
          pos(200, 200);
          mes("Press SPACE to Start");
          redraw(1);
          
          // goto *game
          if (getkey(' ')) {
              sm.jump(Screen::Game);
          }
      });
    
    // *game
    sm.state(Screen::Game)
      .on_update([&](auto& sm) {
          redraw(0);
          color(0, 0, 0);
          boxf();
          mes("Playing...");
          redraw(1);
          
          // goto *result
          if (getkey(VK_ESCAPE)) {
              sm.jump(Screen::Result);
          }
      });
    
    // *result
    sm.state(Screen::Result)
      .on_update([&](auto& sm) {
          redraw(0);
          color(0, 0, 0);
          boxf();
          mes("Game Over");
          redraw(1);
          
          // goto *game
          if (getkey(' ')) {
              sm.jump(Screen::Game);
          }
          await(16);
      });
    
    // ステップ3: メインループ
    sm.jump(Screen::Title);
    sm.run();
}
```

---

## パターン2: ボタンを使ったメニュー

### HSP

```hsp
*title
    cls
    mes "Main Menu"
    
    button "Start Game", *game
    button "Quit", *quit
    
    stop

*game
    cls
    mes "Playing..."
    button "Back to Title", *title
    stop

*quit
    end
```

### HSPPP

```cpp
enum class Screen {
    Title,
    Game
};

void hspMain() {
    auto sm = StateMachine<Screen>();
    
    // ✅ ボタンは on_enter で作成、on_exit で削除
    sm.state(Screen::Title)
      .on_enter([&]() {
          color(0, 0, 0);
          boxf();
          mes("Main Menu");
          
          button("Start Game", [&]() {
              sm.jump(Screen::Game);
              return 0;
          });
          button("Quit", []() {
              end(0);
              return 0;
          });
      })
      .on_exit([]() {
          clrobj();  // ボタンをクリア
      });
    
    sm.state(Screen::Game)
      .on_enter([&]() {
          color(0, 0, 0);
          boxf();
          mes("Playing...");
          
          button("Back to Title", [&]() {
              sm.jump(Screen::Title);
              return 0;
          });
      })
      .on_exit([]() {
          clrobj();
      });
    
    sm.jump(Screen::Title);
    sm.run();
}
```

---

## パターン3: グローバル変数の共有

### HSP

```hsp
; グローバル変数
score = 0
player_x = 320
player_y = 240

*title
    cls
    mes "High Score: " + score
    stick key
    if key & 32 : goto *game
    await 16
    goto *title

*game
    score = 0
    
*game_loop
    cls
    mes "Score: " + score
    circle player_x, player_y, player_x+10, player_y+10
    
    stick key
    if key & 1 : player_x -= 5
    if key & 4 : player_x += 5
    
    score += 10
    
    if key & 128 : goto *result
    
    await 16
    goto *game_loop

*result
    cls
    mes "Final Score: " + score
    stick key
    if key & 32 : goto *game
    await 16
    goto *result
```

### HSPPP

```cpp
enum class Screen {
    Title,
    Game,
    Result
};

void hspMain() {
    auto sm = StateMachine<Screen>();
    
    // グローバル変数（hspMain のローカル変数として定義）
    int score = 0;
    int player_x = 320;
    int player_y = 240;
    
    sm.state(Screen::Title)
      .on_update([&](auto& sm) {  // [&] でキャプチャ
          color(0, 0, 0);
          boxf();
          mes("High Score: " + std::to_string(score));
          
          if (getkey(' ')) {
              sm.jump(Screen::Game);
          }
      });
    
    sm.state(Screen::Game)
      .on_enter([&]() {
          score = 0;  // 初期化
      })
      .on_update([&](auto& sm) {
          color(0, 0, 0);
          boxf();
          mes("Score: " + std::to_string(score));
          circle(player_x, player_y, player_x+10, player_y+10);
          
          if (getkey(VK_LEFT)) player_x -= 5;
          if (getkey(VK_RIGHT)) player_x += 5;
          
          score += 10;
          
          if (getkey(VK_ESCAPE)) {
              sm.jump(Screen::Result);
          }
      });
    
    sm.state(Screen::Result)
      .on_update([&](auto& sm) {
          color(0, 0, 0);
          boxf();
          mes("Final Score: " + std::to_string(score));
          
          if (getkey(' ')) {
              sm.jump(Screen::Game);
          }
          await(16);
      });
    
    sm.jump(Screen::Title);
    sm.run();
}
```

---

## パターン4: 複雑な遷移フロー

### HSP

```hsp
*splash
    cls
    mes "Loading..."
    wait 100
    goto *title

*title
    cls
    mes "Title"
    button "Character Select", *char_select
    button "Load Game", *game
    stop

*char_select
    cls
    mes "Select Character"
    button "Character 1", *game
    button "Character 2", *game
    button "Back", *title
    stop

*game
    cls
    mes "Playing"
    stick key
    if key & 128 : goto *result
    await 16
    goto *game

*result
    cls
    mes "Result"
    button "Retry", *char_select
    button "Title", *title
    stop
```

### HSPPP

```cpp
enum class GameFlow {
    Splash,
    Title,
    CharSelect,
    Game,
    Result
};

void hspMain() {
    auto sm = StateMachine<GameFlow>();
    
    // スプラッシュ
    sm.state(GameFlow::Splash)
      .on_enter([&]() {
          color(0, 0, 0);
          boxf();
          mes("Loading...");
          sm.set_timer(GameFlow::Title, 1666);  // 約1.7秒後に自動遷移
      });
    
    // タイトル
    sm.state(GameFlow::Title)
      .on_enter([&]() {
          color(0, 0, 0);
          boxf();
          mes("Title");
          button("Character Select", [&]() {
              sm.jump(GameFlow::CharSelect);
              return 0;
          });
          button("Load Game", [&]() {
              sm.jump(GameFlow::Game);
              return 0;
          });
      })
      .on_exit([]() {
          clrobj();
      });
    
    // キャラ選択
    sm.state(GameFlow::CharSelect)
      .on_enter([&]() {
          color(0, 0, 0);
          boxf();
          mes("Select Character");
          button("Character 1", [&]() {
              sm.jump(GameFlow::Game);
              return 0;
          });
          button("Character 2", [&]() {
              sm.jump(GameFlow::Game);
              return 0;
          });
          button("Back", [&]() {
              sm.jump(GameFlow::Title);
              return 0;
          });
      })
      .on_exit([]() {
          clrobj();
      });
    
    // ゲーム
    sm.state(GameFlow::Game)
      .on_update([&](auto& sm) {
          cls();
          mes("Playing");
          
          if (getkey(VK_ESCAPE)) {
              sm.jump(GameFlow::Result);
          }
      });
    
    // リザルト
    sm.state(GameFlow::Result)
      .on_enter([&]() {
          cls();
          mes("Result");
          button("Retry", [&]() {
              sm.jump(GameFlow::CharSelect);
              return 0;
          });
          button("Title", [&]() {
              sm.jump(GameFlow::Title);
              return 0;
          });
      })
      .on_exit([]() {
          clrobj();
      });
    
    sm.jump(GameFlow::Splash);
    sm.run();
}
```

---

## 移行チェックリスト

### ステップ1: ラベルを列挙

- [ ] すべての `*label` をリストアップ
- [ ] `enum class` として定義

### ステップ2: goto を変換

- [ ] `goto *label` → `sm.jump(State::Label)` に置き換え
- [ ] `await` / `stop` → ステートマシンに組み込み

### ステップ3: GUIオブジェクトを整理

- [ ] ボタン作成 → `on_enter` に移動
- [ ] ボタン削除 → `on_exit` に移動

### ステップ4: グローバル変数をキャプチャ

- [ ] HSPのグローバル変数 → hspMainのローカル変数 + `[&]` キャプチャ

---

## よくある質問

### Q: await 16 はどこに書けば？

**A:** `on_update` 内で自分で書きます。

```cpp
sm.state(Screen::Title)
  .on_update([](auto& sm) {
      // 処理
      await(16);  // 60 FPS = 約16ms/frame
  });

sm.jump(Screen::Title);
sm.run();
```

### Q: goto より面倒では？

**A:** 最初は手間ですが、以下のメリットがあります：

- ✅ タイポエラーがコンパイル時に検出
- ✅ IDE補完が効く
- ✅ GUIオブジェクトの自動管理
- ✅ デバッグしやすい

---

## まとめ

| 項目 | HSP | HSPPP |
|------|-----|-------|
| ラベル定義 | `*title` | `enum class Screen { Title }` |
| ジャンプ | `goto *title` | `sm.jump(Screen::Title)` |
| ループ | `await 16 : goto *title` | `sm.run()` + `on_update` 内で `await(16)` |
| タイポチェック | コンパイル時 ✅ | コンパイル時 ✅ |
| GUI管理 | 手動 | 自動（on_enter/exit） |

HSPPP ステートマシンは、HSPの安全性を継承しつつ、さらに強力な機能を提供します。

---

## 関連項目

- [StateMachine API リファレンス](../api/statemachine.md) - 全メソッドの詳細説明
- [ステートパターンガイド](state-pattern.md) - 設計パターンとベストプラクティス
