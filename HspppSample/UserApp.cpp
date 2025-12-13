// HspppSample/UserApp.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモ: cls, font, title, width, groll 機能テスト
// ═══════════════════════════════════════════════════════════════════

import hsppp;
using namespace hsppp;

// デモモード
enum class DemoMode {
    Cls,
    Font,
    Title,
    Width,
    Groll
};

DemoMode g_mode = DemoMode::Width;  // width/grollテスト用に初期モード変更
int g_clsMode = 0;
int g_fontStyle = 0;
int g_fontSize = 12;
int g_scrollX = 0;
int g_scrollY = 0;

// ユーザーのエントリーポイント
int hspMain() {
    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  メインウィンドウ作成（バッファサイズ640x480）                ║
    // ╚═══════════════════════════════════════════════════════════════╝
    auto win = screen({.width = 640, .height = 480, .title = "HSPPP Width/Groll Demo"});
    
    // メインループ
    while (true) {
        win.redraw(0);
        
        // 背景（clsのデモ表示）
        if (g_mode == DemoMode::Cls) {
            win.cls(g_clsMode);  // クリアモードを適用
        } else {
            win.cls(0);  // 通常は白でクリア
        }
        
        // タイトル
        win.font("MS Gothic", 16, 1);
        win.color(0, 0, 128).pos(20, 20);
        win.mes("=== HSPPP cls/font/title/width/groll Demo ===");
        
        // 現在のデモモード表示
        win.font("MS Gothic", 14, 0);
        win.color(0, 0, 0).pos(20, 60);
        switch (g_mode) {
        case DemoMode::Cls:
            win.mes("Current: CLS (画面クリア) - Press 1");
            win.pos(20, 85);
            win.mes("Mode: " + str(g_clsMode) + " (0=白 1=黒 2=前回の色 3=透明保持 4=黒)");
            win.pos(20, 110);
            win.mes("Press UP/DOWN to change cls mode");
            
            // clsモードのビジュアルデモ
            win.color(255, 0, 0).boxf(50, 150, 150, 250);
            win.color(0, 255, 0).boxf(200, 150, 300, 250);
            win.color(0, 0, 255).boxf(350, 150, 450, 250);
            win.color(255, 255, 0).pos(50, 270);
            win.mes("These boxes are cleared by cls(" + str(g_clsMode) + ")");
            break;
            
        case DemoMode::Font:
            win.mes("Current: FONT (フォント設定) - Press 2");
            win.pos(20, 85);
            win.mes("Style: " + str(g_fontStyle) + " Size: " + str(g_fontSize));
            win.pos(20, 110);
            win.mes("UP/DOWN: size, LEFT/RIGHT: style");
            
            // フォントデモ
            win.font("MS Gothic", g_fontSize, g_fontStyle);
            win.color(0, 0, 200).pos(50, 150);
            win.mes("MS Gothic サンプルテキスト");
            
            win.font("Arial", g_fontSize, g_fontStyle);
            win.pos(50, 180);
            win.mes("Arial Sample Text");
            
            win.font("MS Mincho", g_fontSize, g_fontStyle);
            win.pos(50, 210);
            win.mes("MS 明朝 サンプル");
            
            win.font("MS Gothic", 12, 0);
            win.color(100, 100, 100).pos(50, 250);
            win.mes("Style: 0=Normal 1=Bold 2=Italic 3=Bold+Italic");
            break;
            
        case DemoMode::Title:
            win.mes("Current: TITLE (タイトル設定) - Press 3");
            win.pos(20, 85);
            win.mes("Press T to change window title");
            win.pos(20, 110);
            win.mes("(Look at the window title bar)");
            
            win.color(0, 128, 0).pos(50, 150);
            win.mes("タイトルバーの文字列を変更できます");
            win.pos(50, 180);
            win.mes("HSP互換: title(\"テキスト\")");
            win.pos(50, 210);
            win.mes("OOP版: win.title(\"テキスト\")");
            break;
            
        case DemoMode::Width:
            win.mes("Current: WIDTH (ウィンドウサイズ) - Press 4");
            win.pos(20, 85);
            win.mes(std::string("Client: ") + str(win.width()) + "x" + str(win.height()));
            win.pos(20, 110);
            win.mes("Buffer: 640x480 (fixed, NO SCALING)");
            win.pos(20, 135);
            win.mes("Press W/S to resize (max = buffer size)");
            
            win.color(128, 0, 128).pos(50, 170);
            win.mes("ウィンドウサイズを動的に変更できます");
            win.pos(50, 195);
            win.mes("クライアントサイズは画面サイズを超えられません");
            
            // 640x480バッファの内容を視覚的に確認するためのマーカー
            win.color(255, 0, 0);
            win.boxf(0, 0, 10, 480);       // 左端
            win.boxf(630, 0, 640, 480);    // 右端
            win.boxf(0, 0, 640, 10);       // 上端
            win.boxf(0, 470, 640, 480);    // 下端
            
            win.color(0, 128, 0).pos(50, 250);
            win.mes("Red borders mark 640x480 buffer edges");
            win.pos(50, 275);
            win.mes("(No blur/scaling - dot by dot)");
            break;
            
        case DemoMode::Groll:
            win.mes("Current: GROLL (スクロール) - Press 5");
            win.pos(20, 85);
            win.mes(std::string("Scroll: ") + str(g_scrollX) + ", " + str(g_scrollY));
            win.pos(20, 110);
            win.mes("Arrow keys to scroll viewport");
            
            win.color(128, 0, 0).pos(50, 150);
            win.mes("width()でクライアントサイズ < 画面サイズにすると");
            win.pos(50, 175);
            win.mes("groll()でビューポートをスクロールできます");
            
            // グリッドパターンを描画（スクロール効果が見えるように）
            for (int x = 0; x < 640; x += 50) {
                win.color(200, 200, 200);
                win.line(x, 0, x, 480);
            }
            for (int y = 0; y < 480; y += 50) {
                win.color(200, 200, 200);
                win.line(0, y, 640, y);
            }
            
            // 四隅にマーカー
            win.color(255, 0, 0).boxf(0, 0, 50, 50);        // 左上
            win.color(0, 255, 0).boxf(590, 0, 640, 50);     // 右上
            win.color(0, 0, 255).boxf(0, 430, 50, 480);     // 左下
            win.color(255, 255, 0).boxf(590, 430, 640, 480);// 右下
            
            win.font("MS Gothic", 12, 0);
            win.color(0, 0, 0).pos(5, 55);
            win.mes("TL(0,0)");
            win.pos(570, 55);
            win.mes("TR(640,0)");
            win.pos(5, 400);
            win.mes("BL(0,480)");
            win.pos(560, 400);
            win.mes("BR(640,480)");
            break;
        }
        
        // 共通ヘルプ
        win.font("MS Gothic", 11, 0);
        win.color(128, 128, 128).pos(20, 420);
        win.mes("Keys: 1=cls 2=font 3=title 4=width 5=groll | ESC=Exit");
        
        win.redraw(1);
        
        // キー入力処理
        if (getkey('1')) { g_mode = DemoMode::Cls; await(200); }
        if (getkey('2')) { g_mode = DemoMode::Font; await(200); }
        if (getkey('3')) { g_mode = DemoMode::Title; await(200); }
        if (getkey('4')) { g_mode = DemoMode::Width; await(200); }
        if (getkey('5')) { g_mode = DemoMode::Groll; await(200); }
        
        // モード別操作
        switch (g_mode) {
        case DemoMode::Cls:
            if (getkey(0x26)) { g_clsMode = (g_clsMode + 1) % 5; await(200); }  // VK_UP
            if (getkey(0x28)) { g_clsMode = (g_clsMode + 4) % 5; await(200); }  // VK_DOWN
            break;
            
        case DemoMode::Font:
            if (getkey(0x26)) { g_fontSize++; if (g_fontSize > 32) g_fontSize = 32; await(100); }  // VK_UP
            if (getkey(0x28)) { g_fontSize--; if (g_fontSize < 8) g_fontSize = 8; await(100); }    // VK_DOWN
            if (getkey(0x27)) { g_fontStyle = (g_fontStyle + 1) % 4; await(200); }  // VK_RIGHT
            if (getkey(0x25)) { g_fontStyle = (g_fontStyle + 3) % 4; await(200); }  // VK_LEFT
            break;
            
        case DemoMode::Title:
            if (getkey('T')) {
                static int titleNum = 1;
                win.title("New Title " + str(titleNum++));
                await(200);
            }
            break;
            
        case DemoMode::Width:
            if (getkey('W')) {
                // 現在のサイズを取得して50ピクセル増やす（クランプされる）
                int newW = win.width() + 50;
                int newH = win.height() + 50;
                win.width(newW, newH);
                await(200);
            }
            if (getkey('S')) {
                // 現在のサイズを取得して50ピクセル減らす
                int newW = win.width() - 50;
                int newH = win.height() - 50;
                if (newW < 200) newW = 200;
                if (newH < 150) newH = 150;
                win.width(newW, newH);
                await(200);
            }
            break;
            
        case DemoMode::Groll:
            // まずウィンドウを小さくしてスクロール効果が見えるようにする
            if (win.width() == 640) {
                win.width(400, 300);
            }
            if (getkey(0x25)) { g_scrollX -= 10; if (g_scrollX < 0) g_scrollX = 0; win.groll(g_scrollX, g_scrollY); await(50); }  // VK_LEFT
            if (getkey(0x27)) { g_scrollX += 10; if (g_scrollX > 240) g_scrollX = 240; win.groll(g_scrollX, g_scrollY); await(50); }  // VK_RIGHT
            if (getkey(0x26)) { g_scrollY -= 10; if (g_scrollY < 0) g_scrollY = 0; win.groll(g_scrollX, g_scrollY); await(50); }  // VK_UP
            if (getkey(0x28)) { g_scrollY += 10; if (g_scrollY > 180) g_scrollY = 180; win.groll(g_scrollX, g_scrollY); await(50); }  // VK_DOWN
            break;
        }
        
        // ESCで終了
        if (stick() & 128) break;
        
        await(16);
    }
    
    return 0;
}
