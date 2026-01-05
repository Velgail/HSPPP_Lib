---
layout: default
title: API リファレンス
---

# API リファレンス

HSPPP の全 API リファレンスです。

## クイックリファレンス（関数一覧）

### ウィンドウ・画面制御

| 関数 | 説明 | 備考 |
|------|------|------|
| [`screen`](/HSPPP_Lib/api/screen#screen) | ウィンドウの初期化 | HSP互換版とOOP版（[Screen クラス](#screen-クラス)、ScreenParams）あり |
| [`buffer`](/HSPPP_Lib/api/screen#buffer) | オフスクリーンバッファの作成 | HSP互換版とOOP版（[Screen クラス](#screen-クラス)、BufferParams）あり |
| [`bgscr`](/HSPPP_Lib/api/screen#bgscr) | 枠なしウィンドウの初期化 | HSP互換版とOOP版（[Screen クラス](#screen-クラス)、BgscrParams）あり |
| [`gsel`](/HSPPP_Lib/api/screen#gsel) | 描画先の変更 | ウィンドウID指定。OOP版は `Screen::select()` |
| [`width`](/HSPPP_Lib/api/screen#width) | ウィンドウサイズの変更 | クライアントサイズ・位置設定。OOP版は `Screen::width()` / `height()` で取得 |
| [`title`](/HSPPP_Lib/api/screen#title) | ウィンドウタイトルの設定 | `std::string_view` 受け取り |
| [`cls`](/HSPPP_Lib/api/screen#cls) | 画面クリア | mode: 0=白, 1=明灰, 2=灰, 3=暗灰, 4=黒 |
| [`redraw`](/HSPPP_Lib/api/screen#redraw) | 再描画制御 | 0=開始, 1=終了 |
| [`groll`](/HSPPP_Lib/api/screen#groll) | スクロール位置設定 | 描画基点座標を設定 |

**詳細:** [画面制御 API](/HSPPP_Lib/api/screen)

---

### 描画

| 関数 | 説明 | 備考 |
|------|------|------|
| [`color`](/HSPPP_Lib/api/drawing#color) | 描画色の設定 | RGB (0-255) |
| [`pos`](/HSPPP_Lib/api/drawing#pos) | 描画位置の設定 | カレントポジション設定 |
| [`line`](/HSPPP_Lib/api/drawing#line) | 直線の描画 | 終点座標、始点は省略可（カレントポジション） |
| [`boxf`](/HSPPP_Lib/api/drawing#boxf) | 矩形の塗りつぶし | 引数なしで画面全体 |
| [`circle`](/HSPPP_Lib/api/drawing#circle) | 円・楕円の描画 | fillMode: 0=線, 1=塗りつぶし |
| [`pset`](/HSPPP_Lib/api/drawing#pset) | 点の描画 | 1ドット描画 |
| [`pget`](/HSPPP_Lib/api/drawing#pget) | 点の色取得 | 取得した色を選択色に設定 |
| [`mes`](/HSPPP_Lib/api/drawing#mes--print) | テキストの描画 | sw: 1=改行なし, 2=影, 4=縁取り等 |
| [`print`](/HSPPP_Lib/api/drawing#mes--print) | テキストの描画 | `mes` の別名 |
| [`gradf`](/HSPPP_Lib/api/drawing#gradf) | グラデーション矩形 | mode: 0=横, 1=縦 |
| [`grect`](/HSPPP_Lib/api/drawing#grect) | 回転矩形 | 角度はラジアン |

**詳細:** [描画 API](/HSPPP_Lib/api/drawing)

---

### 画像

| 関数 | 説明 | 備考 |
|------|------|------|
| [`picload`](/HSPPP_Lib/api/drawing#picload) | 画像ファイルの読み込み | mode: 0=初期化, 1=重ねる, 2=黒初期化 |
| [`bmpsave`](/HSPPP_Lib/api/drawing#bmpsave) | BMPファイルに保存 | |
| [`gcopy`](/HSPPP_Lib/api/screen#gcopy) | 画像のコピー | サイズ省略時はgmode設定 |
| [`gzoom`](/HSPPP_Lib/api/screen#gzoom) | 拡大縮小コピー | mode: 0=高速, 1=高品質 |
| [`grotate`](/HSPPP_Lib/api/drawing#grotate) | 回転コピー | 角度はラジアン |
| [`gmode`](/HSPPP_Lib/api/screen#gmode) | コピーモードの設定 | gmode_copy, gmode_and, gmode_alpha等 |
| [`gsquare`](/HSPPP_Lib/api/drawing#gsquare) | 4頂点描画 | Quad/QuadUV/QuadColors使用 |

**詳細:** [画面制御 API](/HSPPP_Lib/api/screen), [描画 API](/HSPPP_Lib/api/drawing)

---

### GUI コントロール

| 関数 | 説明 | 備考 |
|------|------|------|
| [`button`](/HSPPP_Lib/api/gui#button) | ボタンの作成 | `std::function<int()>` コールバック |
| [`input`](/HSPPP_Lib/api/gui#input) | 入力ボックスの作成 | **`shared_ptr<string>` のみ** |
| [`mesbox`](/HSPPP_Lib/api/gui#mesbox) | メッセージボックスの作成 | **`shared_ptr<string>` のみ** |
| [`chkbox`](/HSPPP_Lib/api/gui#chkbox) | チェックボックスの作成 | **`shared_ptr<int>` のみ** |
| [`combox`](/HSPPP_Lib/api/gui#combox) | コンボボックスの作成 | **`shared_ptr<int>` のみ** |
| [`listbox`](/HSPPP_Lib/api/gui#listbox) | リストボックスの作成 | **`shared_ptr<int>` のみ** |
| [`objsize`](/HSPPP_Lib/api/gui#objsize) | オブジェクトサイズの設定 | デフォルト64x24 |
| [`objmode`](/HSPPP_Lib/api/gui#objmode) | オブジェクトモード設定 | フォント設定モード |
| [`objcolor`](/HSPPP_Lib/api/gui#objcolor) | オブジェクトカラー設定 | RGB (0-255) |
| [`objprm`](/HSPPP_Lib/api/gui#objprm) | オブジェクトパラメータの変更 | 文字列または整数 |
| [`objenable`](/HSPPP_Lib/api/gui#objenable) | オブジェクト有効/無効 | 0=無効, 1=有効 |
| [`objsel`](/HSPPP_Lib/api/gui#objsel) | フォーカス設定 | オブジェクトIDを指定 |
| [`clrobj`](/HSPPP_Lib/api/gui#clrobj) | オブジェクトの削除 | 範囲指定可 |

**詳細:** [GUI API](/HSPPP_Lib/api/gui)

---

### 入力

| 関数 | 説明 | 備考 |
|------|------|------|
| [`stick`](/HSPPP_Lib/api/input#stick) | キー状態の取得 | 戻り値: ビットフラグ |
| [`getkey`](/HSPPP_Lib/api/input#getkey) | 特定キーの状態取得 | 仮想キーコード指定 |
| [`mousex`](/HSPPP_Lib/api/input#mousex) | マウスX座標の取得 | ウィンドウ内座標 |
| [`mousey`](/HSPPP_Lib/api/input#mousey) | マウスY座標の取得 | ウィンドウ内座標 |
| [`mousew`](/HSPPP_Lib/api/input#mousew) | マウスホイール値の取得 | 移動量 |
| [`mouse`](/HSPPP_Lib/api/input#mouse) | マウスカーソル座標設定 | ディスプレイ座標 |

**詳細:** [入力 API](/HSPPP_Lib/api/input)

---

### 割り込み

| 関数 | 説明 | 備考 |
|------|------|------|
| [`onclick`](/HSPPP_Lib/api/interrupt#onclick) | クリック割り込み | `InterruptHandler` または 0/1 |
| [`onkey`](/HSPPP_Lib/api/interrupt#onkey) | キー割り込み | `InterruptHandler` または 0/1 |
| [`oncmd`](/HSPPP_Lib/api/interrupt#oncmd) | Windowsメッセージ割り込み | メッセージID指定 |
| [`onexit`](/HSPPP_Lib/api/interrupt#onexit) | 終了割り込み | `InterruptHandler` または 0/1 |
| [`onerror`](/HSPPP_Lib/api/interrupt#onerror) | エラー割り込み | `ErrorHandler` または 0/1 |
| [`iparam`](/HSPPP_Lib/api/interrupt#iparam--wparam--lparam) | 割り込みパラメータ取得 | wparam相当 |
| [`lparam`](/HSPPP_Lib/api/interrupt#iparam--wparam--lparam) | 割り込みパラメータ取得 | lparam相当 |

**詳細:** [割り込み API](/HSPPP_Lib/api/interrupt)

---

### 時間・待機

| 関数 | 説明 | 備考 |
|------|------|------|
| [`await`](/HSPPP_Lib/api/interrupt#await) | 高精度待機（メッセージ処理並行） | QueryPerformanceCounter使用、マイクロ秒精度 |
| [`vwait`](/HSPPP_Lib/api/interrupt#vwait) | VSync同期待機 | フレームドロップ検出可能、経過時間を返す |
| `gettime` | 時間・日付取得 | 0=年, 1=月, ... 7=ミリ秒 |

**詳細:** [割り込み API](/HSPPP_Lib/api/interrupt)

---

### プログラム制御

| 関数 | 説明 | 備考 |
|------|------|------|
| [`end`](/HSPPP_Lib/api/interrupt#end) | プログラム終了 | `[[noreturn]]` |
| [`stop`](/HSPPP_Lib/api/interrupt#stop) | 実行一時停止 | 割り込み待機 |

**詳細:** [割り込み API](/HSPPP_Lib/api/interrupt)

---

### ファイル

| 関数 | 説明 | 備考 |
|------|------|------|
| [`exist`](/HSPPP_Lib/api/file#exist) | ファイル存在確認 | |
| [`bload`](/HSPPP_Lib/api/file#bload) | バイナリ読み込み | `string&` または `vector<uint8_t>&` |
| [`bsave`](/HSPPP_Lib/api/file#bsave) | バイナリ保存 | `string&` または `vector<uint8_t>&` |
| [`noteload`](/HSPPP_Lib/api/file#noteload) | テキスト読み込み | |
| [`notesave`](/HSPPP_Lib/api/file#notesave) | テキスト保存 | |
| [`dirlist`](/HSPPP_Lib/api/file#dirlist) | ディレクトリ一覧 | `vector<string>` を返す |
| [`dirinfo`](/HSPPP_Lib/api/file#dirinfo) | ディレクトリ情報 | dir_type_* 定数使用 |

**詳細:** [ファイル操作 API](/HSPPP_Lib/api/file)

---

### ダイアログ

| 関数 | 説明 | 備考 |
|------|------|------|
| [`dialog`](/HSPPP_Lib/api/file#dialog) | ダイアログ表示 | `DialogResult` を返す |

**詳細:** [ファイル操作 API](/HSPPP_Lib/api/file)

---

### メディア

| 関数 | 説明 | 備考 |
|------|------|------|
| [`mmload`](/HSPPP_Lib/api/media#mmload) | メディアファイル読み込み | WAV/MP3/MP4等 |
| [`mmplay`](/HSPPP_Lib/api/media#mmplay) | メディア再生 | |
| [`mmstop`](/HSPPP_Lib/api/media#mmstop) | メディア停止 | -1で全停止 |
| [`mmvol`](/HSPPP_Lib/api/media#mmvol) | 音量設定 | -1000〜0 |
| [`mmpan`](/HSPPP_Lib/api/media#mmpan) | パン設定 | -1000〜1000 |
| [`mmstat`](/HSPPP_Lib/api/media#mmstat) | メディア状態取得 | |

**詳細:** [メディア API](/HSPPP_Lib/api/media)

---

### 情報取得

| 関数 | 説明 | 備考 |
|------|------|------|
| [`ginfo`](/HSPPP_Lib/api/screen#ginfo) | ウィンドウ情報取得 | ginfo_type_* 定数使用 |
| [`ginfo_mx`](/HSPPP_Lib/api/screen#便利関数) | マウスX座標取得 | `ginfo(ginfo_type_mx)` の別名 |
| [`ginfo_my`](/HSPPP_Lib/api/screen#便利関数) | マウスY座標取得 | `ginfo(ginfo_type_my)` の別名 |
| [`ginfo_sel`](/HSPPP_Lib/api/screen#便利関数) | 描画先ID取得 | `ginfo(ginfo_type_sel)` の別名 |
| [`ginfo_sizex`](/HSPPP_Lib/api/screen#便利関数) | 画面幅取得 | `ginfo(ginfo_type_sizex)` の別名 |
| [`ginfo_sizey`](/HSPPP_Lib/api/screen#便利関数) | 画面高さ取得 | `ginfo(ginfo_type_sizey)` の別名 |
| [`ginfo_r`](/HSPPP_Lib/api/screen#便利関数) | 描画色R取得 | `ginfo(ginfo_type_r)` の別名 |
| [`ginfo_g`](/HSPPP_Lib/api/screen#便利関数) | 描画色G取得 | `ginfo(ginfo_type_g)` の別名 |
| [`ginfo_b`](/HSPPP_Lib/api/screen#便利関数) | 描画色B取得 | `ginfo(ginfo_type_b)` の別名 |
| [`ginfo_fps`](/HSPPP_Lib/api/screen#便利関数) | リフレッシュレート取得 | `ginfo(ginfo_type_fps)` の別名 |

**詳細:** [画面制御 API](/HSPPP_Lib/api/screen)

---

## OOP版 クラスリファレンス

### Screen クラス

`screen()`, `buffer()`, `bgscr()` が返す軽量ハンドルクラス。メソッドチェーンに対応。

| メソッド | 説明 |
|---------|------|
| `id()` | ウィンドウIDを取得 |
| `valid()` | 有効なハンドルか確認 |
| `select()` | このScreenを描画先に設定（gsel相当） |
| `show()` | ウィンドウを表示（gsel id, 1 相当） |
| `hide()` | ウィンドウを非表示（gsel id, -1 相当） |
| `activate()` | 最前面でアクティブ化（gsel id, 2 相当） |
| `width()` / `height()` | サイズ取得 |
| `color()` | 描画色設定（メソッドチェーン対応） |
| `pos()` | 描画位置設定（メソッドチェーン対応） |
| `line()`, `boxf()`, `circle()` 等 | 描画メソッド |

**詳細:** [画面制御 API - Screen クラス](/HSPPP_Lib/api/screen#screen-クラスoop版)

---

### NotePad クラス

メモリノートパッド（テキストバッファ）のOOP版クラス。

| メソッド | 説明 |
|---------|------|
| `count()` | 行数取得 |
| `empty()` | 空かどうか |
| `size()` | 総バイト数 |
| `get(index)` | 行取得 |
| `add(text, index, overwrite)` | 行追加 |
| `del(index)` | 行削除 |
| `clear()` | 全クリア |
| `find(search, mode, startIndex)` | 検索 |
| `load(filename, maxSize)` | ファイル読み込み |
| `save(filename)` | ファイル保存 |
| `buffer()` | 内部バッファアクセス |

**詳細:** [文字列操作 API - NotePad クラス](/HSPPP_Lib/api/string#notepadクラスoop版)

---

### Media クラス

メディア再生のOOP版クラス。メソッドチェーンに対応。

| メソッド | 説明 |
|---------|------|
| `load(filename)` | ファイル読み込み |
| `unload()` | アンロード |
| `play()` | 再生開始 |
| `stop()` | 停止 |
| `vol(v)` | 音量設定（メソッドチェーン対応） |
| `pan(p)` | パン設定（メソッドチェーン対応） |
| `loop(l)` | ループ設定（メソッドチェーン対応） |
| `mode(m)` | 再生モード設定（メソッドチェーン対応） |
| `target(screenId)` | 動画再生先指定 |
| `stat()` | 状態取得 |
| `playing()` | 再生中かどうか |
| `loaded()` | 読み込み済みかどうか |
| `id()` | メディアID取得 |

**詳細:** [メディア API - Media クラス](/HSPPP_Lib/api/media#基本使用法)

---

### HSPPP独自拡張機能

標準HSPには存在しない、HSPPP独自の拡張機能一覧です。

| 関数 | 説明 | カテゴリ |
|------|------|----------|
| [`vwait`](/HSPPP_Lib/api/interrupt#vwait) | VSync同期待機（フレームドロップ検出） | 時間・待機 |

**詳細:** [割り込み API](/HSPPP_Lib/api/interrupt)

---

#### ステートマシン

| クラス/メソッド | 説明 |
|----------------|------|
| [`StateMachine<T>`](/HSPPP_Lib/api/statemachine) | enum class ベースのステートマシン |
| [`state()`](/HSPPP_Lib/api/statemachine#state) | ステート定義（Builder パターン） |
| [`on_enter()`](/HSPPP_Lib/api/statemachine#on_enter) | ステート初回実行時のコールバック |
| [`on_update()`](/HSPPP_Lib/api/statemachine#on_update) | 毎フレーム実行されるコールバック |
| [`on_exit()`](/HSPPP_Lib/api/statemachine#on_exit) | ステート終了時のコールバック |
| [`jump()`](/HSPPP_Lib/api/statemachine#jump) | 状態遷移（HSP `goto` 相当） |
| [`defer_jump()`](/HSPPP_Lib/api/statemachine#defer_jump) | 状態遷移予約 |
| [`run()`](/HSPPP_Lib/api/statemachine#run) | メインループ実行 |
| [`quit()`](/HSPPP_Lib/api/statemachine#基本メソッド) | ループ終了 |
| [`current_state()`](/HSPPP_Lib/api/statemachine#current_state) | 現在のステート取得 |
| [`previous_state()`](/HSPPP_Lib/api/statemachine#previous_state) | 前回のステート取得 |
| [`frame_count()`](/HSPPP_Lib/api/statemachine#frame_count) | 総フレーム数取得 |
| [`state_frame_count()`](/HSPPP_Lib/api/statemachine#state_frame_count) | ステート滞在フレーム数 |
| [`set_unrestricted_transitions()`](/HSPPP_Lib/api/statemachine#set_unrestricted_transitions) | All-to-All遷移の有効/無効 |
| [`allow_transition()`](/HSPPP_Lib/api/statemachine#allow_transition) | 特定遷移を許可 |
| [`deny_transition()`](/HSPPP_Lib/api/statemachine#deny_transition) | 特定遷移を禁止 |
| [`enable_history()`](/HSPPP_Lib/api/statemachine#enable_history) | 履歴記録を有効化 |
| [`back()`](/HSPPP_Lib/api/statemachine#back) | 前のステートに戻る |
| [`set_timer()`](/HSPPP_Lib/api/statemachine#set_timer) | 指定ミリ秒後に自動遷移 |
| [`cancel_timer()`](/HSPPP_Lib/api/statemachine#cancel_timer) | タイマーキャンセル |
| [`enable_debug_log()`](/HSPPP_Lib/api/statemachine#enable_debug_log) | デバッグログ有効化 |
| [`export_graph()`](/HSPPP_Lib/api/statemachine#export_graph) | Graphviz出力 |

**詳細:** [ステートマシン API](/HSPPP_Lib/api/statemachine)

**関連ガイド:**
- [ステートパターンガイド](/HSPPP_Lib/guides/state-pattern) - 設計パターンと実装例
- [HSP goto 移行ガイド](/HSPPP_Lib/guides/hsp-goto-migration) - HSPからの移行方法

**備考:**
- 各関数の詳細は該当カテゴリのページを参照してください

---

## 設計上の注意

### ライフタイム安全性

以下の関数は `shared_ptr` 版のみ提供しています（[FAQ](/HSPPP_Lib/faq#q-chkbox-に-int-を渡せないのはなぜですか) 参照）：

- `chkbox(label, std::shared_ptr<int>)`
- `combox(std::shared_ptr<int>, expandY, items)`
- `listbox(std::shared_ptr<int>, expandY, items)`

```cpp
// 使用例
auto state = std::make_shared<int>(0);
chkbox("Enable", state);
```

<!-- 根拠: hsppp.ixx のコメントおよび CLAUDE.md の「非安全コーディングの禁止」方針。 -->

### デュアルスタイル

多くの関数は、グローバル関数版と Screen メンバ関数版の両方が使用できます：

```cpp
// グローバル関数（HSP互換）
screen(0, 640, 480);
color(255, 0, 0);
boxf(0, 0, 100, 100);

// Screen メンバ関数（OOP）
auto win = screen({.width = 640, .height = 480});
win.color(255, 0, 0).boxf(0, 0, 100, 100);
```

<!-- 根拠: CLAUDE.md の「デュアルスタイル対応」。 -->

---

## 型定義

型定義の詳細は [型定義リファレンス](/HSPPP_Lib/api/types) を参照してください。

### 主要な型

| 型 | 説明 |
|----|------|
| `OptInt` / `OptDouble` | 省略可能なパラメータ（`omit` または `{}` で省略） |
| `Screen` | ウィンドウ/バッファの軽量ハンドル |
| `Cel` | 画像素材の軽量ハンドル |
| `Media` | メディア再生クラス（OOP版） |
| `NotePad` | メモリノートパッドクラス（OOP版） |
| `Quad` / `QuadUV` / `QuadColors` | gsquare用4頂点構造体 |
| `DialogResult` | dialog命令の戻り値 |
| `InterruptHandler` / `ErrorHandler` | 割り込みハンドラ型 |
| `HspError` / `HspWeakError` | エラー例外クラス |

---

## 参照

- [FAQ](/HSPPP_Lib/faq)
- [チュートリアル](/HSPPP_Lib/guides/tutorial)
- [HSPからの移行ガイド](/HSPPP_Lib/guides/migration-from-hsp)
