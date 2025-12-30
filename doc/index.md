# HSPPP ドキュメント

**HSPPP** は HSP (Hot Soup Processor) 互換の C++23 ライブラリです。

HSP の親しみやすい API を C++ で使用でき、HSP互換スタイルとオブジェクト指向スタイルの両方をサポートします。

## クイックスタート

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
    
    // hspMain を抜けると、ライブラリがメッセージループを回し続ける（HSPの stop 相当）
    return 0;
}
```

<!-- 根拠: WinMain.cpp でライブラリ側に WinMain が定義されており、hspMain() を呼び出す。hspMain() を抜けた後は GetMessage ループで待機。 -->

## ドキュメント

- [インストールガイド](guides/installation.md)
- [チュートリアル](guides/tutorial.md)
- [HSPからの移行ガイド](guides/migration-from-hsp.md)
- [API リファレンス](api/index.md)
- [FAQ](faq.md)

## 特徴

### デュアルスタイル対応

HSP互換のグローバル関数スタイルと、C++らしいオブジェクト指向スタイルの両方で記述できます。

```cpp
// HSP互換スタイル
screen(0, 640, 480);
color(255, 0, 0);
boxf(100, 100, 200, 200);

// OOPスタイル（メソッドチェーン対応）
auto win = screen({.width = 640, .height = 480});
win.color(255, 0, 0).boxf(100, 100, 200, 200);
```

<!-- 根拠: hsppp.ixx で両スタイルの API を export。Screen クラスは軽量ハンドルで、メソッドチェーンが可能。 -->

### 型安全性

C++23 の機能を活用し、コンパイル時に多くのエラーを検出できます。

### ライフタイム安全性

GUI コントロールなど、変数のライフタイムが重要な箇所では `shared_ptr` を使用した安全な API を提供しています。

<!-- 根拠: CLAUDE.md の「非安全コーディングの禁止」方針に基づき、chkbox/combox/listbox は shared_ptr<int> 版のみ提供。 -->

## ライセンス

MIT License

## リンク

- [GitHub リポジトリ](https://github.com/Velgail/HspppLib)
- [HSP公式サイト](https://hsp.tv/)
