---
layout: default
title: 描画API
---

# 描画 API リファレンス

図形・テキスト・画像の描画に関するAPIです。

## 目次

- [基本描画](#基本描画)
- [図形描画](#図形描画)
- [テキスト描画](#テキスト描画)
- [画像操作](#画像操作)
- [色設定](#色設定)
- [フォント設定](#フォント設定)

---

## 基本描画

### color

描画色を設定します。

```cpp
void color(int r, int g, int b);
```

| パラメータ | 範囲 | 説明 |
|-----------|------|------|
| `r` | 0-255 | 赤成分 |
| `g` | 0-255 | 緑成分 |
| `b` | 0-255 | 青成分 |

**使用例:**

```cpp
color(255, 0, 0);   // 赤
color(0, 255, 0);   // 緑
color(0, 0, 255);   // 青
```

---

### pos

描画位置（カレントポジション）を設定します。

```cpp
void pos(int x, int y);
```

**使用例:**

```cpp
pos(100, 50);
mes("ここに表示");
```

---

### boxf

矩形を塗りつぶします。

```cpp
// 座標指定版
void boxf(int x1, int y1, int x2, int y2);

// 画面全体版
void boxf();
```

**使用例:**

```cpp
color(255, 0, 0);
boxf(10, 10, 100, 100);   // 赤い矩形

color(0, 0, 0);
boxf();                    // 画面全体を黒で塗りつぶし
```

---

## 図形描画

### line

直線を描画します。

```cpp
void line(OptInt x2 = {}, OptInt y2 = {}, OptInt x1 = {}, OptInt y1 = {});
```

- `x1, y1` 省略時: カレントポジションから描画
- 描画後、終点がカレントポジションになる

**使用例:**

```cpp
pos(0, 0);
line(100, 100);           // (0,0) から (100,100) へ
line(200, 100);           // 続けて (200,100) へ

line(50, 50, 150, 150);   // 始点・終点両方指定
```

---

### circle

円・楕円を描画します。

```cpp
void circle(
    OptInt x1       = {},   // 左上X
    OptInt y1       = {},   // 左上Y
    OptInt x2       = {},   // 右下X
    OptInt y2       = {},   // 右下Y
    OptInt fillMode = {}    // 0=線, 1=塗りつぶし
);
```

**使用例:**

```cpp
color(0, 255, 0);
circle(10, 10, 110, 110, 1);   // 塗りつぶし円
circle(150, 10, 250, 110, 0);  // 線のみ
```

---

### pset

1ドットの点を描画します。

```cpp
void pset(OptInt x = {}, OptInt y = {});
```

座標省略時はカレントポジションに描画します。

---

### pget

指定座標の色を取得し、選択色として設定します。

```cpp
void pget(OptInt x = {}, OptInt y = {});
```

取得した色は `ginfo_r()`, `ginfo_g()`, `ginfo_b()` で参照できます。

---

### gradf

矩形をグラデーションで塗りつぶします。

```cpp
void gradf(
    OptInt x      = {},    // 左上X
    OptInt y      = {},    // 左上Y
    OptInt w      = {},    // 幅
    OptInt h      = {},    // 高さ
    OptInt mode   = {},    // 0=横, 1=縦
    OptInt color1 = {},    // 開始色（0xRRGGBB）
    OptInt color2 = {}     // 終了色（0xRRGGBB）
);
```

**使用例:**

```cpp
// 横方向グラデーション（赤→青）
gradf(0, 0, 200, 100, 0, 0xFF0000, 0x0000FF);

// 縦方向グラデーション
gradf(0, 120, 200, 100, 1, 0x00FF00, 0xFFFF00);
```

---

### grect

回転する矩形で塗りつぶします。

```cpp
void grect(
    OptInt cx        = {},   // 中心X
    OptInt cy        = {},   // 中心Y
    OptDouble angle  = {},   // 回転角度（ラジアン）
    OptInt w         = {},   // 幅
    OptInt h         = {}    // 高さ
);
```

**使用例:**

```cpp
color(255, 128, 0);
grect(100, 100, deg2rad(45), 80, 40);  // 45度回転した矩形
```

---

### gsquare

任意の4頂点で描画します。

```cpp
// 単色塗りつぶし
void gsquare(int srcId, const Quad& dst);

// 画像コピー
void gsquare(int srcId, const Quad& dst, const QuadUV& src);

// グラデーション塗りつぶし
void gsquare(int srcId, const Quad& dst, const QuadColors& colors);
```

**使用例:**

{% raw %}
```cpp
// 4頂点指定で描画
Quad dst = {{0, 0}, {100, 20}, {80, 120}, {-20, 100}};
gsquare(-1, dst);  // srcId=-1 で単色塗り

// グラデーション
QuadColors colors = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
gsquare(gsquare_grad, dst, colors);
```
{% endraw %}

---

## テキスト描画

### mes / print

文字列を描画します。`print` は `mes` の別名です。

```cpp
void mes(std::string_view text, OptInt sw = {});
void print(std::string_view text, OptInt sw = {});
```

| sw | 説明 |
|----|------|
| 0 | 通常（改行あり） |
| 1 | 改行しない |
| 2 | 影付き |
| 4 | 縁取り |
| 8 | 簡易描画 |
| 16 | gmode設定使用 |

オプションは組み合わせ可能です（例: `2+4` で影+縁取り）。

**使用例:**

```cpp
color(255, 255, 255);
pos(10, 10);
mes("Hello, HSPPP!");

pos(10, 50);
mes("影付きテキスト", 2);

pos(10, 90);
mes("縁取り付き", 4);
```

---

### messize

文字列の描画サイズを取得します。

```cpp
std::pair<int, int> messize(std::string_view text);
```

**使用例:**

```cpp
auto [w, h] = messize("Sample Text");
// w: 幅, h: 高さ
```

---

## 画像操作

### picload

画像ファイルを読み込みます。

```cpp
void picload(std::string_view filename, OptInt mode = {});
```

| mode | 説明 |
|------|------|
| 0 | 画面を初期化して読み込み |
| 1 | 現在の画面に重ねる |
| 2 | 黒で初期化して読み込み |

対応形式: BMP, PNG, JPEG, GIF 等

**使用例:**

```cpp
buffer(1);
picload("image.png");

gsel(0);
gcopy(1, 0, 0, 100, 100);
```

---

### bmpsave

画面イメージをBMPファイルに保存します。

```cpp
void bmpsave(std::string_view filename);
```

---

### celload

画像ファイルをCelとしてロードします。

```cpp
int celload(std::string_view filename, OptInt celId = {});
```

**戻り値:** 割り当てられたCel ID

---

### celdiv

画像素材の分割サイズを設定します。

```cpp
void celdiv(int celId, int divX, int divY);
```

---

### celput

画像素材を描画します。

```cpp
void celput(int celId, int cellIndex, OptInt x = {}, OptInt y = {});
```

---

### loadCel（OOP版）

画像ファイルをロードしてCelオブジェクトを作成します。

```cpp
Cel loadCel(std::string_view filename, OptInt celId = {});
```

**使用例:**

```cpp
auto sprite = loadCel("character.png");
sprite.divide(4, 4);   // 4x4に分割

// 描画
sprite.put(0);         // セル0を描画
sprite.put(1, 100, 50); // セル1を(100, 50)に描画
```

---

### grotate

画像を回転してコピーします。

```cpp
void grotate(
    OptInt srcId    = {},    // コピー元ID
    OptInt srcX     = {},    // コピー元X
    OptInt srcY     = {},    // コピー元Y
    OptDouble angle = {},    // 回転角度（ラジアン）
    OptInt dstW     = {},    // 出力幅
    OptInt dstH     = {}     // 出力高さ
);
```

---

## 色設定

### hsvcolor

HSV形式でカラーを設定します。

```cpp
void hsvcolor(int h, int s, int v);
```

| パラメータ | 範囲 | 説明 |
|-----------|------|------|
| `h` | 0-191 | 色相（Hue） |
| `s` | 0-255 | 彩度（Saturation） |
| `v` | 0-255 | 明度（Value） |

---

### rgbcolor

RGB値（0xRRGGBB形式）でカラーを設定します。

```cpp
void rgbcolor(int rgb);
```

**使用例:**

```cpp
rgbcolor(0xFF0000);  // 赤
rgbcolor(0x00FF00);  // 緑
```

---

### syscolor

システムカラーを設定します。

```cpp
void syscolor(int colorId);
```

---

## フォント設定

### font

フォントを設定します。

```cpp
int font(
    std::string_view fontName,
    OptInt size            = {},    // フォントサイズ（デフォルト: 12）
    OptInt style           = {},    // スタイル
    OptInt decorationWidth = {}     // 装飾幅
);
```

| style | 説明 |
|-------|------|
| 0 | 標準 |
| 1 | 太字 |
| 2 | 斜体 |
| 4 | 下線 |
| 8 | 取り消し線 |
| 16 | アンチエイリアス |

スタイルは組み合わせ可能です。

**使用例:**

```cpp
font("MS Gothic", 24, 1);      // MSゴシック 24pt 太字
font("Arial", 16, 2 + 16);     // Arial 16pt 斜体+アンチエイリアス
```

---

### sysfont

システム標準のフォントを選択します。

```cpp
void sysfont(OptInt type = {});
```

| type | 説明 |
|------|------|
| 0 | 標準フォント |
| 17 | 固定幅フォント |

---

## Cel クラス

画像素材を扱う軽量ハンドルクラスです。

```cpp
class Cel {
    bool valid() const;           // 有効なハンドルか
    int id() const;               // Cel IDを取得
    Cel& divide(int divX, int divY);  // 分割設定
    Cel& put(int cellIndex, OptInt x = {}, OptInt y = {}); // 描画
    int width() const;            // 画像幅
    int height() const;           // 画像高さ
};
```

**使用例:**

```cpp
auto cel = loadCel("spritesheet.png");
cel.divide(8, 8);  // 8x8 = 64セルに分割

// アニメーション
for (int frame = 0; frame < 8; frame++) {
    redraw(0);
    cls(4);
    cel.put(frame, 100, 100);
    redraw(1);
    await(100);
}
```

---

## 参照

- [画面制御 API](screen.md)
- [型定義](types.md)
- [数学 API](math.md)（`deg2rad` など）
