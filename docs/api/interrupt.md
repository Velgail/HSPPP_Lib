---
layout: default
title: 割り込みAPI
---

# 割り込み API リファレンス

イベント処理・割り込み・エラー処理に関するAPIです。

## 目次

- [割り込みハンドラ設定](#割り込みハンドラ設定)
- [割り込み情報取得](#割り込み情報取得)
- [プログラム制御](#プログラム制御)
- [時間・待機](#時間待機)
- [エラー処理](#エラー処理)
- [システムハンドル](#システムハンドル)
- [メッセージ送信](#メッセージ送信)

---

## 割り込みハンドラ設定

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

## 割り込み情報取得

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

- [入力 API](input.md)
- [型定義](types.md)（InterruptHandler, ErrorHandler）
