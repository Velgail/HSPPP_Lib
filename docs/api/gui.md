---
layout: default
title: GUI API
---

# GUI API リファレンス

GUIコントロールの作成と操作に関するAPIです。

## 目次

- [オブジェクト設定](#オブジェクト設定)
- [コントロール作成](#コントロール作成)
- [オブジェクト操作](#オブジェクト操作)

---

## オブジェクト設定

### objsize

GUIオブジェクトのサイズを設定します。

```cpp
void objsize(OptInt sizeX = {}, OptInt sizeY = {}, OptInt spaceY = {});
```

| パラメータ | デフォルト | 説明 |
|-----------|----------|------|
| `sizeX` | 64 | オブジェクト幅 |
| `sizeY` | 24 | オブジェクト高さ |
| `spaceY` | 0 | オブジェクト間の垂直間隔 |

**使用例:**

```cpp
objsize(200, 30, 10);  // 200x30、間隔10
```

---

### objmode

オブジェクトモードを設定します。

```cpp
void objmode(OptInt mode = {}, OptInt tabMove = {});
```

| mode定数 | 値 | 説明 |
|---------|---|------|
| `objmode_normal` | 0 | 標準モード |
| `objmode_guifont` | 1 | システムGUIフォントを使用 |
| `objmode_usefont` | 2 | 現在のフォント設定を使用 |
| `objmode_usecolor` | 4 | 現在の色設定を使用 |

フラグは組み合わせ可能です。

---

### objcolor

オブジェクトの色を設定します。

```cpp
void objcolor(OptInt r = {}, OptInt g = {}, OptInt b = {});
```

`objmode_usecolor` と併用します。

---

## コントロール作成

### button

ボタンを作成します。

```cpp
int button(std::string_view name, std::function<int()> callback);
```

| パラメータ | 説明 |
|-----------|------|
| `name` | ボタンのラベル |
| `callback` | クリック時に呼び出される関数 |

**戻り値:** オブジェクトID

**使用例:**

```cpp
pos(10, 10);
button("Click me!", []() {
    dialog("Button clicked!", dialog_info);
    return 0;
});

// カウンター付きボタン
int count = 0;
auto countPtr = std::make_shared<int>(0);
button("Count", [countPtr]() {
    (*countPtr)++;
    logmes(format("Count: {}", *countPtr));
    return 0;
});
```

---

### input

テキスト入力ボックスを作成します。

```cpp
int input(std::shared_ptr<std::string> var, OptInt sizeX = {}, OptInt sizeY = {}, OptInt maxLen = {});
```

| パラメータ | 説明 |
|-----------|------|
| `var` | 入力内容を格納する文字列（shared_ptr） |
| `sizeX` | 幅（省略時: objsize設定値） |
| `sizeY` | 高さ（省略時: objsize設定値） |
| `maxLen` | 最大入力長 |

**戻り値:** オブジェクトID

**注意:** ライフタイム安全性のため、`shared_ptr` のみ対応しています。

**使用例:**

```cpp
auto text = std::make_shared<std::string>("初期値");

pos(10, 10);
input(text, 200);

pos(10, 50);
button("Show", [text]() {
    dialog(*text, dialog_info);
    return 0;
});
```

---

### mesbox

複数行テキストボックスを作成します。

```cpp
int mesbox(std::shared_ptr<std::string> var, OptInt sizeX = {}, OptInt sizeY = {}, OptInt style = {}, OptInt maxLen = {});
```

| style | 説明 |
|-------|------|
| 0 | 読み取り専用 |
| 1 | 編集可能 |
| 4 | 水平スクロールバー |
| 8 | 垂直スクロールバー |

スタイルは組み合わせ可能です（例: `1+8` で編集可能+垂直スクロール）。

**使用例:**

```cpp
auto log = std::make_shared<std::string>();

objsize(400, 200);
pos(10, 10);
int logboxId = mesbox(log, omit, omit, 8);  // 垂直スクロール付き

// ログを追加
*log += "Line 1\n";
*log += "Line 2\n";
objprm(logboxId, *log);
```

---

### chkbox

チェックボックスを作成します。

```cpp
int chkbox(std::string_view label, std::shared_ptr<int> var);
```

| パラメータ | 説明 |
|-----------|------|
| `label` | チェックボックスのラベル |
| `var` | 状態を格納する変数（0=OFF, 1=ON） |

**戻り値:** オブジェクトID

**注意:** ライフタイム安全性のため、`shared_ptr<int>` のみ対応しています。

**使用例:**

```cpp
auto enabled = std::make_shared<int>(1);
auto debug = std::make_shared<int>(0);

pos(10, 10);
chkbox("Enable feature", enabled);

pos(10, 40);
chkbox("Debug mode", debug);

pos(10, 80);
button("Check", [enabled, debug]() {
    logmes(format("Enabled: {}, Debug: {}", *enabled, *debug));
    return 0;
});
```

---

### combox

コンボボックスを作成します。

```cpp
int combox(std::shared_ptr<int> var, OptInt expandY, std::string_view items);
```

| パラメータ | 説明 |
|-----------|------|
| `var` | 選択インデックス（0始まり） |
| `expandY` | ドロップダウン展開時の高さ |
| `items` | 項目リスト（改行区切り） |

**戻り値:** オブジェクトID

**使用例:**

```cpp
auto selection = std::make_shared<int>(0);

pos(10, 10);
combox(selection, 100, "Option 1\nOption 2\nOption 3");

pos(10, 50);
button("Get Selection", [selection]() {
    dialog(format("Selected: {}", *selection), dialog_info);
    return 0;
});
```

---

### listbox

リストボックスを作成します。

```cpp
int listbox(std::shared_ptr<int> var, OptInt expandY, std::string_view items);
```

パラメータは `combox` と同じです。

---

## オブジェクト操作

### objprm

オブジェクトの内容を変更します。

```cpp
void objprm(int objectId, std::string_view value);
void objprm(int objectId, int value);
```

**使用例:**

```cpp
int inputId = input(text, 200);

// 後から内容を変更
objprm(inputId, "New text");

// コンボボックスの選択を変更
objprm(comboxId, 2);  // インデックス2を選択
```

---

### objenable

オブジェクトの有効/無効を設定します。

```cpp
void objenable(int objectId, OptInt enable = {});
```

| enable | 説明 |
|--------|------|
| 0 | 無効（グレーアウト） |
| 1 | 有効 |

---

### objsel

オブジェクトにフォーカスを設定します。

```cpp
int objsel(OptInt objectId = {});
```

---

### objskip

オブジェクトのフォーカス移動モードを設定します。

```cpp
void objskip(int objectId, OptInt mode = {});
```

| mode | 説明 |
|------|------|
| 0 | 通常（TABキーで移動） |
| 1 | スキップ |
| 2 | フォーカス移動不可 |

---

### clrobj

オブジェクトを削除します。

```cpp
void clrobj(OptInt startId = {}, OptInt endId = {});
```

| パラメータ | 説明 |
|-----------|------|
| `startId` | 削除開始ID（省略時: 0） |
| `endId` | 削除終了ID（省略時: 最後まで） |

**使用例:**

```cpp
clrobj();       // すべて削除
clrobj(3);      // ID 3以降を削除
clrobj(2, 5);   // ID 2〜5を削除
```

---

## 使用例

### 設定画面

```cpp
void hspMain() {
    screen(0, 400, 300);
    title("Settings");
    
    auto playerName = std::make_shared<std::string>("Player1");
    auto volume = std::make_shared<int>(1);
    auto difficulty = std::make_shared<int>(1);
    
    // 名前入力
    pos(10, 10);
    mes("Player Name:");
    pos(10, 30);
    objsize(180, 24);
    input(playerName, 180);
    
    // 音量チェックボックス
    pos(10, 70);
    chkbox("Sound enabled", volume);
    
    // 難易度選択
    pos(10, 100);
    mes("Difficulty:");
    pos(10, 120);
    combox(difficulty, 80, "Easy\nNormal\nHard");
    
    // 保存ボタン
    pos(10, 170);
    objsize(100, 30);
    button("Save", [=]() {
        logmes(format("Name: {}", *playerName));
        logmes(format("Sound: {}", *volume));
        logmes(format("Difficulty: {}", *difficulty));
        dialog("Settings saved!", dialog_info);
        return 0;
    });
    
    return 0;
}
```

### 計算機

```cpp
void hspMain() {
    screen(0, 300, 200);
    title("Calculator");
    
    auto num1 = std::make_shared<std::string>("0");
    auto num2 = std::make_shared<std::string>("0");
    auto result = std::make_shared<std::string>("0");
    
    objsize(80, 24);
    
    pos(10, 10);
    mes("Number 1:");
    pos(100, 10);
    input(num1, 80);
    
    pos(10, 40);
    mes("Number 2:");
    pos(100, 40);
    input(num2, 80);
    
    pos(10, 80);
    mes("Result:");
    pos(100, 80);
    input(result, 80);  // 読み取り専用として使用
    
    // 演算ボタン
    pos(10, 120);
    objsize(50, 30);
    
    button("+", [=]() {
        *result = str(toInt(*num1) + toInt(*num2));
        return 0;
    });
    
    pos(70, 120);
    button("-", [=]() {
        *result = str(toInt(*num1) - toInt(*num2));
        return 0;
    });
    
    pos(130, 120);
    button("×", [=]() {
        *result = str(toInt(*num1) * toInt(*num2));
        return 0;
    });
    
    pos(190, 120);
    button("÷", [=]() {
        int n2 = toInt(*num2);
        if (n2 != 0) {
            *result = str(toInt(*num1) / n2);
        } else {
            *result = "Error";
        }
        return 0;
    });
    
    return 0;
}
```

---

## 参照

- [画面制御 API](screen.md)
- [型定義](types.md)
- [FAQ](../faq.md)（shared_ptr を使う理由）
