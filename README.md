# HSPPP - Hot Soup Processor Plus Plus

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
- **⚡ ゼロオーバーヘッド** - C++の哲学「使わないものにコストを払わない」

## 🚀 クイックスタート

```cpp
// UserApp.cpp
import hsppp;
using namespace hsppp;

// エントリポイント: hspMain() を定義（main/WinMain は書かない）
int hspMain() {
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

int hspMain() {
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

- [チュートリアル](doc/guides/tutorial.md)
- [インストールガイド](doc/guides/installation.md)
- [HSPからの移行ガイド](doc/guides/migration-from-hsp.md)
- [API リファレンス](doc/api/index.md)
- [FAQ](doc/faq.md)

## 🎯 対応API一覧

### 画面制御
`screen`, `buffer`, `bgscr`, `gsel`, `gmode`, `gcopy`, `gzoom`, `redraw`, `await`, `cls`, `title`, `width`

### 描画命令
`color`, `pos`, `mes`, `boxf`, `line`, `circle`, `pset`, `pget`, `gradf`, `grect`, `grotate`, `gsquare`, `font`, `sysfont`, `hsvcolor`, `rgbcolor`

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

- [HSP公式サイト](https://hsp.tv/) - HSPとは何かを知りたい方向け

---

**HSPPP** - HSPの手軽さとC++のパワーを両立させた、新しいプログラミング体験を。
