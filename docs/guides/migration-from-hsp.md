---
layout: default
title: HSPからの移行ガイド
---

# HSP からの移行ガイド

HSP (Hot Soup Processor) から HSPPP への移行方法を解説します。

## 目次

- [基本的な違い](#基本的な違い)
- [変数](#変数)
- [文字列](#文字列)
- [制御構文](#制御構文)
- [GUI コントロール](#gui-コントロール)
- [移行例](#移行例)

---

## 基本的な違い

| 項目 | HSP | HSPPP (C++) |
|------|-----|-------------|
| エントリポイント | なし（自動実行） | `void hspMain()` |
| 変数宣言 | 不要 | 必須 |
| 行末 | 改行 | `;` |
| 文字列 | `"..."` または `'...'` | `"..."` のみ |
| コメント | `//` または `;` | `//` または `/* */` |
| ラベル | `*label` | 関数/ラムダ |
| goto/gosub | サポート | 非推奨（関数を使用） |
| stop | 実行停止 | `hspMain()` から `return` |

<!-- 根拠: CONCEPT.md の「ユーザーへの移行ガイド」セクション。 -->

---

## 変数

### HSP

```hsp
a = 10
b = 3.14
s = "Hello"
dim arr, 10
```

### HSPPP

```cpp
int a = 10;
double b = 3.14;
std::string s = "Hello";
std::vector<int> arr(10);
```

### 注意: ローカル変数とグローバル変数

HSP では変数はすべてグローバルですが、C++ では関数内で宣言した変数はローカル変数となり、関数を抜けると破棄されます。

```cpp
// ❌ 危険: ローカル変数
void setup() {
    int count = 0;
    button("Click", [&count]() {  // count はすでに破棄されている！
        count++;
        return 0;
    });
}

// ✅ 安全: グローバル変数またはstatic
int count = 0;  // グローバル
void setup() {
    button("Click", []() {
        count++;
        return 0;
    });
}
```

<!-- 根拠: C++ のスコープ規則。HSP との根本的な違い。 -->

---

## 文字列

### HSP

```hsp
s = "Hello"
s += " World"
a = strlen(s)
b = instr(s, 0, "World")
```

### HSPPP

```cpp
std::string s = "Hello";
s += " World";
int a = s.length();  // または strlen(s)
int b = instr(s, 0, "World");
```

### 文字列操作の対応表

| HSP | HSPPP |
|-----|-------|
| `strlen(s)` | `s.length()` または `strlen(s)` |
| `strmid(s, p, n)` | `s.substr(p, n)` または `strmid(s, p, n)` |
| `instr(s, p, t)` | `instr(s, p, t)` |
| `s + t` | `s + t` |

<!-- 根拠: CONCEPT.md の「文字列処理」セクション。HSP 便利命令は std::string に対するラッパー関数として提供。 -->

---

## 制御構文

### if 文

```hsp
; HSP
if a > 10 {
    mes "big"
} else {
    mes "small"
}
```

```cpp
// HSPPP
if (a > 10) {
    mes("big");
} else {
    mes("small");
}
```

### ループ

```hsp
; HSP
repeat 10
    mes cnt
loop
```

```cpp
// HSPPP
for (int cnt = 0; cnt < 10; cnt++) {
    mes(std::to_string(cnt));
}
```

### goto/gosub → 関数

HSPPP では `goto` / `gosub` の代わりに C++ の関数を使用します。

```hsp
; HSP
gosub *draw
stop

*draw
    color 255, 0, 0
    boxf 0, 0, 100, 100
    return
```

```cpp
// HSPPP
void draw() {
    color(255, 0, 0);
    boxf(0, 0, 100, 100);
}

void hspMain() {
    screen(0, 640, 480);
    draw();
    stop();  // 割り込みを待機
}
```

<!-- 根拠: CONCEPT.md の「ラベルジャンプは卒業」。 -->

---

## GUI コントロール

### 重要な違い: ライフタイム

HSP では変数がグローバルなので、GUI コントロールに変数を渡しても問題ありませんでした。HSPPP では、一部の GUI コントロールは `shared_ptr` を使用する必要があります。

### ボタン

```hsp
; HSP
button "Click", *onclick
stop

*onclick
    mes "Clicked!"
    return
```

```cpp
// HSPPP
button("Click", []() {
    mes("Clicked!");
    return 0;
});
```

<!-- 根拠: hsppp.ixx で button は std::function<int()> を受け取る。 -->

### 入力ボックス

```hsp
; HSP
sdim s, 256
input s, 200, 24
```

```cpp
// HSPPP (shared_ptr版のみ提供)
auto s = std::make_shared<std::string>("");
input(s, 200, 24);
```

<!-- 根拠: hsppp_file.ixx で input は shared_ptr<std::string> 版のみ提供。ライフタイム安全性のため。 -->

### チェックボックス

```hsp
; HSP
chk = 0
chkbox "Enable", chk
```

```cpp
// HSPPP (shared_ptr 必須)
auto chk = std::make_shared<int>(0);
chkbox("Enable", chk);

// 値の取得
if (*chk) {
    mes("Enabled");
}
```

<!-- 根拠: hsppp.ixx で chkbox は shared_ptr<int> 版のみ提供。int& 版は安全性のため提供していない。 -->

### コンボボックス / リストボックス

```hsp
; HSP
sel = 0
combox sel, 100, "A\nB\nC"
```

```cpp
// HSPPP (shared_ptr 必須)
auto sel = std::make_shared<int>(0);
combox(sel, 100, "A\nB\nC");

// 選択値の取得
mes("Selected: " + std::to_string(*sel));
```

<!-- 根拠: combox, listbox も chkbox と同様、shared_ptr<int> 版のみ提供。 -->

---

## 移行例

### HSP の典型的なプログラム

```hsp
; HSP
screen 0, 640, 480
title "Sample"

x = 320 : y = 240

*main
    redraw 0
    color 255, 255, 255 : cls
    
    color 255, 0, 0
    circle x-20, y-20, x+20, y+20, 1
    
    redraw 1
    
    stick key, 15
    if key & 1 : x -= 5
    if key & 4 : x += 5
    if key & 2 : y -= 5
    if key & 8 : y += 5
    
    await 16
    goto *main
```

### HSPPP への移行

```cpp
// HSPPP
import hsppp;
using namespace hsppp;

int x = 320, y = 240;

void hspMain() {
    screen(0, 640, 480);
    title("Sample");
    
    // while ループでゲームループを実装
    while (true) {
        redraw(0);
        color(0, 0, 0);
        boxf();
        
        color(255, 0, 0);
        circle(x - 20, y - 20, x + 20, y + 20, 1);
        
        redraw(1);
        
        int key = stick(15);
        if (key & 1) x -= 5;
        if (key & 4) x += 5;
        if (key & 2) y -= 5;
        if (key & 8) y += 5;
        
        // ESCで終了
        if (key & 128) break;
        
        await(16);
    }
    
    return 0;
}
```

<!-- 根拠: UserApp.cpp の hspMain() 実装パターン。HSP の goto *main ループは C++ の while (true) + break で表現。 -->

---

## goto / gosub の置き換え

HSPの `goto` と `gosub` を C++ で実装する方法は、複雑さによって異なります。

### Simple goto: 単純な制御フロー

複数の画面遷移を含むゲームやアプリケーションでは、**ステートマシン**を使用します。

```hsp
; HSP例
*title
    mes "Title"
    if key & 32 : goto *game

*game
    mes "Playing"
    if key & 128 : goto *title
```

```cpp
// HSPPP推奨: StateMachine を使用
enum class Screen { Title, Game };
auto sm = StateMachine<Screen>();

sm.state(Screen::Title)
  .on_update([&](auto& sm) {
      mes("Title");
      if (getkey(' ')) sm.jump(Screen::Game);
  });

sm.state(Screen::Game)
  .on_update([&](auto& sm) {
      color(0, 0, 0);
      boxf();
      mes("Playing");
      if (getkey(VK_ESCAPE)) sm.jump(Screen::Title);
      await(16);
  });

sm.jump(Screen::Title);
sm.run();
```

詳細は [HSP goto 移行ガイド](/HSPPP_Lib/guides/hsp-goto-migration) を参照してください。

### gosub: サブルーチン呼び出し

`gosub` は**単純に関数に置き換えます**。**ステートマシンは不要です。**

```hsp
; HSP例
*main
    gosub *draw_bg
    gosub *draw_player

*draw_bg
    boxf 0, 0, 640, 480
    return

*draw_player
    // ...
```

```cpp
// HSPPP: 関数に置き換え（ステートマシン不要）
void draw_bg() {
    boxf(0, 0, 640, 480);
}

void draw_player() {
    // ...
}

void hspMain() {
    // 直接呼び出すだけ
    draw_bg();
    draw_player();
}
```

---

## チェックリスト

移行時に確認するポイント：

- [ ] `void hspMain()` を定義したか（`main()` ではない）
- [ ] 変数はすべて型を指定して宣言したか
- [ ] 行末に `;` を付けたか
- [ ] `goto` を ステートマシン（`sm.jump()`）に置き換えたか
- [ ] `gosub` を関数呼び出しに置き換えたか
- [ ] `chkbox`/`combox`/`listbox` は `shared_ptr` を使用しているか
- [ ] ローカル変数を GUI コールバックで使用していないか
- [ ] メインループは `while (true)` + `await()` + `break` で終了するか

---

## 関連項目

- [ステートパターンガイド](/HSPPP_Lib/guides/state-pattern) - ステートマシンの設計パターン
- [HSP goto 移行ガイド](/HSPPP_Lib/guides/hsp-goto-migration) - goto の具体的な移行例
