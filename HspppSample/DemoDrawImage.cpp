// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppSample/DemoDrawImage.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 画像デモ描画
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
using namespace hsppp;

// 静的変数: bgscrウィンドウの管理
static int g_bgscrId = -1;

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
        win.mes("celload/celdiv/celput/loadCel: セル画像の操作");
        win.pos(20, 110);
        win.mes("C=load, D=divide, Arrow keys=select cell");
        win.pos(20, 135);
        win.mes(std::string("Cel ID: ") + str(g_celId) + ", Cell Index: " + str(g_celIndex));
        
        win.color(0, 128, 0).pos(50, 180);
        win.mes("celload(filename) - 画像をセル素材としてロード (HSP互換)");
        win.pos(50, 200);
        win.mes("loadCel(filename) - OOP版、Celオブジェクトを返す");
        win.pos(50, 220);
        win.mes("celdiv(id, divX, divY) - セル分割サイズ設定");
        win.pos(50, 240);
        win.mes("celput(id, index, x, y) - セル描画");
        
        if (g_celId >= 0) {
            win.color(0, 0, 200).pos(50, 280);
            win.mes("Cel loaded! Use arrows to change cell index");
            celput(g_celId, g_celIndex, 300, 300);
            win.color(0, 0, 0).pos(300, 420);
            win.mes("celput(" + str(g_celId) + ", " + str(g_celIndex) + ", 300, 300)");
        } else if (!g_testImageSaved) {
            win.color(255, 0, 0).pos(50, 280);
            win.mes("※ 先にShift+1 (bmpsave)でテスト画像を作成してください");
        }
        
        // OOP版 loadCel の説明
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 128).pos(350, 180);
        win.mes("loadCel (OOP版) 使用例:");
        win.font("MS Gothic", 11, 0);
        win.color(0, 0, 0).pos(350, 200);
        win.mes("auto cel = loadCel(\"image.bmp\");");
        win.pos(350, 218);
        win.mes("cel.divide(4, 4);");
        win.pos(350, 236);
        win.mes("cel.put(0, 100, 100);");
        break;
        
    case ImageDemo::Bgscr:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("bgscr: borderless window - Press B to show/hide popup");
        
        win.font("MS Gothic", 12, 1);
        win.color(255, 128, 0).pos(50, 120);
        win.mes("B: Create and show bgscr popup");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 145);
        win.mes("Status: " + std::string(g_bgscrVisible ? "Popup VISIBLE" : "Popup hidden"));
        
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(50, 180);
        win.mes("bgscr usage:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 205);
        win.mes("auto popup = bgscr({.width=200, .height=100});");
        win.pos(50, 223);
        win.mes("popup.color(255,0,0).boxf();");
        win.pos(50, 241);
        win.mes("popup.mes(\"Borderless!\");");
        
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 0).pos(50, 280);
        win.mes("Notes:");
        win.font("MS Gothic", 11, 0);
        win.color(0, 0, 0).pos(50, 305);
        win.mes("- No title bar, no drag");
        win.pos(50, 323);
        win.mes("- No close button");
        win.pos(50, 341);
        win.mes("- Good for splash/overlay");
        
        // 視覚的イメージ
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(350, 150);
        win.mes("Visual comparison:");
        
        // 通常ウィンドウのイメージ
        win.color(200, 200, 200);
        win.boxf(350, 180, 490, 280);
        win.color(0, 0, 128);
        win.boxf(350, 180, 490, 200);
        win.color(255, 255, 255).pos(355, 183);
        win.mes("screen()");
        
        // bgscrウィンドウのイメージ
        win.color(255, 200, 200);
        win.boxf(350, 310, 490, 410);
        win.color(0, 0, 0).pos(355, 350);
        win.mes("bgscr()");
        win.pos(355, 368);
        win.mes("No border!");
        break;
        
    default:
        break;
    }
}

// ═══════════════════════════════════════════════════════════════════
// 画像デモのアクション処理
// ═══════════════════════════════════════════════════════════════════

void processImageAction(Screen& win) {
    // 修飾キーが押されている場合はアクション無効
    if (isModifierKeyPressed()) return;
    
    switch (static_cast<ImageDemo>(g_demoIndex)) {
    case ImageDemo::Bmpsave:
        if (getkey('B')) {
            win.bmpsave("test_sprite.bmp");
            g_testImageSaved = true;
            await(200);
        }
        break;
        
    case ImageDemo::Picload:
        if (getkey('P') && g_testImageSaved) {
            win.pos(0, 0);
            win.picload("test_sprite.bmp", 1);
            g_testImageLoaded = true;
            await(200);
        }
        break;
        
    case ImageDemo::Celload:
        if (getkey('C') && g_testImageSaved) {
            g_celId = celload("test_sprite.bmp");
            await(200);
        }
        if (getkey('D') && g_celId >= 0) {
            celdiv(g_celId, 4, 4);
            await(200);
        }
        if (g_celId >= 0) {
            if (getkey(VK::RIGHT)) { g_celIndex = (g_celIndex + 1) % 16; await(150); }
            if (getkey(VK::LEFT)) { g_celIndex = (g_celIndex + 15) % 16; await(150); }
            if (getkey(VK::DOWN)) { g_celIndex = (g_celIndex + 4) % 16; await(150); }
            if (getkey(VK::UP)) { g_celIndex = (g_celIndex + 12) % 16; await(150); }
        }
        break;
        
    case ImageDemo::Bgscr:
        if (getkey('B')) {
            if (!g_bgscrVisible) {
                // bgscrを作成して表示
                auto popup = bgscr({.width = 200, .height = 100, .pos_x = 100, .pos_y = 100});
                g_bgscrId = popup.id();
                popup.color(255, 100, 100).boxf();
                popup.color(255, 255, 255).font("MS Gothic", 16, 1);
                popup.pos(30, 35);
                popup.mes("Borderless!");
                popup.font("MS Gothic", 10, 0);
                popup.pos(20, 70);
                popup.mes("Bキーで閉じる");
                gsel(g_bgscrId, 1);  // 表示してアクティブ化
                g_bgscrVisible = true;
                win.select();  // メインウィンドウを操作対象に戻す
            } else {
                // bgscrを非表示
                gsel(g_bgscrId, -1);  // 非表示
                g_bgscrVisible = false;
                win.select();  // メインウィンドウを操作対象に戻す
            }
            win.select();
            await(200);
        }
        break;
        
    default:
        break;
    }
}
