%ver
3.7
%date
2023/10/10
%group
入出力制御命令

%type
ユーザー拡張命令

%note
Windows Vista以降の環境でのみ動作します。
mod_mmaudio.asをインクルードすること。

%author
inovia,onitama

%dll
mod_mmaudio

%port
Win


%index
SetMasterVolume
サウンドのマスターボリューム設定
%prm
p1
p1(0) : マスターボリューム値(0〜100)
%inst
Windowsサウンド出力のマスターボリュームを設定します。
p1パラメーターで設定された値が設定されます。0(無音)から100(最大)までの値を設定できます。
サウンドデバイスが存在しないなどエラーが発生した場合は、システム変数statにマイナス値が代入されます。
%href
GetMasterVolume


%index
GetMasterVolume
サウンドのマスターボリューム取得
%prm
()
%inst
Windowsサウンド出力のマスターボリュームを取得し値を返します。
値は、0(無音)から100(最大)までの値になります。
サウンドデバイスが存在しないなどエラーが発生した場合は、マイナス値が返されます。
%href
SetMasterVolume


%index
SetMute
サウンドのミュート設定
%prm
p1
p1(0) : ミュート状態設定値(0=OFF/1=ON)
%inst
Windowsサウンド出力のミュート状態を設定します。
p1パラメーターで設定された値が設定されます。1(ミュート)、0(ミュート解除)の値を設定できます。
サウンドデバイスが存在しないなどエラーが発生した場合は、システム変数statにマイナス値が代入されます。
%href
GetMute


%index
GetMute
サウンドのミュート状態取得
%prm
()
%inst
Windowsサウンド出力のミュート状態を取得します。
1(ミュート)、0(ミュート解除)の値を返します。
サウンドデバイスが存在しないなどエラーが発生した場合は、マイナス値が返されます。
%href
SetMute


%index
GetPeakValue
サウンドのピーク値を取得
%prm
()
%inst
Windowsサウンドが出力しているリアルタイムの波形ピーク値を取得します。
リアルタイムにデバイスで再生されている内容を直接取得するため、レベルメーターなどを再現することができます。
取得される値は、0.0〜1.0までの実数値で、0.0が最も低いレベル、1.0が最も高いレベルを示しています。
チャンネルごとのピーク値を取得する場合は、GetChannelsPeakValues命令を使用してください。
サウンドデバイスが存在しないなどエラーが発生した場合は、マイナス値が返されます。
%href
GetChannelsPeakValues


%index
GetChannelsPeakValues
サウンドのチャンネルごとのピーク値を取得
%prm
var,p1
var   : ピーク値が代入される変数
p1(0) : チャンネル数
%inst
Windowsサウンドが出力しているリアルタイムの波形ピーク値を取得します。
GetPeakValue関数との違いは、チャンネルごとのピーク値を取得する点です。
チャンネルの数だけ、指定された変数にピーク値が代入されます。変数(0),変数(1),変数(2)...の順番に、チャンネルごとの値が実数の配列として代入されます。
あらかじめ、ddim命令によりチャンネルの数を要素として確保した実数型の配列変数を用意する必要があります。
チャンネルの数は、GetMeteringChannelCount関数によって取得することが可能です。
単純なトータルのピーク値を取得する場合は、GetPeakValue命令を使用してください。
%href
GetPeakValue
GetMeteringChannelCount


%index
GetMeteringChannelCount
サウンドの出力チャンネル数を取得
%prm
()
%inst
Windowsサウンドが出力するチャンネル数を取得を取得します。
チャンネル数はサウンドデバイスにより変化します。1の場合はモノラル、2の場合はステレオ出力となります。
サウンドデバイスが存在しないなどエラーが発生した場合は、マイナス値が返されます。
%href
GetChannelsPeakValues




