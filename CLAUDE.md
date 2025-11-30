# CLAUDE.md - AI Assistant Guidelines for HSPPP

このファイルは、Claude（AI アシスタント）がこのリポジトリで作業する際の
ガイドラインと参照情報を提供します。

---

## プロジェクト概要

**HSPPP** は HSP (Hot Soup Processor) 互換のC++23ライブラリです。
HSP互換スタイルとオブジェクト指向スタイルの両方をサポートします。

### アーキテクチャドキュメント

詳細な設計情報は `.claude/ARCHITECTURE.md` を参照してください。

---

## コーディング規約

### デュアルスタイル対応

新しい描画命令を追加する際は、必ず以下の両方を実装すること：

1. **グローバル関数**（HSP互換）
   ```cpp
   export void newcmd(int param1, int param2);
   ```

2. **Screen メンバ関数**（OOP）
   ```cpp
   Screen& Screen::newcmd(int param1, int param2);
   ```

### パラメータ省略

省略可能なパラメータには `OptInt` / `OptDouble` を使用：

```cpp
export void screen(
    OptInt id       = {},
    OptInt width    = {},
    OptInt height   = {},
    // ...
);
```

### メソッドチェーン

Screen のメンバ関数は必ず `Screen&` を返すこと：

```cpp
Screen& Screen::color(int r, int g, int b) {
    // 処理
    return *this;  // 必須
}
```

---

## ビルド手順

### VS 18 (Visual Studio 2025 Preview) を使用

```powershell
# リビルド
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" `
    "c:\Users\takumi\source\repos\Velgail\HspppLib\HspppLib.slnx" `
    /t:Rebuild /p:Configuration=Debug /p:Platform=x64

# ビルド
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" `
    "c:\Users\takumi\source\repos\Velgail\HspppLib\HspppLib.slnx" `
    /t:Build /p:Configuration=Debug /p:Platform=x64
```

### 実行

```powershell
& "c:\Users\takumi\source\repos\Velgail\HspppLib\x64\Debug\HspppSample.exe"
```

**注意**: VS 2022 (VS 17) ではなく VS 18 を使用すること！

---

## 重要な設計判断

### `omit` vs `_`

C++26 で `_` は「名前独立宣言」として特別な意味を持つようになるため、
パラメータ省略には `omit` キーワードを使用する。

### Screen クラスの軽量ハンドル方式

`Screen` クラスは `shared_ptr<HspSurface>` を保持せず、ID のみを保持する。
これにより：
- モジュール境界の型問題を回避
- コピーコストを最小化
- HSPのウィンドウID概念と一致

### 実装の分離

- `hsppp.ixx`: 公開インターフェース（export）
- `hsppp.cpp`: 実装（Internal.h をインクルード）
- `Internal.h`: 内部クラス定義（HspSurface, HspWindow）

---

## ファイル構成

```
HspppLib/
├── .claude/
│   ├── ARCHITECTURE.md  # 詳細アーキテクチャ
│   └── (このファイルは CLAUDE.md としてルートに配置)
├── module/
│   └── hsppp.ixx        # メインモジュール
├── src/
│   ├── core/
│   │   ├── hsppp.cpp    # API実装
│   │   ├── Internal.h   # 内部定義
│   │   └── ...
│   └── ...
└── ...

HspppSample/
└── UserApp.cpp          # デモアプリ
```

---

## 変更時のチェックリスト

新機能追加時：
- [ ] グローバル関数版を `hsppp.ixx` に宣言
- [ ] Screen メンバ関数版を `hsppp.ixx` に宣言
- [ ] グローバル関数を `hsppp.cpp` に実装
- [ ] Screen メンバ関数を `hsppp.cpp` に実装
- [ ] ビルド確認（VS 18 MSBuild）
- [ ] サンプル（UserApp.cpp）に使用例追加
