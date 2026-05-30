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
ライブラリは描画コマンド発行時点で論理座標を **物理ピクセル** に変換し、
ウィンドウのレターボックス内有効描画領域と同じ **物理サイズ** のオフスクリーンビットマップへ
直接描画します。`present()` は SwapChain のバックバッファへ **単純転送するだけ** で、
拡縮処理は行いません。余白はレターボックス / ピラーボックスとして背景色で塗られ、
オフスクリーンビットマップは余白の内側にオフセット配置されます。

これにより、テキスト（DWrite）や線描画は最終物理解像度で直接ラスタライズされ、
HiDPI 環境でもサブピクセル精度のアンチエイリアスが効きます。

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

オフスクリーンビットマップは物理サイズで保持され、`present()` は単純転送のみを行うため、
**仮想画面の「論理→物理」変換時に追加の補間処理は発生しません**。代わりに、
`picload` / `celput` / `gcopy` / `gzoom` 等の **ラスタ画像転送系コマンド** で使用される
D2D 補間モードを `gmode_interp()` で切り替えます（仮想画面 OFF 時も同じ命令で制御可能）。

```cpp
gmode_interp(1);    // LINEAR（既定 / 写真・滑らかな描画向け）
gmode_interp(0);    // NEAREST（ピクセルアート向け）
gmode_interp(2);    // ANISOTROPIC（拡大率が大きく品質を最優先する場合）
```

| 値 | 意味 | 用途 |
|----|------|------|
| 0 | NEAREST | ドット絵・ピクセルアート |
| 1 | LINEAR（既定） | 写真・図形・テキスト |
| 2 | ANISOTROPIC | 拡大率が大きく品質を最優先する場合 |

> **後方互換性に関する重要な変更:** `gcopy` / `gzoom` を `mode` 省略で
> 呼び出した場合の既定補間モードは、旧 NEAREST から **新 LINEAR** に変更されました。
> 詳細と従来挙動への復帰方法は [移行ガイド](MigrationGuide-HiDPI-v2.md) を参照してください。

> **`vscalemode()` について（v2 描画パイプラインで機能縮退）:** 従来は present 時の論理→物理
> 拡縮補間モードを切り替える命令でしたが、v2 では present が単純転送になったため、
> `vscalemode()` の指定は present 経路では作用しません。命令自体は後方互換のため
> 残置されていますが、新規コードでは `gmode_interp()` の利用を推奨します。

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
| `picload` / `bmpsave` / `celload` | 物理 px | **論理 px**（ユーザ IF は論理 px 座標／内部の `m_pTargetBitmap` は物理 px で保持し、転送時に DPI スケーリングが自動適用される）[^impl-physical] |
| 余白 | なし | レターボックス / ピラーボックス（クリア色は黒） |

[^impl-physical]: 内部実装は v2 描画パイプライン改修で「オフスクリーン物理 px / 描画時点で論理→物理スケール変換」に改修されました。利用者から見える IF（座標単位）は **論理 px のまま** ですが、`pget` / `bmpsave` 等の細部挙動には注意点があります。詳細は [移行ガイド](MigrationGuide-HiDPI-v2.md) を参照してください。

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

仮想画面で扱われるオフスクリーンビットマップは **物理サイズ** で保持されるため、
`boxf` / `line` / `circle` / `mes` 等のベクトル描画コマンドは最終物理解像度で直接
ラスタライズされます（HiDPI 環境でもサブピクセル AA が効きます）。

一方、`picload` / `celload` で読み込んだ **ラスタ素材自体** は元解像度のまま扱われます。
論理 1920×1080 で設計したラスタ素材を 4K で表示する場合、`gmode_interp()` で指定した
補間モード（既定 LINEAR）で拡大されるため、ピクセルアート以外では LINEAR 以上の品質
モードを推奨します。

高 DPI でもクリアに見せたいラスタ素材は、利用者側で高解像度版を用意してください。

## 8. 関連 API

| API | 説明 |
|-----|------|
| `screen({.virtual_resolution = true})` | OOP 版で仮想画面を有効化 |
| `bgscr({.virtual_resolution = true})` | 枠なしウィンドウで仮想画面を有効化 |
| `screen_mode_virtual` (=128) | HSP 互換 `mode` ビットフラグ |
| `gmode_interp(int mode)` | ラスタ画像転送の補間モード切替（0=NEAREST / 1=LINEAR / 2=ANISOTROPIC、既定 LINEAR）。引数省略時は LINEAR にリセット |
| `gline_width(float w)` | 線描画の幅指定（論理 px / 既定 1.0、`w <= 0` は 1.0 にクランプ） |
| `vscalemode(int mode)` | （v2 で機能縮退）API 後方互換のため残置。新規コードでは `gmode_interp()` を推奨 |
| `anchor_pos` / `anchor_box` / `AnchorRect` | 解像度独立のレイアウト記述（[AnchorLayout](/HSPPP_Lib/AnchorLayout)） |

---

## 参照

- [HiDPI 対応](/HSPPP_Lib/HiDPI)
- [アンカーレイアウト API](/HSPPP_Lib/AnchorLayout)
- [画面制御 API](/HSPPP_Lib/api/screen)
- [移行ガイド（HiDPI / 仮想画面 v2）](MigrationGuide-HiDPI-v2.md)
