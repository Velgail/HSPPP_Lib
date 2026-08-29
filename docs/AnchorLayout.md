---
layout: default
title: アンカーレイアウト API
---

# アンカーレイアウト API

「**画面の右下から内側に 10px**」「**画面中央に 200×100 の矩形**」のように、
**バッファのアンカー辺を基準とした相対位置**で UI を配置するための API です。

仮想画面と組み合わせることで、4:3 / 16:9 / 16:10 / 21:9 といった
さまざまなアスペクト比の表示環境に追加実装なしで適応できます。

> 関連: [HiDPI 対応](/HSPPP_Lib/HiDPI) / [仮想画面ガイド](/HSPPP_Lib/VirtualScreen)

---

## 1. 概念

すべてのアンカー解決は、**現在の描画先サーフェスのバッファサイズ** を基準に行われます。

| 仮想画面の状態 | アンカー基準サイズ |
|--------------|--------------------|
| OFF（既定） | 物理クライアント px |
| ON | 論理 px（`screen()` で指定した `width` / `height`） |

水平基準 (`AnchorH`) と垂直基準 (`AnchorV`) を組み合わせて 9 通りの基準点を表現できます。

| 定数 | 値 | 意味 |
|------|----|------|
| `ah_left`   | 0 | 左端基準 |
| `ah_center` | 1 | 中央基準 |
| `ah_right`  | 2 | 右端基準 |
| `av_top`    | 0 | 上端基準 |
| `av_middle` | 1 | 中央基準 |
| `av_bottom` | 2 | 下端基準 |

## 2. HSP 互換命令

### 2.1 `anchor_pos`

```cpp
void anchor_pos(int anchorH, int anchorV, int offsetX, int offsetY);
```

`(anchorH, anchorV)` で示される基準点から `(offsetX, offsetY)` だけずらした
座標を、`pos` と同じカレント位置に設定します。

```cpp
// 右下から内側 10px の位置に "OK" を描画
anchor_pos(ah_right, av_bottom, -10, -10);
mes("OK");
```

### 2.2 `anchor_box`

```cpp
void anchor_box(int anchorH, int anchorV, int offsetX, int offsetY, int w, int h);
```

矩形側の基準角 `(anchorH, anchorV)` をバッファ側の同一基準点に合わせ、
さらに `(offsetX, offsetY)` ずらした位置に `w × h` の矩形を塗りつぶします。

```cpp
// 画面中央に 200x100 の矩形
color(255, 200, 0);
anchor_box(ah_center, av_middle, 0, 0, 200, 100);

// 右上端から 10px 内側に 64x64 のアイコン枠
color(0, 0, 0);
anchor_box(ah_right, av_top, -10, 10, 64, 64);
```

## 3. OOP API: `AnchorRect`

`AnchorRect` 構造体を `boxf()` に渡す形式で、より宣言的に書けます。

```cpp
struct AnchorRect {
    AnchorH h_anchor = ah_left;
    AnchorV v_anchor = av_top;
    int     offset_x = 0;
    int     offset_y = 0;
    int     width    = 0;
    int     height   = 0;

    constexpr RectI resolve(int bufferW, int bufferH) const noexcept;
};
```

`resolve()` はバッファサイズから解決後の `RectI{x1, y1, x2, y2}` を返します。

### 3.1 使用例

```cpp
// HSP 互換スタイル: グローバル boxf に AnchorRect を渡す
boxf(AnchorRect{
    .h_anchor = ah_right,
    .v_anchor = av_bottom,
    .offset_x = -20,
    .offset_y = -20,
    .width    = 120,
    .height   = 40,
});

// OOP スタイル: Screen::boxf に AnchorRect を渡す（メソッドチェーン）
auto win = screen({.width = 1920, .height = 1080, .virtual_resolution = true});
win.color(255, 255, 255)
   .boxf(AnchorRect{
       .h_anchor = ah_center,
       .v_anchor = av_middle,
       .width    = 400,
       .height   = 200,
   });
```

### 3.2 `anchor_pos(...) ≡ AnchorRect{..., w=0, h=0}.resolve(...)`

`anchor_pos(ah, av, ox, oy)` は次と等価です。

```cpp
const AnchorRect r{ ah, av, ox, oy, 0, 0 };
const RectI s = r.resolve(bufferW, bufferH);
pos(s.x1, s.y1);
```

つまり「幅・高さがゼロの `AnchorRect` の左上点を解決する」操作が `anchor_pos` です。

## 4. 解決ルール（`AnchorRect::resolve`）

```cpp
switch (h_anchor) {
    case ah_left:   baseX = 0;                          break;
    case ah_center: baseX = bufferW / 2 - width  / 2;   break;
    case ah_right:  baseX = bufferW - width;            break;
}
switch (v_anchor) {
    case av_top:    baseY = 0;                          break;
    case av_middle: baseY = bufferH / 2 - height / 2;   break;
    case av_bottom: baseY = bufferH - height;           break;
}
x1 = baseX + offset_x;
y1 = baseY + offset_y;
x2 = x1 + width;
y2 = y1 + height;
```

### 4.1 中央寄せ時の ±1px の偏りについて

`ah_center` / `av_middle` の解決は整数除算 `bufferW / 2 - width / 2` を用いるため、
**バッファ幅または width が奇数の場合に表示位置が ±1px 偏る** ことがあります。

例: `bufferW = 1921`, `width = 100` のとき `baseX = 960 - 50 = 910`。
このとき右側の余白は `1921 - (910 + 100) = 911`、左側は `910` で、1px ぶん左寄りになります。

これは HSP 描画慣習（整数 px 座標）に従った仕様であり、誤差を避けたい場合は
論理サイズと width をともに偶数に揃えてください。サブピクセル精度の中央寄せは
本 API のスコープ外です。

## 5. 仮想画面との組み合わせ

仮想画面 ON / OFF いずれでも **同一 API・追加分岐なし** で機能します。
仮想画面 ON 時はバッファサイズが論理 px なので、`anchor_pos(ah_right, av_bottom, -10, -10)`
は「論理座標系での右下から内側 10px」を意味し、表示時の物理 px は拡縮率に応じて自動で
スケールされます。

```cpp
// 論理 1920x1080、物理ウィンドウは任意サイズ
auto win = screen({
    .width = 1920, .height = 1080,
    .virtual_resolution = true,
});

// 物理サイズに関わらず、論理座標で右下 -10,-10 の位置にラベル
color(255, 255, 255);
anchor_pos(ah_right, av_bottom, -10, -10);
mes("v1.0");
```

## 6. 関連型

| 型 | 説明 |
|----|------|
| `RectI` | `int x1, y1, x2, y2` を持つ整数矩形。`width()` / `height()` メソッドあり |
| `AnchorRect` | アンカー基準矩形。`resolve(bufferW, bufferH)` で `RectI` に変換 |
| `AnchorH` / `AnchorV` | アンカー基準定数（enum） |

## 7. 関連 API

| API | 説明 |
|-----|------|
| `anchor_pos(h, v, ox, oy)` | カレント位置をアンカー基準で設定 |
| `anchor_box(h, v, ox, oy, w, h)` | アンカー基準矩形を塗りつぶし |
| `boxf(const AnchorRect& rect)` | `AnchorRect` を塗りつぶし（OOP / HSP 共用） |
| `Screen::anchor_pos / anchor_box / boxf(AnchorRect)` | OOP 版（メソッドチェーン対応） |

---

## 参照

- [HiDPI 対応](/HSPPP_Lib/HiDPI)
- [仮想画面ガイド](/HSPPP_Lib/VirtualScreen)
- [描画 API](/HSPPP_Lib/api/drawing)
- [型定義リファレンス](/HSPPP_Lib/api/types)
