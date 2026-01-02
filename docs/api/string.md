---
layout: default
title: 文字列API
---

# 文字列操作 API リファレンス

文字列操作・型変換・メモリノートパッドに関するAPIです。

## 目次

- [型変換](#型変換)
- [文字列操作](#文字列操作)
- [文字コード変換](#文字コード変換)
- [メモリノートパッド（HSP互換）](#メモリノートパッドhsp互換)
- [NotePadクラス（OOP版）](#notepadクラスoop版)
- [書式化](#書式化)
- [デバッグ](#デバッグ)
- [再エクスポート](#再エクスポート)

---

## 型変換

### toInt

値を整数に変換します。

```cpp
[[nodiscard]] int toInt(double p1);
[[nodiscard]] int toInt(const std::string& p1);
```

**使用例:**

```cpp
int a = toInt(3.14);       // 3
int b = toInt("42");       // 42
int c = toInt("123abc");   // 123（数値部分のみ）
```

---

### toDouble

値を実数に変換します。

```cpp
[[nodiscard]] double toDouble(int p1);
[[nodiscard]] double toDouble(const std::string& p1);
```

---

### str

値を文字列に変換します。

```cpp
[[nodiscard]] std::string str(double value);
[[nodiscard]] std::string str(int value);
[[nodiscard]] std::string str(int64_t value);
```

**使用例:**

```cpp
std::string s = str(123);      // "123"
std::string t = str(3.14);     // "3.14"
```

---

### strlen

文字列の長さ（バイト数）を取得します。

```cpp
[[nodiscard]] int64_t strlen(const std::string& p1) noexcept;
```

---

## 文字列操作

### instr

文字列を検索します。

```cpp
[[nodiscard]] int64_t instr(const std::string& p1, int64_t p2, const std::string& search);
[[nodiscard]] int64_t instr(const std::string& p1, const std::string& search);
```

| パラメータ | 説明 |
|-----------|------|
| `p1` | 検索対象文字列 |
| `p2` | 検索開始位置（省略時: 0） |
| `search` | 検索文字列 |

**戻り値:** 見つかった位置（見つからない場合: -1）

**使用例:**

```cpp
int64_t pos = instr("Hello World", "World");  // 6
int64_t pos2 = instr("abcabc", 3, "abc");     // 3（2番目のabc）
```

---

### strmid

文字列の一部を取り出します。

```cpp
[[nodiscard]] std::string strmid(const std::string& p1, int64_t p2, int64_t p3);
```

| パラメータ | 説明 |
|-----------|------|
| `p1` | 元の文字列 |
| `p2` | 開始位置（-1で末尾から） |
| `p3` | 取り出す文字数 |

**使用例:**

```cpp
std::string s = strmid("Hello World", 0, 5);   // "Hello"
std::string t = strmid("Hello World", 6, 5);   // "World"
std::string u = strmid("Hello World", -5, 5);  // "World"（末尾から5文字）
```

---

### strtrim

指定した文字を取り除きます。

```cpp
[[nodiscard]] std::string strtrim(const std::string& p1, int p2 = 0, int p3 = 32);
```

| パラメータ | 説明 |
|-----------|------|
| `p1` | 元の文字列 |
| `p2` | 0=両端, 1=左端のみ, 2=右端のみ, 3=すべて |
| `p3` | 取り除く文字コード（デフォルト: スペース） |

---

### strrep

文字列を置換します。

```cpp
int64_t strrep(std::string& p1, const std::string& search, const std::string& replace);
```

**戻り値:** 置換された回数

**使用例:**

```cpp
std::string s = "Hello World World";
int count = strrep(s, "World", "HSPPP");  // count=2, s="Hello HSPPP HSPPP"
```

---

### getstr

バッファから文字列を読み出します。

```cpp
int64_t getstr(std::string& dest, const std::string& src, int64_t index,
               int delimiter = 0, int64_t maxLen = 1024);
int64_t getstr(std::string& dest, const std::vector<uint8_t>& src, int64_t index,
               int delimiter = 0, int64_t maxLen = 1024);
```

| パラメータ | 説明 |
|-----------|------|
| `dest` | 結果を格納する文字列 |
| `src` | ソースバッファ |
| `index` | 読み出し開始位置 |
| `delimiter` | 区切り文字コード（0=改行, 他=指定文字） |
| `maxLen` | 最大読み出し長 |

**戻り値:** 読み出したバイト数

---

### split

文字列を区切り文字で分割します。

```cpp
std::vector<std::string> split(const std::string& src, const std::string& delimiter);
```

**使用例:**

```cpp
auto parts = split("apple,banana,cherry", ",");
// parts = {"apple", "banana", "cherry"}
```

---

### getpath

パスの一部を取得します。

```cpp
[[nodiscard]] std::string getpath(const std::string& p1, int p2);
```

| p2 | 説明 |
|----|------|
| 0 | フルパス |
| 1 | ディレクトリ |
| 2 | ファイル名（拡張子含む） |
| 8 | ファイル名（拡張子なし） |
| 16 | 拡張子 |
| 32 | 小文字に変換 |

フラグは組み合わせ可能です。

**使用例:**

```cpp
std::string path = "C:\\Users\\Name\\document.txt";
std::string dir  = getpath(path, 1);   // "C:\\Users\\Name\\"
std::string file = getpath(path, 2);   // "document.txt"
std::string name = getpath(path, 8);   // "document"
std::string ext  = getpath(path, 16);  // ".txt"
```

---

## 文字コード変換

### cnvstow

UTF-8文字列をUTF-16（unicode）に変換します。

```cpp
[[nodiscard]] std::u16string cnvstow(const std::string& str);
```

---

### cnvwtos

UTF-16（unicode）をUTF-8文字列に変換します。

```cpp
[[nodiscard]] std::string cnvwtos(const std::u16string& wstr);
```

---

### cnvstoa

UTF-8文字列をANSI（Shift_JIS）に変換します。

```cpp
[[nodiscard]] std::string cnvstoa(const std::string& str);
```

---

### cnvatos

ANSI（Shift_JIS）をUTF-8文字列に変換します。

```cpp
[[nodiscard]] std::string cnvatos(const std::string& astr);
```

---

## メモリノートパッド（HSP互換）

行単位でテキストを操作する命令群です。

### notesel

操作対象バッファを指定します。

```cpp
void notesel(std::string& buffer);
```

---

### noteunsel

対象バッファの指定を解除します。

```cpp
void noteunsel();
```

---

### noteadd

行を追加・変更します。

```cpp
void noteadd(std::string_view text, OptInt index = {}, OptInt overwrite = {});
```

| パラメータ | 説明 |
|-----------|------|
| `text` | 追加する文字列 |
| `index` | 挿入位置（省略時: 末尾、-1も末尾） |
| `overwrite` | 0=挿入, 1=上書き |

---

### notedel

行を削除します。

```cpp
void notedel(int index);
```

---

### noteget

指定行を読み込みます。

```cpp
void noteget(std::string& dest, OptInt index = {});
```

---

### noteload / notesave

ファイルの読み書きを行います。

```cpp
void noteload(std::string_view filename, OptInt maxSize = {});
void notesave(std::string_view filename);
```

---

### notefind

文字列を検索します。

```cpp
int notefind(std::string_view search, OptInt mode = {});
```

| mode定数 | 値 | 説明 |
|---------|---|------|
| `notefind_match` | 0 | 完全一致 |
| `notefind_first` | 1 | 先頭一致 |
| `notefind_instr` | 2 | 部分一致 |

**戻り値:** 見つかった行番号（見つからない場合: -1）

---

### noteinfo

バッファ情報を取得します。

```cpp
int noteinfo(OptInt mode = {});
```

| mode定数 | 値 | 説明 |
|---------|---|------|
| `notemax` | 0 | 行数 |
| `notesize` | 1 | 総バイト数 |

---

## NotePadクラス（OOP版）

```cpp
class NotePad {
public:
    NotePad();                              // 空のノートパッド
    explicit NotePad(std::string_view text); // 文字列から構築
    explicit NotePad(std::string&& text) noexcept;
    
    // 情報取得
    [[nodiscard]] size_t count() const noexcept;    // 行数
    [[nodiscard]] bool empty() const noexcept;      // 空かどうか
    [[nodiscard]] size_t size() const noexcept;     // 総バイト数
    
    // 行操作
    [[nodiscard]] std::string get(size_t index) const;
    NotePad& add(std::string_view text, int index = -1, int overwrite = 0);
    NotePad& del(size_t index);
    NotePad& clear() noexcept;
    
    // 検索
    [[nodiscard]] int find(std::string_view search, int mode = 0, size_t startIndex = 0) const;
    
    // ファイル操作
    NotePad& load(std::string_view filename, size_t maxSize = 0);
    [[nodiscard]] bool save(std::string_view filename) const;
    
    // バッファアクセス
    [[nodiscard]] std::string& buffer() noexcept;
    [[nodiscard]] const std::string& buffer() const noexcept;
    [[nodiscard]] const std::string& toString() const noexcept;
    explicit operator const std::string&() const noexcept;
};
```

**使用例:**

```cpp
NotePad note;
note.load("readme.txt");

// 全行を表示
for (size_t i = 0; i < note.count(); i++) {
    logmes(note.get(i));
}

// 行を追加
note.add("New line at end");
note.add("Insert at line 3", 2);

// 検索
int line = note.find("keyword", notefind_instr);
if (line >= 0) {
    logmes(format("Found at line {}", line));
}

// 保存
note.save("output.txt");
```

---

## 書式化

### strf

書式付き文字列を変換します（HSP互換）。

```cpp
[[nodiscard]] std::string strf(const std::string& format);
[[nodiscard]] std::string strf(const std::string& format, int arg1);
[[nodiscard]] std::string strf(const std::string& format, double arg1);
[[nodiscard]] std::string strf(const std::string& format, const std::string& arg1);
[[nodiscard]] std::string strf(const std::string& format, int arg1, int arg2);
// ... 他のオーバーロード
```

### format（C++20 std::format）

推奨される書式化方法です。

```cpp
using std::format;
```

**使用例:**

```cpp
// HSP互換スタイル
std::string s1 = strf("Value: %d", 42);
std::string s2 = strf("Float: %.2f", 3.14159);

// C++20 format（推奨）
std::string s3 = format("Value: {}", 42);
std::string s4 = format("Float: {:.2f}", 3.14159);
std::string s5 = format("{} + {} = {}", 1, 2, 3);
```

---

## デバッグ

### logmes

デバッグメッセージを出力します。

```cpp
void logmes(std::string_view message);
void logmes(int value);
void logmes(double value);
```

出力先: デバッガの出力ウィンドウ（OutputDebugString）

**使用例:**

```cpp
logmes("Processing started");
logmes(42);
logmes(format("x={}, y={}", x, y));
```

---

## 再エクスポート

標準ライブラリの文字列関連機能が再エクスポートされています。

### 書式化（std::format）

```cpp
using std::format;
using std::format_to;
using std::format_to_n;
using std::formatted_size;
using std::vformat;
using std::vformat_to;
using std::make_format_args;
using std::format_error;
```

### 文字列型

```cpp
using std::string;
using std::wstring;
using std::u8string;
using std::u16string;
using std::u32string;

using std::string_view;
using std::wstring_view;
using std::u8string_view;
using std::u16string_view;
using std::u32string_view;
```

### 数値変換

```cpp
using std::to_string;
using std::to_wstring;
using std::stoi;
using std::stol;
using std::stoll;
using std::stoul;
using std::stoull;
using std::stof;
using std::stod;
using std::stold;
```

### アルゴリズム

```cpp
using std::transform;
using std::copy;
using std::copy_if;
using std::fill;
using std::find;
using std::find_if;
using std::find_if_not;
using std::count;
using std::count_if;
using std::replace;
using std::replace_if;
using std::remove;
using std::remove_if;
using std::unique;
using std::reverse;
using std::sort;
using std::stable_sort;
using std::all_of;
using std::any_of;
using std::none_of;
using std::equal;
using std::mismatch;
using std::search;
```

---

## 参照

- [ファイル操作 API](file.md)
- [数学 API](math.md)（str, toInt, toDouble の数値版）
