---
layout: default
title: 画面API
---

# 画面制御 API リファレンス

ウィンドウとバッファの作成・制御に関するAPIです。

## 目次

- [ウィンドウ・画面制御](#ウィンドウ画面制御)
- [画像操作](#画像操作)
- [情報取得](#情報取得)
- [Screen クラス（OOP版）](#screen-クラスoop版)
- [定数](#定数)

---

## ウィンドウ・画面制御

### screen

ウィンドウを初期化します。

```cpp
// OOP版（ID自動採番）
[[nodiscard]] Screen screen(const ScreenParams& params);
[[nodiscard]] Screen screen();

// HSP互換版（ID明示指定）
Screen screen(
    int id,
    OptInt width    = {},      // 画面サイズX（仮想画面 ON 時は論理 px。デフォルト: 640）
    OptInt height   = {},      // 画面サイズY（仮想画面 ON 時は論理 px。デフォルト: 480）
    OptInt mode     = {},      // 画面モード（screen_* フラグ。screen_mode_virtual で仮想画面 ON）
    OptInt pos_x    = {},      // ウィンドウ位置X（-1=システム規定）
    OptInt pos_y    = {},      // ウィンドウ位置Y（-1=システム規定）
    OptInt client_w = {},      // クライアントサイズX（物理 px。0=widthと同じ）
    OptInt client_h = {},      // クライアントサイズY（物理 px。0=heightと同じ）
    std::string_view title = "HSPPP Window"
);
```

**使用例:**

```cpp
// HSP互換スタイル
screen(0, 800, 600);

// HSP互換スタイル + 仮想画面（論理 1920x1080）
screen(0, 1920, 1080, screen_mode_virtual);

// OOP版（構造体）
auto win = screen({.width = 800, .height = 600, .title = "My App"});

// OOP版（構造体 / 仮想画面）
auto win = screen({
    .width  = 1920, .height  = 1080,
    .client_w = 1280, .client_h = 720,
    .virtual_resolution = true,
});

// OOP版（デフォルト設定）
auto win = screen();
```

> 仮想画面（`virtual_resolution = true` または `screen_mode_virtual` 指定）の詳細は
> [仮想画面ガイド](/HSPPP_Lib/VirtualScreen) を参照してください。
> HiDPI awareness は標準で有効化されています（[HiDPI ガイド](/HSPPP_Lib/HiDPI) 参照）。

---

### buffer

オフスクリーンバッファ（仮想画面）を作成します。

```cpp
// OOP版（ID自動採番）
[[nodiscard]] Screen buffer(const BufferParams& params);
[[nodiscard]] Screen buffer();

// HSP互換版（ID明示指定）
Screen buffer(
    int id,
    OptInt width  = {},    // 画面サイズX（デフォルト: 640）
    OptInt height = {},    // 画面サイズY（デフォルト: 480）
    OptInt mode   = {}     // 画面モード
);
```

**使用例:**

```cpp
// ダブルバッファリング用
buffer(1, 640, 480);

// OOP版
auto buf = buffer({.width = 256, .height = 256});
```

---

### bgscr

枠のないウィンドウを初期化します。

```cpp
// OOP版（ID自動採番）
[[nodiscard]] Screen bgscr(const BgscrParams& params);
[[nodiscard]] Screen bgscr();

// HSP互換版（ID明示指定）
Screen bgscr(
    int id,
    OptInt width    = {},
    OptInt height   = {},
    OptInt mode     = {},      // 0=フルカラー, 2=非表示
    OptInt pos_x    = {},
    OptInt pos_y    = {},
    OptInt client_w = {},
    OptInt client_h = {}
);
```

---

### gsel

描画先ウィンドウを変更します。

```cpp
void gsel(OptInt id = {}, OptInt mode = {});
```

| パラメータ | 説明 |
|-----------|------|
| `id` | ウィンドウID |
| `mode` | -1=非表示, 0=影響なし, 1=アクティブ, 2=アクティブ+最前面 |

**使用例:**

```cpp
gsel(0, 1);   // ウィンドウ0をアクティブに
gsel(1);      // ウィンドウ1を描画先に（表示状態は変更しない）
```

---

### width

ウィンドウサイズと位置を設定します。

```cpp
void width(
    OptInt clientW = {},   // クライアント幅
    OptInt clientH = {},   // クライアント高さ
    OptInt posX    = {},   // ウィンドウ位置X
    OptInt posY    = {},   // ウィンドウ位置Y
    OptInt option  = {}    // オプション
);
```

---

### title

ウィンドウタイトルを設定します。

```cpp
void title(std::string_view str);
```

---

### cls

画面をクリアします。

```cpp
void cls(OptInt p1 = {});
```

| 値 | 色 |
|----|----|
| 0 | 白 |
| 1 | 明るい灰色 |
| 2 | 灰色 |
| 3 | 暗い灰色 |
| 4 | 黒 |

---

### redraw

再描画制御を行います。

```cpp
void redraw(int p1 = 1);
```

| 値 | 説明 |
|----|------|
| 0 | 描画予約開始（オフスクリーン描画） |
| 1 | 画面反映（Present） |

**使用例:**

```cpp
redraw(0);        // 描画開始
// ... 描画処理 ...
redraw(1);        // 画面に反映
```

---

### groll

描画基点座標を設定します。

```cpp
void groll(int scrollX, int scrollY);
```

---

### vscalemode

> ⚠️ **機能縮退（v2 描画パイプライン）:** 本命令は **API 互換のために残置されているのみ** で、
> v2 では描画パイプラインに **作用しません**。
>
> v1 では `present()` の論理→物理 拡縮で使用される補間モードを設定する命令でしたが、
> v2 では描画コマンド発行時点で論理→物理変換が完了し、`present()` は SwapChain への
> **単純転送のみ**を行うため、`vscalemode()` の指定は present 経路に影響しません
> （詳細は [移行ガイド](../MigrationGuide-HiDPI-v2.md) §4.2 参照）。

```cpp
void vscalemode(int mode);   // 互換のため受理。状態を保持するのみ。
```

| 定数 | 値 | （旧）用途 |
|------|----|------|
| `vscale_nearest` | 0 | ニアレストネイバー（ピクセルアート向け） |
| `vscale_linear`  | 1 | バイリニア |
| `vscale_aniso`   | 2 | 異方性 |

**推奨対応:**

| 状況 | 推奨対応 |
|------|---------|
| API 互換のため命令呼び出しを残したい | そのままで良い（命令自体は残置） |
| ラスタ画像転送の補間モードを変えたい | `gmode_interp()` に置き換える |
| 新規コード | 最初から `gmode_interp()` を使用する |

詳細は [移行ガイド §4.2](/HSPPP_Lib/MigrationGuide-HiDPI-v2) を参照してください。

---

## 画像操作

### gmode

画面コピーモードを設定します。

```cpp
void gmode(
    OptInt mode       = {},    // 画面コピーモード（0〜6）
    OptInt size_x     = {},    // コピーする大きさX（デフォルト: 32）
    OptInt size_y     = {},    // コピーする大きさY（デフォルト: 32）
    OptInt blend_rate = {}     // ブレンド率（0〜256）
);
```

| モード | 定数 | 説明 |
|-------|------|------|
| 0 | `gmode_copy` | 通常コピー |
| 1 | `gmode_mem` | メモリ間コピー |
| 2 | `gmode_and` | AND合成 |
| 3 | `gmode_or` | OR合成 |
| 4 | `gmode_alpha` | 半透明合成 |
| 5 | `gmode_add` | 加算合成 |
| 6 | `gmode_sub` | 減算合成 |

---

### gmode_interp

ラスタ画像転送（`picload` / `celput` / `gcopy` / `gzoom`）の補間モードを切り替えます。

```cpp
void gmode_interp(OptInt mode = {});
```

| 値 | 定数 | 用途 |
|----|------|------|
| 0 | `vscale_nearest` | NEAREST（ピクセルアート向け） |
| 1 | `vscale_linear`  | LINEAR（**既定** / 写真・図形・テキスト） |
| 2 | `vscale_aniso`   | ANISOTROPIC（拡大率が大きいとき高品質を最優先） |

**引数省略時の挙動:** `gmode_interp()` のように引数を省略した呼び出しは、補間モードを **LINEAR（既定）に強制リセット** します。
「現状維持」を期待する場合は、引数を明示してください（例: `gmode_interp(0)`）。

**使用例:**

```cpp
gmode_interp(0);            // NEAREST に変更
gcopy(1, 0, 0, 64, 64);     // NEAREST で転送

gmode_interp();             // 引数省略 → LINEAR に強制リセット
gcopy(2, 0, 0, 64, 64);     // LINEAR で転送
```

> **HSP3 後方互換性に関する注意:** v2 では `m_gmodeInterp` 既定が LINEAR に統一されたため、
> `gcopy` / `gzoom` を `mode` 省略で呼び出した場合の既定が **旧 NEAREST → 新 LINEAR** に変化しています。
> ピクセルアート用途で `mode` 省略を使っていたスクリプトは、スクリプト冒頭で `gmode_interp(0)` を一度呼んで
> NEAREST 既定に戻してください。詳細は
> [移行ガイド §2.1 / §2.2](/HSPPP_Lib/MigrationGuide-HiDPI-v2) を参照してください。
>
> `gzoom` の `mode` 引数を **明示指定** した場合は、明示値が `m_gmodeInterp` より優先されます（per-call 指定）。

---

### gcopy

画像をコピーします。

```cpp
void gcopy(
    OptInt src_id = {},    // コピー元ウィンドウID
    OptInt src_x  = {},    // コピー元X座標
    OptInt src_y  = {},    // コピー元Y座標
    OptInt size_x = {},    // コピーサイズX（省略時: gmode設定値）
    OptInt size_y = {}     // コピーサイズY（省略時: gmode設定値）
);
```

**使用例:**

```cpp
gmode(gmode_alpha, 0, 0, 128);  // 半透明モード、50%
gcopy(1, 0, 0, 64, 64);         // バッファ1から64x64をコピー
```

---

### gzoom

拡大縮小してコピーします。

```cpp
void gzoom(
    OptInt dest_w = {},    // コピー先の幅
    OptInt dest_h = {},    // コピー先の高さ
    OptInt src_id = {},    // コピー元ウィンドウID
    OptInt src_x  = {},    // コピー元X座標
    OptInt src_y  = {},    // コピー元Y座標
    OptInt src_w  = {},    // コピー元の幅
    OptInt src_h  = {},    // コピー元の高さ
    OptInt mode   = {}     // 0=高速, 1=高品質
);
```

---

## 情報取得

### ginfo

ウィンドウ関連情報を取得します。

```cpp
int ginfo(int type);
```

| 定数 | 値 | 説明 |
|------|----|------|
| `ginfo_type_mx` | 0 | マウスX座標 |
| `ginfo_type_my` | 1 | マウスY座標 |
| `ginfo_type_act` | 2 | アクティブウィンドウID |
| `ginfo_type_sel` | 3 | 描画先ウィンドウID |
| `ginfo_type_wx1` | 4 | ウィンドウ左端座標 |
| `ginfo_type_wy1` | 5 | ウィンドウ上端座標 |
| `ginfo_type_wx2` | 6 | ウィンドウ右端座標 |
| `ginfo_type_wy2` | 7 | ウィンドウ下端座標 |
| `ginfo_type_sizex` | 10 | 画面サイズX |
| `ginfo_type_sizey` | 11 | 画面サイズY |
| `ginfo_type_mesx` | 12 | カレントX座標 |
| `ginfo_type_mesy` | 13 | カレントY座標 |
| `ginfo_type_r` | 16 | 現在の描画色R |
| `ginfo_type_g` | 17 | 現在の描画色G |
| `ginfo_type_b` | 18 | 現在の描画色B |
| `ginfo_type_dispx` | 20 | ディスプレイ幅 |
| `ginfo_type_dispy` | 21 | ディスプレイ高さ |
| `ginfo_type_fps` | 28 | モニター最大リフレッシュレート(Hz) |

### 便利関数

各 `ginfo` 値に対応する便利関数も提供されています：

```cpp
int ginfo_mx();      // マウスX座標
int ginfo_my();      // マウスY座標
int ginfo_sel();     // 描画先ウィンドウID
int ginfo_sizex();   // 画面サイズX
int ginfo_sizey();   // 画面サイズY
int ginfo_r();       // 描画色R
int ginfo_g();       // 描画色G
int ginfo_b();       // 描画色B
int ginfo_fps();     // モニター最大リフレッシュレート
// ... 他多数
```

---

## Screen クラス（OOP版）

`screen()`, `buffer()`, `bgscr()` が返す軽量ハンドルクラスです。メソッドチェーンに対応しています。

```cpp
auto win = screen({.width = 640, .height = 480});
win.color(255, 0, 0)
   .boxf(0, 0, 100, 100)
   .pos(10, 10)
   .mes("Hello!");
```

### 主要メンバ関数

| メソッド | 説明 |
|---------|------|
| `id()` | ウィンドウIDを取得 |
| `valid()` | 有効なハンドルか確認 |
| `select()` | このScreenを描画先に設定（gsel相当） |
| `show()` | ウィンドウを表示（gsel id, 1 相当） |
| `hide()` | ウィンドウを非表示（gsel id, -1 相当） |
| `activate()` | 最前面でアクティブ化（gsel id, 2 相当） |
| `width()` / `height()` | サイズ取得 |

描画関連メソッドは [描画 API](/HSPPP_Lib/api/drawing) を参照してください。

---

## 定数

### 画面モードフラグ

```cpp
inline constexpr int screen_normal      = 0;    // フルカラーモード
inline constexpr int screen_palette     = 1;    // パレットモード（未実装）
inline constexpr int screen_hide        = 2;    // 非表示ウィンドウ
inline constexpr int screen_fixedsize   = 4;    // サイズ固定
inline constexpr int screen_tool        = 8;    // ツールウィンドウ
inline constexpr int screen_frame       = 16;   // 深い縁のあるウィンドウ
inline constexpr int screen_offscreen   = 32;   // 描画先として初期化
inline constexpr int screen_usergcopy   = 64;   // 描画用シェーダー (HGIMG4)
inline constexpr int screen_mode_virtual = 128; // 仮想画面（論理→物理 自動拡縮）
inline constexpr int screen_fullscreen  = 256;  // フルスクリーン（bgscr用）
```

> `screen_mode_virtual`（`0x80`）は HSPPP 拡張です。詳細は [仮想画面ガイド](/HSPPP_Lib/VirtualScreen) 参照。

### 仮想画面 補間モード定数

```cpp
inline constexpr int vscale_nearest = 0;  // ニアレストネイバー
inline constexpr int vscale_linear  = 1;  // バイリニア（既定）
inline constexpr int vscale_aniso   = 2;  // 異方性（高品質・高負荷）
```

### コピーモード定数

```cpp
inline constexpr int gmode_copy  = 0;   // 通常コピー
inline constexpr int gmode_mem   = 1;   // メモリ間コピー
inline constexpr int gmode_and   = 2;   // AND合成
inline constexpr int gmode_or    = 3;   // OR合成
inline constexpr int gmode_alpha = 4;   // 半透明合成
inline constexpr int gmode_add   = 5;   // 加算合成
inline constexpr int gmode_sub   = 6;   // 減算合成
```

---

## Screen クラス（OOP版）

`screen()`, `buffer()`, `bgscr()` が返す軽量ハンドルクラスです。メソッドチェーンに対応しています。

```cpp
auto win = screen({.width = 640, .height = 480});
win.color(255, 0, 0)
   .boxf(0, 0, 100, 100)
   .pos(10, 10)
   .mes("Hello!");
```

### 主要メンバ関数

| メソッド | 説明 |
|---------|------|
| `id()` | ウィンドウIDを取得 |
| `valid()` | 有効なハンドルか確認 |
| `select()` | このScreenを描画先に設定（gsel相当） |
| `show()` | ウィンドウを表示（gsel id, 1 相当） |
| `hide()` | ウィンドウを非表示（gsel id, -1 相当） |
| `activate()` | 最前面でアクティブ化（gsel id, 2 相当） |
| `width()` / `height()` | サイズ取得 |

描画関連メソッドは [描画 API](/HSPPP_Lib/api/drawing) を参照してください。

---

## 参照

- [描画 API](/HSPPP_Lib/api/drawing)
- [入力 API](/HSPPP_Lib/api/input)
- [型定義](/HSPPP_Lib/api/types)
