---
layout: default
title: HiDPI / 仮想画面 v2 移行ガイド
---

# HiDPI / 仮想画面 v2 移行ガイド

HSPPP の **HiDPI / 仮想画面 v2 描画パイプライン改修** に伴う、後方互換性影響と
移行手順を 1 ページに集約したガイドです。本改修は **ライブラリ実装本体**（`Surface.cpp` /
`Window.cpp` / `hsppp_*.inl` 系）と docs の両方を新ポリシーに整合させたものであり、
Sample（HspppSample / HspppStateSample）のみの修正で済むものではありません。

> 関連: [HiDPI 対応](HiDPI.md) / [仮想画面ガイド](VirtualScreen.md) / [アンカーレイアウト API](AnchorLayout.md)

---

## 1. 何が変わったか（要点）

| 観点 | v1（旧） | v2（新） |
|------|---------|----------------------|
| オフスクリーンビットマップ（`m_pTargetBitmap`） | **論理サイズ** で保持 | **物理サイズ**（letterbox 内有効描画領域）で保持 |
| 描画コマンドの実行点 | 論理 px でオフスクリーンに描画 | 描画コマンド発行時点で `SetDpi(96)` + `SetTransform(Scale(s))` により論理→物理変換、物理 px で `m_pTargetBitmap` に書込 |
| `present()` の責務 | 論理→物理 アスペクト比保持の **拡縮転送** | SwapChain への **単純転送のみ**（拡縮なし） |
| 余白（letterbox / pillarbox） | present の拡縮転送で生成 | 背景色クリア + オフスクリーンビットマップを内側にオフセット配置 |
| テキスト副パイプライン | `mes` / `font` / `sysfont` に専用経路 | **廃止**（他描画 API と同経路に統一 / DWrite サブピクセル AA が最終物理解像度で動作） |
| `font_mode_buffer` 命令 | 新設候補として言及 | **新設せず**（PM Q-H 完全廃止裁定） |
| `vscalemode()` | present 拡縮時の補間モード切替 | **機能縮退**（present が単純転送のため作用しない / API は残置） |
| 線幅指定 | ハードコード `1.0f` | **新規命令 `gline_width(w)`**（論理 px / 既定 1.0） |
| ラスタ補間モード | ハードコード LINEAR / `gcopy` `gzoom` は NEAREST | **新規命令 `gmode_interp(mode)`**（D2D転送用）。`gzoom` のp8省略値はHSPどおり0 |

ユーザー IF（描画コマンドの座標単位）は **論理 px のまま** であり、既定挙動（仮想画面 OFF）
では既存コードは無変更で動作します。ただし、いくつかの **後方互換性に関する注意点**
があります（§3 以降）。

---

## 2. HSP互換値とHspppLib拡張の境界（最重要）

### 2.1 `gzoom` のp8省略値

`gzoom` のp8（C++ APIの `mode`）を省略した場合は、HSPどおり0（補間なし）です。
`gmode_interp()` の現在値を暗黙に使う仕様ではありません。

```cpp
gmode_interp(1);  // D2D転送をLINEARへ

// p8省略: HSPどおり0。上の設定を暗黙には使わない
gzoom(128, 128, 1, 0, 0, 64, 64);

// HspppLib拡張: -1を明示した時だけgmode_interpを使う
gzoom(128, 128, 1, 0, 0, 64, 64, -1);
```

HSPのp8=1はLINEAR、HspppLib拡張のp8=2はANISOTROPICです。

### 2.2 `gmode_interp` 引数省略時は LINEAR 強制リセット

```cpp
gmode_interp(0);    // NEAREST に変更
gcopy(1, 0, 0, 64, 64);  // NEAREST で転送

gmode_interp();     // 引数省略 → LINEAR にリセット
gcopy(2, 0, 0, 64, 64);  // LINEAR で転送
```

「現状維持」を期待する場合は引数を明示してください（`gmode_interp(0)` 等）。

---

## 3. 内部実装変更による細部挙動の差異

ユーザー IF の意味論は維持されていますが、内部が「論理 px 保持」から「物理 px 保持」に
変わったため、以下の API には細部挙動の注意点があります。

### 3.1 `pget(x, y)` の往復一致性

**仕様:** `pget(x, y)` は論理 px 座標で色値を返します（IF 互換）。

**内部実装:** 仮想 ON / DPI≠96 時、「論理 1 px = 物理 `s × s` 領域」となるため、
中心 1 物理 px の代表値を返します。

**注意:**

```cpp
int c = pget(10, 10);   // 物理座標 round(10*s) の 1 px 取得
pset(c, 10, 10);        // 物理座標 round(10*s) の 1 px 書込

int c2 = pget(10, 10);  // 通常は c2 == c になるが、s が非整数の場合は
                        // 補間境界の影響で僅差が生じうる
```

HSP3 公式実装でも非整数倍率時は同様に近似動作するため、互換性として致命的ではありません。
ピクセルパーフェクト保証が必要な場合は仮想画面 OFF + DPI 100% で運用してください。

### 3.2 `bmpsave(filename)` の解像度

**仕様:** 論理 px サイズの BMP として保存します（HSP3 互換）。

**内部実装:** 物理 px の `m_pTargetBitmap` を論理サイズへ D2D `DrawBitmap`（LINEAR）で
ダウンサンプリングしてから WIC で出力します。

**注意:** HiDPI 環境で **物理解像度の情報** を保存したい場合（スクリーンショット高品質化等）、
現状の `bmpsave` では論理サイズに丸められます。物理サイズ保存 API（`bmpsave_phys` 等）は
将来検討事項です。

### 3.3 `gcopy` のサブピクセル位置

**仕様:** 論理座標 `srcX, srcY, srcW, srcH` で指定（IF 互換）。

**内部実装:** 内部で `*s` 倍して物理 px に変換してから D2D `DrawBitmap` を発行します。
`*s` が非整数になる場合、コピー元矩形は物理 px 境界に整列しません。

**注意:** D2D の `DrawBitmap` は LINEAR 補間で滑らかに扱いますが、ピクセルアート用途で
1 px のズレが問題化する場合は `gmode_interp(0)` で NEAREST 指定してください。

### 3.4 `redraw 0` フレーム間累積描画

**変更なし。** 物理 px の `m_pTargetBitmap` に累積描画され、`redraw 1` で `presentInternal`
が呼ばれた時点で SwapChain に転送されます。`redraw 0` のセマンティクスは v1 と同等です。

### 3.5 `mes` 結果のオフスクリーン蓄積

**変更なし。** v2 でも `mes` の描画結果は他描画 API と同じく `m_pTargetBitmap`（物理 px）に
蓄積されます。`gcopy` ソースとして引き続き使用可能であり、v1 設計仮で想定された「副パイ
プライン廃止に伴うテキストバッファ破壊」は **発生しません**。

### 3.6 HiDPI 環境でのテキスト品質向上

v2 ではテキスト副パイプラインが廃止され、`mes` / `font` / `sysfont` の描画も
`m_pTargetBitmap`（物理 px）に直接書込まれるため、DWrite のサブピクセル AA が最終物理
解像度で動作します。

- **HiDPI（DPI 150% / 200% / 4K 等）**: テキスト品質が v1 比で **向上**
- **100% DPI 環境**: 従来と差なし

---

## 4. 削除・廃止された機能

### 4.1 `font_mode_buffer` 命令（新設取りやめ）

v1 設計仮では HiDPI 環境でテキスト品質を維持する補助命令として `font_mode_buffer(int onoff)`
の新設が言及されていましたが、v2 で副パイプラインそのものを廃止し物理 px 経路に
統一したため、本命令は **新設されませんでした**（完全廃止）。

旧設計案を参考にしてオプトイン経路の利用を検討されていた場合、その記述は **無効** です。
v2 では追加コードなしで HiDPI 品質が自動的に得られます。

### 4.2 `vscalemode()` 機能縮退

v1 では `present()` の拡縮時に使用される D2D 補間モード（NEAREST / LINEAR / ANISOTROPIC）を
切り替える命令でしたが、v2 では `present()` が単純転送のみとなったため、`vscalemode()` の
指定は present 経路では **作用しません**。

| 状況 | 推奨対応 |
|------|---------|
| API 互換のため命令呼び出しを残したい | そのままで良い（命令自体は残置） |
| ラスタ画像転送の補間モードを変えたい | `gmode_interp()` に置き換える |
| 新規コード | 最初から `gmode_interp()` を使用 |

---

## 5. 新規命令

### 5.1 `gline_width(float w)`

線描画（`line` / `circle` / `pset`）の strokeWidth を **論理 px** で指定します。

```cpp
gline_width(2.5f);  // 以降の line / circle / pset の線幅を 2.5 論理 px に
line(0, 0, 100, 100);

gline_width(0);     // w <= 0 は 1.0f に自動クランプ
```

| 項目 | 仕様 |
|------|------|
| 引数 | `float w`（論理 px 幅 / 既定 1.0f） |
| 境界値 | `w <= 0` は 1.0f にクランプ |
| 反映先 | `line` / `circle`（`DrawEllipse`） / `pset` の strokeWidth |
| 仮想画面 / DPI との関係 | strokeWidth は **論理 px** として渡され、D2D が `SetTransform(Scale(s))` により `s` 倍に物理化する。例: `gline_width(1)` は仮想 ON / DPI 200% 時に物理 `1 × s` px の太さで描画される |

### 5.2 `gmode_interp(int mode)`

D2Dラスタ画像転送（`picload` / 変形 `celput` / `gcopy`）の補間モードを切り替えます。

```cpp
gmode_interp(0);    // NEAREST（ピクセルアート）
gmode_interp(1);    // LINEAR（既定）
gmode_interp(2);    // ANISOTROPIC（高品質・高負荷）
gmode_interp();     // 引数省略 = LINEAR 強制リセット
```

| 値 | 定数（D2D1_INTERPOLATION_MODE） | 用途 |
|----|--------------------------------|------|
| 0 | NEAREST_NEIGHBOR | ドット絵・ピクセルアート |
| 1 | LINEAR（既定） | 写真・図形・テキスト |
| 2 | ANISOTROPIC | 拡大率が大きく品質を最優先する場合 |

`gzoom` は常にp8指定を優先します。省略時は0、`-1` を明示した場合だけ `gmode_interp` を使います。

---

## 6. 移行チェックリスト

既存 HSP / HSPPP プロジェクトを v2 に移行する際の確認項目です。

- [ ] **`gzoom` 利用**: p8省略はHSPどおり0。`gmode_interp` を反映したい箇所だけ拡張値 `-1` を明示する
- [ ] **`vscalemode()` 利用**: 命令呼出は残しても害はないが、新規コードでは
      `gmode_interp()` に置き換える
- [ ] **`font_mode_buffer` を期待していた場合**: コード上にダミー定義等を置いていれば削除可。
      v2 では追加コードなしで HiDPI 品質が得られる
- [ ] **`bmpsave` でスクリーンショットを保存**している場合: HiDPI 環境では論理サイズに
      丸められる点を認識する（必要なら物理サイズ別途取得を検討）
- [ ] **`pget` → `pset` 往復**で完全一致を期待するロジックがある場合: 仮想 OFF + DPI 100%
      環境での運用を推奨
- [ ] **線幅を変更したい**場合: `gline_width(w)` を導入
- [ ] **テキストの見え方**: HiDPI 環境では v2 で品質向上（差を確認する場合は 100% DPI と
      150% DPI の両方で動作確認）

---

## 7. 設計方針の要約

本改修は、プロジェクトオーナーから提示された次の方針に基づいて確定されました:

> 描画コマンドは論理サイズ座標で発行 / ライブラリは内部のオフスクリーンビットマップに
> **物理サイズで拡縮描画** / `present()` で SwapChain バックバッファへ **単純転送** /
> 余白はレターボックス・ピラーボックス

ライブラリ実装本体（`Surface.cpp` / `Window.cpp` / `hsppp_*.inl` 系）と本ドキュメント群の
**両方** が新ポリシーに整合した状態で、本ガイドが発行されています。Sample のみの修正で
済む扱いではありません。

---

## 参照

- [HiDPI 対応](HiDPI.md)
- [仮想画面ガイド](VirtualScreen.md)
- [アンカーレイアウト API](AnchorLayout.md)
- [CHANGELOG](../CHANGELOG.md)
