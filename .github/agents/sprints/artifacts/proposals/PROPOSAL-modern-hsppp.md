# モダン HspppLib 設計提言



## はじめに

本文書の目的は一言で表せる：

> **「HSP の人が気づかないうちにモダンなアプリを作れている」状態を実現する**

HSP ユーザーは `*label` と `goto` でシーン遷移を書いてきた。HspppLib はその感覚を `StateGraph<Scene>` として型安全に提供している。ここからさらに進化させるとき、「C++ らしく書き直す」のではなく「HSP の書き心地を保ちながら、裏側でモダンな設計が動いている」構造を目指す。

本提言は **調査で確認したコード上の事実のみ**を根拠とし、5章構成で方向性を示す。

---

## 第1章: StateMachine の拡張方向

### 1.1 現状の正直な評価

調査で確認された事実：

| 機能 | 現状 | コード箇所 |
|------|------|-----------|
| `defer_jump()` | **削除済み**（`jump()` と同一のまま削除） | — |
| 複数タイマー | `timer_target_` が `std::optional<StateType>`（1つのみ） | 内部データ構造 |
| ガード条件付き遷移 | `allow_transition()` は無条件許可/禁止のみ | `allowed_transitions_` |
| 並列ステート | `current_state_` が `std::optional<StateType>` 1つのみ | 内部データ構造 |
| back() の実体 | `std::deque<StateType>` 履歴（真のスタックではない） | `history_` メンバ |
| LocalData リセット | ステート再入時に**リセットされない**（手動必要） | `local_data_storage_` |

### 1.2 `defer_jump()` の修正方針

> ⚠️ **本セクションは非適用**: `defer_jump()` は削除済みです。
> `[[deprecated]]` 属性付きであったが意味論的差がなく実用価値がなかったため、`jump()` に一本化されました。
> 以下は削除前の提言内容であり、将来の実装計画として参照する必要はありません。

~~**結論: 「現フレームの on_update 完了後に遷移する」セマンティクスを実装する**~~

~~現在 `defer_jump()` は `jump()` と同一（`next_state_ = target_state` のみ）。この関数が存在する意義は、`on_enter` 内で遷移予約をしたときに「今の on_enter が完了してから遷移する」ことを明示するためであった。現状の実装ではその保証がない。~~

**修正設計**：

```cpp
// 内部データ構造に追加
std::optional<StateType> deferred_state_;   // 追加
// next_state_ は「即時遷移予約」として残す

// jump() はそのまま
void jump(StateType target_state) {
    next_state_ = target_state;  // 現行維持
}

// defer_jump() は deferred_state_ に記録
void defer_jump(StateType target_state) {
    deferred_state_ = target_state;  // on_update 完了後に処理
}

// run() ループ内の遷移チェック順序
// 1. next_state_ を先にチェック（即時）
// 2. next_state_ がなければ deferred_state_ をチェック（遅延）
while (running_) {
    update_timer();
    if (next_state_.has_value()) { /* 即時遷移 */ }
    else if (deferred_state_.has_value()) {
        // on_update 完了後に適用
        next_state_ = deferred_state_;
        deferred_state_.reset();
    }
    // ...
    state_data.on_update(*this);
}
```

**使用場面の違い**：
- `jump()`: ゲームループ中の通常遷移（スペースキー押下など）
- `defer_jump()`: `on_enter` 内での条件分岐（「入ったら即別ステートに移りたい」場合）

### 1.3 ガード条件付き遷移の設計案

**結論: `allow_transition_if(from, to, pred)` を追加する**

現行の `allow_transition(from, to)` は静的な許可/禁止のみ。ゲームでよくある「HPが0以下なら GameOver への遷移を強制する」「アイテムを持っていないと特定シーンに行けない」はこれでは表現できない。

```cpp
// 追加するAPI
sm.allow_transition_if(Scene::Game, Scene::Boss, []() {
    return services().data<PlayerData>().level >= 5;  // レベル5以上のみ
});

// 内部実装
using GuardFn = std::function<bool()>;
std::map<std::pair<StateType, StateType>, GuardFn> guarded_transitions_;

bool check_transition_allowed(StateType from, StateType to) {
    // 既存ロジック（allow/deny）に加え
    auto key = std::make_pair(from, to);
    if (auto it = guarded_transitions_.find(key); it != guarded_transitions_.end()) {
        return it->second();  // ガード関数を評価
    }
    // 既存の allow/deny チェックへ fallthrough
}
```

**既存 API との後方互換**: `allow_transition()` / `deny_transition()` はそのまま動作する。`allow_transition_if()` は**追加のみ**であり既存コードは壊れない。

### 1.4 複数タイマーの同時管理

**結論: `timer_target_` を `std::vector<TimerEntry>` に変更する**

現状 `timer_target_` は `std::optional<StateType>` 1つのみ。`set_timer()` を2回呼ぶと前のタイマーが上書きされる（制約確認済み）。

```cpp
// 内部構造の変更
struct TimerEntry {
    StateType target;
    std::chrono::steady_clock::time_point fire_at;
    std::string tag;  // cancel_timer("tag") でキャンセル可能
};
std::vector<TimerEntry> timers_;  // timer_target_ / timer_start_ を廃止

// 新API
sm.set_timer(Scene::GameOver, 3000);           // 既存互換
sm.set_timer(Scene::Warning, 1000, "warning"); // タグ付き
sm.cancel_timer();                              // 全キャンセル（既存互換）
sm.cancel_timer("warning");                     // タグ指定キャンセル（新規）

// update_timer() の変更
void update_timer() {
    auto now = std::chrono::steady_clock::now();
    for (auto it = timers_.begin(); it != timers_.end(); ) {
        if (now >= it->fire_at) {
            jump(it->target);
            it = timers_.erase(it);
        } else { ++it; }
    }
}
```

**後方互換**: 既存の `set_timer(state, ms)` / `cancel_timer()` は動作維持。

### 1.5 並列ステート・HSM について

**結論: `tick()` での明示的合成パターンを「公式推奨パターン」として正式化する。組み込みサポートは不要**

確認した通り、`current_state_` は `std::optional<StateType>` 1つ。並列ステートを組み込むには内部設計の大幅変更が必要となる。

しかし `NewStateSampleMain.cpp` の Battle ステートを見ると、`tick()` パターンで十分な表現力があることが確認できる：

```cpp
// 現状でも書ける。これを「公式推奨パターン」として文書化する
sm.state(Scene::Battle)
  .on_update([](auto& sm) {
      StateGraph<BattlePhase> sub;
      // sub.state(...) を定義
      sub.jump(BattlePhase::Start);
      while (!sm.is_transitioning()) {
          sub.tick();
          await(16);
          if (!sub.is_running()) sm.jump(Scene::Result);
      }
  });
```

このパターンの利点：
- HSP ユーザーに「サブルーチンを呼ぶ」感覚で理解できる
- 親 SM とサブ SM の独立性が高い（子が親の型を知らなくていい）
- 実装の複雑度が低い（`current_state_` 1つの設計を維持）

**採用しない理由（Orthogonal States の組み込み）**：現行の `run()` / `tick()` パターンで HspppLib の主要ユースケースは全て実現できる。組み込みによる複雑化は「HSP らしさ」を損なう。

### 1.6 push/pop スタック型遷移と `back()` の整理

**結論: `back()` の deque 履歴を「真のスタック的に使う方法」を明文化する**

現状 `history_` は `std::deque<StateType>` であり、`back()` で前ステートに戻れる。これは「スタック型遷移」の基本動作を既に提供している。

問題は「push/pop の概念が露出していない」こと。以下の命名を追加し、スタック的操作を明示する：

```cpp
sm.push_and_jump(Scene::Pause);   // jump() + 遷移前ステートをスタックに積む
sm.pop_state();                    // back() の別名（スタック的意味を明確化）

// 内部は既存の history_ / back() を使うだけ
void push_and_jump(StateType target) {
    // history_ への push は既存の perform_transition() 内で行われているため
    // jump() と同一でよい（意味を明確にするための別名）
    jump(target);
}
void pop_state() { back(); }  // エイリアス
```

`enable_history()` との組み合わせでポーズメニューなど「前の画面に戻る」パターンを明確に表現できる。

### 1.7 追加すべき機能・追加すべきでない機能の整理

**追加すべき（優先度順）**:

| 機能 | 優先度 | 理由 |
|------|--------|------|
| ~~`defer_jump()` の真の実装~~ | ~~高~~ | **削除済み** |
| 複数タイマー | 高 | ゲームでは「BGM フェード2秒 + シーン切替3秒」等が当然必要 |
| `allow_transition_if()` ガード条件 | 中 | 宣言的な遷移ルール管理の完成 |
| LocalData 自動リセットオプション | 中 | ステート再入時の意図しないデータ残留バグの回避 |
| `push_and_jump()` / `pop_state()` エイリアス | 低 | 意図明示のためのシンタックスシュガー |

**追加すべきでない機能**:

| 機能 | 理由 |
|------|------|
| 並列ステート（Orthogonal States）の組み込み | `tick()` で十分。複雑化が HSP らしさを壊す |
| HSM の親子継承機構 | サブ SM の明示的合成で代替可能。型システムが複雑化する |
| クロス SM 通信 API | `Repository` 経由の間接通信が疎結合で正解 |
| 遷移アニメーションフック | `on_exit` / `on_enter` の組み合わせと `hsppp_easing.inl` で実現可能 |

---

## 第2章: 非同期/ゲームループモデル

### 2.1 `vwait()` の SM 非連携問題

**結論: `vwait()` に `should_transition()` チェックを追加する（小規模変更）**

確認した事実：

```
await(ms):  SM コンテキスト取得 → should_transition() が true なら即リターン ✅
stop():     SM コンテキスト取得 → should_transition() が true なら即リターン ✅
vwait():    presentVsync() のみ → should_transition() チェック**なし** ❌
```

`vwait()` のみのゲームループで `jump()` を呼ぶと、遷移に最大1フレーム分の遅延が生じる。実害は軽微だが、「`await()` と `vwait()` で挙動が違う」という非一貫性はユーザーの混乱を招く。

**修正案**（`hsppp_screen.inl` の `Screen::vwait()` 内）：

```cpp
int Screen::vwait() {
    // 既存: presentVsync() 呼び出し
    auto elapsed = presentVsync();
    
    // 追加: SM コンテキストチェック（await() と同じパターン）
    if (auto* sm = detail::get_current_statemachine()) {
        if (sm->should_transition()) {
            return static_cast<int>(elapsed);  // 遷移予約があれば早期リターン
        }
    }
    
    return static_cast<int>(elapsed);
}
```

**影響**: 既存の `vwait()` 呼び出しコードは変更不要。`StateSampleMain.cpp` の `vwait()` 利用パターンも壊れない。

### 2.2 `await()` / `vwait()` / `stop()` の役割再整理

**結論: 3つの関数の役割を明確に定義し、ドキュメントに記載する**

| 関数 | 役割 | 使用場面 |
|------|------|---------|
| `await(ms)` | 指定時間待機（高精度タイマー、SM連携） | ゲームループ内の FPS 制御（`await(16)` で60fps相当） |
| `vwait()` | VSync 同期待機（GPU が引っ張る） | VSync に合わせたゲームループ（ティアリング防止） |
| `stop()` | イベント到着まで無限待機（CPU 0%） | GUI 画面・ダイアログ型画面での入力待ち |

命名の混乱は「HSP の `wait` 命令に引っ張られた」歴史的経緯による。**リネームは後方互換破壊になるため行わない**。代わりにコメントとドキュメントで意図を明確化する。

**追加検討: `await_until(predicate)`**：

```cpp
// ユーザーが書ける「条件付き待機」
await_until([]() { return getkey(' '); });  // スペースキーまで待機

// 内部実装
void await_until(std::function<bool()> pred, int timeout_ms = -1) {
    auto start = std::chrono::steady_clock::now();
    while (!pred()) {
        if (auto* sm = detail::get_current_statemachine()) {
            if (sm->should_transition()) return;
        }
        PeekMessage(/* ... */);
        if (timeout_ms > 0) {
            auto elapsed = /* ... */;
            if (elapsed >= timeout_ms) return;
        }
        Sleep(1);
    }
}
```

これにより `while (!getkey(' ')) { await(16); }` というボイラープレートを1行で書ける。

### 2.3 `co_await` / `std::generator` でのコルーチン化の現実性評価

**結論: `std::generator` は C++23 MSVC で利用可能だが、HspppLib の文脈での採用は現時点では見送る**

**事実の確認**：
- MSVC は Visual Studio 2022 17.9 以降で `std::generator` を実装済み（C++23 機能）
- `std::generator` は `<generator>` ヘッダで使用可能
- HspppLib は既に C++23 named modules を使用（技術スタック確認済み）

**コルーチン化の可能性**：

```cpp
// 現在の書き方（ボイラープレートあり）
sm.state(Scene::Game)
  .on_update([](auto& sm) {
      while (!sm.is_transitioning()) {  // ← このボイラープレートを排除したい
          // フレーム処理
          await(16);
      }
  });

// コルーチン化した場合の理想（仮想コード）
sm.state(Scene::Game)
  .on_update_coro([](auto& sm) -> std::generator<FrameResult> {
      while (true) {
          // フレーム処理
          co_yield FrameResult::Continue;  // await(16) 相当
      }
  });
```

**採用しない理由**：

1. **`await()` との統合が複雑**: `co_yield` から戻るタイミングと `await()` の VSync/SM連携を統合するには `std::generator` の実行モデルを変更する必要がある
2. **ユーザーの習熟コストが高い**: HSP ユーザーに `co_yield` を説明するコストは「モダンな設計を気づかないうちに使っている」という目標に反する
3. **既存 `while(!is_transitioning())` パターンの習得コストは低い**: サンプルコードを見ると、このパターンは数行であり、説明コストが低い

**代替案（推奨）**: `while(!sm.is_transitioning())` は HSP の `repeat ... loop` に近い発想であり、HSP ユーザーに説明しやすい。`await_until()` の追加（§2.2）でボイラープレートをさらに削減できる。

**将来的な検討項目**: もし将来「tick() + コルーチン」の組み合わせで SM を駆動するユースケースが増えた場合、`on_update` を `std::generator<FrameResult>` を返す関数として定義できる `StateBuilderWithCoroutine` の追加は価値がある。ただし現スプリントの実装対象ではない。

### 2.4 固定タイムステップの導入検討

**結論: 今は不要。ゲームの性質に応じてユーザーが `await()` で制御する現行方式を推奨**

物理演算を含むゲームでは「描画は可変、物理は固定60Hz」の分離が必要になる。現状の HspppLib は VSync/`await()` 依存であり固定タイムステップ機能はない。

しかし本提言のターゲットは「Direct2D + Windows GUI/Game」であり、物理演算エンジンの統合は現時点のスコープ外。ユーザーが必要なら：

```cpp
// ユーザーが自分で書ける固定タイムステップ
constexpr int FIXED_MS = 16;
auto last_time = std::chrono::steady_clock::now();

while (!sm.is_transitioning()) {
    auto now = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count();
    
    if (delta >= FIXED_MS) {
        update_physics(delta);
        last_time = now;
    }
    
    render();
    vwait();
}
```

この書き方を「ライブラリに組み込む」必要はない。ドキュメントで「固定タイムステップが必要な場合の書き方」として提示するに留める。

### 2.5 複数タスクの並行管理

**結論: `std::jthread` や非同期タスクキューの採用は見送る**

シングルスレッドブロッキング設計（確認済み）は Direct2D の「描画はメインスレッドのみ」という制約に整合している。バックグラウンドでのデータ読み込みなど限定的な用途には、ユーザーが `std::jthread` を直接使えばよい。HspppLib の責務はゲームループの管理であり、汎用スレッド管理ライブラリではない。

---

## 第3章: モダンなアプリ構造への道

### 3.1 Repository の scoped-reset 問題の修正

**結論: `reset_all()` にタグフィルタリングを実装する**

確認した事実：`reset_all()` は `registered_repositories_` に登録された全リポジトリをタグを無視して一括リセットする。`struct GlobalTag` と `struct StateTag` は識別子として存在するが、`reset_all()` ではフィルタリングに使われていない。

**修正設計**：

```cpp
// GameServices 内部
template<typename Tag = void>
void reset_all() {
    for (auto& [key, resetter] : registered_repositories_) {
        // Tag が void の場合は全リセット（既存互換）
        if constexpr (std::is_void_v<Tag>) {
            resetter();
        } else {
            // タグが一致するものだけリセット
            if (key.tag_type == typeid(Tag)) {
                resetter();
            }
        }
    }
}

// 使用例
services().reset_all<StateTag>();   // シーンローカルデータのみリセット
services().reset_all<GlobalTag>();  // グローバルデータのみリセット
services().reset_all();             // 全リセット（既存互換）
```

**後方互換**: 引数なしの `reset_all()` は既存通り全リセット。

### 3.2 ECS 的考え方と Repository パターンの融合可能性

**結論: 完全な ECS 採用は不要。「Repository を Component として扱う」軽量 ECS で十分**

HspppLib のターゲットは「HSP ユーザー」であり、Entity/Component/System の概念を明示的に学ぶことを強制しない。しかし既存の `Repository<T>` は事実上「ゲームワールド内のシングルトン Component」として機能している。

**提案: 命名と構造の整理**

```cpp
// 現在
Repository<PlayerData>::instance().get().hp = 100;
services().data<PlayerData>().hp = 100;  // 短縮形

// 「Component」という概念を意識した命名（将来的に）
// ただし既存 API を廃止せず、追加のみ
services().component<PlayerData>().hp = 100;  // エイリアス
```

完全な ECS（EntityID + ComponentStorage + System スケジューラ）は HspppLib のスコープ外。Repository パターンがカバーするのは「ゲーム全体で共有されるデータ」であり、それで十分。

### 3.3 イベントシステム（sm.emit() パターン）の設計案

**結論: `sm.emit(EventType)` → 条件に応じた遷移を実装する**

現状は `jump()` の直接呼び出しのみ。これは「遷移元が遷移先を知っている」密結合を生む。イベント駆動にすることで疎結合化できる。

```cpp
// イベント型定義（enum class で型安全）
enum class GameEvent {
    PlayerDied,
    BossDead,
    TimerExpired,
    PlayerLevelUp,
};

// イベントハンドラの登録
sm.on_event(GameEvent::PlayerDied, Scene::GameOver);   // イベント → 遷移
sm.on_event(GameEvent::BossDead, Scene::Result);
sm.on_event(GameEvent::TimerExpired, [&sm]() {        // イベント → カスタム処理
    if (sm.current_state() == Scene::Boss) {
        sm.jump(Scene::GameOver);
    }
});

// どこからでもイベントを発行（Repository 経由不要）
sm.emit(GameEvent::PlayerDied);  // → Scene::GameOver に遷移

// 内部実装
template<typename EventType>
    requires std::is_enum_v<EventType>
void emit(EventType event) {
    if (auto it = event_transitions_.find(event); it != event_transitions_.end()) {
        jump(it->second);  // 対応するステートに遷移
    } else if (auto it2 = event_handlers_.find(event); it2 != event_handlers_.end()) {
        it2->second();  // カスタムハンドラを呼ぶ
    }
}
```

**`jump()` 直接呼び出しとの使い分け**:
- `jump()`: 「今の状態からこのステートに行く」という明示的意図がある場合
- `emit()`: 「何かが起きた（PlayerDied）。どこに行くかは SM が知っている」疎結合な場合

**注意**: イベント型 `EventType` は `StateType` と別の enum class にすること。これにより「ステート = 画面」「イベント = ゲーム内出来事」の分離が明確になる。

### 3.4 シーン管理の発展形

**結論: 現状の StateMachine ベースで十分。「リソースのシーン別管理」を補完する**

現状不足しているのは「シーン切り替え時のリソースロード/アンロード」機能。`on_enter` / `on_exit` でリソース管理コードを書くことは既に可能だが、パターンが統一されていない。

```cpp
// 推奨パターンの明文化（実装変更なし、ドキュメント整備のみ）
sm.state(Scene::Game)
  .on_enter([](auto& local) {
      // リソースロード
      local.bg_image = load_image("game_bg.png");
      local.se_hit = load_sound("hit.wav");
  })
  .on_exit([](auto& local) {
      // リソース解放（RAII で自動解放されるなら不要）
      local.bg_image.reset();
      local.se_hit.reset();
  });
```

将来的には `SceneResourceManager`（シーン別にロード/アンロードを自動管理）の追加が価値を持つ。ただし現スプリントのスコープ外。

---

## 第4章: C++23 活用方針

### 4.1 `std::generator` の評価

**メリット**:
- `on_update` を `while(!is_transitioning())` ループなしで書ける
- コルーチンの中断/再開が `co_yield` 1行で済む

**導入コスト**:
- `std::generator` の概念を HSP ユーザーに説明するコストが高い
- `await()` との組み合わせで内部ループモデルが複雑化
- 既存の `on_update` シグネチャとの非互換（`StateBuilderWithCoroutine` という新クラスが必要）

**結論**: **現時点では採用しない**。`tick()` パターンとの組み合わせで将来的に `StateBuilderWithCoroutine` として追加可能な設計を保つ。

```cpp
// 将来追加候補（現時点では実装しない）
sm.state_coro(Scene::Game)
  .on_update_coro([](auto& sm) -> std::generator<FrameYield> {
      int frame = 0;
      while (true) {
          // 描画処理
          frame++;
          co_yield FrameYield{};  // await(16) 相当
      }
  });
```

### 4.2 `std::expected<T, E>` の評価

**メリット**:
- `safe_call` + 例外スロー方式より戻り値で明示的にエラーを扱える
- エラーを無視するとコンパイル警告が出る（`[[nodiscard]]` 相当）
- パフォーマンスが例外より有利（例外は throw 時のコストが大きい）

**導入コスト**:
- 既存の `safe_call` / `HspError` 例外機構をすべて変更が必要（大規模）
- HSP ユーザーは「エラーチェックを書かなくてもいい」環境に慣れている

**結論**: **部分採用を推奨**。公開 API（`hspMain()` から呼ぶ関数）は既存の例外方式を維持し、**内部実装の新規追加部分**（ガード条件付き遷移、イベントシステム等）では `std::expected` を活用する。

```cpp
// 新規追加する内部関数での使用例
std::expected<StateType, TransitionError> try_get_event_target(EventType event) {
    if (auto it = event_transitions_.find(event); it != event_transitions_.end()) {
        return it->second;  // 成功: 遷移先ステートを返す
    }
    return std::unexpected(TransitionError::NoHandlerRegistered);
}
```

HSP ユーザーが直接触れる公開 API は従来通り例外方式を維持し、エラーは `WinMain` の `catch` ブロックで処理する。

### 4.3 Concepts の活用

**結論: 積極的に活用する。既存の `requires std::is_enum_v<StateType>` を拡張**

```cpp
// 現状（hsppp_statemachine.ixx）
template<typename StateType>
    requires std::is_enum_v<StateType>
class StateGraph;

// 拡張案（イベント型の制約）
template<typename EventType>
concept HspEvent = std::is_enum_v<EventType>;

template<typename StateType, typename EventType = void>
    requires std::is_enum_v<StateType> && (std::is_void_v<EventType> || HspEvent<EventType>)
class StateGraph;

// LocalData の制約（デフォルト構築可能であること）
template<typename LocalData>
concept StateLocalData = std::is_default_constructible_v<LocalData>;

sm.state<PlayerData>(Scene::Game)  // PlayerData が default constructible でないとコンパイルエラー
```

Concepts による制約強化は**コンパイルエラーメッセージをわかりやすくする**効果が大きい。HSP ユーザーが誤った型を渡したときに即座に原因を特定できる。

### 4.4 Ranges / Views の活用

**メリット**: システム更新ループを宣言的に書ける

```cpp
// 将来的な活用例（ステート一覧フィルタリング等）
auto active_states = states_ 
    | std::views::filter([](const auto& kv) { return kv.second.entered; })
    | std::views::keys;

// 遷移グラフの export_graph() 改善
auto edges = transition_graph_ 
    | std::views::transform([&](const auto& pair) {
          return std::format("  {} -> {};", 
              state_to_string(pair.first), 
              state_to_string(pair.second));
      });
```

**結論**: 内部実装の可読性向上に**積極活用**する。公開 API には不要（ユーザーは内部実装を見ない）。

---

## 第5章: 「HSP らしさ」と「モダン設計」の両立哲学

### 5.1 HspppLib の設計思想の定義

CONCEPT.md で定義された「Pragmatic Hybrid」を技術的に言い換えると：

> **「使い始めは HSP のように書け、気づいたらモダンな設計になっている」**

これを実現する具体的な原則：

1. **エントリーポイントは `hspMain()` から始まる**: WinMain を書かない。HSP の `hspMain()` から始める
2. **1つの目的は1つの関数呼び出しで書ける**: `sm.jump(Scene::Next)` 1行でシーン遷移できる
3. **型エラーはコンパイル時に止まる**: 存在しないステートへの遷移はコンパイルエラー（HSP の実行時エラーではなく）
4. **段階的に深く使える**: `sm.state(x).on_update(lambda)` で動き、必要になったら `sm.state<LocalData>(x)` にグレードアップできる

### 5.2 「使いやすさを犠牲にしてモダンにする」べきでない境界線

**以下のパターンは「モダンにすべきでない」**:

| NGパターン | 理由 |
|-----------|------|
| `on_update` の引数を `std::coroutine_handle<>` にする | HSP ユーザーがコルーチンの概念を理解しないと書けなくなる |
| エラー処理を `std::expected<T, E>` の `and_then().or_else()` チェーンにする | 「動けばいい」HSP ユーザーには過剰。エラーは例外で一括処理でよい |
| ステート定義を Builder パターンから変更する | 現状の `.on_enter().on_update().on_exit()` チェーンは直感的 |
| 遷移を `jump()` ではなく `emit()` のみにする | `jump()` の直感性は残す。`emit()` は追加オプション |
| DI コンテナ / IoC フレームワークの導入 | `services().data<T>()` 相当の機能は既にある |

**基準**: 「HSP に慣れた人に30秒で説明できるか」。30秒で説明できないAPIは HspppLib の公開 API にふさわしくない。

### 5.3 「気づかずにモダンな設計を使っている」状態の実現方法

**層分け設計**（アイスバーグモデル）：

```
【ユーザーが見る面（水面上）】
  sm.state(Scene::Title)
    .on_update([](auto& sm) {
        while (!sm.is_transitioning()) { await(16); }
    });
  ↑ HSP に近い書き方。enum class がコンパイル時安全を提供

【ユーザーが気づかない部分（水面下）】
  - StateMachineBase + RAII コンテキスト管理
  - thread_local による await/stop との自動連携
  - shared_ptr<void> 型消去による LocalData 管理
  - Concepts による型制約
  - std::expected による内部エラー処理
```

具体的実現方法：

1. **Concepts は制約として機能させる**: `requires std::is_enum_v<StateType>` のようにユーザーには「コンパイルエラーで間違いを教えてくれる」として体験される
2. **RAII は自動で動く**: `StateMachineScope` はユーザーコードに出てこない。`run()` の中で自動管理される
3. **`services().data<T>()` は「グローバル変数の型安全版」として体験される**: シングルトン・型消去・Registry パターンを使っているが、ユーザーには「グローバル変数みたいに使えるやつ」として映る

### 5.4 既存 API の後方互換性維持方針

**絶対に壊さない API**:

サンプルコード（`StateSampleMain.cpp` / `NewStateSampleMain.cpp`）が変更なしでコンパイル・実行できることを後方互換の基準とする。

- `sm.state(x).on_enter(f).on_update(f).on_exit(f)` のシグネチャを変更しない
- `sm.jump()` / `sm.back()` / `sm.start()` / `sm.run()` / `sm.tick()` を維持
- `sm.is_transitioning()` / `sm.is_running()` を維持
- `await()` / `vwait()` / `stop()` の基本動作を維持
- `services().data<T>()` / `services().register_repository<T>()` を維持

**拡張のルール**:

1. **新機能は新しいメソッド/オーバーロードとして追加**: 既存メソッドのシグネチャを変えない
2. **オプション引数はデフォルト値付き**: `cancel_timer()` → `cancel_timer(std::string_view tag = "")` のように既存呼び出しが壊れない
3. **型パラメータの追加はデフォルト付き**: `reset_all<Tag = void>()` のように引数なしでも動く

### 5.5 「やりたいことが1つの関数呼び出しで書ける」原則

HSP の最大の強みは命令密度の高さ。`boxf 0,0,100,100` の1行で四角形が描ける。HspppLib は `boxf(0, 0, 100, 100)` で同じことができる。

この原則をステート管理にも適用する：

```cpp
// ❌ モダンだが HSP 原則に反する（設定が多すぎる）
sm.add_state(
    StateConfig{
        .id = Scene::Title,
        .on_enter = []() { ... },
        .on_update = [](auto& sm) { ... },
        .transitions = { { Scene::Game, GuardAlways{} } }
    }
);

// ✅ HSP 原則に従う（チェーンで1セット）
sm.state(Scene::Title)
  .on_enter([]() { ... })
  .on_update([](auto& sm) { ... });
sm.allow_transition(Scene::Title, Scene::Game);  // 遷移ルールは別行でも簡潔
```

新機能（ガード条件・イベントシステム・複数タイマー）を追加する際も、この「チェーン形式での設定」原則を維持することを設計制約とする。

---

## まとめ: 実装優先度マップ

### Priority 1（今すぐやるべき）

| 作業 | 影響 | 実装コスト |
|------|------|-----------|
| ~~`defer_jump()` の真の実装（遅延遷移）~~ | **削除済み** | — |
| `vwait()` への `should_transition()` チェック追加 | 非一貫性解消 | 極小（3行追加） |
| 複数タイマー（`timers_` を `vector` 化） | ゲームの実用性向上 | 小〜中 |

### Priority 2（次スプリントで着手すべき）

| 作業 | 影響 | 実装コスト |
|------|------|-----------|
| `allow_transition_if()` ガード条件付き遷移 | 宣言的ルール管理 | 中 |
| `reset_all<Tag>()` タグ別リセット修正 | Repository の完成 | 小 |
| LocalData 自動リセットオプション | バグ防止 | 小 |
| `await_until(predicate)` 追加 | ボイラープレート削減 | 小 |

### Priority 3（中期的に検討）

| 作業 | 影響 | 実装コスト |
|------|------|-----------|
| `sm.emit(EventType)` イベントシステム | 疎結合化 | 中 |
| Concepts による型制約強化 | 開発者体験向上 | 小〜中 |
| `push_and_jump()` / `pop_state()` エイリアス | 意図明示 | 極小 |
| `export_graph()` の Ranges 活用による改善 | 内部品質 | 小 |

### 採用しない（スコープ外）

- 並列ステート / HSM の組み込み（`tick()` パターンで代替可能）
- `std::generator` による `on_update` コルーチン化（HSP らしさに反する）
- 完全な ECS フレームワーク導入（Repository パターンで十分）
- クロスプラットフォーム化（Direct2D 依存は設計の前提）
- `std::jthread` / 非同期タスクキューの標準組み込み

---

*本提言はコード調査（2026-03-02T22:20:07+0900）を唯一の事実的根拠とする.*
