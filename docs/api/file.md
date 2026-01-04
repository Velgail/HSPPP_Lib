---
layout: default
title: ファイルAPI
---

# ファイル操作 API リファレンス

ファイル・ディレクトリ操作に関するAPIです。

## 目次

- [ファイル操作](#ファイル操作)
- [バイナリ操作](#バイナリ操作)
- [ディレクトリ操作](#ディレクトリ操作)
- [ダイアログ](#ダイアログ)
- [システム情報](#システム情報)

---

## ファイル操作

### exist

ファイルの存在確認とサイズ取得を行います。

```cpp
[[nodiscard]] int64_t exist(const std::string& filename);
```

**戻り値:**
- ファイルが存在する場合: ファイルサイズ（バイト）
- ファイルが存在しない場合: -1

**使用例:**

```cpp
int64_t size = exist("data.txt");
if (size >= 0) {
    logmes(format("File size: {} bytes", size));
} else {
    logmes("File not found");
}
```

---

### exec

Windowsのファイルを実行します。

```cpp
int exec(const std::string& filename, OptInt mode = {}, const std::string& command = "");
```

| mode定数 | 値 | 説明 |
|---------|---|------|
| `exec_normal` | 0 | ノーマル実行 |
| `exec_minimized` | 2 | 最小化モードで実行 |
| `exec_shellexec` | 16 | 関連付けされたアプリケーションを実行 |
| `exec_print` | 32 | ファイルを印刷する |

**使用例:**

```cpp
exec("notepad.exe", exec_normal);
exec("https://example.com", exec_shellexec);  // ブラウザで開く
exec("document.pdf", exec_shellexec);         // 関連付けアプリで開く
```

---

### deletefile

ファイルを削除します。

```cpp
void deletefile(const std::string& filename);
```

---

### bcopy

ファイルをコピーします。

```cpp
void bcopy(const std::string& src, const std::string& dest);
```

---

## バイナリ操作

### bload

バイナリファイルを読み込みます。

```cpp
// string版
int64_t bload(const std::string& filename, std::string& buffer,
              OptInt64 size = {}, OptInt64 offset = {});

// vector版
int64_t bload(const std::string& filename, std::vector<uint8_t>& buffer,
              OptInt64 size = {}, OptInt64 offset = {});
```

| パラメータ | 説明 |
|-----------|------|
| `filename` | ファイル名 |
| `buffer` | 読み込み先バッファ |
| `size` | 読み込みサイズ（省略時: ファイル全体） |
| `offset` | 読み込み開始位置（省略時: 0） |

**戻り値:** 読み込んだバイト数

**使用例:**

```cpp
std::string text;
bload("data.txt", text);

std::vector<uint8_t> binary;
bload("data.bin", binary);

// 部分読み込み
bload("large.dat", binary, 1024, 512);  // 512バイト目から1024バイト読み込み
```

---

### bsave

バイナリファイルを保存します。

```cpp
// string版
int64_t bsave(const std::string& filename, const std::string& buffer,
              OptInt64 size = {}, OptInt64 offset = {});

// vector版
int64_t bsave(const std::string& filename, const std::vector<uint8_t>& buffer,
              OptInt64 size = {}, OptInt64 offset = {});
```

| パラメータ | 説明 |
|-----------|------|
| `filename` | ファイル名 |
| `buffer` | 保存するデータ |
| `size` | 保存サイズ（省略時: バッファ全体） |
| `offset` | バッファ内の開始位置（省略時: 0） |

**戻り値:** 書き込んだバイト数

---

### noteload / notesave

テキストファイルの読み書きを行います。

```cpp
void noteload(std::string_view filename, OptInt maxSize = {});
void notesave(std::string_view filename);
```

`notesel` で選択されたバッファに対して操作します。

**使用例:**

```cpp
std::string buffer;
notesel(buffer);
noteload("readme.txt");
// buffer にテキストが読み込まれる
```

---

## ディレクトリ操作

### chdir

カレントディレクトリを変更します。

```cpp
void chdir(const std::string& dirname);
```

---

### mkdir

ディレクトリを作成します。

```cpp
void mkdir(const std::string& dirname);
```

---

### dirlist

ディレクトリ内のファイル一覧を取得します。

```cpp
[[nodiscard]] std::vector<std::string> dirlist(const std::string& filemask, OptInt mode = {});
```

| mode | 説明 |
|------|------|
| 0 | ファイルのみ |
| 1 | ディレクトリのみ |
| 5 | ファイルとディレクトリ |

**使用例:**

```cpp
auto files = dirlist("*.txt");
for (const auto& f : files) {
    logmes(f);
}

auto dirs = dirlist("*", 1);  // ディレクトリのみ
```

---

### dirinfo

ディレクトリ情報を取得します。

```cpp
[[nodiscard]] std::string dirinfo(int type);
```

| type定数 | 値 | 説明 |
|---------|---|------|
| `dir_type_cur` | 0 | カレントディレクトリ |
| `dir_type_exe` | 1 | 実行ファイルのディレクトリ |
| `dir_type_win` | 2 | Windowsディレクトリ |
| `dir_type_sys` | 3 | Windowsシステムディレクトリ |
| `dir_type_cmdline` | 4 | コマンドライン文字列 |

### 便利関数

```cpp
[[nodiscard]] std::string dir_cur();       // カレントディレクトリ
[[nodiscard]] std::string dir_exe();       // 実行ファイルのディレクトリ
[[nodiscard]] std::string dir_win();       // Windowsディレクトリ
[[nodiscard]] std::string dir_sys();       // Windowsシステムディレクトリ
[[nodiscard]] std::string dir_cmdline();   // コマンドライン文字列
[[nodiscard]] std::string dir_desktop();   // デスクトップディレクトリ
[[nodiscard]] std::string dir_mydoc();     // マイドキュメントディレクトリ
```

---

## ダイアログ

### dialog

ダイアログを表示します。

```cpp
DialogResult dialog(const std::string& message, OptInt type = {}, const std::string& option = "");
```

| type定数 | 値 | 説明 |
|---------|---|------|
| `dialog_info` | 0 | 情報ダイアログ |
| `dialog_warning` | 1 | 警告ダイアログ |
| `dialog_yesno` | 2 | Yes/No ダイアログ |
| `dialog_yesno_w` | 3 | Yes/No ダイアログ（警告アイコン） |
| `dialog_open` | 16 | ファイルを開くダイアログ |
| `dialog_save` | 17 | ファイルを保存ダイアログ |
| `dialog_color` | 32 | カラー選択ダイアログ |
| `dialog_colorex` | 33 | カラー選択ダイアログ（拡張） |

**戻り値:** `DialogResult` 構造体

**使用例:**

```cpp
// メッセージボックス
dialog("処理が完了しました", dialog_info);

// Yes/No 確認
auto result = dialog("終了しますか？", dialog_yesno);
if (result.stat == 6) {  // Yes
    end();
}

// ファイルを開く
auto result = dialog("画像ファイル", dialog_open, "*.png;*.jpg");
if (result) {
    std::string filename = result;  // refstr を取得
    picload(filename);
}

// ファイルを保存
auto result = dialog("保存先", dialog_save, "*.bmp");
if (result) {
    bmpsave(result.refstr);
}
```

---

## システム情報

### sysinfo_str / sysinfo_int

システム情報を取得します。

```cpp
[[nodiscard]] std::string sysinfo_str(int type);
[[nodiscard]] int64_t sysinfo_int(int type);
```

| type | sysinfo_str | sysinfo_int |
|------|-------------|-------------|
| 0 | OS名 | - |
| 1 | ユーザー名 | - |
| 2 | コンピュータ名 | - |
| 3 | - | 総メモリ容量 |
| 4 | - | 利用可能メモリ |
| 16 | - | プロセッサ数 |

---

## 使用例

### ファイル選択と読み込み

```cpp
void hspMain() {
    screen(0, 640, 480);
    
    auto result = dialog("テキストファイル", dialog_open, "*.txt");
    if (result) {
        std::string text;
        bload(result.refstr, text);
        
        pos(10, 10);
        mes(text);
    }
    
    return 0;
}
```

### 設定ファイルの保存と読み込み

```cpp
void saveConfig(int volume, int difficulty) {
    std::vector<uint8_t> data(8);
    data[0] = volume;
    data[4] = difficulty;
    bsave("config.dat", data);
}

void loadConfig(int& volume, int& difficulty) {
    if (exist("config.dat") >= 0) {
        std::vector<uint8_t> data;
        bload("config.dat", data);
        if (data.size() >= 8) {
            volume = data[0];
            difficulty = data[4];
        }
    }
}
```

---

## 参照

- [文字列操作 API](/HSPPP_Lib/api/string)（NotePad クラス）
- [型定義](/HSPPP_Lib/api/types)（DialogResult）
