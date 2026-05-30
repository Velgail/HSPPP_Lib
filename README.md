# HSPPP - Hot Soup Processor Plus Plus

[![Version](https://img.shields.io/badge/version-0.1.0-blue.svg)](https://github.com/Velgail/HspppLib)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![License: BSL-1.0](https://img.shields.io/badge/License-BSL--1.0-blue.svg)](LICENSE.txt)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)

**HSPPP** は HSP (Hot Soup Processor) 互換の C++23 ライブラリです。

HSP の親しみやすい API を C++ で使用でき、**HSP互換スタイル**と**オブジェクト指向スタイル**の両方をサポートします。

## ✨ 特徴

- **🎮 HSP互換API** - `screen`, `color`, `boxf`, `mes` などお馴染みの命令をそのまま使用可能
- **📦 モダンC++** - C++23 の機能を活用した型安全・メモリ安全な設計
- **🔧 デュアルスタイル** - HSP風のグローバル関数とOOP風のメソッドチェーン、お好みで選択
- **🖼️ Direct2D描画** - 高品質なハードウェアアクセラレーション描画
- **🔍 HiDPI 対応** - Per Monitor V2 を標準で有効化。4K/8K でもぼやけない描画
- **📐 仮想画面（論理→物理 自動拡縮）** - 1920×1080 で設計すれば任意解像度に自動適応
- **⚓ アンカー基準レイアウト** - `anchor_pos` / `AnchorRect` で 4:3 / 16:9 / 21:9 を 1 コードで吸収
- **⚡ ゼロオーバーヘッド** - C++の哲学「使わないものにコストを払わない」

## 🚀 クイックスタート

```cpp
// UserApp.cpp
import hsppp;
using namespace hsppp;

// エントリポイント: hspMain() を定義（main/WinMain は書かない）
void hspMain() {
    // ウィンドウ作成
    screen(0, 640, 480);
    title("Hello HSPPP!");
    
    // 描画
    color(255, 0, 0);
    boxf(100, 100, 200, 200);
    
    color(255, 255, 255);
    pos(120, 140);
    mes("Hello, HSPPP!");
    
    // hspMain を抜けると stop 相当（ウィンドウは閉じずに待機）
    return 0;
}
```

### OOPスタイル（メソッドチェーン対応）

```cpp
import hsppp;
using namespace hsppp;

void hspMain() {
    // 構造体による初期化
    auto win = screen({.width = 800, .height = 600, .title = "OOP Style"});
    
    // メソッドチェーンで連続描画
    win.color(255, 0, 0)
       .boxf(100, 100, 200, 200)
       .color(255, 255, 255)
       .pos(120, 140)
       .mes("Method Chaining!");
    
    return 0;
}
```

## 🖥️ HiDPI / 仮想画面 / アンカーレイアウト

HSPPP は 4K / 8K / ウルトラワイドといった多様な表示環境を「1 つの論理座標系」で書ける仕組みを標準提供します。

```cpp
import hsppp;
using namespace hsppp;

void hspMain() {
    // 論理 1920x1080 で設計、物理ウィンドウは 1280x720
    // HSP互換: screen(0, 1920, 1080, screen_mode_virtual);
    auto win = screen({
        .width  = 1920, .height  = 1080,
        .client_w = 1280, .client_h = 720,
        .virtual_resolution = true,
        .title = "Virtual Screen Demo",
    });

    color(20, 20, 30);
    boxf(0, 0, 1920, 1080);              // 論理 px で全面塗り

    // 解像度独立な右下アンカー配置
    color(255, 255, 255);
    anchor_pos(ah_right, av_bottom, -20, -20);
    mes("v1.0");

    redraw();
}
```

- HiDPI は **`SetProcessDpiAwarenessContext` を `init_system()` 内で呼出** するため、利用側に特別な手順は不要です。より厳密に保証したい場合は `app.manifest` 同梱を推奨します（詳細は [HiDPI ガイド](docs/HiDPI.md) 参照）。
- 仮想画面有効時、`boxf` / `mes` / `ginfo_mx` / `picload` などはすべて **論理 px** で扱われます。内部のオフスクリーンビットマップは **物理サイズ** で保持され、描画コマンド発行時点で論理→物理スケール変換（`SetTransform(Scale(s))`）が適用されます。`present()` は SwapChain への単純転送のみで、余白はレターボックス / ピラーボックスとして背景塗りとオフセット配置で実現されます（詳細は [仮想画面ガイド](docs/VirtualScreen.md) §1 参照）。
- ラスタ画像（`picload` / `celload` の素材）の高品質スケーリングは本機能のスコープ外です。`gmode_interp()` で補間モード（NEAREST / LINEAR / ANISOTROPIC、既定 LINEAR）を選択できます。
- ⚠️ **後方互換性に関する重要な変更**: `gcopy` / `gzoom` を `mode` 省略で呼び出した場合の既定補間モードが旧 NEAREST → 新 LINEAR に変更されました。ピクセルアート利用者は `gmode_interp 0` で従来挙動に復帰可能です。詳細は [移行ガイド](docs/MigrationGuide-SPRINT007.md) を参照してください。

📖 詳細: [HiDPI](docs/HiDPI.md) / [仮想画面](docs/VirtualScreen.md) / [アンカーレイアウト](docs/AnchorLayout.md)

## 📋 必要環境

- **OS**: Windows 11 (64-bit)
- **コンパイラ**: Visual Studio 2026 (VS 18)
- **C++標準**: C++23 (`/std:c++latest`)
- **必須ライブラリ**: Direct2D, DirectWrite (Windows SDK)

## 🔧 ビルド方法

### Visual Studio

1. `HspppLib.slnx` を Visual Studio 2026 で開く
2. プラットフォームを `x64` に設定
3. ビルド (F7 または Ctrl+Shift+B)

### コマンドライン (MSBuild)

```powershell
# Debug ビルド
MSBuild HspppLib.slnx /p:Configuration=Debug /p:Platform=x64 /m

# Release ビルド
MSBuild HspppLib.slnx /p:Configuration=Release /p:Platform=x64 /m
```

## 📖 ドキュメント

- [チュートリアル](docs/guides/tutorial.md)
- [インストールガイド](docs/guides/installation.md)
- [HSPからの移行ガイド](docs/guides/migration-from-hsp.md)
- [API リファレンス](docs/api/index.md)
- [HiDPI 対応](docs/HiDPI.md)
- [仮想画面（論理→物理 自動拡縮）](docs/VirtualScreen.md)
- [アンカーレイアウト API](docs/AnchorLayout.md)
- [移行ガイド（HiDPI / 仮想画面 設計大改修）](docs/MigrationGuide-SPRINT007.md)
- [FAQ](docs/faq.md)

## 🎯 対応API一覧

### 画面制御
`screen`, `buffer`, `bgscr`, `gsel`, `gmode`, `gcopy`, `gzoom`, `redraw`, `await`, `vwait`, `cls`, `title`, `width`, `vscalemode`（v2 で機能縮退）, `gmode_interp`, `gline_width`

### 描画命令
`color`, `pos`, `mes`, `boxf`, `line`, `circle`, `pset`, `pget`, `gradf`, `grect`, `grotate`, `gsquare`, `font`, `sysfont`, `hsvcolor`, `rgbcolor`

### レイアウト（アンカー基準）
`anchor_pos`, `anchor_box`, `boxf(AnchorRect)`, `AnchorRect`, `RectI`, `ah_left`/`ah_center`/`ah_right`, `av_top`/`av_middle`/`av_bottom`

### 画像操作
`picload`, `bmpsave`, `celload`, `celdiv`, `celput`, `loadCel`

### 入力
`stick`, `getkey`, `mouse`, `mousex`, `mousey`, `mousew`

### 数学関数
`sin`, `cos`, `tan`, `atan`, `sqrt`, `pow`, `abs`, `rnd`, `deg2rad`, `rad2deg`, `limit`, `dist`

### 文字列操作
`strlen`, `strmid`, `instr`, `strrep`, `strtrim`, `getstr`, `split`, `strf`, `getpath`

### ファイル操作
`exist`, `bload`, `bsave`, `dirlist`, `chdir`, `mkdir`, `deletefile`, `bcopy`, `dialog`, `dirinfo`, `exec`

### GUI オブジェクト
`button`, `input`, `mesbox`, `chkbox`, `combox`, `listbox`, `objprm`, `objsel`, `objenable`, `clrobj`

### 割り込み・エラー処理
`onclick`, `onkey`, `onerror`, `stop`

## 📁 プロジェクト構成

```
HspppLib/
├── HspppLib/          # ライブラリ本体
│   └── module/        # C++23 モジュール (.ixx)
├── HspppSample/       # サンプルアプリケーション
├── HspppTest/         # 単体テスト
└── doc/               # ドキュメント
```

## 🤝 貢献

バグ報告、機能リクエスト、プルリクエストを歓迎します。

## 📄 ライセンス

[Boost Software License 1.0](LICENSE.txt)

## 🔗 リンク

### C++ 学習リソース

- [江添亮の詳説C++17](https://ezoeryou.github.io/cpp17book/) - C++17 の体系的な解説書（日本語）
- [cpprefjp](https://cpprefjp.github.io/) - C++ 標準ライブラリリファレンス（日本語）

### 関連プロジェクト

- [HSP公式サイト](https://hsp.tv/) - HSPとは何かを知りたい方向け

---

**HSPPP** - HSPの手軽さとC++のパワーを両立させた、新しいプログラミング体験を。
