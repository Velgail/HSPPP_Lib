---
layout: default
title: チュートリアル
---

# チュートリアル

このチュートリアルでは、HSPPP の基本的な使い方を学びます。

## 目次

1. [最初のプログラム](#最初のプログラム)
2. [図形を描く](#図形を描く)
3. [ユーザー入力](#ユーザー入力)
4. [GUIコントロール](#guiコントロール)
5. [画像の表示](#画像の表示)

---

## 最初のプログラム

### Hello World

```cpp
import hsppp;
using namespace hsppp;

int hspMain() {
    // ウィンドウを作成
    screen(0, 640, 480);
    title("Hello HSPPP!");
    
    // テキストを表示
    pos(100, 100);
    mes("Hello, World!");
    
    // hspMain を抜けると stop() と同等の動作になる
    return 0;
}
```

<!-- 根拠: WinMain.cpp で hspMain() を呼び出し、戻った後は GetMessage ループで待機。 -->

### 解説

- `hspMain()`: ユーザーが定義するエントリポイント（`main` は書かない）
- `screen(id, width, height)`: ウィンドウを作成
- `title(text)`: ウィンドウタイトルを設定
- `pos(x, y)`: 描画位置を設定
- `mes(text)`: テキストを表示

---

## 図形を描く

### 基本図形

```cpp
import hsppp;
using namespace hsppp;

int hspMain() {
    screen(0, 640, 480);
    
    // 背景を白でクリア
    color(255, 255, 255);
    cls();
    
    // 赤い四角形
    color(255, 0, 0);
    boxf(50, 50, 150, 150);
    
    // 青い円
    color(0, 0, 255);
    circle(200, 100, 250, 150);
    
    // 緑の線
    color(0, 255, 0);
    line(400, 150, 300, 50);
    
    return 0;
}
```

<!-- 根拠: hsppp.ixx で boxf, circle, line を export。 -->

### OOP スタイル

同じことを OOP スタイルで書くこともできます：

```cpp
import hsppp;
using namespace hsppp;

int hspMain() {
    auto win = screen({.width = 640, .height = 480});
    
    win.color(255, 255, 255).cls()
       .color(255, 0, 0).boxf(50, 50, 150, 150)
       .color(0, 0, 255).circle(200, 100, 250, 150)
       .color(0, 255, 0).line(400, 150, 300, 50);
    
    return 0;
}
```

<!-- 根拠: Screen クラスのメンバ関数は Screen& を返し、メソッドチェーンが可能。hsppp.ixx の Screen クラス定義を参照。 -->

---

## ユーザー入力

### キーボード入力（ゲームループ）

```cpp
import hsppp;
using namespace hsppp;

int x = 320, y = 240;

int hspMain() {
    screen(0, 640, 480);
    
    while (true) {
        redraw(0);
        color(255, 255, 255);
        cls();
        
        color(255, 0, 0);
        circle(x - 20, y - 20, x + 20, y + 20, 1);
        
        redraw(1);
        
        // キー状態を取得（第1引数: トリガーでなく押しっぱなしを検出するキー）
        int key = stick(15);  // 15 = 1+2+4+8 = 矢印キーを押しっぱなし検出
        if (key & 1) x -= 5;  // stick_left
        if (key & 4) x += 5;  // stick_right
        if (key & 2) y -= 5;  // stick_up
        if (key & 8) y += 5;  // stick_down
        
        // ESCで終了
        if (key & 128) break;  // stick_esc
        
        await(16);
    }
    
    return 0;
}
```

<!-- 根拠: UserApp.cpp のメインループ実装。stick() の戻り値ビットは hsppp.ixx の stick_* 定数を参照。 -->

### キーボード入力（割り込み）

```cpp
import hsppp;
using namespace hsppp;

int x = 320, y = 240;

int hspMain() {
    screen(0, 640, 480);
    
    // キー入力の割り込みを設定
    onkey([]() {
        int key = iparam();  // 押されたキーコード
        
        if (key == 37) x -= 10;  // VK_LEFT
        if (key == 39) x += 10;  // VK_RIGHT
        if (key == 38) y -= 10;  // VK_UP
        if (key == 40) y += 10;  // VK_DOWN
        
        // 再描画
        color(255, 255, 255);
        cls();
        color(255, 0, 0);
        circle(x - 20, y - 20, x + 20, y + 20, 1);
        
        return 0;
    });
    
    // 初期描画
    color(255, 0, 0);
    circle(x - 20, y - 20, x + 20, y + 20, 1);
    
    stop();  // 割り込みを待機（return 0 でも同様の動作）
}
```

<!-- 根拠: hsppp.ixx で onkey(InterruptHandler) と iparam() を export。 -->

### マウス入力

```cpp
import hsppp;
using namespace hsppp;

int hspMain() {
    screen(0, 640, 480);
    
    onclick([]() {
        int mx = mousex();
        int my = mousey();
        
        color(0, 0, 255);
        circle(mx - 10, my - 10, mx + 10, my + 10, 1);
        
        return 0;
    });
    
    return 0;
}
```

<!-- 根拠: hsppp.ixx で onclick, mousex, mousey を export。 -->

---

## GUIコントロール

### ボタンと入力ボックス

```cpp
import hsppp;
using namespace hsppp;

auto inputText = std::make_shared<std::string>("名前を入力");
int clickCount = 0;

int hspMain() {
    screen(0, 400, 300);
    
    pos(20, 20);
    mes("名前:");
    
    pos(20, 50);
    objsize(200, 24);
    input(inputText);
    
    pos(20, 90);
    button("クリック", [inputText]() {
        clickCount++;
        
        // 表示を更新
        color(255, 255, 255);
        boxf(20, 130, 300, 160);
        
        color(0, 0, 0);
        pos(20, 130);
        mes("こんにちは、" + *inputText + "さん！ (" + 
            std::to_string(clickCount) + "回目)");
        
        return 0;
    });
    
    return 0;
}
```

<!-- 根拠: hsppp_file.ixx で input は shared_ptr<std::string> 版のみ提供。button は std::function<int()> を受け取る。 -->

### チェックボックス・コンボボックス

```cpp
import hsppp;
using namespace hsppp;

// shared_ptr で状態変数を作成（ライフタイム安全）
auto checkState = std::make_shared<int>(0);
auto comboIndex = std::make_shared<int>(0);

int hspMain() {
    screen(0, 400, 300);
    
    pos(20, 20);
    objsize(200, 24);
    
    // チェックボックス
    chkbox("オプションを有効にする", checkState);
    
    // コンボボックス
    pos(20, 60);
    combox(comboIndex, 100, "選択肢1\n選択肢2\n選択肢3");
    
    // 状態確認ボタン
    pos(20, 120);
    button("状態を表示", []() {
        std::string msg = "チェック: " + 
            std::string(*checkState ? "ON" : "OFF") +
            "\n選択: " + std::to_string(*comboIndex);
        
        dialog(msg);
        return 0;
    });
    
    return 0;
}
```

<!-- 根拠: hsppp.ixx で chkbox, combox, listbox は shared_ptr<int> 版のみ提供。int& 版は安全性のため提供していない。 -->

---

## 画像の表示

### 画像の読み込みと表示

```cpp
import hsppp;
using namespace hsppp;

int hspMain() {
    screen(0, 640, 480);
    
    // バッファに画像を読み込み
    buffer(1);
    picload("image.png");
    
    // メインウィンドウに戻る
    gsel(0);
    
    // 画像をコピー
    pos(100, 100);
    gcopy(1, 0, 0, 200, 200);
    
    return 0;
}
```

<!-- 根拠: hsppp.ixx で buffer, picload, gsel, gcopy を export。 -->

### 透過コピー

```cpp
// 指定色を透明として扱う
gmode(2);  // gmode_and: 透過モード
color(255, 0, 255);  // 透過色（マゼンタ）
pos(100, 100);
gcopy(1, 0, 0, 200, 200);
```

<!-- 根拠: hsppp.ixx の gmode_* 定数を参照。 -->

---

## 次のステップ

- [HSPからの移行ガイド](migration-from-hsp.md): HSP経験者向け
- [APIリファレンス](../api/index.md): 全命令の詳細
- [FAQ](../faq.md): よくある質問
