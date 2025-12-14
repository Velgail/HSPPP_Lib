// HspppSample/DemoDrawExtended.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 拡張デモ描画
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
import <format>;
using namespace hsppp;

void drawExtendedDemo(Screen& win) {
    switch (static_cast<ExtendedDemo>(g_demoIndex)) {
    case ExtendedDemo::Math:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("数学関数デモ: sin, cos, rnd, limit, sqrt, pow");
        
        // Sin/Cos波形描画
        win.color(0, 128, 0).pos(50, 120);
        win.mes("sin/cos 波形 (角度を自動更新中)");
        
        // 波形の背景
        win.color(240, 240, 240);
        win.boxf(50, 150, 590, 250);
        
        // X軸
        win.color(128, 128, 128);
        win.line(590, 200, 50, 200);
        
        // Sin波形（赤）
        win.color(255, 0, 0);
        for (int x = 0; x < 540; x++) {
            double angle = hsppp::deg2rad(g_angle + x * 2);
            int y = 200 - static_cast<int>(hsppp::sin(angle) * 40);
            if (x == 0) {
                win.pos(50 + x, y);
            } else {
                win.line(50 + x, y);
            }
        }
        
        // Cos波形（青）
        win.color(0, 0, 255);
        for (int x = 0; x < 540; x++) {
            double angle = hsppp::deg2rad(g_angle + x * 2);
            int y = 200 - static_cast<int>(hsppp::cos(angle) * 40);
            if (x == 0) {
                win.pos(50 + x, y);
            } else {
                win.line(50 + x, y);
            }
        }
        
        // 乱数表示
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 270);
        win.mes("rnd(100) の結果:");
        for (int i = 0; i < 10; i++) {
            win.pos(50 + i * 50, 290);
            win.mes(str(rnd(100)));
        }
        
        // limit表示
        win.pos(50, 320);
        win.mes("limit デモ:");
        win.pos(50, 340);
        win.mes("limit(-50, 0, 100) = " + str(hsppp::limit(-50, 0, 100)));
        win.pos(50, 355);
        win.mes("limit(150, 0, 100) = " + str(hsppp::limit(150, 0, 100)));
        
        // sqrt/pow表示
        win.pos(300, 320);
        win.mes("sqrt/pow デモ:");
        win.pos(300, 340);
        win.mes("sqrt(2) = " + str(hsppp::sqrt(2.0)));
        win.pos(300, 355);
        win.mes("pow(2, 10) = " + str(hsppp::pow(2.0, 10.0)));
        
        // 角度を更新（アニメーション）
        g_angle += 2.0;
        if (g_angle >= 360.0) g_angle -= 360.0;
        break;
        
    case ExtendedDemo::Color:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("色関連関数デモ: hsvcolor, rgbcolor, syscolor");
        
        // HSVカラーグラデーション
        win.color(0, 0, 0).pos(50, 120);
        win.mes("hsvcolor グラデーション (H: 0-191):");
        for (int h = 0; h < 192; h++) {
            hsvcolor(h, 255, 255);
            win.boxf(50 + h * 2, 140, 50 + h * 2 + 2, 180);
        }
        
        // 彩度グラデーション
        win.color(0, 0, 0).pos(50, 190);
        win.mes("hsvcolor 彩度グラデーション (S: 0-255):");
        for (int s = 0; s < 256; s++) {
            hsvcolor(0, s, 255);
            win.boxf(50 + s * 2, 210, 50 + s * 2 + 2, 250);
        }
        
        // RGBカラー
        win.color(0, 0, 0).pos(50, 270);
        win.mes("rgbcolor サンプル:");
        rgbcolor(0xFF0000); win.boxf(50, 290, 100, 340);
        rgbcolor(0x00FF00); win.boxf(110, 290, 160, 340);
        rgbcolor(0x0000FF); win.boxf(170, 290, 220, 340);
        rgbcolor(0xFFFF00); win.boxf(230, 290, 280, 340);
        rgbcolor(0xFF00FF); win.boxf(290, 290, 340, 340);
        rgbcolor(0x00FFFF); win.boxf(350, 290, 400, 340);
        
        // システムカラー
        win.color(0, 0, 0).pos(50, 360);
        win.mes("syscolor サンプル (システムカラー):");
        for (int i = 0; i < 8; i++) {
            syscolor(i);
            win.boxf(50 + i * 60, 380, 100 + i * 60, 420);
            win.color(0, 0, 0).pos(50 + i * 60, 425);
            win.mes(str(i));
        }
        break;
        
    case ExtendedDemo::Gradf:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("gradf: 矩形をグラデーションで塗りつぶす");
        
        // 横方向グラデーション
        win.color(0, 0, 0).pos(50, 120);
        win.mes("横方向グラデーション (mode=0):");
        gradf(50, 140, 200, 60, 0, 0xFF0000, 0x0000FF);
        gradf(50, 210, 200, 60, 0, 0x00FF00, 0xFFFF00);
        
        // 縦方向グラデーション
        win.color(0, 0, 0).pos(300, 120);
        win.mes("縦方向グラデーション (mode=1):");
        gradf(300, 140, 200, 60, 1, 0xFF00FF, 0x00FFFF);
        gradf(300, 210, 200, 60, 1, 0xFFFFFF, 0x000000);
        
        // OOP版グラデーション
        win.color(0, 0, 0).pos(50, 290);
        win.mes("Screen OOP版 gradf:");
        win.gradf(50, 310, 450, 80, 0, 0xFF8800, 0x0088FF);
        
        // gettime デモ
        win.color(0, 0, 0).pos(50, 410);
        win.mes("gettime 関数:");
        win.pos(50, 430);
        win.mes(std::format("現在時刻: {:04d}/{:02d}/{:02d} {:02d}:{:02d}:{:02d}",
            gettime(0), gettime(1), gettime(3),
            gettime(4), gettime(5), gettime(6)));
        break;
        
    case ExtendedDemo::Grect:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("grect: 回転する矩形で塗りつぶす (← / → で回転)");
        
        // 回転矩形デモ
        win.color(255, 0, 0);
        grect(200, 250, g_angle * 0.0174533, 100, 60);
        
        win.color(0, 255, 0);
        grect(350, 250, g_angle * 0.0174533 + 1.0, 80, 80);
        
        win.color(0, 0, 255);
        grect(500, 250, -g_angle * 0.0174533, 120, 40);
        
        // OOP版
        win.color(255, 128, 0);
        win.grect(320, 380, g_angle * 0.0174533 * 2, 60, 60);
        
        win.color(0, 0, 0).pos(50, 420);
        win.mes("角度: " + str(static_cast<int>(g_angle)) + "度");
        
        // 自動回転
        g_angle += 1.0;
        if (g_angle >= 360.0) g_angle -= 360.0;
        break;
        
    case ExtendedDemo::Gsquare:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("gsquare: 任意の四角形を描画");
        
        // 単色塗りつぶし
        win.color(255, 0, 0);
        {
            Quad solidQuad = {{50, 150}, {200, 150}, {220, 280}, {30, 280}};
            gsquare(-1, solidQuad);
        }
        win.color(0, 0, 0).pos(50, 290);
        win.mes("単色 (srcId=-1)");
        
        // グラデーション
        {
            Quad gradQuad = {{250, 150}, {400, 150}, {420, 280}, {230, 280}};
            QuadColors gradColors = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
            gsquare(gsquare_grad, gradQuad, gradColors);
        }
        win.color(0, 0, 0).pos(250, 290);
        win.mes("グラデーション (srcId=-257)");
        
        // 台形
        win.color(0, 128, 255);
        {
            Quad trapezoid = {{500, 200}, {580, 200}, {600, 280}, {480, 280}};
            gsquare(-1, trapezoid);
        }
        win.color(0, 0, 0).pos(480, 290);
        win.mes("台形");
        break;
        
    case ExtendedDemo::Gcopy:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("gcopy: 画面コピー (Press C to create buffer and copy)");
        
        if (!g_bufferCreated) {
            win.color(128, 0, 0).pos(50, 150);
            win.mes("Cキーを押してバッファを作成し、gcopyを実行します");
        } else {
            win.pos(50, 150);
            win.mes("バッファからコピーしました:");
        }
        
        // デモ用の図形を描画
        win.color(255, 128, 0);
        win.boxf(400, 150, 550, 250);
        win.color(0, 128, 255);
        win.circle(425, 175, 525, 225, 1);
        win.color(0, 0, 0).pos(400, 260);
        win.mes("コピー元の領域");
        break;
        
    case ExtendedDemo::Gzoom:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("gzoom: 変倍して画面コピー (Press Z to demonstrate)");
        
        // デモ用の図形を描画
        win.color(255, 0, 0);
        win.boxf(50, 150, 150, 250);
        win.color(0, 0, 0).pos(50, 260);
        win.mes("元画像 100x100");
        
        // 拡大・縮小の説明
        win.color(0, 128, 0).pos(200, 150);
        win.mes("gzoom(dest_w, dest_h, src_id, src_x, src_y, src_w, src_h, mode)");
        win.pos(200, 170);
        win.mes("mode: 0=高速, 1=高品質ハーフトーン");
        break;
        
    case ExtendedDemo::Grotate:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("grotate: 矩形画像を回転してコピー");
        
        // 回転コピーの説明
        win.color(0, 128, 0).pos(50, 150);
        win.mes("grotate(srcId, srcX, srcY, angle, dstW, dstH)");
        win.pos(50, 170);
        win.mes("指定した画像を回転してカレントポジションにコピー");
        win.pos(50, 200);
        win.mes("角度はラジアン単位で指定");
        
        // 回転デモの視覚化
        win.color(0, 0, 0).pos(300, 250);
        win.mes("回転角度: " + str(static_cast<int>(g_angle)) + "度");
        
        g_angle += 1.0;
        if (g_angle >= 360.0) g_angle -= 360.0;
        break;
        
    default:
        break;
    }
}
