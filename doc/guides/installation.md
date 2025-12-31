# インストールガイド

## 必要環境

- **OS**: Windows 11
- **コンパイラ**: Visual Studio 2026 (VS 18) 以降
- **C++ 標準**: C++23（モジュール対応必須）

<!-- 根拠: CONCEPT.md より「Visual Studio 2026 / C++23 Modules を前提とする」 -->

## インストール方法

### 方法 1: ソースからビルド（推奨）

1. リポジトリをクローン:
   ```powershell
   git clone https://github.com/Velgail/HspppLib.git
   cd HspppLib
   ```

2. Visual Studio でソリューションを開く:
   ```
   HspppLib.slnx
   ```

3. ビルド:
   - 構成: `Debug` または `Release`
   - プラットフォーム: `x64`

<!-- 根拠: tasks.json のビルドタスクで x64 指定。 -->

## プロジェクトへの組み込み

### 1. プロジェクト参照の追加

Visual Studio で:
1. ソリューションエクスプローラーで右クリック → 追加 → 参照
2. HspppLib プロジェクトを選択

### 2. モジュールのインポート

```cpp
import hsppp;
using namespace hsppp;
```

<!-- 根拠: hsppp.ixx が export module hsppp; でモジュールを公開。 -->

### 3. エントリポイント

`WinMain` はライブラリ側で定義済みのため、ユーザーが書く必要はありません。ユーザーは **`hspMain()` 関数を定義**するだけです。

```cpp
// UserApp.cpp
import hsppp;
using namespace hsppp;

int hspMain() {
    screen(0, 640, 480);
    title("My App");
    
    // 初期化処理、または while ループでゲームループを実装
    
    return 0;  // hspMain を抜けると stop() と同等の動作になる
}
```

<!-- 根拠: WinMain.cpp よりライブラリ側で WinMain を定義し、extern int hspMain(); を呼び出す。hspMain() から戻った後は GetMessage ループで待機。 -->

### ゲームループの実装例

アニメーションやリアルタイム更新が必要な場合は、`hspMain()` 内で `while` ループを書きます：

```cpp
int hspMain() {
    screen(0, 640, 480);
    
    int x = 320, y = 240;
    
    while (true) {
        redraw(0);
        color(255, 255, 255);
        cls();
        
        color(255, 0, 0);
        circle(x - 20, y - 20, x + 20, y + 20, 1);
        
        redraw(1);
        
        // キー入力
        int key = stick(15);
        if (key & 1) x -= 5;  // 左
        if (key & 4) x += 5;  // 右
        if (key & 2) y -= 5;  // 上
        if (key & 8) y += 5;  // 下
        
        // ESCで終了
        if (key & 128) break;
        
        await(16);  // 約60FPS
    }
    
    return 0;
}
```

<!-- 根拠: UserApp.cpp の hspMain() 実装を参照。while (true) + await(16) + stick() でゲームループを実現。 -->

## ビルド設定

### 必須設定

| 設定 | 値 |
|------|-----|
| C++ 言語標準 | C++23 (`/std:c++latest`) |
| モジュール | 有効 |
| プラットフォーム | x64 |

### 推奨設定

| 設定 | 値 |
|------|-----|
| 警告レベル | /W4 |
| 最適化 (Release) | /O2 |

## トラブルシューティング

### エラー: モジュールが見つからない

**原因**: C++23 モジュールが有効になっていない

**解決策**: プロジェクトプロパティで「C++ 言語標準」を「C++23」または「最新」に設定

### エラー: hsppp.ixx がコンパイルできない

**原因**: Visual Studio のバージョンが古い

**解決策**: Visual Studio 2026 (VS 18) 以降を使用してください

### リンクエラー: 未解決の外部シンボル `hspMain`

**原因**: `hspMain()` 関数が定義されていない

**解決策**: ユーザーコードで `int hspMain()` を定義してください

### リンクエラー: 未解決の外部シンボル（その他）

**原因**: HspppLib がリンクされていない

**解決策**: プロジェクト参照または `.lib` ファイルのリンクを確認
