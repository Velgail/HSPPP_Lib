// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppSample/DemoDrawBasic.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 基本デモ描画
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
using namespace hsppp;

void drawBasicDemo(Screen& win) {
    switch (static_cast<BasicDemo>(g_demoIndex)) {
    case BasicDemo::Line:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("line命令: 直線を描画");
        win.pos(20, 105);
        win.mes("始点から終点へ、現在の色で直線を引きます");
        
        // 格子状の線
        for (int i = 0; i <= 10; i++) {
            win.color(200, 200, 200);
            win.line(50 + i * 40, 400, 50 + i * 40, 150);  // 縦線
            win.line(450, 150 + i * 25, 50, 150 + i * 25);  // 横線
        }
        
        // カラフルな放射状の線
        for (int i = 0; i < 36; i++) {
            double angle = hsppp::deg2rad(i * 10.0);
            int r = static_cast<int>((hsppp::sin(angle) + 1.0) * 127);
            int g = static_cast<int>((hsppp::cos(angle) + 1.0) * 127);
            int b = static_cast<int>((hsppp::sin(angle + 2.0) + 1.0) * 127);
            win.color(r, g, b);
            int endX = 540 + static_cast<int>(hsppp::cos(angle) * 80);
            int endY = 280 + static_cast<int>(hsppp::sin(angle) * 80);
            win.line(endX, endY, 540, 280);
        }
        break;
        
    case BasicDemo::Circle:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("circle命令: 円を描画 (外接矩形を指定)");
        win.pos(20, 105);
        win.mes("fillMode: 0=線, 1=塗りつぶし");
        
        // 塗りつぶし円
        win.color(255, 0, 0);
        win.circle(50, 150, 150, 250, 1);  // 赤の塗りつぶし
        win.color(0, 255, 0);
        win.circle(170, 150, 270, 250, 1);  // 緑の塗りつぶし
        win.color(0, 0, 255);
        win.circle(290, 150, 390, 250, 1);  // 青の塗りつぶし
        
        // 線のみの円
        win.color(0, 0, 0);
        win.circle(50, 280, 150, 380, 0);
        win.circle(170, 280, 270, 380, 0);
        win.circle(290, 280, 390, 380, 0);
        
        // 楕円
        win.color(255, 128, 0);
        win.circle(420, 150, 590, 250, 1);  // 横長楕円
        win.color(128, 0, 255);
        win.circle(450, 280, 510, 420, 1);  // 縦長楕円
        break;
        
    case BasicDemo::Pset:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("pset/pget命令: 点の描画と色取得");
        win.pos(20, 105);
        win.mes("pset(x,y)で点を描画、pget(x,y)で色を取得");
        
        // ランダムな点を描画
        for (int i = 0; i < 500; i++) {
            int x = 50 + rnd(400);
            int y = 150 + rnd(200);
            win.color(rnd(256), rnd(256), rnd(256));
            win.pset(x, y);
        }
        
        // グラデーションの点描
        for (int y = 0; y < 50; y++) {
            for (int x = 0; x < 100; x++) {
                win.color(x * 2 + 50, y * 4 + 50, 150);
                win.pset(480 + x, 150 + y);
            }
        }
        
        // pgetのデモ
        win.color(0, 0, 0).pos(50, 370);
        win.mes("← 上の領域をクリックするとpgetで色を取得");
        break;
        
    case BasicDemo::Boxf:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("boxf命令: 矩形を塗りつぶし");
        win.pos(20, 105);
        win.mes("boxf()で全画面、boxf(x1,y1,x2,y2)で指定範囲");
        
        // カラフルな矩形
        for (int i = 0; i < 10; i++) {
            win.color(i * 25, 255 - i * 25, 128);
            win.boxf(50 + i * 50, 150 + i * 20, 100 + i * 50, 200 + i * 20);
        }
        
        // 半透明風の重なり（色を混ぜて表現）
        win.color(255, 0, 0);
        win.boxf(400, 200, 500, 300);
        win.color(0, 255, 0);
        win.boxf(430, 230, 530, 330);
        win.color(0, 0, 255);
        win.boxf(460, 260, 560, 360);
        break;
        
    case BasicDemo::Cls:
        win.mes("Current: CLS (画面クリア)");
        win.pos(20, 85);
        win.mes("Mode: " + str(g_clsMode) + " (0=白 1=明灰 2=灰 3=暗灰 4=黒)");
        win.pos(20, 110);
        win.mes("Press UP/DOWN to change cls mode");
        
        win.color(255, 0, 0).boxf(50, 150, 150, 250);
        win.color(0, 255, 0).boxf(200, 150, 300, 250);
        win.color(0, 0, 255).boxf(350, 150, 450, 250);
        win.color(0, 0, 0).pos(50, 270);
        win.mes("These boxes are cleared by cls(" + str(g_clsMode) + ")");
        break;
        
    case BasicDemo::Font:
        win.mes("Current: FONT (フォント設定)");
        win.pos(20, 85);
        win.mes("Style: " + str(g_fontStyle) + " Size: " + str(g_fontSize));
        win.pos(20, 110);
        win.mes("UP/DOWN: size, LEFT/RIGHT: style");
        
        win.font("MS Gothic", g_fontSize, g_fontStyle);
        win.color(0, 0, 200).pos(50, 150);
        win.mes("MS Gothic サンプル");
        
        win.font("Arial", g_fontSize, g_fontStyle);
        win.pos(50, 180);
        // print は mes の別名として使用
        print("Arial Sample (print 関数)");
        
        win.font("MS Gothic", 12, 0);
        win.color(100, 100, 100).pos(50, 220);
        win.mes("Style: 0=Normal 1=Bold 2=Italic 3=Bold+Italic");
        win.pos(50, 240);
        win.mes("※ print() は mes() の別名");
        
        // sysfontのデモ
        win.color(0, 128, 0).pos(50, 280);
        win.mes("sysfont デモ:");
        
        win.sysfont(0);  // HSP標準フォント
        win.color(0, 0, 0).pos(50, 300);
        win.mes("sysfont(0): HSP標準");
        
        win.sysfont(17); // デフォルトGUIフォント
        win.pos(50, 320);
        win.mes("sysfont(17): デフォルトGUI");
        
        win.font("MS Gothic", 12, 0);  // 元に戻す
        break;
        
    case BasicDemo::Title:
        win.mes("Current: TITLE (タイトル設定)");
        win.pos(20, 85);
        win.mes("Press T to change window title");
        win.color(0, 128, 0).pos(50, 150);
        win.mes("タイトルバーを変更: title() / win.title()");
        break;
        
    case BasicDemo::Width:
        win.mes("Current: WIDTH (ウィンドウサイズ)");
        win.pos(20, 85);
        win.mes(std::string("Client: ") + str(win.width()) + "x" + str(win.height()));
        win.pos(20, 110);
        win.mes("Buffer: 640x480 (fixed, NO SCALING)");
        win.pos(20, 135);
        win.mes("Press W/S to resize");
        
        win.color(255, 0, 0);
        win.boxf(0, 0, 10, 480);
        win.boxf(630, 0, 640, 480);
        win.boxf(0, 0, 640, 10);
        win.boxf(0, 470, 640, 480);
        
        win.color(0, 0, 0).pos(50, 200);
        win.mes("Red borders = 640x480 buffer edges");
        break;
        
    case BasicDemo::Groll:
        win.mes("Current: GROLL (スクロール)");
        win.pos(20, 85);
        win.mes(std::string("Scroll: ") + str(g_scrollX) + ", " + str(g_scrollY));
        win.pos(20, 110);
        win.mes("Arrow keys to scroll viewport");
        
        for (int x = 0; x < 640; x += 50) {
            win.color(200, 200, 200);
            win.line(x, 480, x, 0);
        }
        for (int y = 0; y < 480; y += 50) {
            win.color(200, 200, 200);
            win.line(640, y, 0, y);
        }
        
        win.color(255, 0, 0).boxf(0, 0, 50, 50);
        win.color(0, 255, 0).boxf(590, 0, 640, 50);
        win.color(0, 0, 255).boxf(0, 430, 50, 480);
        win.color(255, 255, 0).boxf(590, 430, 640, 480);
        break;
        
    default:
        break;
    }
}

// ═══════════════════════════════════════════════════════════════════
// 基本デモのアクション処理
// ═══════════════════════════════════════════════════════════════════

void processBasicAction(Screen& win) {
    // 修飾キーが押されている場合はアクション無効
    if (isModifierKeyPressed()) return;
    
    switch (static_cast<BasicDemo>(g_demoIndex)) {
    case BasicDemo::Cls:
        if (getkey(VK::UP)) { g_clsMode = (g_clsMode + 1) % 5; await(200); }
        if (getkey(VK::DOWN)) { g_clsMode = (g_clsMode + 4) % 5; await(200); }
        break;
        
    case BasicDemo::Font:
        if (getkey(VK::UP)) { g_fontSize++; if (g_fontSize > 32) g_fontSize = 32; await(100); }
        if (getkey(VK::DOWN)) { g_fontSize--; if (g_fontSize < 8) g_fontSize = 8; await(100); }
        if (getkey(VK::RIGHT)) { g_fontStyle = (g_fontStyle + 1) % 4; await(200); }
        if (getkey(VK::LEFT)) { g_fontStyle = (g_fontStyle + 3) % 4; await(200); }
        break;
        
    case BasicDemo::Title:
        if (getkey('T')) {
            static int titleNum = 1;
            win.title("New Title " + str(titleNum++));
            await(200);
        }
        break;
        
    case BasicDemo::Width:
        if (getkey('W')) {
            int newW = win.width() + 50;
            int newH = win.height() + 50;
            win.width(newW, newH);
            await(200);
        }
        if (getkey('S')) {
            int newW = win.width() - 50;
            int newH = win.height() - 50;
            if (newW < 200) newW = 200;
            if (newH < 150) newH = 150;
            win.width(newW, newH);
            await(200);
        }
        break;
        
    case BasicDemo::Groll:
        // Grollデモ開始時にウィンドウサイズを縮小
        if (win.width() == 640) {
            win.width(400, 300);
        }
        if (getkey(VK::LEFT)) { g_scrollX -= 10; if (g_scrollX < 0) g_scrollX = 0; win.groll(g_scrollX, g_scrollY); await(50); }
        if (getkey(VK::RIGHT)) { g_scrollX += 10; if (g_scrollX > 240) g_scrollX = 240; win.groll(g_scrollX, g_scrollY); await(50); }
        if (getkey(VK::UP)) { g_scrollY -= 10; if (g_scrollY < 0) g_scrollY = 0; win.groll(g_scrollX, g_scrollY); await(50); }
        if (getkey(VK::DOWN)) { g_scrollY += 10; if (g_scrollY > 180) g_scrollY = 180; win.groll(g_scrollX, g_scrollY); await(50); }
        break;
        
    default:
        break;
    }
}
