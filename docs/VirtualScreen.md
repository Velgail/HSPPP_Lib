---
layout: default
title: 仮想画面（論理→物理 自動拡縮）
---

# 仮想画面ガイド

「**1920×1080 を前提にレイアウトを書けば、4K でも 8K でも、
21:9 ウルトラワイドでも、自動的にきれいに拡縮表示される**」を実現する仕組みです。

> 関連: [HiDPI 対応](/HSPPP_Lib/HiDPI) / [アンカーレイアウト API](/HSPPP_Lib/AnchorLayout)

---

## 1. 概念

仮想画面を有効化すると、ウィンドウは次の 2 つのサイズを持つようになります。

| 名称 | 意味 |
|------|------|
| 論理サイズ | ユーザコードが基準にする座標系（例: `1920 × 1080`） |
| 物理クライアントサイズ | OS から見た実際のクライアント px（ウィンドウサイズ + DPI 倍率） |

描画コマンド（`boxf`, `mes`, `line`, `circle` …）はすべて **論理サイズ** の座標で発行します。
ライブラリは内部のオフスクリーンビットマップに論理サイズで描画したあと、
`present()` で SwapChain のバックバッファへ **アスペクト比を保ったまま拡縮転送** します。
余白はレターボックス / ピラーボックスになります。

マウス座標（`ginfo_mx` / `ginfo_my`）も、仮想画面 ON 時は **論理 px** が返されます。

## 2. 有効化方法

### 2.1 OOP スタイル（推奨）

```cpp
import hsppp;
using namespace hsppp;

void hspMain() {
    // 論理 1920x1080 で設計、物理ウィンドウは 1280x720 で表示
    auto win = screen({
        .width    = 1920,
        .height   = 1080,
        .client_w = 1280,
        .client_h = 720,
        .title    = "Virtual Screen Demo",
        .virtual_resolution = true,
    });

    color(255, 255, 255);
    boxf(0, 0, 1920, 1080);     // 論理座標で全面塗りつぶし

    color(255, 0, 0);
    boxf(100, 100, 400, 300);   // 論理座標 100,100 〜 400,300

    redraw();
}
```

### 2.2 HSP 互換スタイル

`mode` ビットフラグに `screen_mode_virtual`（値: `128`）を含めます。

```cpp
import hsppp;
using namespace hsppp;

void hspMain() {
    // ID 0 を screen_mode_virtual で初期化
    screen(0, 1920, 1080, screen_mode_virtual);
    title("Virtual Screen (HSP style)");

    color(255, 255, 255);
    boxf(0, 0, 1920, 1080);

    color(0, 128, 255);
    boxf(100, 100, 400, 300);

    redraw();
}
```

> `screen_mode_virtual = 128`（`0x80`）は HSPPP 拡張の画面モードフラグです。
> 既存ビット 64 (`screen_usergcopy`) と衝突しないように選定されています。

## 3. 補間モードの切替

論理→物理 拡縮時の補間アルゴリズムを `vscalemode()` で切り替えられます。

```cpp
vscalemode(vscale_linear);    // バイリニア（既定 / 写真・滑らかな描画向け）
vscalemode(vscale_nearest);   // ニアレストネイバー（ピクセルアート向け）
vscalemode(vscale_aniso);     // 異方性（高品質・高負荷）
```

| 定数 | 値 | 用途 |
|------|----|------|
| `vscale_nearest` | 0 | ドット絵・ピクセルアート |
| `vscale_linear`  | 1 | 既定。写真・図形・テキスト |
| `vscale_aniso`   | 2 | 拡大率が大きく品質を最優先する場合 |

`vscalemode()` は仮想画面 OFF 時に呼び出しても状態を保持するだけで描画には影響しません。

## 4. 座標変換モデル

仮想画面 ON 時、論理座標 `(lx, ly)` から物理クライアント座標 `(px, py)` への変換は次の通りです。

```text
sx = physClientW / float(logicalW)
sy = physClientH / float(logicalH)
s  = min(sx, sy)                       // uniform scale（アスペクト維持）
offsetX = (physClientW - logicalW * s) * 0.5
offsetY = (physClientH - logicalH * s) * 0.5

px = offsetX + lx * s
py = offsetY + ly * s
```

逆変換（マウス座標などで使用）:

```text
lx = (px - offsetX) / s
ly = (py - offsetY) / s
```

物理クライアントサイズには **DPI 倍率がすでに含まれている** ため、
追加で DPI 補正を掛ける必要はありません（[HiDPI ガイド](/HSPPP_Lib/HiDPI) §1 参照）。

## 5. 仮想画面 OFF 時との挙動差

| 項目 | OFF（既定） | ON |
|------|------------|----|
| `boxf(0,0,W,H)` の意味 | 物理クライアントの (0,0)〜(W,H) | 論理 (0,0)〜(W,H)（拡縮後に物理画面へ転送） |
| `width` / `height` の意味 | 物理クライアント px | 論理 px |
| `client_w` / `client_h` | （無視されることが多い） | 物理クライアント px の初期サイズ |
| `ginfo_mx` / `ginfo_my` | 物理クライアント px | 論理 px |
| `picload` / `bmpsave` / `celload` | 物理 px | **論理 px**（バッファ自体が論理座標空間） |
| 余白 | なし | レターボックス / ピラーボックス（クリア色は黒） |

`picload` 等のラスタ画像入出力は仮想画面 ON 時も **論理 px** で扱われます。
バッファ自体が論理座標空間であるため、HSP 既存仕様（バッファ座標基準）と整合します。

## 6. レターボックス / ピラーボックス

ウィンドウのアスペクト比と論理サイズのアスペクト比が一致しない場合、
アスペクトを保ったまま拡縮するため、上下または左右に余白が生じます。

- 論理 16:9 (1920×1080) → 物理 21:9 ウィンドウ: **左右にピラーボックス**
- 論理 16:9 (1920×1080) → 物理 4:3 ウィンドウ: **上下にレターボックス**

余白部分は背景色でクリアされ、論理座標 (0,0)〜(W,H) の外側はマウス座標として
論理範囲外の値（負値や `width` / `height` を超える値）が返ることがあります。

## 7. ラスタ画像のスコープ外事項

仮想画面で拡縮されるのは **ベクトル描画コマンドの結果（オフスクリーンビットマップ）**
であり、`picload` / `celload` で読み込んだ素材自体は元解像度のまま扱われます。
論理 1920×1080 で設計したラスタ素材を 4K で表示すると `vscalemode` の補間
で拡大されるため、ピクセルアート以外では `vscale_linear` 以上の品質モードを推奨します。

高 DPI でもクリアに見せたいラスタ素材は、利用者側で高解像度版を用意してください。

## 8. 関連 API

| API | 説明 |
|-----|------|
| `screen({.virtual_resolution = true})` | OOP 版で仮想画面を有効化 |
| `bgscr({.virtual_resolution = true})` | 枠なしウィンドウで仮想画面を有効化 |
| `screen_mode_virtual` (=128) | HSP 互換 `mode` ビットフラグ |
| `vscalemode(int mode)` | 拡縮補間モード切替 |
| `anchor_pos` / `anchor_box` / `AnchorRect` | 解像度独立のレイアウト記述（[AnchorLayout](/HSPPP_Lib/AnchorLayout)） |

---

## 参照

- [HiDPI 対応](/HSPPP_Lib/HiDPI)
- [アンカーレイアウト API](/HSPPP_Lib/AnchorLayout)
- [画面制御 API](/HSPPP_Lib/api/screen)
