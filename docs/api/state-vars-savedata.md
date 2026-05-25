---
layout: default
title: StateScope / state_vars / SaveData API
---

# StateScope / state_vars / SaveData API

ステート別変数（**StateScope** / `state_vars`）と、バイナリ・セーブ／ロード（**SaveWriter** / **SaveReader** / `Serializable`）の API リファレンス。

State Machineと組み合わせて、HSP の「ステートに紐づくグローバル変数」感覚を C++ 側で型安全に再現するためのモジュールです。

---

## 目次

- [StateScope&lt;TState&gt;](#statescopetstate)
- [state_vars ショートカット](#state_vars-ショートカット)
- [Serializable concept](#serializable-concept)
- [SaveWriter](#savewriter)
- [SaveReader](#savereader)
- [snapshot / restore（StateScope と連携）](#snapshot--restorestatescope-と連携)
- [使い分けまとめ](#使い分けまとめ)

---

## StateScope&lt;TState&gt;

### 概要

`StateScope<TState>` は **「ステートごとの変数バインディング」** を担当するクラスです。`StateGraph<TState>` に対して弱参照（ポインタ）を持ち、`(TState, std::type_index)` をキーとして任意型の値を `std::any` で保持します。

- 同じキーで `bind()` を再呼出した場合は **冪等**（既存スロットを返却し、引数は無視）。
- 値型が `Serializable` を満たす場合のみ `snapshot()` / `restore()` の対象になる（非 Serializable 型は黙ってスキップされる）。
- StateMachine 本体は StateScope を知らない（一方向依存）。

### コンストラクタ

```cpp
template <typename TState>
    requires std::is_enum_v<TState>
class StateScope {
public:
    explicit StateScope(StateGraph<TState>& sm) noexcept;
    // コピー禁止 / ムーブ可
};
```

### 主要メソッド

| メソッド | 説明 |
|---------|------|
| `L& bind(TState s, Args&&... args)` | 変数を登録（既存があれば 冪等 に返却） |
| `L& get(TState s)` | 取得。未登録 / 型不一致は `std::out_of_range` を throw |
| `L* try_get(TState s) noexcept` | 取得（無ければ `nullptr`） |
| `bool contains(TState s) const noexcept` | 存在チェック |
| `void release(TState s) noexcept` | 単一型を破棄 |
| `void release_all_for(TState s) noexcept` | 指定ステートに紐付く全変数を破棄 |
| `void release_all() noexcept` | 全変数を破棄 |
| `std::size_t size() const noexcept` | 登録済みエントリ数 |
| `std::vector<StateVarEntry> enumerate() const` | 登録一覧（Serializable 可否を含む） |
| `std::vector<std::byte> snapshot() const` | Serializable 変数のみバイト列化 |
| `void restore(std::span<const std::byte>)` | バイト列から復元（事前 `bind()` 必須） |

### 基本使用例

```cpp
import hsppp;
using namespace hsppp;

enum class Scene { Title, Game };

struct GameLocal {
    int score = 0;
    int lives = 3;
};

void hspMain() {
    StateGraph<Scene> sm;
    StateScope<Scene>  scope(sm);

    // Game ステートに GameLocal を bind（冪等）
    auto& game = scope.bind<GameLocal>(Scene::Game);
    game.score = 0;

    sm.state(Scene::Game)
      .on_update([&](auto& sm) {
          auto& g = scope.get<GameLocal>(Scene::Game);
          g.score += 10;
          // ...
          await(16);
      });

    sm.start(Scene::Game);
}
```

> **ヒント:** `bind()` を `on_enter` 内で呼んでも安全です（冪等 なので再入で重複初期化されない）。

---

## state_vars ショートカット

`StateScope::bind` を **HSPPP 流儀の小文字グローバル関数** として呼ぶ形のショートカットです。

```cpp
template <typename L, typename TState, typename... Args>
    requires std::is_enum_v<TState>
L& state_vars(StateScope<TState>& scope, TState s, Args&&... args);
```

`scope.bind<L>(s, args...)` の単純委譲。HSP の「グローバル変数を `*label` ごとに使い分ける」感覚を保ちたい場合に推奨。

```cpp
auto& g = state_vars<GameLocal>(scope, Scene::Game);
g.score += 10;
```

---

## Serializable concept

`SaveWriter::write` / `StateScope::snapshot` でセーブ対象になる型の要件。

```cpp
template <typename L>
concept Serializable = requires(const L& v,
                                std::vector<std::byte>& out,
                                std::span<const std::byte> in,
                                L& dst)
{
    { L::serialize(v, out)       } -> std::same_as<void>;
    { L::deserialize(in, dst)    } -> std::same_as<std::size_t>;   // 消費バイト数を返す
    { L::type_tag()              } -> std::convertible_to<std::string_view>;
};
```

### 実装例

```cpp
struct PlayerProfile {
    std::int32_t high_score = 0;
    std::string  name        = "Player";

    static std::string_view type_tag() noexcept { return "PlayerProfile/v1"; }

    static void serialize(const PlayerProfile& v, std::vector<std::byte>& out) {
        // 任意のバイト列フォーマット（little-endian など）で書き出す
        // ...
    }

    static std::size_t deserialize(std::span<const std::byte> in, PlayerProfile& dst) {
        // 読み出し、消費した byte 数を返す
        return /* consumed */ 0;
    }
};
```

> ⚠️ **MSVC 18 / モジュール境界の罠（build-config §4.5）:** 利用側 .cpp で `serialize` のシグネチャに `std::vector<std::byte>&` を使う場合、`import hsppp;` の他に `import <vector>; import <span>; import <cstddef>;` を明示すると `C2660` を回避できます。

---

## SaveWriter

バイナリ・セーブデータ書き出し器。

```cpp
class SaveWriter {
public:
    SaveWriter();

    template <typename L> requires Serializable<L>
    void write(std::string_view key, const L& value);

    // 型消去版（StateScope::snapshot が内部で使用）
    void write_raw(std::string_view key,
                   std::string_view type_tag,
                   std::span<const std::byte> payload);

    std::vector<std::byte> finalize();   // マジック / version / block 群を返す
};
```

- `finalize()` の戻り値はマジック（`'H','S','P','P'`）+ format version (`kSaveFormatVersion`) + block 数 + 各 block（key / type_tag / payload）。
- 同一 key に対する `write()` の複数回呼出は **最後の値で上書き**。

### バージョン定数

```cpp
inline constexpr std::uint32_t kSaveFormatVersion = 1;
inline constexpr std::array<char, 4> kSaveMagic   = { 'H','S','P','P' };
```

---

## SaveReader

```cpp
class SaveReader {
public:
    explicit SaveReader(std::span<const std::byte> data);   // header / index をパース

    template <typename L> requires Serializable<L>
    bool read(std::string_view key, L& dst);                // type_tag mismatch は HspError

    struct RawBlock { std::string_view type_tag; std::span<const std::byte> payload; };
    std::optional<RawBlock> find_raw(std::string_view key) const;
};
```

- マジック / version 不整合は `HspError(ERR_TYPE_MISMATCH ...)` 相当を throw。
- `read<L>()` は `L::type_tag()` と保存時の type_tag を照合し、不一致なら throw。

---

## snapshot / restore（StateScope と連携）

`StateScope::snapshot()` は登録された **Serializable 変数のみ** をバイト列化します。

```cpp
auto bytes = scope.snapshot();
hsppp::bsave("save.dat", bytes);   // file API へ橋渡し
```

復元側は **事前 `bind()` が必須**（型情報がバイト列に含まれないため、復元先スロットが先に必要）。

```cpp
auto& g = scope.bind<GameLocal>(Scene::Game);   // 復元先 slot を先に用意
auto bytes = hsppp::bload("save.dat");
scope.restore(bytes);                            // 該当 key が見つかればその slot を上書き
```

- 復元対象が見つからない key は **保持されたまま**（部分復元を許容）。
- type_tag が一致しなければ `HspError` を throw（design §17 Risk-2 / §18 Q-6）。
- 非 Serializable な bind は snapshot から漏れる（`enumerate()` で確認可能）。

---

## 使い分けまとめ

| 用途 | 推奨 API |
|------|---------|
| `on_update` 内だけで使う一時変数 | ローカル変数 |
| `on_enter` / `on_update` / `on_exit` で共有 | `StateScope::bind` / `state_vars()` |
| シーンをまたいで永続させたいデータ | `Repository<T>`（`services().data<T>()`） |
| ディスクに保存／復元したい | `Serializable<L>` を満たす型 + `SaveWriter` / `SaveReader` または `StateScope::snapshot/restore` |

> **実装サンプル:** `HspppStateSample/StateSampleMain.cpp` が StateGraph + StateScope + state_vars + SaveWriter/Reader + bsave/bload を 1 本に統合した参考実装です。

---

## 関連項目

- [StateMachine API](statemachine.md) — StateGraph / step() / pause/resume_timer
- [Repository API](repository.md) — 永続データ管理
- [Data Sharing Guide](../guides/data-sharing.md) — データ共有パターンの使い分け
