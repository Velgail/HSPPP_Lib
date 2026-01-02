# FAQ（よくある質問）

## 目次

- [基本](#基本)
- [C++20以降の機能](#c20以降の機能)
- [GUI コントロール](#gui-コントロール)
- [ライフタイムと安全性](#ライフタイムと安全性)
- [HSPとの互換性](#hspとの互換性)
- [その他](#その他)

---

## 基本

### Q: HSPPP は何ですか？

A: HSPPP は HSP (Hot Soup Processor) の API を C++23 で再実装したライブラリです。HSP の直感的な描画・GUI 命令を C++ で使用できます。

### Q: HSP を知らなくても使えますか？

A: はい。HSP の知識がなくても、C++ の一般的な知識があれば使用できます。ただし、HSP のドキュメントも参考になります。

### Q: サポートされているコンパイラは？

A: Visual Studio 2026 (VS 18) が必要です。C++23 のモジュール機能と `.slnx` ソリューション形式を使用しているため、それ以前のバージョンでは動作しません。

### Q: `main()` を書かないのはなぜですか？

A: HSPPP ライブラリが `WinMain` を実装し、ユーザー定義の `hspMain()` を提供する設計です。これはWinMainとWindowsの複雑なお作法を隠蔽することで HSP の「エントリポイント不要」の体験を再現しています。

---

## C++20以降の機能

C++17 の書籍（[江添亮の詳説C++17](https://ezoeryou.github.io/cpp17book/) など）でカバーされていない、このプロジェクトで使われている機能です。

| 機能 | 規格 | 用途 | cpprefjp |
|------|------|------|----------|
| モジュール | C++20 | `import hsppp;` | [modules](https://cpprefjp.github.io/lang/cpp20/modules.html) |
| std::format | C++20 | `strf()` の内部実装 | [format](https://cpprefjp.github.io/reference/format.html) |
| std::source_location | C++20 | エラー発生箇所の自動取得 | [source_location](https://cpprefjp.github.io/reference/source_location.html) |

### Q: `import hsppp;` とは何ですか？

A: C++20 で導入された**モジュール**機能です。従来の `#include` と異なり、コンパイル時間の短縮とマクロ汚染の防止ができます。

```cpp
// C++20 モジュール方式（HSPPPで使用）
import hsppp;

// 従来の #include 方式（HSPPPでは使用しない）
// #include "hsppp.h"
```

### Q: `strf()` の中で使われている `std::format` とは？

A: C++20 で導入された書式化ライブラリです。Python の f-string に似た記法で文字列を組み立てられます。

```cpp
std::string s = strf("x={}, y={}", 100, 200);  // "x=100, y=200"
```

### Q: 関数の引数にある `std::source_location` とは？

A: C++20 で導入された、呼び出し元のソース位置（ファイル名・行番号・関数名）を自動取得する機能です。HSPPP ではエラー発生時に「どこで問題が起きたか」を表示するために使用しています。

ユーザーが意識する必要はありません（デフォルト引数で自動取得されます）。

---

## GUI コントロール

### Q: `chkbox` に `int&` を渡せないのはなぜですか？

A: **ライフタイム安全性のため**です。

HSP では変数はすべてグローバルスコープなので、GUI オブジェクトが変数を参照し続けても問題ありませんでした。しかし C++ では、ローカル変数を参照で渡すと、関数を抜けた後に GUI がその変数を参照し続け、**ダングリングポインタ（未定義動作）** となります。

```cpp
// ❌ 危険なパターン（HSPPPでは提供していません）
void createCheckbox() {
    int state = 0;              // ローカル変数
    chkbox("Check", state);     // state のアドレスを保持
}   // ← ここで state が破棄される
    // その後 GUI が state を参照すると未定義動作！

// ✅ 安全なパターン（HSPPPが提供する方法）
void createCheckbox() {
    auto state = std::make_shared<int>(0);  // 共有所有権
    chkbox("Check", state);     // state の所有権を GUI と共有
}   // ← shared_ptr のコピーが GUI にあるので安全
```

同様に `combox` と `listbox` も `shared_ptr<int>` 版のみ提供しています。

### Q: `input` は `string&` を渡せるのに、`chkbox` は `int&` を渡せないのはなぜですか？

A: `input` と `mesbox` の参照版は、**作成時に値をコピーする設計**のため、比較的安全に使用できます（ただし双方向同期は取れません）。

一方、`chkbox`/`combox`/`listbox` は状態変数を**継続的に参照・更新する**必要があるため、参照版では安全性を保証できません。

完全な双方向バインディングが必要な場合は、`input`/`mesbox` も `shared_ptr` 版の使用を推奨します。

### Q: グローバル変数なら `int&` でも安全では？

A: 技術的にはグローバル変数や `static` 変数であれば安全ですが、HSPPP は「利用者に非安全なコーディングを強制しない」という設計方針を採用しています。

API として参照版を提供すると、ユーザーが誤ってローカル変数を渡してしまうリスクがあります。そのリスクを排除するため、`shared_ptr` 版のみを提供しています。

---

## ライフタイムと安全性

### Q: `shared_ptr` を使うと遅くなりませんか？

A: GUI オブジェクトの作成は通常、アプリケーション起動時や画面遷移時など、パフォーマンスが重要でない場面で行われます。`shared_ptr` のオーバーヘッドは実用上無視できるレベルです。

描画やゲームループのような高頻度処理では `shared_ptr` を使用しない API を提供しています。

### Q: なぜ `unique_ptr` ではなく `shared_ptr` ですか？

A: GUI オブジェクトとユーザーコードの**両方が変数を所有**する必要があるためです。

- ユーザーコード: 変数の値を読み書き
- GUI オブジェクト: 状態変更時に変数を更新

両者が独立して所有権を持つには `shared_ptr` が適切です。

### Q: `weak_ptr` は使えますか？

A: 現在の API では直接サポートしていませんが、`shared_ptr` を渡す前に `weak_ptr` を保持しておくことは可能です。

---

## HSPとの互換性

### Q: HSP のコードをそのまま移植できますか？

A: 完全な互換性はありませんが、多くの命令は同じ名前・引数で使用できます。主な違いは：

1. **エントリポイント**: `int hspMain()` を定義する必要がある
2. **変数宣言が必要**: C++ では変数を宣言する必要があります
3. **文字列はダブルクォート**: `"文字列"` （HSP はシングルクォートも可）
4. **行末にセミコロン**: `;` が必要です
5. **一部の命令は shared_ptr が必要**: `chkbox`, `combox`, `listbox` など

### Q: HSP の `goto`/`gosub` は使えますか？

A: **使えません。** C++ の関数を使用してください。

- `gosub` → 関数呼び出しで代替
- `goto` → C++ では不要（関数・ループ・条件分岐で構造化）

```cpp
// gosub *draw → 関数呼び出し
void draw() {
    color(255, 0, 0);
    boxf(0, 0, 100, 100);
}

int hspMain() {
    screen(0, 640, 480);
    draw();  // 関数を呼ぶだけ
    stop();
}
```

### Q: HSP の `dim`/`sdim` に相当するものは？

A: C++ の標準機能を使用してください：

```cpp
// HSP: dim a, 10
std::vector<int> a(10);

// HSP: sdim s, 256
std::string s;
s.reserve(256);
```

### Q: HSP の `stop` に相当するものは？

A: `stop()` 関数をそのまま使用できます。

```cpp
stop();   // 割り込みを待機（HSPのstopと同じ）
end(0);   // プログラムを終了（HSPのendと同じ）
```

`hspMain()` から `return` した場合も `stop()` と同等の動作になります。

### Q: パラメータの省略はどうすればいいですか？

A: HSP では `screen , 800, 600` のようにパラメータを省略できました。HSPPP では `omit` または `{}` を使用します。

```cpp
// HSPスタイル: screen ,,, 2  (ID, width, height を省略して mode だけ指定)
screen(omit, omit, omit, screen_hide);  // omit で省略
screen({}, {}, {}, screen_hide);        // {} でも省略可能
```

---

## その他

### Q: バグを見つけました。どこに報告すればいいですか？

A: [GitHub Issues](https://github.com/Velgail/HspppLib/issues) に報告してください。

### Q: 機能追加のリクエストはできますか？

A: はい、GitHub Issues で受け付けています。HSP の命令で未実装のものがあれば、リクエストしてください。

### Q: ドキュメントに誤りを見つけました

A: プルリクエストまたは Issue で報告してください。修正します。
