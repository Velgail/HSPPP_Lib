# API リファレンス

HSPPP の全 API リファレンスです。

## 詳細リファレンス（カテゴリ別）

各カテゴリの詳細なドキュメントは以下のページを参照してください：

| カテゴリ | 説明 |
|---------|------|
| [画面制御](screen.md) | screen, buffer, gsel, gmode, gcopy, gzoom 等 |
| [描画](drawing.md) | color, pos, mes, boxf, circle, line, gradf, grect, gsquare 等 |
| [入力](input.md) | stick, getkey, mousex, mousey, mouse 等 |
| [GUI](gui.md) | button, input, mesbox, chkbox, combox, listbox 等 |
| [ファイル操作](file.md) | exist, bload, bsave, dialog, dirlist 等 |
| [割り込み](interrupt.md) | onclick, onkey, onexit, onerror, stop, await, wait 等 |
| [文字列操作](string.md) | strmid, instr, strrep, split, NotePad クラス等 |
| [数学](math.md) | rnd, deg2rad, limit, イージング, 標準数学関数等 |
| [メディア](media.md) | mmload, mmplay, mmstop, Media クラス等 |
| [型定義](types.md) | OptInt, Screen, Cel, Quad, DialogResult 等 |

---

## クイックリファレンス（関数一覧）

### ウィンドウ・画面制御

| 関数 | 説明 | 備考 |
|------|------|------|
| `screen` | ウィンドウの初期化 | HSP互換版とOOP版（ScreenParams）あり |
| `buffer` | オフスクリーンバッファの作成 | HSP互換版とOOP版（BufferParams）あり |
| `bgscr` | 枠なしウィンドウの初期化 | HSP互換版とOOP版（BgscrParams）あり |
| `gsel` | 描画先の変更 | ウィンドウID指定 |
| `width` | ウィンドウサイズの変更 | クライアントサイズ・位置設定 |
| `title` | ウィンドウタイトルの設定 | `std::string_view` 受け取り |
| `cls` | 画面クリア | mode: 0=白, 1=明灰, 2=灰, 3=暗灰, 4=黒 |
| `redraw` | 再描画制御 | 0=開始, 1=終了 |
| `groll` | スクロール位置設定 | 描画基点座標を設定 |

### 描画

| 関数 | 説明 | 備考 |
|------|------|------|| `color` | 描画色の設定 | RGB (0-255) |
| `pos` | 描画位置の設定 | カレントポジション設定 |
| `line` | 直線の描画 | 終点座標、始点は省略可（カレントポジション） |
| `boxf` | 矩形の塗りつぶし | 引数なしで画面全体 |
| `circle` | 円・楕円の描画 | fillMode: 0=線, 1=塗りつぶし |
| `pset` | 点の描画 | 1ドット描画 |
| `pget` | 点の色取得 | 取得した色を選択色に設定 |
| `mes` | テキストの描画 | sw: 1=改行なし, 2=影, 4=縁取り等 |
| `print` | テキストの描画 | `mes` の別名 |
| `gradf` | グラデーション矩形 | mode: 0=横, 1=縦 |
| `grect` | 回転矩形 | 角度はラジアン |

### 画像

| 関数 | 説明 | 備考 |
|------|------|------|| `picload` | 画像ファイルの読み込み | mode: 0=初期化, 1=重ねる, 2=黒初期化 |
| `bmpsave` | BMPファイルに保存 | |
| `gcopy` | 画像のコピー | サイズ省略時はgmode設定 |
| `gzoom` | 拡大縮小コピー | mode: 0=高速, 1=高品質 |
| `grotate` | 回転コピー | 角度はラジアン |
| `gmode` | コピーモードの設定 | gmode_copy, gmode_and, gmode_alpha等 |
| `gsquare` | 4頂点描画 | Quad/QuadUV/QuadColors使用 |

### GUI コントロール

| 関数 | 説明 | 備考 |
|------|------|------|| `button` | ボタンの作成 | `std::function<int()>` コールバック |
| `input` | 入力ボックスの作成 | `string&` または `shared_ptr<string>` |
| `mesbox` | メッセージボックスの作成 | `string&` または `shared_ptr<string>` |
| `chkbox` | チェックボックスの作成 | **`shared_ptr<int>` のみ** |
| `combox` | コンボボックスの作成 | **`shared_ptr<int>` のみ** |
| `listbox` | リストボックスの作成 | **`shared_ptr<int>` のみ** |
| `objsize` | オブジェクトサイズの設定 | デフォルト64x24 |
| `objmode` | オブジェクトモード設定 | フォント設定モード |
| `objcolor` | オブジェクトカラー設定 | RGB (0-255) |
| `objprm` | オブジェクトパラメータの変更 | 文字列または整数 |
| `objenable` | オブジェクト有効/無効 | 0=無効, 1=有効 |
| `objsel` | フォーカス設定 | オブジェクトIDを指定 |
| `clrobj` | オブジェクトの削除 | 範囲指定可 |

### 入力

| 関数 | 説明 | 備考 |
|------|------|------|| `stick` | キー状態の取得 | 戻り値: ビットフラグ |
| `getkey` | 特定キーの状態取得 | 仮想キーコード指定 |
| `mousex` | マウスX座標の取得 | ウィンドウ内座標 |
| `mousey` | マウスY座標の取得 | ウィンドウ内座標 |
| `mousew` | マウスホイール値の取得 | 移動量 |
| `mouse` | マウスカーソル座標設定 | ディスプレイ座標 |

### 割り込み

| 関数 | 説明 | 備考 |
|------|------|------|| `onclick` | クリック割り込み | `InterruptHandler` または 0/1 |
| `onkey` | キー割り込み | `InterruptHandler` または 0/1 |
| `oncmd` | Windowsメッセージ割り込み | メッセージID指定 |
| `onexit` | 終了割り込み | `InterruptHandler` または 0/1 |
| `onerror` | エラー割り込み | `ErrorHandler` または 0/1 |
| `iparam` | 割り込みパラメータ取得 | wparam相当 |
| `lparam` | 割り込みパラメータ取得 | lparam相当 |

### 時間・待機

| 関数 | 説明 | 備考 |
|------|------|------|| `await` | 待機（CPU負荷あり） | ミリ秒指定 |
| `wait` | 待機（CPU負荷軽） | 10ms単位 |
| `gettime` | 時間・日付取得 | 0=年, 1=月, ... 7=ミリ秒 |

### プログラム制御

| 関数 | 説明 | 備考 |
|------|------|------|| `end` | プログラム終了 | `[[noreturn]]` |
| `stop` | 実行一時停止 | 割り込み待機 |

### ファイル

| 関数 | 説明 | 備考 |
|------|------|------|| `exist` | ファイル存在確認 | |
| `bload` | バイナリ読み込み | `string&` または `vector<uint8_t>&` |
| `bsave` | バイナリ保存 | `string&` または `vector<uint8_t>&` |
| `noteload` | テキスト読み込み | |
| `notesave` | テキスト保存 | |
| `dirlist` | ディレクトリ一覧 | `vector<string>` を返す |
| `dirinfo` | ディレクトリ情報 | dir_type_* 定数使用 |

### ダイアログ

| 関数 | 説明 | 備考 |
|------|------|------|| `dialog` | ダイアログ表示 | `DialogResult` を返す |

### メディア

| 関数 | 説明 | 備考 |
|------|------|------|| `mmload` | メディアファイル読み込み | WAV/MP3/MP4等 |
| `mmplay` | メディア再生 | |
| `mmstop` | メディア停止 | -1で全停止 |
| `mmvol` | 音量設定 | -1000〜0 |
| `mmpan` | パン設定 | -1000〜1000 |
| `mmstat` | メディア状態取得 | |

### 情報取得

| 関数 | 説明 | 備考 |
|------|------|------|| `ginfo` | ウィンドウ情報取得 | ginfo_type_* 定数使用 |
| `ginfo_*` | 個別情報取得関数 | ginfo_mx, ginfo_my, ginfo_sel 等 |

---

## 設計上の注意

### ライフタイム安全性

以下の関数は `shared_ptr` 版のみ提供しています（[FAQ](../faq.md#q-chkbox-に-int-を渡せないのはなぜですか) 参照）：

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

型定義の詳細は [型定義リファレンス](types.md) を参照してください。

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

- [FAQ](../faq.md)
- [チュートリアル](../guides/tutorial.md)
- [HSPからの移行ガイド](../guides/migration-from-hsp.md)
