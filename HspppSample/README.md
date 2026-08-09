# HspppSample - HSPPP 機能デモアプリケーション

HSPPP の各機能を実機で確認するためのデモアプリです。`UserApp.cpp` のメインループから、カテゴリ / デモ ID に応じた個別の描画関数 (`DemoDraw*.cpp`) にディスパッチします。

## 操作概要

| キー | 動作 |
|------|------|
| F1 | ヘルプウィンドウ 表示 / 非表示 |
| 1-9 | 基本デモ選択 |
| Ctrl + 0-9 / - / = | 拡張デモ選択 |
| Shift + 1-4 | 画像関連デモ選択 |
| Alt + 1-5 | 割り込みデモ選択 |
| ESC | 終了 |

カテゴリ・デモ ID の詳細は `DemoState.h` を参照してください。各デモ画面でのアクションは、修飾キーなしの文字キー（画面遷移キーと別系統）で実行します。

## 表示系デモ (`DemoDrawDisplay.cpp`)

`Display::HiDPI / Virtual / Anchor` の 3 サブデモがあります。

### HiDPI デモ

- 初期化サイズ（論理 px）、現クライアント、推定物理 px、プライマリモニタ論理 px、`ginfo_mx/my` のデスクトップ座標と `mousex/y` の論理クライアント座標を表示。
- `WM_DPICHANGED` 受信回数 / 最終通知 DPI / 末尾 8 件ログを表示。

### Virtual デモ

`g_virtScalingScreen`（論理 640×480 / `virtual_resolution = true`）を Virtual サブウィンドウとして表示し、実行時にホットキーで切替できます。

| キー | 動作 |
|------|------|
| S / Shift+S | 物理サイズプリセット 次 / 前 |
| M | `vscalemode` 切替 (nearest / linear / aniso) |
| C | `letterboxColor` 切替 |
| V | サブウィンドウ 表示 / 非表示 |
| I | ラスタ補間 (`gmode_interp`) nearest / linear / aniso 切替（サンプルは拡張p8=-1で `gzoom` へ明示反映） |
| F | FHD 基準 (1920×1080) へ即時切替 |
| R | 既定値リセット (1920×1080 / linear / dark cyan / interp=linear) |

#### 描画内容の見どころ

- **線幅スケール**: `gline_width(1/2/4/8)` + `line` で論理 px 線幅を描画。DPI / 仮想倍率に応じて物理 px へ自動拡縮されます (`gline_width` 機能)。
- **ラスタ転送**: `gcopy` (等倍) + `gzoom` (2.5 倍) を並置。`gmode_interp` 切替によって nearest（モザイク）と linear / aniso（平滑化）の差が可視化されます (`gmode_interp` 機能)。
- **フォント品質**: 10 / 14 / 18px の複数サイズを並置し、DWrite ネイティブ HiDPI 経路の品質を目視確認できます (テキスト副パイプライン廃止 / DWrite 統一経路)。
  - なお `font_mode_buffer` 命令は v2 描画パイプライン設計裁定により廃止されたため、新挙動の固定経路のみ提供しています（旧挙動切替はありません）。詳細は `docs/Migration.md` を参照。

### マルチウィンドウ + 異 DPI モニタ 確認手順

このサンプルは、メインウィンドウと Virtual サブウィンドウの 2 ウィンドウ構成です。両者を **異なる DPI のモニタへ独立してドラッグ** することで、ウィンドウごとの `WM_DPICHANGED` と描画スケールの追従挙動を観察できます。

#### 手順

1. アプリ起動後、`8` キー等で `Display` カテゴリへ移動し、`Virtual` サブデモを開く（自動的にサブウィンドウが可視化されます）。
2. メインウィンドウを 100% DPI モニタに、Virtual サブウィンドウ（タイトル「Virtual Scaling Demo (HSPPP Display Demo)」）を 150% / 200% モニタへドラッグ。
3. `HiDPI` サブデモへ切替（メインウィンドウ側のデモタブ）、`WM_DPICHANGED 受信回数` と末尾 8 件ログでメインウィンドウ側の DPI 遷移を確認。
4. Virtual サブウィンドウ側では描画パターン（線幅・フォント・ラスタ転送）が DPI に応じて物理 px で正しく拡縮されること、`letterbox` 帯が論理 640×480 と物理アスペクト比の差として現れることを確認。
5. 逆方向（メインを高 DPI、サブを低 DPI）でも同様に観察。

#### 期待される挙動

- 各ウィンドウは独立した `m_currentDpi` を持ち、`WM_DPICHANGED` 受信のたびに `gline_width` / `gmode_interp` / フォント描画は新 DPI 基準で再合成される。
- `gsel` 等によるサーフェス切替後の描画は、各ウィンドウの現在 DPI で正しくスケールされる。
- letterbox 領域は `letterboxColor` で設定した色で塗りつぶされる（既定 = 濃シアン）。

#### 不具合発覚時の対応

異 DPI モニタ間ドラッグで新規不具合が発覚した場合は、**サンプル側 / ライブラリ側いずれも独自修正を加えず**、PM へ報告して追加 ticket 起票を依頼してください（Sprint 範囲膨張防止 / PM Q-5 (c)）。

### Anchor デモ

`DemoDrawDisplayAnchor.cpp` 参照。`A` キー等でアスペクト比プリセットを切替可能。

## ファイル構成

| ファイル | 内容 |
|----------|------|
| `UserApp.cpp` | `hspMain` / グローバル状態 / メインループ / 入力ディスパッチ |
| `DemoState.h` | カテゴリ / デモ ID enum / 状態構造 |
| `DemoDrawBasic.cpp` | 基本描画 (line / boxf / circle / mes / scroll 等) |
| `DemoDrawDisplay.cpp` | HiDPI / Virtual / Anchor サブデモ |
| `DemoDrawDisplayAnchor.cpp` | Anchor Playground 実装 |
| `DemoDrawExtended.cpp` | 拡張デモ |
| `DemoDrawGUI.cpp` | GUI オブジェクト |
| `DemoDrawImage.cpp` | 画像系 |
| `DemoDrawInterrupt.cpp` | 割り込み / oncmd / onkey |
| `DemoDrawMedia.cpp` | メディア |

## 関連ドキュメント

- `docs/HiDPI.md` - HiDPI 対応設計
- `docs/VirtualScreen.md` - 仮想画面（論理→物理 自動拡縮）
- `docs/AnchorLayout.md` - アンカー基準レイアウト
- `docs/Migration.md` - HSP3 からの移行ガイド（`font_mode_buffer` 廃止等を含む）
