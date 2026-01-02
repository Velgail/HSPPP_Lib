# HSPPP (HSP Plus Plus) 設計仕様書

**Version:** 0.1 (Draft)
**Target:** C++23 / Windows (Direct2D)

## 1\. コアコンセプト

**「Pragmatic Hybrid（実用主義的ハイブリッド）」**

  * **目的:** HSPの「手軽さ（命令型記述）」と、C++の「拡張性・速度（オブジェクト指向）」の完全な融合。
  * **ターゲット:** HSPでの開発に限界を感じ、C++へ移行したいが、Javaのような過度な儀式（ボイラープレート）は嫌う層。
  * **デザインゴール:**
    1.  **HSP再現性:** `hspMain` 内ではHSPとほぼ同じロジックで記述できる。
    2.  **安全性:** C++の型システムとRAII（自動リソース管理）により、メモリリークや型エラーを防ぐ。
    3.  **拡張性:** ユーザーはいつでも「HSPの皮」を破り、DirectXやWin32 APIの深層へアクセスできる。

-----

## 2\. アーキテクチャ構成

### 2.1. エントリーポイントの隠蔽

ユーザーは `main` / `WinMain` を記述しない。ライブラリ側がブートストラップを提供する。

  * **System Boot:** ライブラリ内の `WinMain` が起動。
  * **Init:** COM, Direct2D Factory, システムフォント等の初期化。
  * **User Code:** `hspMain()` をコール。
  * **Persistence:** `await` や `stop` 中も、ライブラリがウィンドウメッセージ（`WM_PAINT`等）を処理し、画面内容を維持する。

### 2.2. 描画システム（Surface Architecture）

すべての描画対象を「Surface」として抽象化し、IDマップで管理する。

  * **基底クラス: `HspSurface`**
      * 共通プロパティ: カレントカラー、カレントポジション、フォント設定。
      * 共通メソッド: `mes`, `boxf`, `line`, `get_bitmap`。
  * **派生クラス:**
      * **`HspWindow` (screen/bgscr):** `HWND` と `IDXGISwapChain` を持つ。実画面への `Present` を担当。
      * **`HspBuffer` (buffer):** `ID2D1BitmapRenderTarget` を持つ。メモリ内描画のみ。
  * **ID管理:** `std::map<int, std::shared_ptr<HspSurface>>`
      * `gsel(id)`: グローバルな `current_surface` ポインタを切り替える。
      * `gcopy(id, ...)`: IDマップから対象を探し、ポリモーフィズムを利用して画像を転送する。

### 2.3. 描画パイプライン（HSP互換の保持モード）

DirectXのImmediate Mode（毎フレーム全消去）ではなく、HSPのCanvas挙動を再現する。

1.  **Offscreen First:** 全ての描画命令は、まずメモリ上の「バックバッファ」に対して行われる。
2.  **Redraw Command:**
      * `redraw(0)`: 描画バッチ開始。
      * `redraw(1)`: バックバッファの内容をスワップチェーン（画面）に転送し、VSync同期でフリップ。
3.  **Auto-Repaint:** ウィンドウが隠れたり最小化から復帰した際は、`WM_PAINT` をフックしてバックバッファの内容を自動で再転送する。

-----

## 3\. API仕様ガイドライン

### 3.1. 関数と手続きの役割分担

  * **手続き（Void Function）:** 「動作」を表すもの。
      * `mes`, `boxf`, `color`, `redraw`
  * **関数（Return Value）:** 「値の取得」を表すもの。システム変数は関数化する。
      * `mousex()`, `mousey()`, `sin()`, `rnd()`
  * **参照渡し（Reference）:** 状態の書き換えを行うもの。
      * `stick(key)` (key変数を書き換える)

### 3.2. 数学・計算ライブラリ

**方針: Input Permissiveness, Output Strictness**

  * **入力:** テンプレートを活用し、`int`, `float`, `double` すべてを受け入れる（暗黙キャスト）。
  * **出力:** 原則 `double` を返す。ユーザーに型を意識させる（警告による教育）。
  * **拡張:**
      * `deg2rad(deg)`, `rad2deg(rad)`: 度数法⇔ラジアン変換。
      * `dist(x, y)`: 距離計算。
      * `limit(v, min, max)`: 値の制限。

### 3.3. 乱数生成

二層構造で提供する。

1.  **`rnd(max)` (Standard):** 線形合同法 (LCG)。Windows標準の `rand()` 互換計算式を内蔵。再現性と「HSPらしいチープさ」を保証。
2.  **`rnd_mt(max)` (Extension):** メルセンヌ・ツイスタ (`std::mt19937`)。高品質な乱数が必要な場合に使用。

### 3.4. 文字列処理

  * **型:** C++標準の `std::string` (UTF-8) を採用。
  * **命令:**
      * `mes` は `std::format` を活用可能にする。
      * `strmid`, `instr` 等のHSP便利命令は、`std::string` に対するラッパー関数として提供する（内部で `substr`, `find` を呼ぶ）。

### 3.5. メモリノートパッド

  * `std::vector<std::string>` のラッパーとして実装。
  * `notesel`, `noteadd`, `noteget` を再現し、行単位のテキスト処理を容易にする。

-----

## 4\. プロジェクト・ファイル構成

Visual Studio 2026 / C++23 Modules を前提とする。

```text
HSPPP_Solution.slnx
│
├── HspppLib/ (Static Library)
│   ├── module/
│   │   ├── hsppp.ixx       (ユーザー向け公開インターフェース)
│   │   ├── graphics.ixx    (描画モジュール)
│   │   └── input.ixx       (入力モジュール)
│   │
│   └── src/
│       ├── boot/
│       │   └── WinMain.cpp (エントリーポイントの実体)
│       ├── core/
│       │   ├── Surface.cpp (HspSurface / Window / Buffer)
│       │   └── System.cpp  (初期化・終了処理)
│       └── utils/
│           └── Random.cpp
│
└── HspppSample/ (Application .exe)
    ├── HspppSample.vcxproj
    └── UserApp.cpp (ユーザーコード: hspMain を記述)
```

-----

## 5\. ユーザーへの移行ガイド（Short）

1.  **WinMainは書かない:** `void hspMain()` から書き始める。
2.  **変数は宣言する:** 使う前に `int x = 0;` のように書く。
3.  **IncludeではなくImport:** `import hsppp;` を使う。
4.  **ラベルジャンプは卒業:** `button goto` ではなく `if (button())` を使う。
