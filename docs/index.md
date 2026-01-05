---
layout: default
title: ホーム
---

# HSPPP ドキュメント

**HSPPP** は HSP (Hot Soup Processor) 互換の C++23 ライブラリです。

HSP の親しみやすい API を C++ で使用でき、HSP互換スタイルとオブジェクト指向スタイルの両方をサポートします。

---

## クイックスタート

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
    
    // hspMain を抜けると stop() と同等の動作になる
}
```

<!-- 根拠: WinMain.cpp でライブラリ側に WinMain が定義されており、hspMain() を呼び出す。hspMain() を抜けた後は GetMessage ループで待機。 -->

## ドキュメント

### クイックスタートガイド

- [インストールガイド](/HSPPP_Lib/guides/installation) - 環境構築とビルド方法
- [チュートリアル](/HSPPP_Lib/guides/tutorial) - 基本的な使い方を学ぶ

### アーキテクチャ・パターンガイド

- [ステートパターンガイド](/HSPPP_Lib/guides/state-pattern) - ゲームフロー・UI遷移の設計パターン
- [HSP goto 移行ガイド](/HSPPP_Lib/guides/hsp-goto-migration) - HSPの `*label` / `goto` をステートマシンに移行

### HSPユーザー向けガイド

- [HSPからの移行ガイド](/HSPPP_Lib/guides/migration-from-hsp) - HSPと C++ の構文・概念の対応表

### リファレンス

- [API リファレンス](/HSPPP_Lib/api/) - 全API一覧と詳細説明

### その他

- [FAQ](/HSPPP_Lib/faq) - よくある質問

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

### パラメータ省略

HSPのようにパラメータを省略できます。`omit` または `{}` を使用します。

```cpp
// HSPスタイル: screen ,,, 2  (ID, width, height を省略して mode だけ指定)
screen(omit, omit, omit, screen_hide);  // omit で省略
screen({}, {}, {}, screen_hide);        // {} でも省略可能
```

## 必要環境

- **OS**: Windows 11 (64-bit)
- **コンパイラ**: Visual Studio 2026 (VS 18)
- **C++標準**: C++23 (`/std:c++latest`)
- **必須ライブラリ**: Direct2D, DirectWrite (Windows SDK)

## ライセンス

Boost Software License 1.0

## リンク

- [GitHub リポジトリ](https://github.com/Velgail/HspppLib)
- [HSP公式サイト](https://hsp.tv/)
