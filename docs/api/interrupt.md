---
layout: default
title: 割り込みAPI
---

# 割り込み API リファレンス

イベント処理・割り込み・エラー処理に関するAPIです。

## 目次

- [割り込みハンドラ](#割り込みハンドラ)
- [時間・待機](#時間待機)
- [プログラム制御](#プログラム制御)

---

## 割り込みハンドラ

### onclick

マウスクリック時の割り込みを設定します。

```cpp
// ハンドラ設定
void onclick(InterruptHandler handler);

// 一時停止/再開
void onclick(int enable);  // 0=停止, 1=再開
```

**使用例:**

```cpp
onclick([]() { /* ... */ });

// 一時的に無効化
onclick(0);

// 再度有効化
onclick(1);
```

---

### onkey

キー入力時の割り込みを設定します。

```cpp
// ハンドラ設定
void onkey(InterruptHandler handler);

// 一時停止/再開
void onkey(int enable);
```

**使用例:**

```cpp
onkey([]() { /* ... */ });
```

---

### onexit

終了ボタン押下時の割り込みを設定します。

```cpp
// ハンドラ設定
void onexit(InterruptHandler handler);

// 一時停止/再開
void onexit(int enable);
```

**使用例:**

```cpp
onexit([]() { /* ... */ });
```

---

### oncmd

Windowsメッセージ受信時の割り込みを設定します。

```cpp
// ハンドラ設定（メッセージID指定）
void oncmd(InterruptHandler handler, int messageId);

// 特定メッセージIDの一時停止/再開
void oncmd(int enable, int messageId);

// oncmd割り込み全体の一時停止/再開
void oncmd(int enable);
```

**使用例:**

```cpp
// WM_MOUSEMOVE (0x0200) を捕捉
oncmd([]() { /* ... */ }, 0x0200);
```

---

### onerror

エラー発生時の割り込みを設定します。

```cpp
// ハンドラ設定
void onerror(ErrorHandler handler);

// 一時停止/再開
void onerror(int enable);
```

**使用例:**

```cpp
onerror([](const HspErrorBase& e) { /* ... */ });
```

---

### iparam / wparam / lparam

割り込み発生時のパラメータを取得します。

```cpp
int iparam() noexcept;   // wparam相当
int wparam() noexcept;   // wparam相当
int lparam() noexcept;   // lparam相当
```

**使用例:**

```cpp
onkey([]() { /* ... */ });
```

---

## 時間・待機

### await

マイクロ秒精度の待機処理を行います（メッセージ処理並行）。

```cpp
void await(int time_ms);
```

**特徴:**
- `QueryPerformanceCounter` を使用した高精度タイマー（マイクロ秒単位）
- 待機中もWindows メッセージを処理
- 割り込みハンドラも処理可能

**パラメータ:**
- `time_ms` : 待機時間（ミリ秒、0以上）

**使用例:**

```cpp
// 16ms待機（約60FPS）
await(16);

// フレームレート制御
int target_fps = 144;
while (true) {
    redraw(0);
    // ... 描画処理 ...
    redraw(1);
    await(1000 / target_fps);
}
```

---

### vwait

VSync同期待機を行います。**ティアリング防止機能を内蔵**（HSPPP独自の拡張機能）。

```cpp
// グローバル版
double vwait();

// OOP版
double Screen::vwait();
```

**特徴:**
- **呼び出されたウィンドウの描画バッファを自動フラッシュ**してからVSync同期Present
- 戻り値で前回からの経過時間を取得
- フレームドロップ検出が可能
- **`redraw(0)→描画→vwait()`だけでティアリング防止が保証される**
- **標準HSPには存在しない、HSPPP独自の拡張機能**

**戻り値:**
- 前回の `vwait()` 呼び出しからの経過時間（ミリ秒）

**ティアリング防止の基本パターン:**

```cpp
// シンプルパターン（推奨）
while (true) {
    redraw(0);              // 描画開始（バッファリング）
    // ... 描画処理 ...
    vwait();                // 自動的にEndDraw + VSync同期Present
}

// 従来パターン（redraw(1)は省略可能）
while (true) {
    redraw(0);
    // ... 描画処理 ...
    redraw(1);              // 明示的にEndDraw
    await(16);              // 大体60fps
}
```

**フレームドロップ検出例:**

```cpp
int fps = ginfo_fps();  // 60 or 144 等
double expected_ms = 1000.0 / fps;

while (true) {
    redraw(0);
    // ... 描画処理 ...
    
    double elapsed = vwait();
    
    // フレームドロップ検出
    if (elapsed > expected_ms * 1.5) {
        logmes("FPS DROPPED: " + str(static_cast<int>(elapsed)) + "ms");
    }
}
```

**重要:**
- **`vwait()` は呼び出されたウィンドウの描画バッファのフラッシュとVSync同期を自動的に行います**
- OOP版では `Screen::vwait()` を呼び出したウィンドウのみ処理
- グローバル版では現在アクティブなウィンドウ（`gsel`で選択中）のみ処理

---

## プログラム制御

### stop

プログラムを一時停止し、割り込みを待機します。

```cpp
void stop();
```

**使用例:**

```cpp
void hspMain() {
    screen(0, 640, 480);
    
    onclick([]() {
        mes("Clicked!");
    });
    
    mes("Click to test...");
    stop();  // クリック待ち
}
```

---

### end

プログラムを終了します。

```cpp
[[noreturn]] void end(int exitcode = 0);
```

**使用例:**

```cpp
if (getkey(27)) { /* ... */ }
```

---

## 参照

- [入力 API](/HSPPP_Lib/api/input)
- [型定義](/HSPPP_Lib/api/types)（InterruptHandler, ErrorHandler）
