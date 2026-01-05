---
layout: default
title: ステートマシンAPI
---

# ステートマシン API リファレンス

HSP の `*label` / `goto` を型安全に実装するステートマシンライブラリ。

## 目次

- [概要](#概要)
- [基本メソッド](#基本メソッド)
- [ライフサイクル](#ライフサイクル)
- [状態遷移](#状態遷移)
- [状態クエリ](#状態クエリ)
- [高度な機能](#高度な機能)

---

## 概要

### StateMachine クラス

```cpp
template<typename StateType>
    requires std::is_enum_v<StateType>
class StateMachine;
```

enum class ベースの型安全なステートマシン。HSP の `*label` / `goto` と同等のコンパイル時チェックを提供します。

**HSPとの対応:**

| HSP | HSPPP |
|-----|-------|
| `*title` | `sm.state(State::Title)` |
| `goto *game` | `sm.jump(State::Game)` |

**基本的な使用例:**

```cpp
enum class Screen { Title, Game, Result };

auto sm = StateMachine<Screen>();

sm.state(Screen::Title)
  .on_enter([]() { button("Start", []() { /* ... */ }); })
  .on_update([&](auto& sm) { if (getkey(' ')) sm.jump(Screen::Game); })
  .on_exit([]() { clrobj(); });

sm.jump(Screen::Title);  // 初期ステート
while (sm.run()) {
    await(16);  // 約60FPS
}
```

---

## 基本メソッド

### state

ステートの定義を開始します。

```cpp
StateBuilder<StateType>& state(StateType state_enum);
```

| パラメータ | 説明 |
|-----------|------|
| `state_enum` | 定義するステート |

**戻り値:** `StateBuilder` オブジェクト（メソッドチェーン用）

**使用例:**

```cpp
sm.state(Screen::Title)
  .on_enter([]() { /* 初期化 */ })
  .on_update([](auto& sm) { /* メインループ */ })
  .on_exit([]() { /* 後処理 */ });
```

---

### run

1フレーム分の更新を実行します。

```cpp
bool run();
```

**戻り値:** 継続する場合 `true`、終了する場合 `false`

現在のステートの `on_update` コールバックを実行し、遷移処理を行います。`quit()` が呼ばれると `false` を返します。

**使用例:**

```cpp
sm.jump(Screen::Title);  // 初期ステート設定

while (sm.run()) {
    await(16);  // 自分で待機を制御
}
```

---

### quit

メインループを終了します。

```cpp
void quit();
```

---

## ライフサイクル

### on_enter

ステート開始時のコールバックを設定します。

```cpp
StateBuilder& on_enter(std::function<void()> callback);
```

GUIオブジェクトの作成、リソースのロードなど、重い初期化処理に使用します。同じステートに再度遷移した場合も再実行されます。

**使用例:**

```cpp
sm.state(Screen::Menu)
  .on_enter([]() {
      button("Start", []() { /* ... */ });
      button("Quit", []() { /* ... */ });
  });
```

---

### on_update

毎フレーム実行されるコールバックを設定します。

```cpp
StateBuilder& on_update(std::function<void(StateMachine&)> callback);
```

入力チェック、画面描画など、軽い処理に使用します。コールバックは `StateMachine&` を受け取ります。

**使用例:**

```cpp
sm.state(Screen::Game)
  .on_update([&](auto& sm) {
      cls();
      mes("Playing...");
      if (getkey(VK_ESCAPE)) sm.jump(Screen::Result);
  });
```

---

### on_exit

ステート終了時のコールバックを設定します。

```cpp
StateBuilder& on_exit(std::function<void()> callback);
```

GUIオブジェクトの削除、リソースの解放など、後処理に使用します。

**使用例:**

```cpp
sm.state(Screen::Title)
  .on_exit([]() {
      clrobj();
  });
```

---

## 状態遷移

### jump

指定したステートに遷移します（HSP の `goto` 相当）。

```cpp
void jump(StateType target_state);
```

| パラメータ | 説明 |
|-----------|------|
| `target_state` | 遷移先のステート |

存在しないステートを指定するとコンパイルエラーになります。遷移は次フレームから適用されます。

---

### defer_jump

状態遷移を予約します。

```cpp
void defer_jump(StateType target_state);
```

`jump()` と同じですが、将来的な拡張用に分離しています。

---

## 状態クエリ

### current_state

現在のステートを取得します。

```cpp
StateType current_state() const;
```

---

### previous_state

前回のステートを取得します。

```cpp
StateType previous_state() const;
```

---

### frame_count

起動からの総フレーム数を取得します（HSP の `cnt` 相当）。

```cpp
int frame_count() const;
```

---

### state_frame_count

現在のステートに滞在しているフレーム数を取得します。

```cpp
int state_frame_count() const;
```

**使用例:**

```cpp
sm.state(Screen::Splash)
  .on_update([&](auto& sm) {
      if (sm.state_frame_count() > 120) {  // 2秒経過
          sm.jump(Screen::Title);
      }
  });
```

---

## 高度な機能

### 遷移制約

#### set_unrestricted_transitions

All-to-All 遷移の有効/無効を設定します。

```cpp
void set_unrestricted_transitions(bool enabled);
```

| パラメータ | 説明 |
|-----------|------|
| `enabled` | `true` で制約なし（デフォルト）、`false` で厳格チェック |

---

#### allow_transition

特定の遷移を許可します（厳格モード時のみ有効）。

```cpp
void allow_transition(StateType from, StateType to);
```

---

#### deny_transition

特定の遷移を禁止します。

```cpp
void deny_transition(StateType from, StateType to);
```

**使用例:**

```cpp
sm.set_unrestricted_transitions(false);
sm.allow_transition(Screen::Title, Screen::Game);
sm.allow_transition(Screen::Game, Screen::Result);
```

---

### 履歴機能

#### enable_history

履歴記録を有効化します。

```cpp
void enable_history(int max_size = 10);
```

| パラメータ | 説明 |
|-----------|------|
| `max_size` | 最大履歴サイズ |

---

#### back

前のステートに戻ります。

```cpp
void back();
```

**使用例:**

```cpp
sm.enable_history(5);

sm.state(Screen::Option)
  .on_update([&](auto& sm) {
      if (getkey(VK_ESCAPE)) sm.back();
  });
```

---

### タイマー機能

#### set_timer

指定ミリ秒後に自動遷移します。

```cpp
void set_timer(StateType target_state, int milliseconds);
```

| パラメータ | 説明 |
|-----------|------|
| `target_state` | 遷移先 |
| `milliseconds` | ミリ秒 |

**使用例:**

```cpp
sm.state(Screen::Splash)
  .on_enter([&]() {
      sm.set_timer(Screen::Title, 2000);  // 2秒後に自動遷移
  });
```

---

#### cancel_timer

タイマーをキャンセルします。

```cpp
void cancel_timer();
```

---

### デバッグ支援

#### enable_debug_log

状態遷移のログ出力を有効化します。

```cpp
void enable_debug_log(bool enabled = true);
```

**出力例:**

```
[StateMachine] Enter state: Title
[StateMachine] Transition: Title -> Game (frame: 120)
```

---

#### export_graph

状態遷移グラフを Graphviz dot 形式で出力します。

```cpp
void export_graph(const std::string& filename);
```

**使用例:**

```cpp
sm.export_graph("state_graph.dot");
// dot -Tpng state_graph.dot -o graph.png
```

---

## 参照

- [ステートパターンガイド](../guides/state-pattern.md)
- [HSP goto 移行ガイド](../guides/hsp-goto-migration.md)
