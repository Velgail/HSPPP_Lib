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
onclick([]() {
    logmes(format("Click at ({}, {})", mousex(), mousey()));
    return 0;
});

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
onkey([]() {
    int key = iparam();  // 押されたキーコード
    logmes(format("Key pressed: {}", key));
    return 0;
});
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
onexit([]() {
    auto result = dialog("終了しますか？", dialog_yesno);
    if (result.stat == 6) {  // Yes
        return 1;  // 終了を許可
    }
    return 0;  // 終了をキャンセル
});
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
oncmd([]() {
    int x = lparam() & 0xFFFF;
    int y = (lparam() >> 16) & 0xFFFF;
    logmes(format("Mouse move: ({}, {})", x, y));
    return 0;
}, 0x0200);
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
onerror([](const HspErrorBase& e) {
    logmes(format("Error {}: {}", e.error_code(), e.message()));
    logmes(format("  at {}:{}", e.file_name(), e.line_number()));
    
    if (e.is_fatal()) {
        return 1;  // 致命的エラー: 終了
    }
    return 0;  // 処理を継続
});
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
onkey([]() {
    int keyCode = iparam();
    int scanCode = lparam() >> 16;
    return 0;
});
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
int hspMain() {
    screen(0, 640, 480);
    
    onclick([]() {
        logmes("Clicked!");
        return 0;
    });
    
    mes("Click to test...");
    stop();  // クリック待ち
    
    return 0;
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
if (getkey(27)) {  // ESC
    end();
}

end(1);  // エラーコード1で終了
```

---

## 時間・待機

### await

メッセージを処理しながら待機します（CPU負荷あり）。

```cpp
void await(int time_ms);
```

| パラメータ | 説明 |
|-----------|------|
| `time_ms` | 待機時間（ミリ秒） |

ゲームループなど、正確なフレームレートが必要な場合に使用します。

**使用例:**

```cpp
while (true) {
    redraw(0);
    // 描画処理
    redraw(1);
    await(16);  // 約60FPS
}
```

---

### wait

軽い待機を行います（CPU負荷軽）。

```cpp
void wait(OptInt time = {});
```

| パラメータ | 説明 |
|-----------|------|
| `time` | 待機時間（10ms単位、デフォルト: 100=1秒） |

**使用例:**

```cpp
wait(50);   // 500ms待機
wait(100);  // 1秒待機
```

---

### gettime

時間・日付を取得します。

```cpp
int gettime(int type);
```

| type | 説明 |
|------|------|
| 0 | 年 |
| 1 | 月 |
| 2 | 曜日（0=日〜6=土） |
| 3 | 日 |
| 4 | 時 |
| 5 | 分 |
| 6 | 秒 |
| 7 | ミリ秒 |

**使用例:**

```cpp
int year = gettime(0);
int month = gettime(1);
int day = gettime(3);
int hour = gettime(4);
int minute = gettime(5);
int second = gettime(6);

mes(format("{}/{}/{} {}:{}:{}", year, month, day, hour, minute, second));
```

---

## エラー処理

### HspErrorBase

エラー例外の基底クラスです。

```cpp
class HspErrorBase : public std::runtime_error {
public:
    [[nodiscard]] int error_code() const noexcept;
    [[nodiscard]] int line_number() const noexcept;
    [[nodiscard]] const std::string& file_name() const noexcept;
    [[nodiscard]] const std::string& function_name() const noexcept;
    [[nodiscard]] const std::string& message() const noexcept;
    [[nodiscard]] std::exception_ptr original_exception() const noexcept;
    [[nodiscard]] bool has_original_exception() const noexcept;
    void rethrow_original() const;
    [[nodiscard]] virtual bool is_fatal() const noexcept = 0;
};
```

---

### HspError

致命的エラー例外クラスです。

```cpp
class HspError : public HspErrorBase {
public:
    HspError(int errorCode, std::string_view message);
    HspError(int errorCode, std::string_view message, std::exception_ptr originalException);
    HspError(int errorCode, const std::exception& originalException);
    
    [[nodiscard]] bool is_fatal() const noexcept override;  // 常にtrue
};
```

---

### HspWeakError

復帰可能なエラー例外クラスです。

```cpp
class HspWeakError : public HspErrorBase {
public:
    HspWeakError(int errorCode, std::string_view message);
    HspWeakError(int errorCode, std::string_view message, std::exception_ptr originalException);
    HspWeakError(int errorCode, const std::exception& originalException);
    
    [[nodiscard]] bool is_fatal() const noexcept override;  // 常にfalse
};
```

---

### エラーコード定数

```cpp
inline constexpr int ERR_NONE              = 0;
inline constexpr int ERR_SYNTAX            = 1;
inline constexpr int ERR_ILLEGAL_FUNCTION  = 2;
inline constexpr int ERR_LABEL_REQUIRED    = 3;
inline constexpr int ERR_OUT_OF_MEMORY     = 4;
inline constexpr int ERR_TYPE_MISMATCH     = 5;
inline constexpr int ERR_OUT_OF_ARRAY      = 6;
inline constexpr int ERR_OUT_OF_RANGE      = 7;
inline constexpr int ERR_DIVIDE_BY_ZERO    = 8;
inline constexpr int ERR_BUFFER_OVERFLOW   = 9;
inline constexpr int ERR_UNSUPPORTED       = 10;
inline constexpr int ERR_EXPRESSION        = 11;
inline constexpr int ERR_FILE_IO           = 12;
inline constexpr int ERR_WINDOW_INIT       = 13;
inline constexpr int ERR_INVALID_HANDLE    = 14;
inline constexpr int ERR_EXTERNAL_EXECUTE  = 15;
inline constexpr int ERR_SYSTEM_ERROR      = 16;
inline constexpr int ERR_INTERNAL          = 17;
```

---

## システムハンドル

### hwnd / hdc / hinstance

Windowsのハンドルを取得します。

```cpp
int64_t hwnd();       // 現在選択されているウィンドウのハンドル
int64_t hdc();        // 現在のデバイスコンテキスト
int64_t hinstance();  // 現在のインスタンスハンドル
```

---

## メッセージ送信

### sendmsg

Windowsメッセージを送信します。

```cpp
int64_t sendmsg(int64_t hwndValue, int msg, int64_t wparam = 0, int64_t lparam = 0);
int64_t sendmsg(int64_t hwndValue, int msg, int64_t wparam, std::string_view lparamText);
```

---

## 使用例

### ゲームループとイベント処理

```cpp
int x = 320, y = 240;
bool running = true;

int hspMain() {
    screen(0, 640, 480);
    
    // ESCキーで終了
    onexit([]() {
        running = false;
        return 1;
    });
    
    // エラーハンドリング
    onerror([](const HspErrorBase& e) {
        dialog(format("Error: {}", e.message()), dialog_warning);
        return e.is_fatal() ? 1 : 0;
    });
    
    while (running) {
        redraw(0);
        cls(4);
        
        int key = stick(15);  // 方向キーは押しっぱなし対応
        if (key & stick_left)  x -= 4;
        if (key & stick_right) x += 4;
        if (key & stick_up)    y -= 4;
        if (key & stick_down)  y += 4;
        
        color(255, 255, 0);
        circle(x - 16, y - 16, x + 16, y + 16, 1);
        
        redraw(1);
        await(16);
    }
    
    return 0;
}
```

---

## 参照

- [入力 API](input.md)
- [型定義](types.md)（InterruptHandler, ErrorHandler）
