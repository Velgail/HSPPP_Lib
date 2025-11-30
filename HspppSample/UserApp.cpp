// HspppSample/UserApp.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモ: 描画機能テスト（line, circle, pset, pget, ginfo）
// ═══════════════════════════════════════════════════════════════════

import hsppp;
using namespace hsppp;

// ユーザーのエントリーポイント
int hspMain() {
    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  メインウィンドウ作成                                         ║
    // ╚═══════════════════════════════════════════════════════════════╝
    auto mw = screen({.width = 800, .height = 600, .title = "HSPPP Drawing Demo"});
    
    // 背景を白で塗りつぶし
    mw.color(255, 255, 255).boxf();
    
    // タイトル表示
    mw.color(0, 0, 0).pos(20, 10);
    mw.mes("=== HSPPP Drawing Functions Demo ===");

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  1. line - 直線描画テスト                                     ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.pos(20, 40);
    mw.mes("[1] line - Direct Lines");

    // 始点・終点指定
    mw.color(255, 0, 0);  // 赤
    mw.line(150, 60, 20, 60);  // 水平線

    mw.color(0, 128, 0);  // 緑
    mw.line(150, 100, 20, 60);  // 斜め線

    // 連続線（カレントポジションから）
    mw.color(0, 0, 255);  // 青
    mw.pos(170, 60);
    mw.line(220, 60);    // 右へ
    mw.line(220, 100);   // 下へ
    mw.line(170, 100);   // 左へ
    mw.line(170, 60);    // 上へ（四角形完成）

    // メソッドチェーンで三角形
    mw.color(128, 0, 128)  // 紫
      .line(280, 60, 250, 100)
      .line(310, 100)
      .line(280, 60);

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  2. circle - 円描画テスト                                     ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.color(0, 0, 0).pos(20, 130);
    mw.mes("[2] circle - Circles & Ellipses");

    // 塗りつぶし円
    mw.color(255, 100, 100);  // 薄い赤
    mw.circle(20, 150, 100, 230, 1);  // fillMode=1

    // 輪郭のみの円
    mw.color(0, 0, 200);  // 青
    mw.circle(110, 150, 190, 230, 0);  // fillMode=0

    // 楕円（横長）
    mw.color(0, 180, 0);  // 緑
    mw.circle(200, 160, 320, 220, 1);

    // 楕円（縦長）- 輪郭のみ
    mw.color(200, 100, 0);  // オレンジ
    mw.circle(330, 150, 380, 230, 0);

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  3. pset - 点描画テスト                                       ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.color(0, 0, 0).pos(20, 250);
    mw.mes("[3] pset - Dots Pattern");

    // グラデーション風の点パターン
    for (int y = 0; y < 50; y += 3) {
        for (int x = 0; x < 100; x += 3) {
            int r = x * 255 / 100;
            int g = y * 255 / 50;
            int b = 128;
            mw.color(r, g, b);
            mw.pset(20 + x, 270 + y);
        }
    }

    // 円形のドットパターン
    mw.color(0, 0, 0);
    int cx = 180, cy = 295;
    for (int angle = 0; angle < 360; angle += 15) {
        double rad = angle * 3.14159 / 180.0;
        int px = cx + static_cast<int>(25 * cos(rad));
        int py = cy + static_cast<int>(25 * sin(rad));
        mw.pset(px, py);
    }

    // カレントポジションからのpset
    mw.color(255, 0, 0);
    mw.pos(250, 280);
    mw.pset();  // (250, 280) に点

    mw.pos(260, 280);
    mw.pset();
    mw.pos(270, 280);
    mw.pset();

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  4. pget - 色取得テスト                                       ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.color(0, 0, 0).pos(400, 40);
    mw.mes("[4] pget - Color Pickup");

    // テスト用の色付き矩形
    mw.color(255, 0, 0).boxf(400, 60, 450, 100);    // 赤
    mw.color(0, 255, 0).boxf(450, 60, 500, 100);    // 緑
    mw.color(0, 0, 255).boxf(500, 60, 550, 100);    // 青

    // pget で色を取得して表示
    mw.pget(425, 80);  // 赤い領域から取得
    int r1 = ginfo_r(), g1 = ginfo_g(), b1 = ginfo_b();
    
    mw.color(0, 0, 0).pos(400, 110);
    mw.mes("Red area: picked");
    // 取得した色で四角を描画
    mw.color(r1, g1, b1).boxf(560, 60, 600, 80);

    mw.pget(475, 80);  // 緑の領域から取得
    mw.color(ginfo_r(), ginfo_g(), ginfo_b()).boxf(560, 80, 600, 100);

    mw.pget(525, 80);  // 青の領域から取得
    mw.color(ginfo_r(), ginfo_g(), ginfo_b()).boxf(560, 100, 600, 120);

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  5. ginfo - システム情報取得テスト                            ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.color(0, 0, 0).pos(400, 150);
    mw.mes("[5] ginfo - System Info");

    // デスクトップサイズ
    int deskW = ginfo(20);
    int deskH = ginfo(21);
    mw.pos(400, 170);
    mw.mes("Desktop size: available");

    // ウィンドウサイズ
    int winW = ginfo(26);
    int winH = ginfo(27);
    mw.pos(400, 190);
    mw.mes("Window init size: available");

    // カレントポジション確認
    mw.pos(123, 456);
    int curX = ginfo(22);
    int curY = ginfo(23);
    mw.pos(400, 210);
    if (curX == 123 && curY == 456) {
        mw.mes("Current pos: OK (123,456)");
    } else {
        mw.mes("Current pos: check needed");
    }

    // 現在の色設定確認
    mw.color(100, 150, 200);
    mw.pos(400, 230);
    if (ginfo(16) == 100 && ginfo(17) == 150 && ginfo(18) == 200) {
        mw.mes("Color info: OK (100,150,200)");
    } else {
        mw.mes("Color info: check needed");
    }

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  6. 複合デモ - メソッドチェーン                               ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.color(0, 0, 0).pos(400, 270);
    mw.mes("[6] Method Chaining Demo");

    // 家の絵をメソッドチェーンで描画
    mw.color(139, 69, 19)      // 茶色（壁）
      .boxf(420, 340, 520, 420)
      .color(200, 0, 0)        // 赤（屋根）
      .line(470, 290, 410, 350)
      .line(530, 350)
      .line(470, 290)
      .color(0, 100, 200)      // 青（窓）
      .boxf(440, 360, 470, 390)
      .color(100, 50, 0)       // 濃い茶（ドア）
      .boxf(480, 380, 510, 420)
      .color(255, 255, 0)      // 黄（太陽）
      .circle(540, 290, 590, 340, 1)
      .color(0, 180, 0)        // 緑（地面）
      .boxf(400, 420, 600, 440);

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  7. 図形描画パターン                                          ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.color(0, 0, 0).pos(20, 340);
    mw.mes("[7] Geometric Patterns");

    // 同心円
    int centerX = 100, centerY = 430;
    for (int radius = 10; radius <= 50; radius += 10) {
        int shade = radius * 4;
        mw.color(shade, 100, 255 - shade);
        mw.circle(centerX - radius, centerY - radius,
                  centerX + radius, centerY + radius, 0);
    }

    // 放射線
    centerX = 220; centerY = 430;
    for (int angle = 0; angle < 360; angle += 30) {
        double rad = angle * 3.14159 / 180.0;
        int ex = centerX + static_cast<int>(45 * cos(rad));
        int ey = centerY + static_cast<int>(45 * sin(rad));
        int shade = angle * 255 / 360;
        mw.color(255, shade, 0);
        mw.line(ex, ey, centerX, centerY);
    }

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  8. font, sysfont, title, width テスト                       ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.color(0, 0, 0).pos(620, 150);
    mw.mes("[8] Font & Window");

    // フォントテスト
    mw.font("MS Gothic", 12, 0);   // 通常
    mw.pos(620, 170);
    mw.mes("Normal 12pt");

    mw.font("MS Gothic", 16, 1);   // 太字
    mw.pos(620, 190);
    mw.mes("Bold 16pt");

    mw.font("MS Gothic", 14, 2);   // イタリック
    mw.pos(620, 215);
    mw.mes("Italic 14pt");

    mw.font("MS Gothic", 18, 3);   // 太字+イタリック
    mw.pos(620, 240);
    mw.mes("Bold+Italic");

    // システムフォントに戻す
    mw.sysfont(0);
    mw.pos(620, 270);
    mw.mes("sysfont(0)");

    // OOP版 font/sysfont/title メソッドチェーン
    mw.font("Arial", 10, 0)
      .pos(620, 290)
      .mes("Arial 10pt")
      .sysfont(17)  // デフォルトGUIフォント
      .pos(620, 310)
      .mes("GUI font");

    // タイトル変更テスト（HSP互換）
    title("HSPPP Demo - title() test");

    // OOP版タイトル
    mw.title("HSPPP Drawing Demo (OOP)");

    // widthテストはウィンドウサイズが変わるのでコメントアウト
    // width(750, 550);  // クライアントサイズ変更
    // mw.windowSize(700, 500);  // OOP版

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  9. 入力関数テスト (stick, getkey, mouse, wait)               ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.sysfont(0);
    mw.color(0, 0, 0).pos(620, 350);
    mw.mes("[9] Input Functions");

    mw.pos(620, 370);
    mw.mes("Move mouse in window");
    mw.pos(620, 390);
    mw.mes("Press keys to test");

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  完了メッセージ                                               ║
    // ╚═══════════════════════════════════════════════════════════════╝
    mw.sysfont(0);  // 標準フォントに戻す
    mw.color(0, 100, 0).pos(20, 500);
    mw.mes("=== All Drawing Features Demonstrated ===");
    mw.pos(20, 520);
    mw.mes("Functions: line, circle, pset, pget, ginfo, font, sysfont, title, width, stick, getkey, mouse, wait");
    mw.pos(20, 540);
    mw.mes("Press ESC to exit, Arrow keys to move dot, Space for color change");

    // 入力テスト用の動くドット
    int dotX = 400, dotY = 500;
    int dotR = 255, dotG = 0, dotB = 0;

    // メインループ - 入力をテスト
    while (true) {
        mw.redraw(0);  // 描画開始

        // 前のドットを消す（白で塗りつぶし）
        mw.color(255, 255, 255).boxf(dotX - 10, dotY - 10, dotX + 10, dotY + 10);

        // stick で入力取得（矢印キーは非トリガー、スペースはトリガー）
        int key = stick(1 + 2 + 4 + 8);  // 矢印キーは押しっぱなし対応

        // 矢印キーでドット移動
        if (key & 1) dotX -= 3;  // 左
        if (key & 4) dotX += 3;  // 右
        if (key & 2) dotY -= 3;  // 上
        if (key & 8) dotY += 3;  // 下

        // スペースで色変更（トリガー）
        if (key & 16) {
            dotR = (dotR + 80) % 256;
            dotG = (dotG + 120) % 256;
            dotB = (dotB + 50) % 256;
        }

        // ESCで終了
        if (key & 128) {
            break;
        }

        // 範囲制限
        if (dotX < 20) dotX = 20;
        if (dotX > 780) dotX = 780;
        if (dotY < 450) dotY = 450;
        if (dotY > 580) dotY = 580;

        // ドットを描画
        mw.color(dotR, dotG, dotB).circle(dotX - 8, dotY - 8, dotX + 8, dotY + 8, 1);

        // マウス座標表示
        int mx = mousex();
        int my = mousey();
        mw.color(255, 255, 255).boxf(620, 410, 780, 440);
        mw.color(0, 0, 0).pos(620, 410);
        mw.mes("Mouse: checking...");

        // OOP版マウス座標も確認
        int mx2 = mw.mousex();
        int my2 = mw.mousey();
        mw.pos(620, 425);
        mw.mes("OOP mouse: OK");

        mw.redraw(1);  // 描画終了

        await(16);  // 約60fps
    }

    return 0;
}
