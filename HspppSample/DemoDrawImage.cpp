// HspppSample/DemoDrawImage.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 画像デモ描画
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
using namespace hsppp;

void drawImageDemo(Screen& win) {
    switch (static_cast<ImageDemo>(g_demoIndex)) {
    case ImageDemo::Bmpsave:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("bmpsave: 画面イメージをBMPファイルに保存");
        win.pos(20, 110);
        win.mes("Press B to save this screen to test_sprite.bmp");
        win.pos(20, 135);
        win.mes(std::string("Status: ") + (g_testImageSaved ? "Saved!" : "Not saved yet"));
        
        // デモ用のカラフルな描画
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                int idx = y * 4 + x;
                int r = ((idx & 1) ? 255 : 0) + ((idx & 4) ? 128 : 0);
                int g = ((idx & 2) ? 255 : 0) + ((idx & 8) ? 128 : 0);
                int b = ((idx & 4) ? 255 : 0) + ((idx & 1) ? 128 : 0);
                win.color(r % 256, g % 256, b % 256);
                win.boxf(350 + x * 64, 200 + y * 64, 350 + (x + 1) * 64 - 2, 200 + (y + 1) * 64 - 2);
            }
        }
        break;
        
    case ImageDemo::Picload:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("picload: 画像ファイルをロード");
        win.pos(20, 110);
        win.mes("Press P to load test_sprite.bmp");
        win.pos(20, 135);
        win.mes(std::string("Status: ") + (g_testImageLoaded ? "Loaded!" : "Not loaded"));
        
        win.color(0, 128, 0).pos(50, 180);
        win.mes("picload(filename, mode)");
        win.pos(50, 200);
        win.mes("mode: 0=初期化してロード, 1=重ねる, 2=黒で初期化");
        
        if (!g_testImageSaved) {
            win.color(255, 0, 0).pos(50, 250);
            win.mes("※ 先にShift+1 (bmpsave)でテスト画像を作成してください");
        }
        break;
        
    case ImageDemo::Celload:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("celload/celdiv/celput: セル画像の操作");
        win.pos(20, 110);
        win.mes("C=load, D=divide, Arrow keys=select cell");
        win.pos(20, 135);
        win.mes(std::string("Cel ID: ") + str(g_celId) + ", Cell Index: " + str(g_celIndex));
        
        win.color(0, 128, 0).pos(50, 180);
        win.mes("celload(filename) - 画像をセル素材としてロード");
        win.pos(50, 200);
        win.mes("celdiv(id, divX, divY) - セル分割サイズ設定");
        win.pos(50, 220);
        win.mes("celput(id, index, x, y) - セル描画");
        
        if (g_celId >= 0) {
            win.color(0, 0, 200).pos(50, 260);
            win.mes("Cel loaded! Use arrows to change cell index");
            celput(g_celId, g_celIndex, 300, 280);
            win.color(0, 0, 0).pos(300, 420);
            win.mes("celput(" + str(g_celId) + ", " + str(g_celIndex) + ", 300, 280)");
        } else if (!g_testImageSaved) {
            win.color(255, 0, 0).pos(50, 260);
            win.mes("※ 先にShift+1 (bmpsave)でテスト画像を作成してください");
        }
        break;
        
    default:
        break;
    }
}
