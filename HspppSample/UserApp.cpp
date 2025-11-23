// HspppSample/UserApp.cpp
import hsppp;
using namespace hsppp;

// ユーザーのエントリーポイント
int hspMain() {
    // ウィンドウ作成
    screen(0, 800, 600, 0, "HSPPP Sample Window");

    // 描画開始
    redraw(0);

    // 背景を白で塗りつぶし
    color(255, 255, 255);
    boxf();

    // 黒い矩形を描画
    color(0, 0, 0);
    boxf(50, 50, 200, 150);

    // 赤いテキストを描画
    color(255, 0, 0);
    pos(100, 200);
    mes("Hello, HSPPP World!");

    // 青いテキストを描画
    color(0, 0, 255);
    pos(100, 250);
    mes("Direct2Dで描画しています");

    // 描画終了＆画面反映
    redraw(1);

    // hspMainを抜けると、HSPのstop命令と同じ動作になる
    // ウィンドウは表示され続け、×ボタンで終了できる
    return 0;
}
