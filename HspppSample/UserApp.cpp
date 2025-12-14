// HspppSample/UserApp.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモ: P1 + P2 + Math + Interrupt 機能テスト
// P1: cls, font, title, width, groll
// P2: picload, celload, celput, celdiv, bmpsave
// Math: abs, sin, cos, rnd, limit, hsvcolor, rgbcolor
// Interrupt: onclick, onkey, onexit, onerror, oncmd
// ═══════════════════════════════════════════════════════════════════

import hsppp;
import <format>;
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
    Celload,
    // Math/Color
    Math,
    Color,
    // Interrupt
    Interrupt
};

DemoMode g_mode = DemoMode::Math;  // 数学関数デモを初期モードに
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

// Math用変数
double g_angle = 0.0;
int g_randomSeed = 0;

// Interrupt用変数
int g_clickCount = 0;
int g_keyCount = 0;
int g_lastKey = 0;

// ユーザーのエントリーポイント
int hspMain() {
    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  メインウィンドウ作成（バッファサイズ640x480）                ║
    // ╚═══════════════════════════════════════════════════════════════╝
    auto win = screen({.width = 640, .height = 480, .title = "HSPPP P1+P2+Interrupt Demo"});
    
    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  割り込みハンドラ設定                                        ║
    // ╚═══════════════════════════════════════════════════════════════╝
    
    // onclick - マウスクリック時の割り込み
    onclick([]() -> int {
        g_clickCount++;
        return 0;
    });
    
    // onkey - キー入力時の割り込み
    onkey([]() -> int {
        g_keyCount++;
        g_lastKey = iparam();  // 文字コードを取得
        return 0;
    });
    
    // onexit - 終了時の割り込み（HSP互換：end()を呼ぶまで終了しない）
    hsppp::onexit([]() -> int {
        // 終了確認（簡易実装：2回連続でクローズボタンを押すと終了）
        static int exitAttempts = 0;
        exitAttempts++;
        if (exitAttempts >= 2) {
            end(0);  // 2回目で終了
        }
        return 0;
    });
    
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
            
        // ═══════════════════════════════════════════════════════════
        // Math機能
        // ═══════════════════════════════════════════════════════════
        case DemoMode::Math:
            win.mes("Current: MATH FUNCTIONS - Press 9");
            win.pos(20, 85);
            win.mes("数学関数デモ: sin, cos, rnd, limit, sqrt, powf");
            
            // Sin/Cos波形描画
            win.color(0, 128, 0).pos(50, 120);
            win.mes("sin/cos 波形 (角度を自動更新中)");
            
            // 波形の背景
            win.color(240, 240, 240);
            win.boxf(50, 150, 590, 250);
            
            // X軸
            win.color(128, 128, 128);
            win.line(590, 200, 50, 200);
            
            {
                // 波形描画（スコープで括る）
                // 注: deg2rad() を使用して度数法 → ラジアン変換
                
                // Sin波形（赤）
                win.color(255, 0, 0);
                for (int x = 0; x < 540; x++) {
                    double angle = hsppp::deg2rad(g_angle + x * 2);  // 度数法をラジアン変換
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
                    double angle = hsppp::deg2rad(g_angle + x * 2);  // 度数法をラジアン変換
                    int y = 200 - static_cast<int>(hsppp::cos(angle) * 40);
                    if (x == 0) {
                        win.pos(50 + x, y);
                    } else {
                        win.line(50 + x, y);
                    }
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
            win.pos(50, 370);
            win.mes("limit(50, 0, 100) = " + str(hsppp::limit(50, 0, 100)));
            
            // sqrt/pow表示
            win.pos(300, 320);
            win.mes("sqrt/pow デモ:");
            win.pos(300, 340);
            win.mes("sqrt(2) = " + str(hsppp::sqrt(2.0)));
            win.pos(300, 355);
            win.mes("pow(2, 10) = " + str(hsppp::pow(2.0, 10.0)));
            win.pos(300, 370);
            win.mes("abs(-42) = " + str(hsppp::abs(-42)));
            
            // 角度を更新（アニメーション）
            g_angle += 2.0;
            if (g_angle >= 360.0) g_angle -= 360.0;
            break;
            
        case DemoMode::Color:
            win.mes("Current: COLOR FUNCTIONS - Press 0");
            win.pos(20, 85);
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
                hsvcolor(0, s, 255);  // 赤の彩度変化
                win.boxf(50 + s * 2, 210, 50 + s * 2 + 2, 250);
            }
            
            // RGBカラー
            win.color(0, 0, 0).pos(50, 270);
            win.mes("rgbcolor サンプル:");
            
            rgbcolor(0xFF0000);  // 赤
            win.boxf(50, 290, 100, 340);
            rgbcolor(0x00FF00);  // 緑
            win.boxf(110, 290, 160, 340);
            rgbcolor(0x0000FF);  // 青
            win.boxf(170, 290, 220, 340);
            rgbcolor(0xFFFF00);  // 黄
            win.boxf(230, 290, 280, 340);
            rgbcolor(0xFF00FF);  // マゼンタ
            win.boxf(290, 290, 340, 340);
            rgbcolor(0x00FFFF);  // シアン
            win.boxf(350, 290, 400, 340);
            
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
            
        // ═══════════════════════════════════════════════════════════
        // Interrupt機能
        // ═══════════════════════════════════════════════════════════
        case DemoMode::Interrupt:
            win.mes("Current: INTERRUPT HANDLERS - Press I");
            win.pos(20, 85);
            win.mes("onclick, onkey, onexit, oncmd, onerror デモ");
            
            // 割り込みステータス表示
            win.color(0, 0, 128).pos(50, 130);
            win.mes("=== 割り込みハンドラ状態 ===");
            
            win.color(0, 0, 0).pos(50, 160);
            win.mes("クリック回数 (onclick): " + str(g_clickCount));
            win.pos(50, 180);
            win.mes("キー入力回数 (onkey): " + str(g_keyCount));
            win.pos(50, 200);
            win.mes(std::format("最後のキーコード: {} (0x{:x})", g_lastKey, g_lastKey));
            
            win.color(128, 0, 0).pos(50, 240);
            win.mes("=== 割り込みパラメータ (システム変数) ===");
            win.color(0, 0, 0).pos(50, 270);
            win.mes("iparam() = " + str(iparam()));
            win.pos(50, 290);
            win.mes("wparam() = " + str(wparam()));
            win.pos(50, 310);
            win.mes("lparam() = " + str(lparam()));
            
            // 使い方
            win.color(0, 128, 0).pos(50, 360);
            win.mes("=== 操作方法 ===");
            win.color(0, 0, 0).pos(50, 385);
            win.mes("- マウスクリック: onclick カウンター増加");
            win.pos(50, 405);
            win.mes("- キー入力: onkey カウンター増加 + キーコード表示");
            win.pos(50, 425);
            win.mes("- ウィンドウ閉じる: onexit で確認ダイアログ");
            break;
        }
        
        // 共通ヘルプ
        win.font("MS Gothic", 11, 0);
        win.color(128, 128, 128).pos(20, 440);
        win.mes("P1: 1=cls 2=font 3=title 4=width 5=groll | P2: 6=bmpsave 7=picload 8=celload");
        win.pos(20, 455);
        win.mes("Math: 9=math 0=color | Interrupt: I=interrupt | ESC=Exit");
        
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
        if (getkey('9')) { g_mode = DemoMode::Math; await(200); }
        if (getkey('0')) { g_mode = DemoMode::Color; await(200); }
        if (getkey('I')) { g_mode = DemoMode::Interrupt; await(200); }
        
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
            
        // Math/Color機能（自動更新のみ）
        case DemoMode::Math:
            // 角度は描画ループで自動更新
            if (getkey('R')) {
                // randomizeを再実行
                randomize();
                await(200);
            }
            break;
            
        case DemoMode::Color:
            // 特に操作なし
            break;
        }
        
        // ESCで終了
        if (stick() & 128) break;
        
        await(16);
    }
    
    return 0;
}
