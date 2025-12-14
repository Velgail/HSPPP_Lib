// HspppSample/UserApp.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモ: P1 + P2 機能テスト
// P1: cls, font, title, width, groll
// P2: picload, celload, celput, celdiv, bmpsave
// ═══════════════════════════════════════════════════════════════════

import hsppp;
using namespace hsppp;

// デモモード
enum class DemoMode {
    // P1
    Cls,
    Font,
    Title,
    Width,
    Groll,
    // P2
    Bmpsave,
    Picload,
    Celload
};

DemoMode g_mode = DemoMode::Bmpsave;  // P2テスト用に初期モード変更
int g_clsMode = 0;
int g_fontStyle = 0;
int g_fontSize = 12;
int g_scrollX = 0;
int g_scrollY = 0;

// P2用変数
bool g_testImageSaved = false;
bool g_testImageLoaded = false;
int g_celId = -1;
int g_celIndex = 0;

// ユーザーのエントリーポイント
int hspMain() {
    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  メインウィンドウ作成（バッファサイズ640x480）                ║
    // ╚═══════════════════════════════════════════════════════════════╝
    auto win = screen({.width = 640, .height = 480, .title = "HSPPP P1+P2 Demo"});
    
    // メインループ
    while (true) {
        win.redraw(0);
        
        // 背景（clsのデモ表示）
        if (g_mode == DemoMode::Cls) {
            win.cls(g_clsMode);
        } else {
            win.cls(0);
        }
        
        // タイトル
        win.font("MS Gothic", 16, 1);
        win.color(0, 0, 128).pos(20, 20);
        win.mes("=== HSPPP P1+P2 Feature Demo ===");
        
        // 現在のデモモード表示
        win.font("MS Gothic", 14, 0);
        win.color(0, 0, 0).pos(20, 60);
        
        switch (g_mode) {
        // ═══════════════════════════════════════════════════════════
        // P1機能
        // ═══════════════════════════════════════════════════════════
        case DemoMode::Cls:
            win.mes("Current: CLS (画面クリア) - Press 1");
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
            
        case DemoMode::Font:
            win.mes("Current: FONT (フォント設定) - Press 2");
            win.pos(20, 85);
            win.mes("Style: " + str(g_fontStyle) + " Size: " + str(g_fontSize));
            win.pos(20, 110);
            win.mes("UP/DOWN: size, LEFT/RIGHT: style");
            
            win.font("MS Gothic", g_fontSize, g_fontStyle);
            win.color(0, 0, 200).pos(50, 150);
            win.mes("MS Gothic サンプル");
            
            win.font("Arial", g_fontSize, g_fontStyle);
            win.pos(50, 180);
            win.mes("Arial Sample");
            
            win.font("MS Gothic", 12, 0);
            win.color(100, 100, 100).pos(50, 220);
            win.mes("Style: 0=Normal 1=Bold 2=Italic 3=Bold+Italic");
            break;
            
        case DemoMode::Title:
            win.mes("Current: TITLE (タイトル設定) - Press 3");
            win.pos(20, 85);
            win.mes("Press T to change window title");
            win.color(0, 128, 0).pos(50, 150);
            win.mes("タイトルバーを変更: title() / win.title()");
            break;
            
        case DemoMode::Width:
            win.mes("Current: WIDTH (ウィンドウサイズ) - Press 4");
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
            
        case DemoMode::Groll:
            win.mes("Current: GROLL (スクロール) - Press 5");
            win.pos(20, 85);
            win.mes(std::string("Scroll: ") + str(g_scrollX) + ", " + str(g_scrollY));
            win.pos(20, 110);
            win.mes("Arrow keys to scroll viewport");
            
            for (int x = 0; x < 640; x += 50) {
                win.color(200, 200, 200);
                win.line(x, 0, x, 480);
            }
            for (int y = 0; y < 480; y += 50) {
                win.color(200, 200, 200);
                win.line(0, y, 640, y);
            }
            
            win.color(255, 0, 0).boxf(0, 0, 50, 50);
            win.color(0, 255, 0).boxf(590, 0, 640, 50);
            win.color(0, 0, 255).boxf(0, 430, 50, 480);
            win.color(255, 255, 0).boxf(590, 430, 640, 480);
            break;
            
        // ═══════════════════════════════════════════════════════════
        // P2機能
        // ═══════════════════════════════════════════════════════════
        case DemoMode::Bmpsave:
            win.mes("Current: BMPSAVE (画面イメージセーブ) - Press 6");
            win.pos(20, 85);
            win.mes("Press B to save this screen with bmpsave()");
            win.pos(20, 110);
            win.mes(std::string("Status: ") + (g_testImageSaved ? "test_sprite.bmp saved!" : "Not saved yet"));
            
            win.color(0, 128, 0).pos(50, 150);
            win.mes("bmpsave() は描画対象バッファ全体をビットパーフェクトに保存");
            win.pos(50, 175);
            win.mes("width()で表示サイズを制限しても、バッファ全体(640x480)を保存");
            
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
            
            if (g_testImageSaved) {
                win.color(0, 0, 200).pos(50, 220);
                win.mes("test_sprite.bmp が作成されました (640x480)");
                win.pos(50, 245);
                win.mes("→ 7キーでpicload, 8キーでcelloadをテストできます");
            }
            break;
            
        case DemoMode::Picload:
            win.mes("Current: PICLOAD (画像ファイルロード) - Press 7");
            win.pos(20, 85);
            win.mes("Press P to load test_sprite.bmp");
            win.pos(20, 110);
            win.mes(std::string("Status: ") + (g_testImageLoaded ? "Loaded!" : "Not loaded"));
            
            win.color(0, 128, 0).pos(50, 150);
            win.mes("picload(filename, mode) - 画像をロード");
            win.pos(50, 175);
            win.mes("mode: 0=初期化してロード, 1=重ねる, 2=黒で初期化");
            
            if (!g_testImageSaved) {
                win.color(255, 0, 0).pos(50, 220);
                win.mes("※ 先に6キー(bmpsave)でテスト画像を作成してください");
            }
            break;
            
        case DemoMode::Celload:
            win.mes("Current: CELLOAD/CELDIV/CELPUT - Press 8");
            win.pos(20, 85);
            win.mes("Press C to load, D to divide, Arrow keys to select cell");
            win.pos(20, 110);
            win.mes(std::string("Cel ID: ") + str(g_celId) + ", Cell Index: " + str(g_celIndex));
            
            win.color(0, 128, 0).pos(50, 150);
            win.mes("celload(filename) - 画像をセル素材としてロード");
            win.pos(50, 175);
            win.mes("celdiv(id, divX, divY) - セル分割サイズ設定");
            win.pos(50, 200);
            win.mes("celput(id, index, x, y) - セル描画");
            
            if (g_celId >= 0) {
                win.color(0, 0, 200).pos(50, 240);
                win.mes("Cel loaded! Use arrows to change cell index");
                
                celput(g_celId, g_celIndex, 300, 280);
                
                win.color(0, 0, 0).pos(300, 420);
                win.mes("celput(" + str(g_celId) + ", " + str(g_celIndex) + ", 300, 280)");
            } else if (!g_testImageSaved) {
                win.color(255, 0, 0).pos(50, 240);
                win.mes("※ 先に6キー(bmpsave)でテスト画像を作成してください");
            }
            break;
        }
        
        // 共通ヘルプ
        win.font("MS Gothic", 11, 0);
        win.color(128, 128, 128).pos(20, 440);
        win.mes("P1: 1=cls 2=font 3=title 4=width 5=groll");
        win.pos(20, 455);
        win.mes("P2: 6=bmpsave 7=picload 8=celload | ESC=Exit");
        
        win.redraw(1);
        
        // キー入力処理 - モード切替
        if (getkey('1')) { g_mode = DemoMode::Cls; await(200); }
        if (getkey('2')) { g_mode = DemoMode::Font; await(200); }
        if (getkey('3')) { g_mode = DemoMode::Title; await(200); }
        if (getkey('4')) { g_mode = DemoMode::Width; await(200); }
        if (getkey('5')) { g_mode = DemoMode::Groll; await(200); }
        if (getkey('6')) { g_mode = DemoMode::Bmpsave; await(200); }
        if (getkey('7')) { g_mode = DemoMode::Picload; await(200); }
        if (getkey('8')) { g_mode = DemoMode::Celload; await(200); }
        
        // モード別操作
        switch (g_mode) {
        case DemoMode::Cls:
            if (getkey(0x26)) { g_clsMode = (g_clsMode + 1) % 5; await(200); }
            if (getkey(0x28)) { g_clsMode = (g_clsMode + 4) % 5; await(200); }
            break;
            
        case DemoMode::Font:
            if (getkey(0x26)) { g_fontSize++; if (g_fontSize > 32) g_fontSize = 32; await(100); }
            if (getkey(0x28)) { g_fontSize--; if (g_fontSize < 8) g_fontSize = 8; await(100); }
            if (getkey(0x27)) { g_fontStyle = (g_fontStyle + 1) % 4; await(200); }
            if (getkey(0x25)) { g_fontStyle = (g_fontStyle + 3) % 4; await(200); }
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
            
        case DemoMode::Groll:
            if (win.width() == 640) {
                win.width(400, 300);
            }
            if (getkey(0x25)) { g_scrollX -= 10; if (g_scrollX < 0) g_scrollX = 0; win.groll(g_scrollX, g_scrollY); await(50); }
            if (getkey(0x27)) { g_scrollX += 10; if (g_scrollX > 240) g_scrollX = 240; win.groll(g_scrollX, g_scrollY); await(50); }
            if (getkey(0x26)) { g_scrollY -= 10; if (g_scrollY < 0) g_scrollY = 0; win.groll(g_scrollX, g_scrollY); await(50); }
            if (getkey(0x28)) { g_scrollY += 10; if (g_scrollY > 180) g_scrollY = 180; win.groll(g_scrollX, g_scrollY); await(50); }
            break;
            
        // P2機能
        case DemoMode::Bmpsave:
            if (getkey('B')) {
                // 現在のメインウィンドウの画面をそのままBMPに保存
                // bmpsave()はバッファ全体(640x480)をビットパーフェクトに保存
                win.bmpsave("test_sprite.bmp");
                g_testImageSaved = true;
                await(200);
            }
            break;
            
        case DemoMode::Picload:
            if (getkey('P') && g_testImageSaved) {
                win.pos(0, 0);
                win.picload("test_sprite.bmp", 1);
                g_testImageLoaded = true;
                await(200);
            }
            break;
            
        case DemoMode::Celload:
            if (getkey('C') && g_testImageSaved) {
                g_celId = celload("test_sprite.bmp");
                await(200);
            }
            if (getkey('D') && g_celId >= 0) {
                celdiv(g_celId, 4, 4);
                await(200);
            }
            if (g_celId >= 0) {
                if (getkey(0x27)) { g_celIndex = (g_celIndex + 1) % 16; await(150); }
                if (getkey(0x25)) { g_celIndex = (g_celIndex + 15) % 16; await(150); }
                if (getkey(0x28)) { g_celIndex = (g_celIndex + 4) % 16; await(150); }
                if (getkey(0x26)) { g_celIndex = (g_celIndex + 12) % 16; await(150); }
            }
            break;
        }
        
        // ESCで終了
        if (stick() & 128) break;
        
        await(16);
    }
    
    return 0;
}
