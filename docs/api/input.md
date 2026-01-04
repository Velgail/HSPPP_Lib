---
layout: default
title: 入力API
---

# 入力 API リファレンス

キーボード・マウス入力に関するAPIです。

## 目次

- [キー入力](#キー入力)
- [マウス](#マウス)
- [定数](#定数)

---

## キー入力

### stick

キー入力情報を取得します。

```cpp
int stick(OptInt nonTrigger = {}, OptInt checkActive = {});
```

| パラメータ | 説明 |
|-----------|------|
| `nonTrigger` | 非トリガータイプキー指定（押しっぱなしでも検出するキー） |
| `checkActive` | 0=常に取得, 1=アクティブ時のみ |

**戻り値:** キー入力状態のビットフラグ

**使用例:**

```cpp
// 毎フレームの入力処理
int key = stick(stick_left + stick_right + stick_up + stick_down);

if (key & stick_left)  x -= 4;
if (key & stick_right) x += 4;
if (key & stick_up)    y -= 4;
if (key & stick_down)  y += 4;
if (key & stick_space) fire();  // スペースはトリガー（一度だけ反応）
```

---

### getkey

特定キーの押下状態を取得します。

```cpp
int getkey(int keycode);
```

| パラメータ | 説明 |
|-----------|------|
| `keycode` | 仮想キーコード（VK_LEFT=37, VK_UP=38 等） |

**戻り値:** 押されていれば1、押されていなければ0

**使用例:**

```cpp
if (getkey(32)) {  // スペースキー (VK_SPACE = 32)
    jump();
}

if (getkey(65)) {  // 'A'キー
    attack();
}
```

---

## マウス

### mousex

マウスカーソルのX座標を取得します。

```cpp
int mousex();
```

**戻り値:** 現在のウィンドウ内でのX座標

---

### mousey

マウスカーソルのY座標を取得します。

```cpp
int mousey();
```

**戻り値:** 現在のウィンドウ内でのY座標

---

### mousew

マウスホイールの移動量を取得します。

```cpp
int mousew();
```

**戻り値:** ホイール移動量

---

### mouse

マウスカーソルの座標を設定します。

```cpp
void mouse(OptInt x = {}, OptInt y = {}, OptInt mode = {});
```

| パラメータ | 説明 |
|-----------|------|
| `x` | X座標（ディスプレイ座標） |
| `y` | Y座標（ディスプレイ座標） |
| `mode` | 0=負値で非表示, -1=移動+非表示, 1=移動のみ, 2=移動+表示 |

**使用例:**

```cpp
mouse(320, 240);      // カーソルを移動
mouse(0, 0, -1);      // 移動して非表示
```

---

## 定数

### stickキーコードビットマスク

```cpp
inline constexpr int stick_left    = 1;       // カーソルキー左(←)
inline constexpr int stick_up      = 2;       // カーソルキー上(↑)
inline constexpr int stick_right   = 4;       // カーソルキー右(→)
inline constexpr int stick_down    = 8;       // カーソルキー下(↓)
inline constexpr int stick_space   = 16;      // スペースキー
inline constexpr int stick_enter   = 32;      // Enterキー
inline constexpr int stick_ctrl    = 64;      // Ctrlキー
inline constexpr int stick_esc     = 128;     // ESCキー
inline constexpr int stick_lbutton = 256;     // マウス左ボタン
inline constexpr int stick_rbutton = 512;     // マウス右ボタン
inline constexpr int stick_tab     = 1024;    // TABキー
inline constexpr int stick_z       = 2048;    // [Z]キー
inline constexpr int stick_x       = 4096;    // [X]キー
inline constexpr int stick_c       = 8192;    // [C]キー
inline constexpr int stick_a       = 16384;   // [A]キー
inline constexpr int stick_w       = 32768;   // [W]キー
inline constexpr int stick_d       = 65536;   // [D]キー
inline constexpr int stick_s       = 131072;  // [S]キー
```

### 仮想キーコード（主要なもの）

| キー | コード | 定数（Windows） |
|------|--------|----------------|
| 左矢印 | 37 | VK_LEFT |
| 上矢印 | 38 | VK_UP |
| 右矢印 | 39 | VK_RIGHT |
| 下矢印 | 40 | VK_DOWN |
| スペース | 32 | VK_SPACE |
| Enter | 13 | VK_RETURN |
| Esc | 27 | VK_ESCAPE |
| Shift | 16 | VK_SHIFT |
| Ctrl | 17 | VK_CONTROL |
| Alt | 18 | VK_MENU |
| A-Z | 65-90 | - |
| 0-9 | 48-57 | - |
| F1-F12 | 112-123 | VK_F1〜VK_F12 |

---

## 使用例

### ゲームキャラクターの移動

```cpp
int x = 320, y = 240;

void hspMain() {
    screen(0, 640, 480);
    
    while (true) {
        redraw(0);
        cls(4);
        
        // 入力処理
        int key = stick(stick_left + stick_right + stick_up + stick_down);
        if (key & stick_left)  x -= 4;
        if (key & stick_right) x += 4;
        if (key & stick_up)    y -= 4;
        if (key & stick_down)  y += 4;
        
        // ESCで終了
        if (key & stick_esc) end();
        
        // 描画
        color(255, 255, 0);
        circle(x - 16, y - 16, x + 16, y + 16, 1);
        
        redraw(1);
        await(16);
    }
    return 0;
}
```

### マウスによる描画

```cpp
void hspMain() {
    screen(0, 640, 480);
    title("Paint - Left click to draw");
    
    while (true) {
        int key = stick(stick_lbutton + stick_rbutton);
        
        // 左クリックで描画
        if (key & stick_lbutton) {
            color(255, 0, 0);
            circle(mousex() - 5, mousey() - 5, 
                   mousex() + 5, mousey() + 5, 1);
        }
        
        // 右クリックでクリア
        if (key & stick_rbutton) {
            cls(0);
        }
        
        await(16);
    }
    return 0;
}
```

---

## 参照

- [割り込み API](/HSPPP_Lib/api/interrupt)（onclick, onkey）
- [GUI API](/HSPPP_Lib/api/gui)（button, input）
