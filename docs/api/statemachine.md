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
sm.run();  // quit()まで実行
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

メインループ（dispatcher）を実行します。

```cpp
void run();
```

**戻り値:** なし（`quit()` が呼ばれるまでブロック）

内部で `while (running_) { tick(); }` を回し、各ステートの `on_update` コールバックを呼び出します。

**重要 — `run()` は dispatch only:**

- `run()` 自身は `await()` / `Sleep()` などの待機を **一切呼びません**（design-TICKET-008 §7.1）。
- フレーム制御（`await(16)` 等）は **必ず `on_update` 内でユーザーが書く** こと。書かなければ busy loop になります（ライブラリは警告も出しません）。
- これは HSP の「`stop` で待ち、`await` で間を空ける」感覚を温存するための明示的な責務分担です。

```cpp
sm.state(Screen::Game)
  .on_update([](auto& sm) {
      // ... 描画 / 入力 ...
      await(16);   // ← ユーザーが書く（必須）
  });

sm.jump(Screen::Title);
sm.run();           // dispatch only
```

---

### start

初期ステートを設定してメインループを実行します。

```cpp
void start(StateType initial_state);
```

| パラメータ | 説明 |
|-----------|------|
| `initial_state` | 開始するステート |

`jump()` と `run()` を一度に行う便利メソッドです。

**使用例:**

```cpp
// jump + run の代わり
sm.start(Screen::Title);

// 上記は以下と同じ
// sm.jump(Screen::Title);
// sm.run();
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
      color(0, 0, 0);
      boxf();
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
      color(0, 0, 0);
      boxf();
      mes("Playing...");
      if (getkey(VK_ESCAPE)) sm.jump(Screen::Result);
      await(16);  // フレーム制御
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

> ⚠️ **Deprecated**（design-TICKET-008 §7.1）— `jump()` と同義です。新規コードでは `jump()` を使用してください。

```cpp
[[deprecated("use jump")]]
void defer_jump(StateType target_state);
```

歴史的経緯で残されている alias です。dispatcher は `on_update` から return された直後に予約遷移を処理するため、`jump()` と `defer_jump()` の意味論的差はありません。

---

### on_update の契約（重要）

`on_update` は **「名前付き repeat-loop の 1 iteration」** として定義されます（design-TICKET-008 §7.2）。

- `on_update` から `return` すると、dispatcher は **遷移予約があれば exit→enter→次ステートの on_update**、なければ **同じステートの on_update を再度呼び出します**（= repeat の継続）。
- 利用者は `while (!sm.is_transitioning()) { ... }` を**書かなくてもよい**（return すれば同じ効果）。書いてもよい（ステート内で完結する待機を組みたい場合）。
- 待機（`await` / `stop` / `vwait`）は **on_update 内でユーザーが書く**。dispatcher は介入しない。
- `jump()` を呼んだ場合、同じ on_update 呼出の続きは実行され、return 後に遷移が処理されます。

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

> ⚠️ **Deprecated** — `state_elapsed_ms()` を使用してください（design-TICKET-002 §7.1 / TICKET-008 §18 Q-3、ms 統一方針）。

```cpp
[[deprecated("use state_elapsed_ms()")]]
int state_frame_count() const;
```

現在のステートに滞在しているフレーム数を取得します。

---

### state_elapsed_ms

現在のステートに滞在している経過ミリ秒数を取得します（`steady_clock` 由来 / dispatcher 周回非依存）。

```cpp
int state_elapsed_ms() const;
```

**使用例:**

```cpp
sm.state(Screen::Splash)
  .on_update([&](auto& sm) {
      color(0, 0, 0);
      boxf();
      mes("Loading...");
      if (sm.state_elapsed_ms() > 2000) {  // 2 秒経過
          sm.jump(Screen::Title);
      }
      await(16);
  });
```

---

## 高度な機能

### ローカルデータ: state<T>()

ステート固有のデータを `on_enter`, `on_update`, `on_exit` で共有できます。

```cpp
template<typename LocalDataType>
StateBuilderWithLocal<StateType, LocalDataType>& state(StateType state_enum);
```

| パラメータ | 説明 |
|-----------|------|
| `state_enum` | 定義するステート |

**戻り値:** `StateBuilderWithLocal` オブジェクト（メソッドチェーン用）

ローカルデータは自動的に初期化・破棄されます。

**使用例:**

```cpp
struct GameSceneData {
    int score = 0;
    int lives = 3;
};

sm.state<GameSceneData>(Scene::Game)
  .on_enter([](auto& sm, GameSceneData& data) {
      // ✅ ステート開始時に初期化
      data.score = 0;
      data.lives = 3;
  })
  .on_update([](auto& sm, GameSceneData& data) {
      while (!sm.is_transitioning()) {
          data.score += 10;
          
          if (stick(256)) {  // ESC
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
      // ✅ ステート終了時に後処理
      auto& profile = services().data<PlayerProfile>();
      if (data.score > profile.high_score) {
          profile.high_score = data.score;
      }
  });
```

**注意点:**
- ローカルデータはステートが終了すると破棄される
- シーンをまたいで保持したいデータは `Repository<T>` を使用

---

### is_transitioning()

`jump()` が呼ばれて状態遷移中かどうかを確認します。

```cpp
bool is_transitioning() const;
```

**戻り値:** 遷移中なら `true`、それ以外は `false`

`on_update` 内のループで使用します。`jump()` が呼ばれると `true` になるため、ループを抜けてステート遷移が実行されます。

**使用例:**

```cpp
sm.state(Scene::Game)
  .on_update([](auto& sm) {
      while (!sm.is_transitioning()) {  // ✅ 遷移が始まるまでループ
          // ゲームロジック
          
          if (stick(256)) {  // ESC
              sm.jump(Scene::Title);  // ← ここで is_transitioning() == true
          }
          
          await(16);
      }
  });
```

**is_running() との違い:**
- `is_running()`: ステートマシン全体が動いているか（メインループ用）
- `is_transitioning()`: ステート遷移が始まったか（`on_update` 内のループ用）

---

### tick / step

`on_update` を 1 回だけ実行し、遷移予約があれば処理します（ブロックしません）。

```cpp
void tick();   // 後方互換 alias
void step();   // 推奨名（HSPPP 流儀の小文字、サブ SM 用）
```

**戻り値:** なし

`run()` と違い、`tick()` / `step()` は **1 iteration だけ実行してすぐ戻ります**。`step()` が推奨名で、`tick()` は等価な後方互換 alias です（design-TICKET-008 §7.1）。

**サブステートマシン駆動の規約変更（重要）:**

旧 API にあった `attach_child()` / `detach_child()` は **撤去されました**（design-TICKET-008 §11.1）。サブ SM が必要な場合は、親 SM の `on_update` 内で **明示的に `child.step()` を呼ぶ** のが正規パターンです。

```cpp
enum class BattlePhase { Start, PlayerTurn, EnemyTurn, End };

sm.state(Scene::Battle)
  .on_update([&](auto& sm) {
      // ✅ サブ SM を作成して明示的に step() する
      static StateMachine<BattlePhase> battle;
      static bool initialized = false;
      if (!initialized) {
          battle.state(BattlePhase::Start).on_update([](auto& b) {
              if (b.state_elapsed_ms() > 1000) b.jump(BattlePhase::PlayerTurn);
          });
          // ... 他フェーズ定義 ...
          battle.start(BattlePhase::Start);
          initialized = true;
      }

      battle.step();                      // 明示駆動
      if (!battle.is_running()) sm.jump(Scene::Result);
      await(16);                          // 待機はユーザーが書く
  });
```

**run() との違い:**
- `run()`: `quit()` が呼ばれるまでループ（メインループ用）
- `step()` / `tick()`: 1 iteration だけ実行してすぐ戻る（サブ SM 用 / 任意の手動ティック）

---

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
sm.jump(Screen::Title);
sm.run();
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
      mes("Options");
      if (getkey(VK_ESCAPE)) sm.back();
      await(16);
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
      color(0, 0, 0);
      boxf();
      mes("Welcome!");
      sm.set_timer(Screen::Title, 2000);  // 2秒後に自動遷移
  })
  .on_update([](auto& sm) {
      await(16);
  });
```

---

#### cancel_timer

タイマーをキャンセルします（明示 escape hatch）。

```cpp
void cancel_timer();
```

`pause_timer()` で paused 状態のタイマーも **明示 `cancel_timer()` は上書き破棄** します（design-TICKET-010 §5.3 / design-TICKET-008 §18 Q4「二層契約」）。

---

#### pause_timer / resume_timer

タイマーの一時停止／再開。

```cpp
void pause_timer();
void resume_timer();
```

**意味論（design-TICKET-010 §5.1 で確立）:**

- `pause_timer()` は単に「カウントを止める」だけでなく、**「次の状態遷移を跨いでタイマー状態を保持する明示的意思表示」** を表します。
- これに伴い、`perform_transition()` 内の自動 `cancel_timer()` は **paused 中のタイマーに限り skip** されます（一方、明示の `cancel_timer()` は意思表示を上書きして破棄）。
- これにより `Pause → 別ステート → 元のステート` に戻った後 `resume_timer()` を呼ぶと、保持されていた残時間からカウントが再開できます。

**典型ユースケース — Pause 画面:**

```cpp
sm.state(GameScreen::Game)
  .on_enter([&]() {
      if (sm.previous_state() == GameScreen::Pause) {
          // Pause から復帰: 残時間を再開
          sm.resume_timer();
      } else {
          // 通常開始: 30 秒の GameOver タイマーを仕掛ける
          sm.set_timer(GameScreen::GameOver, 30000);
      }
  })
  .on_update([&](auto& sm) {
      if (getkey(VK_ESCAPE)) {
          sm.pause_timer();        // ← 「Pause を跨いで保持する」意思表示
          sm.jump(GameScreen::Pause);
      }
      // ...
      await(16);
  });
```

> ⚠️ 旧実装（〜TICKET-009）では `perform_transition` が無条件で `cancel_timer()` を呼んでいたため、`Pause → Game` 復帰後にタイマーが永久失効していました（TR-6）。TICKET-010 で API 内部契約を整え、サンプル側を `Game::on_enter` で `resume_timer()` を呼ぶ形に整理しています。詳細は `design-TICKET-010.md` §5 / §7。

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

- [Data Sharing Guide](../guides/data-sharing.md) - データ共有のベストプラクティス
- [Repository API](repository.md) - 永続データ管理
- [State Vars / Save Data ガイド](state-vars-savedata.md) - StateScope / state_vars / Serializable / SaveWriter
- [ステートパターンガイド](../guides/state-pattern.md)
- [HSP goto 移行ガイド](../guides/hsp-goto-migration.md)
