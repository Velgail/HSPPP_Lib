# HSPPP アーキテクチャドキュメント

## 概要

HSPPPは、HSP (Hot Soup Processor) 互換のAPIをC++23モジュールとして提供するライブラリである。
**HSP互換スタイル**と**オブジェクト指向スタイル**の両方を同時にサポートする設計を採用している。

---

## デュアルスタイル設計

### スタイル1: HSP互換（グローバル関数）

HSPからの移植や、HSPに慣れたユーザー向け。

```cpp
screen(0, 800, 600);
color(255, 255, 255);
boxf();
pos(100, 100);
mes("Hello, HSP!");
```

### スタイル2: オブジェクト指向（Screen クラス）

C++らしいモダンな書き方。メソッドチェーンをサポート。

```cpp
auto win = screen(0, 800, 600);
win.color(255, 255, 255)
   .boxf()
   .pos(100, 100)
   .mes("Hello, C++!");
```

### 両スタイルの混在

同一プロジェクト内で両方を混在させることが可能。

```cpp
auto win = screen(0, 800, 600);  // OOPスタイルでハンドル取得
win.select();                     // このウィンドウをカレントに
color(255, 0, 0);                 // グローバル関数で描画
mes("Mixed style!");
```

---

## パラメータ省略機構

### `omit` キーワード

HSPの `screen , 800, 600` のような任意位置でのパラメータ省略をC++で実現。

```cpp
screen(omit, 800, 600);        // ID=0（省略）, 800x600
screen(1, omit, omit, screen_hide);  // サイズはデフォルト、非表示
```

**注意**: C++26で `_` は特別な意味（名前独立宣言）を持つため、`omit` を使用。

### `OptInt` / `OptDouble`

省略可能なパラメータ用のラッパー型。

```cpp
export struct OptInt {
    constexpr OptInt() noexcept;           // 省略（デフォルト）
    constexpr OptInt(detail::OmitTag);     // omit からの変換
    constexpr OptInt(int v) noexcept;      // 値指定
    
    bool is_default() const noexcept;
    int value_or(int def) const noexcept;
};
```

### Designated Initializers（構造体版）

多数のパラメータを名前付きで指定する場合。

```cpp
screen({.width = 800, .height = 600, .title = "My Window"});
```

---

## Screen クラス設計

### 軽量ハンドル方式

`Screen` クラスは **ID のみを保持する軽量ハンドル** である。
実際の Surface は内部のグローバルマップで管理される。

```cpp
export class Screen {
private:
    int m_id;      // ウィンドウID
    bool m_valid;  // 有効フラグ
    
public:
    // メソッドは全て Screen& を返す（メソッドチェーン対応）
    Screen& color(int r, int g, int b);
    Screen& pos(int x, int y);
    Screen& mes(std::string_view text);
    Screen& boxf(int x1, int y1, int x2, int y2);
    Screen& boxf();  // 画面全体
    Screen& redraw(int mode = 1);
    Screen& select();
    
    // ゲッター
    int id() const noexcept;
    int width() const;
    int height() const;
    bool valid() const noexcept;
};
```

### なぜ ID のみを保持するか

1. **モジュール境界の問題回避**: モジュールインターフェース（.ixx）と実装（.cpp）で
   異なる型定義を参照する問題を回避
2. **軽量**: コピーコストが最小（int + bool のみ）
3. **HSP互換**: HSPの「ウィンドウID」概念と一致

---

## ファイル構成

```
HspppLib/
├── module/
│   ├── hsppp.ixx      # メインモジュールインターフェース
│   ├── graphics.ixx   # グラフィックス拡張（将来用）
│   └── input.ixx      # 入力処理（将来用）
├── src/
│   ├── boot/
│   │   └── WinMain.cpp    # エントリーポイント
│   ├── core/
│   │   ├── hsppp.cpp      # HSP互換API実装
│   │   ├── Surface.cpp    # HspSurface 実装
│   │   ├── Window.cpp     # HspWindow 実装
│   │   └── Internal.h     # 内部クラス定義
│   └── utils/
│       └── Random.cpp     # 乱数ユーティリティ
└── include/               # 公開ヘッダー（将来用）

HspppSample/
└── UserApp.cpp            # ユーザーアプリケーション
```

---

## 内部アーキテクチャ

### クラス階層

```
HspSurface（基底クラス）
    └── HspWindow（ウィンドウ付きサーフェス）
```

### グローバル状態管理

```cpp
namespace {
    std::map<int, std::shared_ptr<HspSurface>> g_surfaces;  // ID→Surface
    std::weak_ptr<HspSurface> g_currentSurface;             // カレント
    int g_redrawMode = 1;  // 描画モード
}
```

### Screen メンバ関数の実装パターン

```cpp
Screen& Screen::color(int r, int g, int b) {
    auto surface = getSurfaceById(m_id);  // IDからSurface取得
    if (surface) {
        surface->color(r, g, b);
    }
    return *this;  // メソッドチェーン対応
}
```

---

## ビルド手順

### 環境

- Visual Studio 18 (VS 2026)
- C++23 以降（`/std:c++latest`）
- Platform Toolset: v145

### ビルドコマンド

```powershell
# フルリビルド
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" `
    "c:\Users\takumi\source\repos\Velgail\HspppLib\HspppLib.slnx" `
    /t:Rebuild /p:Configuration=Debug /p:Platform=x64

# インクリメンタルビルド
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" `
    "c:\Users\takumi\source\repos\Velgail\HspppLib\HspppLib.slnx" `
    /t:Build /p:Configuration=Debug /p:Platform=x64
```

### 実行

```powershell
& "c:\Users\takumi\source\repos\Velgail\HspppLib\x64\Debug\HspppSample.exe"
```

---

## 設計原則

1. **HSP互換性優先**: HSPユーザーが違和感なく使えることを最優先
2. **C++らしさも提供**: OOPスタイルでモダンC++の恩恵も受けられる
3. **型安全**: `omit`/`OptInt` で型安全なパラメータ省略
4. **将来互換**: C++26の `_` 問題を考慮して `omit` を採用
5. **軽量ハンドル**: Screen は ID のみ保持、実体はグローバル管理

---

## 今後の拡張予定

- [ ] `picload` 命令（画像読み込み）
- [ ] ウィンドウ別の割り込みハンドラ管理

---

## 変更履歴

### 2025-12-01
- 割り込みハンドラを実装（`onclick`, `oncmd`, `onerror`, `onexit`, `onkey`）
- `stop` 命令を実装
- システム変数 `iparam`, `wparam`, `lparam` を実装
- デュアルスタイル（HSP/OOP）設計を確立
- `omit` キーワードによるパラメータ省略機構を導入
- `Screen` クラスを軽量ハンドル方式に変更
- ビルド手順を確立
