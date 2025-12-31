# CLAUDE.md - AI Assistant Guidelines for HSPPP

このファイルは、Claude（AI アシスタント）がこのリポジトリで作業する際の
ガイドラインと参照情報を提供します。

---

## 🚨 最重要：作業前の誓約

**手癖で書かないで、ちゃんと検証してから実装をします。**

- ライセンス、バージョン、OS要件などは**必ず既存ファイルを確認**してから記載する
- 「たぶんこうだろう」で書かない。不明な点は**質問する**か**調査する**
- 新規ドキュメント作成時は、既存の設定ファイルやソースコードを参照して正確な情報を記載する

---

## プロジェクト概要

**HSPPP** は HSP (Hot Soup Processor) 互換のC++23ライブラリです。
HSP互換スタイルとオブジェクト指向スタイルの両方をサポートします。

### アーキテクチャドキュメント

詳細な設計情報は `.claude/ARCHITECTURE.md` を参照してください。

---

## コーディング規約

### デュアルスタイル対応

新しい描画命令を追加する際は、必ず以下の両方を実装すること：
**（これは「当然」のルールであり、質問せずに実施すること）**

1. **グローバル関数**（HSP互換）
   ```cpp
   export void newcmd(int param1, int param2);
   ```

2. **Screen メンバ関数**（OOP）
   ```cpp
   Screen& Screen::newcmd(int param1, int param2);
   ```

### パラメータ省略

省略可能なパラメータには `OptInt` / `OptDouble` を使用：

```cpp
export void screen(
    OptInt id       = {},
    OptInt width    = {},
    OptInt height   = {},
    // ...
);
```

### メソッドチェーン

Screen のメンバ関数は必ず `Screen&` を返すこと：

```cpp
Screen& Screen::color(int r, int g, int b) {
    // 処理
    return *this;  // 必須
}
```

### 非安全コーディングの禁止

HSPPP「利用者」が、非安全コーディングを使うことを強制することは一切行わない原則。
例えば生ポインターでの受け渡しなど。

ライブラリは標準機能が非安全コーディングのものをできる限り安全に使えるように取り計らうことが責務である。

### assert の使用禁止

`assert` マクロは使用しないこと。理由：

- `assert` はデバッグビルドでのみ有効で、リリースビルドでは保護されない
- ユーザーが気づかないうちに未定義動作を引き起こす可能性がある

代わりに以下を使用すること：

- **コンパイル時チェック**: `static_assert` を使用
- **実行時チェック**: `HspError` または `std::out_of_range` などの例外をスロー

```cpp
// NG: assert は使用禁止
assert(i < size && "index out of range");

// OK: 例外による境界チェック
if (i >= size) throw std::out_of_range("index out of range");

// OK: コンパイル時チェック
static_assert(sizeof(T) == 4, "T must be 4 bytes");
```

### 例外とエラーハンドリング

ライブラリ関数では以下の方針でエラー処理を行うこと：

1. **ライブラリ関数内では `HspError` を使用**
   - `std::source_location` を受け取り、ユーザーコードの場所を記録
   - `onerror` ハンドラで処理可能

2. **データ構造体（Quad等）では `std::out_of_range` を使用**
   - `source_location` を取得できない場合は標準例外を使用
   - WinMain.cpp で `HspError` に変換されて `onerror` で処理される

```cpp
// ライブラリ関数: source_location を受け取れるので HspError を使用
void color(int r, int g, int b, const std::source_location& location) {
    if (r < 0 || r > 255) {
        throw HspError(ERR_OUT_OF_RANGE, "colorの値は0~255の範囲で", location);
    }
}

// データ構造体: source_location が取れないので標準例外
constexpr Point2i& operator[](size_t i) {
    if (i >= vertex_count) throw std::out_of_range("Quad index out of range");
    return v[i];
}
```

---

## ビルド手順

必ず`
```
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8
```
としてビルドすること。文字化けの被害を避けられます。

### VS 18 (Visual Studio 2026) を使用

VS 2022ではなく、必ずVS 18 (Visual Studio 2026) を使用してください。旧バージョンではビルドが通りません。

```powershell
# リビルド
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" `
    "c:\Users\takumi\source\repos\Velgail\HspppLib\HspppLib.slnx" `
    /t:Rebuild /p:Configuration=Debug /p:Platform=x64

# ビルド
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" `
    "c:\Users\takumi\source\repos\Velgail\HspppLib\HspppLib.slnx" `
    /t:Build /p:Configuration=Debug /p:Platform=x64
```

### 実行

```powershell
& "c:\Users\takumi\source\repos\Velgail\HspppLib\x64\Debug\HspppSample.exe"
```

**注意**: VS 2022 (VS 17) ではなく VS 18 を使用すること！

---

## 重要な設計判断

### `omit` vs `_`

C++26 で `_` は「名前独立宣言」として特別な意味を持つようになるため、
パラメータ省略には `omit` キーワードを使用する。

### 数学関数と `<cmath>` の共存

`hsppp` モジュールは `std::sin`, `std::cos` 等を再エクスポートしている。
`using namespace hsppp;` と `#include <cmath>` を併用すると、
`sin` 等の呼び出しが曖昧になる可能性がある。

推奨される書き方：
```cpp
import hsppp;
// OK: 明示的な名前空間
hsppp::sin(hsppp::deg2rad(45.0));

// OK: using namespace を使う場合は <cmath> を避ける
using namespace hsppp;
sin(deg2rad(45.0));  // hsppp::sin を呼ぶ
```

### Screen クラスの軽量ハンドル方式

`Screen` クラスは `shared_ptr<HspSurface>` を保持せず、ID のみを保持する。
これにより：
- モジュール境界の型問題を回避
- コピーコストを最小化
- HSPのウィンドウID概念と一致

### 実装の分離

- `hsppp.ixx`: 公開インターフェース（export）
- `hsppp.cpp`: 実装（Internal.h をインクルード）
- `Internal.h`: 内部クラス定義（HspSurface, HspWindow）

---

## ファイル構成

```
HspppLib/
├── .claude/
│   ├── ARCHITECTURE.md  # 詳細アーキテクチャ
│   └── (このファイルは CLAUDE.md としてルートに配置)
├── module/
│   └── hsppp.ixx        # メインモジュール
├── src/
│   ├── core/
│   │   ├── hsppp.cpp    # API実装
│   │   ├── Internal.h   # 内部定義
│   │   └── ...
│   └── ...
└── ...

HspppSample/
└── UserApp.cpp          # デモアプリ
```

---

## 変更時のチェックリスト

新機能追加時：
- [ ] グローバル関数版を `hsppp.ixx` に宣言
- [ ] Screen メンバ関数版を `hsppp.ixx` に宣言
- [ ] グローバル関数を `hsppp.cpp` に実装
- [ ] Screen メンバ関数を `hsppp.cpp` に実装
- [ ] **全ての実装のビルドが通ること**（これが「とりあえず実装した」の定義）
- [ ] サンプル（UserApp.cpp）に使用例追加

**重要**: 機能追加時は、HspppLib、HspppTest、HspppSample の
**全てのプロジェクトがビルド成功すること**を確認すること。
これにより、コンパイル時検証（Test）と実行時検証（Sample）の両方が保証される。

---

## コーディングルール

### #if 0 の禁止

**コンパイラで `#if 0` によるコメントアウトは禁止**

理由:
- コンパイルされないコードは型エラーやシンタックスエラーに気づけない
- コード腐敗の原因となる
- ビルド検証が不完全になる

代わりに:
- **削除する** - 不要なコードはバージョン管理にあるので削除OK
- **UserApp.cppは破壊OK** - デモコードなので自由に書き換えて良い
- **テストコードで検証** - ApiCompileTest.cppで全パターンをコンパイル検証

### Unicode 必須（ANSI API 禁止）

**このプロジェクトは Unicode ビルドです。ANSI 版 API の使用は禁止です。**

禁止例：
- `OutputDebugStringA` → `OutputDebugStringW` を使用
- `MessageBoxA` → `MessageBoxW` を使用
- `CreateFileA` → `CreateFileW` を使用
- その他すべての `*A` サフィックス API

理由：
- Unicode 文字（日本語等）が正しく処理されない
- 文字化けの原因となる
- プロジェクト設定と不整合

```cpp
// NG: ANSI 版 API
OutputDebugStringA("error message");

// OK: Unicode 版 API
OutputDebugStringW(L"error message");

// OK: std::string から変換
OutputDebugStringW(internal::Utf8ToWide(e.what()).c_str());
```

### 例外握りつぶし禁止

**`catch(...)` や `catch(const std::exception&)` で例外を握りつぶすことは禁止。**

禁止パターン：
```cpp
// NG: 例外を握りつぶしてデフォルト値を返す
try {
    doSomething();
} catch (...) {
    return defaultValue;  // 何が起きたか不明
}

// NG: ログ出力だけして握りつぶす
try {
    doSomething();
} catch (const std::exception& e) {
    OutputDebugStringW(L"error occurred");  // で、どうするの？
}
```

正しい対処：
1. **例外を投げないコードには try-catch を書かない**（過剰防衛禁止）
2. **例外を変換する場合は HspError に変換して rethrow**
3. **意図的なフォールバックは HSP 互換動作としてコメント明記**

```cpp
// OK: HspError に変換して rethrow
try {
    somethingThatThrows();
} catch (const std::exception& e) {
    throw HspError(ERR_SYSTEM_ERROR, e);  // 元の例外を保持
}

// OK: HSP 互換の意図的なフォールバック（数値変換など）
try {
    return std::stoi(str);
} catch (const std::invalid_argument&) {
    return 0;  // HSP 互換: 変換失敗時は 0
}
```
