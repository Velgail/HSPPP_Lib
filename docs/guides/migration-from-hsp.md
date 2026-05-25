---
layout: default
title: HSPからの移行ガイチE
---

# HSP からの移行ガイチE

HSP (Hot Soup Processor) から HSPPP への移行方法を解説します、E

## 目次

- [基本皁E��違い](#基本皁E��違い)
- [変数](#変数)
- [斁E���E](#斁E���E)
- [制御構文](#制御構文)
- [GUI コントロール](#gui-コントロール)
- [移行例](#移行侁E

---

## 基本皁E��違い

| 頁E�� | HSP | HSPPP (C++) |
|------|-----|-------------|
| エントリポインチE| なし（�E動実行！E| `void hspMain()` |
| 変数宣言 | 不要E| 忁E��E|
| 行末 | 改衁E| `;` |
| 斁E���E | `"..."` また�E `'...'` | `"..."` のみ |
| コメンチE| `//` また�E `;` | `//` また�E `/* */` |
| ラベル | `*label` | 関数/ラムダ |
| goto/gosub | サポ�EチE| 非推奨�E�関数を使用�E�E|
| stop | 実行停止 | `hspMain()` から `return` |

<!-- 根拠: CONCEPT.md の「ユーザーへの移行ガイド」セクション、E-->

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

### 注愁E ローカル変数とグローバル変数

HSP では変数はすべてグローバルですが、C++ では関数冁E��宣言した変数はローカル変数となり、E��数を抜けると破棁E��れます、E

```cpp
// ❁E危険: ローカル変数
void setup() {
    int count = 0;
    button("Click", [&count]() {  // count はすでに破棁E��れてぁE���E�E
        count++;
        return 0;
    });
}

// ✁E安�E: グローバル変数また�Estatic
int count = 0;  // グローバル
void setup() {
    button("Click", []() {
        count++;
        return 0;
    });
}
```

<!-- 根拠: C++ のスコープ規則、ESP との根本皁E��違い、E-->

---

## 斁E���E

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
int a = s.length();  // また�E strlen(s)
int b = instr(s, 0, "World");
```

### 斁E���E操作�E対応表

| HSP | HSPPP |
|-----|-------|
| `strlen(s)` | `s.length()` また�E `strlen(s)` |
| `strmid(s, p, n)` | `s.substr(p, n)` また�E `strmid(s, p, n)` |
| `instr(s, p, t)` | `instr(s, p, t)` |
| `s + t` | `s + t` |

<!-- 根拠: CONCEPT.md の「文字�E処琁E��セクション、ESP 便利命令は std::string に対するラチE��ー関数として提供、E-->

---

## 制御構文

### if 斁E

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

### ルーチE

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

### goto/gosub ↁE関数

HSPPP では `goto` / `gosub` の代わりに C++ の関数を使用します、E

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
    stop();  // 割り込みを征E��E
}
```

<!-- 根拠: CONCEPT.md の「ラベルジャンプ�E卒業」、E-->

---

## GUI コントロール

### 重要な違い: ライフタイム

HSP では変数がグローバルなので、GUI コントロールに変数を渡しても問題ありませんでした、ESPPP では、一部の GUI コントロールは `shared_ptr` を使用する忁E��があります、E

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

<!-- 根拠: hsppp.ixx で button は std::function<int()> を受け取る、E-->

### 入力�EチE��ス

```hsp
; HSP
sdim s, 256
input s, 200, 24
```

```cpp
// HSPPP (shared_ptr版�Eみ提侁E
auto s = std::make_shared<std::string>("");
input(s, 200, 24);
```

<!-- 根拠: hsppp_file.ixx で input は shared_ptr<std::string> 版�Eみ提供。ライフタイム安�E性のため、E-->

### チェチE��ボックス

```hsp
; HSP
chk = 0
chkbox "Enable", chk
```

```cpp
// HSPPP (shared_ptr 忁E��E
auto chk = std::make_shared<int>(0);
chkbox("Enable", chk);

// 値の取征E
if (*chk) {
    mes("Enabled");
}
```

<!-- 根拠: hsppp.ixx で chkbox は shared_ptr<int> 版�Eみ提供。int& 版�E安�E性のため提供してぁE��ぁE��E-->

### コンボ�EチE��ス / リスト�EチE��ス

```hsp
; HSP
sel = 0
combox sel, 100, "A\nB\nC"
```

```cpp
// HSPPP (shared_ptr 忁E��E
auto sel = std::make_shared<int>(0);
combox(sel, 100, "A\nB\nC");

// 選択値の取征E
mes("Selected: " + std::to_string(*sel));
```

<!-- 根拠: combox, listbox めEchkbox と同様、shared_ptr<int> 版�Eみ提供、E-->

---

## 移行侁E

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

### HSPPP への移衁E

```cpp
// HSPPP
import hsppp;
using namespace hsppp;

int x = 320, y = 240;

void hspMain() {
    screen(0, 640, 480);
    title("Sample");
    
    // while ループでゲームループを実裁E
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
        
        // ESCで終亁E
        if (key & 128) break;
        
        await(16);
    }
    
    return 0;
}
```

<!-- 根拠: UserApp.cpp の hspMain() 実裁E��ターン、ESP の goto *main ループ�E C++ の while (true) + break で表現、E-->

---

## goto / gosub の置き換ぁE

HSPの `goto` と `gosub` めEC++ で実裁E��る方法�E、褁E��さによって異なります、E

### Simple goto: 単純な制御フロー

褁E��の画面遷移を含むゲームめE��プリケーションでは、E*スチE�Eト�Eシン**を使用します、E

```hsp
; HSP侁E
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
auto sm = StateGraph<Screen>();

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

詳細は [HSP goto 移行ガイド](/HSPPP_Lib/guides/hsp-goto-migration) を参照してください、E

### gosub: サブルーチン呼び出ぁE

`gosub` は**単純に関数に置き換えまぁE*、E*スチE�Eト�Eシンは不要です、E*

```hsp
; HSP侁E
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
// HSPPP: 関数に置き換え（スチE�Eト�Eシン不要E��E
void draw_bg() {
    boxf(0, 0, 640, 480);
}

void draw_player() {
    // ...
}

void hspMain() {
    // 直接呼び出すだぁE
    draw_bg();
    draw_player();
}
```

---

## チェチE��リスチE

移行時に確認する�Eイント！E

- [ ] `void hspMain()` を定義したか！Emain()` ではなぁE��E
- [ ] 変数はすべて型を持E��して宣言したぁE
- [ ] 行末に `;` を付けたか
- [ ] `goto` めEスチE�Eト�Eシン�E�Esm.jump()`�E�に置き換えたぁE
- [ ] `gosub` を関数呼び出しに置き換えたぁE
- [ ] `chkbox`/`combox`/`listbox` は `shared_ptr` を使用してぁE��ぁE
- [ ] ローカル変数めEGUI コールバックで使用してぁE��ぁE��
- [ ] メインループ�E `while (true)` + `await()` + `break` で終亁E��るか

---

## 関連頁E��

- [スチE�Eトパターンガイド](/HSPPP_Lib/guides/state-pattern) - スチE�Eト�Eシンの設計パターン
- [HSP goto 移行ガイド](/HSPPP_Lib/guides/hsp-goto-migration) - goto の具体的な移行侁E
