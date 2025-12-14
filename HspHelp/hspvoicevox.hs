;
;	HSP help manager用 HELPソースファイル
;	(先頭が「;」の行はコメントとして処理されます)
;

%type
拡張命令
%ver
3.7
%note
hspvoicevox.asをインクルードすること。
準備が必要ですので最初に必ず使用方法(hspvoicevox.html)をお読みください。

%date
2023/10/17
%author
onitama
%dll
hspogg
%url
https://hsp.tv/
%port
Win
%portinfo
Windows 10以降の環境が必要です。


%index
voicevoxinit
VOICEVOX COREによる音声合成の初期化
%group
拡張マルチメディア制御命令
%prm
%inst
VOICEVOX COREによる音声合成の初期化を行ないます。
VOICEVOXにより、高い精度の音声読み上げを行うことが可能になります。
プログラム実行時の最初に１回だけ初期化を行なう必要があります。
これ以降は、voicevox〜で始まる命令を使用して、VOICEVOX COREによる音声合成機能を使用することが可能になります。
命令実行後にシステム変数statに結果が代入されます。システム変数statが0の場合は正常に初期化が完了しています。
システム変数statの値が0以外の場合は、初期化に失敗しています。その場合は、voicevoxgeterror命令でエラー文字列を取得することが可能です。
%href
voicevoxbye
voicevoxgeterror


%index
voicevoxbye
VOICEVOX COREによる音声合成の終了処理
%group
拡張マルチメディア制御命令
%prm
%inst
VOICEVOX COREによる音声合成の終了処理を行ないます。
通常は、終了処理が自動的に行なわれるので、この命令を入れる必要はありません。
%href
voicevoxinit


%index
voicevoxload
VOICEVOX話者のデータを読み込み
%group
拡張マルチメディア制御命令
%prm
p1
p1(0) : 話者ID
%inst
VOICEVOXによる音声合成の準備を行います。
p1で指定したVOICEVOX話者IDのデータを読み込みます。
音声合成を行うためには、最初に話者IDを指定して準備を行う必要があります。
VOICEVOX話者のデータは、メモリの制限内で複数を読み込むことができます。
準備のできた話者IDは、voicevoxspeakなどの命令で音声の再生が可能になります。
話者IDに指定する値は、modelフォルダ内のmetas.jsonファイルに記述されています。この情報は、voicevoxgetmetas命令でも取得することができます。
命令実行後にシステム変数statに結果が代入されます。システム変数statが0の場合は正常に準備が完了しています。
システム変数statの値が0以外の場合は、準備に失敗しています。その場合は、voicevoxgeterror命令でエラー文字列を取得することが可能です。
%href
voicevoxgetmetas
voicevoxspeak
voicevoxmmload
voicevoxsave


%index
voicevoxspeak
VOICEVOXによる読み上げを実行する
%group
拡張マルチメディア制御命令
%prm
"text", p1, p2
"text" : 
p1(0)  : 話者ID
p2(0)  : メディアバッファID
%inst
VOICEVOXによる読み上げを行います。
"text"パラメーターで指定された文字列を読み上げます。
あらかじめ、指定された話者のデータをvoicevoxload命令により読み込んでおく必要があります。
命令実行後にシステム変数statに結果が代入されます。システム変数statが0の場合は正常に実行が完了しています。
この命令は、p2パラメーターで指定されたメディアバッファを通して音声の再生を行います。
一度再生されたメディアバッファは、mmplay命令で再度再生することが可能です。メディアバッファへの登録だけを行って、再生のタイミングをコントロールしたい場合は、voicevoxmmload命令を使用してください。
システム変数statの値が0以外の場合は、実行に失敗しています。その場合は、voicevoxgeterror命令でエラー文字列を取得することが可能です。
%href
voicevoxload
voicevoxmmload


%index
voicevoxmmload
VOICEVOXによる読み上げ音声をメディアバッファに登録する
%group
拡張マルチメディア制御命令
%prm
"text", p1, p2
"text" : 
p1(0)  : 話者ID
p2(0)  : メディアバッファID
%inst
VOICEVOXによる読み上げを行います。
"text"パラメーターで指定された文字列を読み上げて、その音声をメディアバッファに登録します。
あらかじめ、指定された話者のデータをvoicevoxload命令により読み込んでおく必要があります。
命令実行後にシステム変数statに結果が代入されます。システム変数statが0の場合は正常に登録が完了しています。
その場合は、mmplay命令により登録されたメディアバッファIDを再生することができます。
システム変数statの値が0以外の場合は、登録に失敗しています。その場合は、voicevoxgeterror命令でエラー文字列を取得することが可能です。
%href
voicevoxload
mmplay


%index
voicevoxsave
VOICEVOXによる読み上げ音声をファイルに保存する
%group
拡張マルチメディア制御命令
%prm
"text", p1, "filename"
"text" : 
p1(0)  : 話者ID
"filename" : 保存するファイル名
%inst
VOICEVOXによる読み上げを行います。
"text"パラメーターで指定された文字列を読み上げて、その音声をwav形式のファイルとして保存します。
あらかじめ、指定された話者のデータをvoicevoxload命令により読み込んでおく必要があります。
命令実行後にシステム変数statに結果が代入されます。システム変数statが0の場合は正常に保存が完了しています。
システム変数statの値が0以外の場合は、作成に失敗しています。その場合は、voicevoxgeterror命令でエラー文字列を取得することが可能です。
%href
voicevoxload
mmplay


%index
voicevoxgeterror
VOICEVOXのエラー文字列を取得する
%group
拡張マルチメディア制御命令
%prm
var
var : 結果が代入される変数
%inst
VOICEVOXの処理中にエラーが発生した場合の、エラー文字列を取得します。
p1パラメーターで指定された変数に結果が代入されます。
%href
voicevoxinit


%index
voicevoxgetversion
VOICEVOXのバージョン文字列を取得する
%group
拡張マルチメディア制御命令
%prm
var
var : 結果が代入される変数
%inst
VOICEVOXのバージョン文字列を取得します。
p1パラメーターで指定された変数に結果が代入されます。
%href
voicevoxinit


%index
voicevoxgetmetas
VOICEVOXのメタ情報を取得する
%group
拡張マルチメディア制御命令
%prm
var
var : 結果が代入される変数
%inst
VOICEVOXのメタ情報を取得します。
これはjson形式で書かれた文字列の情報になります。
p1パラメーターで指定された変数に結果が代入されます。
%href
voicevoxinit


