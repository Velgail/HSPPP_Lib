---
layout: default
title: 画面API
---

# 画面制御 API リファレンス

ウィンドウとバッファの作成・制御に関するAPIです。

## 目次

- [ウィンドウ作成](#ウィンドウ作成)
- [画面制御](#画面制御)
- [画面情報取得](#画面情報取得)
- [定数](#定数)

---

## ウィンドウ作成

### screen

ウィンドウを初期化します。

```cpp
// OOP版（ID自動採番）
[[nodiscard]] Screen screen(const ScreenParams& params);
[[nodiscard]] Screen screen();

// HSP互換版（ID明示指定）
Screen screen(
    int id,
    OptInt width    = {},      // 画面サイズX（デフォルト: 640）
    OptInt height   = {},      // 画面サイズY（デフォルト: 480）
    OptInt mode     = {},      // 画面モード（screen_* フラグ）
    OptInt pos_x    = {},      // ウィンドウ位置X（-1=システム規定）
    OptInt pos_y    = {},      // ウィンドウ位置Y（-1=システム規定）
    OptInt client_w = {},      // クライアントサイズX（0=widthと同じ）
    OptInt client_h = {},      // クライアントサイズY（0=heightと同じ）
    std::string_view title = "HSPPP Window"
);
```

**使用例:**

```cpp
// HSP互換スタイル
screen(0, 800, 600);

// OOP版（構造体）
auto win = screen({.width = 800, .height = 600, .title = "My App"});

// OOP版（デフォルト設定）
auto win = screen();
```

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

## 画面制御

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

### title

ウィンドウタイトルを設定します。

```cpp
void title(std::string_view str);
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

### groll

描画基点座標を設定します。

```cpp
void groll(int scrollX, int scrollY);
```

---

## 画面情報取得

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
// ... 他多数
```

---

## 定数

### 画面モードフラグ

```cpp
inline constexpr int screen_normal     = 0;    // フルカラーモード
inline constexpr int screen_palette    = 1;    // パレットモード（未実装）
inline constexpr int screen_hide       = 2;    // 非表示ウィンドウ
inline constexpr int screen_fixedsize  = 4;    // サイズ固定
inline constexpr int screen_tool       = 8;    // ツールウィンドウ
inline constexpr int screen_frame      = 16;   // 深い縁のあるウィンドウ
inline constexpr int screen_offscreen  = 32;   // 描画先として初期化
inline constexpr int screen_fullscreen = 256;  // フルスクリーン（bgscr用）
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

描画関連メソッドは [描画 API](drawing.md) を参照してください。

---

## 参照

- [描画 API](drawing.md)
- [入力 API](input.md)
- [型定義](types.md)
