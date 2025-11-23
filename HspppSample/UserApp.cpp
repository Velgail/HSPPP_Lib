// HspppSample/UserApp.cpp
import hsppp;
using namespace hsppp;

// ユーザーのエントリーポイント
int hspMain() {
    // ウィンドウ作成
    screen(0, 800, 600, 0, "HSPPP Sample Window");

    // 簡単な描画処理（現在はスタブなので何も表示されない）
    color(255, 255, 255);
    boxf();  // 背景を白で塗りつぶし

    color(0, 0, 0);
    pos(100, 100);
    mes("Hello, HSPPP World!");

    // hspMainを抜けると、HSPのstop命令と同じ動作になる
    // ウィンドウは表示され続け、×ボタンで終了できる
    return 0;
}
