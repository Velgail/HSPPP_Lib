---
layout: default
title: HiDPI 対応
---

# HiDPI 対応ガイド

HSPPP は HiDPI ディスプレイ（4K / 8K / ノートPCの高解像度パネル等）に標準対応しています。
本ページではその仕組みと、利用側で必要な手順を説明します。

> 関連: [仮想画面ガイド](/HSPPP_Lib/VirtualScreen) / [アンカーレイアウト API](/HSPPP_Lib/AnchorLayout)

---

## 1. 概要

HSPPP は **Per Monitor V2 DPI awareness** を採用しています。
ライブラリ側が初期化時に DPI awareness を有効化するため、利用者は通常、
追加コードを書かなくても「ぼやけない」描画が得られます。

- すべての描画コマンド（`boxf`, `mes`, `line`, `circle` 等）は **物理ピクセル** で発行されます。
  仮想画面（[VirtualScreen](/HSPPP_Lib/VirtualScreen)）を有効化した場合は **論理ピクセル** が用いられます。
- マウス座標（`ginfo_mx` / `ginfo_my`）はクライアント座標系（仮想画面有効時は論理 px）として返されます。
- DPI 変更（ウィンドウのモニター移動、システム設定変更）は `WM_DPICHANGED` で自動追従します。

## 2. ライブラリ側の挙動

ライブラリは `init_system()` 内で以下を試行します。

1. `SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)`
2. 失敗時は `DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE`
3. それも失敗時は `DPI_AWARENESS_CONTEXT_SYSTEM_AWARE`
4. すべて失敗した場合は OS 既定挙動（DPI Unaware）で続行

この API は Windows 10 1703 以降で提供されます。
それ以前の環境では関数が解決されず、上記 fallback も働きません。

## 3. 利用側 .exe で必要な手順

### 3.1 通常ケース

**特別な手順は不要です。** `hspMain()` を書いて `HspppLib` をリンクするだけで、
Per Monitor V2 が有効になります。

### 3.2 より厳密に保証したい場合（オプション）

DPI awareness は **ウィンドウクラス登録より前**、できれば **プロセス起動の最初期**で
宣言された方が確実です。ライブラリの API 呼出は `WinMain` 冒頭で行われますが、
利用側で `app.manifest` を同梱すると OS ローダ時点で DPI awareness が確定し、
さらに堅牢になります（API 呼出と manifest は両立可能で、manifest が優先されます）。

`app.manifest` サンプル:

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <application xmlns="urn:schemas-microsoft-com:asm.v3">
    <windowsSettings>
      <dpiAwareness xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">PerMonitorV2</dpiAwareness>
    </windowsSettings>
  </application>
</assembly>
```

Visual Studio プロジェクトで同梱する場合の `.vcxproj` 設定:

```xml
<ItemGroup>
  <Manifest Include="app.manifest" />
</ItemGroup>
```

> Static library である HSPPP からは利用側 .exe に manifest を強制できないため、
> manifest 同梱は **任意の補強手段** として位置づけられます。
> 同梱しなくても API 呼出方式により Per Monitor V2 は有効化されます。

## 4. 描画座標の意味論

| 仮想画面の状態 | 描画コマンドの座標 | `ginfo_mx` / `ginfo_my` |
|--------------|---------------------|--------------------------|
| OFF（既定） | 物理クライアント px | 物理クライアント px |
| ON | 論理 px（`screen()` で指定した `width` / `height`） | 論理 px |

仮想画面 ON 時の自動拡縮の詳細は [仮想画面ガイド](/HSPPP_Lib/VirtualScreen) を参照してください。

## 5. ラスタ画像のスコープ外事項

HiDPI 対応の本 Sprint では、`picload` / `celload` 等で読み込んだ
**ラスタ画像（テクスチャ等）の高品質スケーリングは対象外** です。
ラスタ素材は読み込み時の元解像度で扱われ、拡縮品質は補間モード（`gmode_interp()` / 既定 LINEAR）と
`gzoom` の `mode` 指定に依存します。高 DPI 環境でドット感のない画像表示が必要な場合は、
利用者側で高解像度素材を用意してください。

> ベクトル描画（`boxf` / `line` / `circle` / `mes` 等）は、内部オフスクリーンビットマップが
> **物理サイズ** で保持されるため、最終物理解像度で直接ラスタライズされます。
> HiDPI 環境ではテキストの DWrite サブピクセル AA が物理解像度で動作するため、
> 100% DPI 環境と比較してテキスト品質が向上します。

## 6. 関連 API

| 機能 | API | 詳細 |
|------|-----|------|
| 仮想画面（論理→物理 自動拡縮） | `screen({.virtual_resolution = true})` / `screen_mode_virtual` | [VirtualScreen](/HSPPP_Lib/VirtualScreen) |
| ラスタ画像の補間モード切替 | `gmode_interp(0=NEAREST / 1=LINEAR / 2=ANISOTROPIC)` | [VirtualScreen §3](/HSPPP_Lib/VirtualScreen#3-補間モードの切替) |
| 線幅指定 | `gline_width(w)`（論理 px / 既定 1.0） | [VirtualScreen §8](/HSPPP_Lib/VirtualScreen#8-関連-api) |
| アンカー基準レイアウト | `anchor_pos` / `anchor_box` / `AnchorRect` | [AnchorLayout](/HSPPP_Lib/AnchorLayout) |

---

## 参照

- [仮想画面ガイド](/HSPPP_Lib/VirtualScreen)
- [アンカーレイアウト API](/HSPPP_Lib/AnchorLayout)
- [画面制御 API](/HSPPP_Lib/api/screen)
- [移行ガイド（HiDPI / 仮想画面 v2）](MigrationGuide-HiDPI-v2.md)
