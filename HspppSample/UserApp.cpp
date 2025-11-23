// HspppSample/UserApp.cpp
import hsppp;
using namespace hsppp;

// ユーザーのエントリーポイント
int hspMain() {
    // ウィンドウ作成
    screen(0, 800, 600, 0, "HSPPP Sample Window");

    int x = 100;
    int y = 100;

    // メインループ
    // await() が 0 を返したらウィンドウが閉じられた等の終了合図
    while (true) {
        // 描画開始 (Back Buffer)
        redraw(0);

        // 背景クリア
        color(255, 255, 255);
        boxf(); // 全画面

        // 文字描画
        color(0, 0, 0);
        pos(x, y);
        mes("Hello, HSPPP World!");

        // 描画反映 (Front Buffer / Present)
        redraw(1);

        // 16ms待機 ＆ Windowsメッセージ処理
        // ここでウィンドウの「閉じる」ボタン等が処理される
        if (await(16) == 0) {
            break;
        }
    }

    return 0;
}
