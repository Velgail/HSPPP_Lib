// HspppSample/UserApp.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP 機能デモアプリケーション
// 
// 操作方法:
//   F1: ヘルプウィンドウの表示/非表示
//   1-9: 基本デモの選択
//   Ctrl + 1-8: 拡張デモの選択
//   Shift + 1-3: 画像関連デモの選択
//   Alt + 1-3: 割り込みデモの選択
//   ESC: 終了
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
import <format>;
using namespace hsppp;

// ═══════════════════════════════════════════════════════════════════
// グローバル状態（実体定義）
// ═══════════════════════════════════════════════════════════════════

// 現在のデモ状態
DemoCategory g_category = DemoCategory::Basic;
int g_demoIndex = static_cast<int>(BasicDemo::Line);

// ヘルプウィンドウ状態
bool g_helpVisible = false;

// 描画デモ用変数
int g_clsMode = 0;
int g_fontStyle = 0;
int g_fontSize = 12;
int g_scrollX = 0;
int g_scrollY = 0;
double g_angle = 0.0;

// 画像デモ用変数
bool g_testImageSaved = false;
bool g_testImageLoaded = false;
int g_celId = -1;
int g_celIndex = 0;

// バッファ用変数
bool g_bufferCreated = false;

// 割り込みデモ用変数
int g_clickCount = 0;
int g_keyCount = 0;
int g_lastKey = 0;

// ═══════════════════════════════════════════════════════════════════
// 外部関数宣言（各Demo~.cppで定義）
// ═══════════════════════════════════════════════════════════════════

void drawBasicDemo(Screen& win);
void drawExtendedDemo(Screen& win);
void drawImageDemo(Screen& win);
void drawInterruptDemo(Screen& win);

// ═══════════════════════════════════════════════════════════════════
// ヘルプウィンドウの描画
// ═══════════════════════════════════════════════════════════════════

void drawHelpWindow(Screen& helpWin) {
    helpWin.redraw(0);
    helpWin.cls(4);  // 黒背景
    
    helpWin.font("MS Gothic", 14, 1);
    helpWin.color(255, 255, 0).pos(20, 10);
    helpWin.mes("=== HSPPP 操作ガイド ===");
    
    helpWin.font("MS Gothic", 12, 0);
    helpWin.color(255, 255, 255).pos(20, 40);
    helpWin.mes("【基本操作】");
    helpWin.color(200, 200, 200).pos(20, 60);
    helpWin.mes("  F1: このヘルプの表示/非表示");
    helpWin.pos(20, 80);
    helpWin.mes("  ESC: プログラム終了");
    
    helpWin.color(255, 255, 255).pos(20, 110);
    helpWin.mes("【デモ選択 - 数字キー 1-9】");
    helpWin.color(200, 200, 200).pos(20, 130);
    helpWin.mes("  1: line (直線)     2: circle (円)");
    helpWin.pos(20, 150);
    helpWin.mes("  3: pset/pget (点)  4: boxf (矩形)");
    helpWin.pos(20, 170);
    helpWin.mes("  5: cls (クリア)    6: font (フォント)");
    helpWin.pos(20, 190);
    helpWin.mes("  7: title (タイトル) 8: width (サイズ)");
    helpWin.pos(20, 210);
    helpWin.mes("  9: groll (スクロール)");
    
    helpWin.color(255, 255, 255).pos(20, 240);
    helpWin.mes("【拡張デモ - Ctrl + 数字キー】");
    helpWin.color(200, 200, 200).pos(20, 260);
    helpWin.mes("  Ctrl+1: 数学関数    Ctrl+2: 色関数");
    helpWin.pos(20, 280);
    helpWin.mes("  Ctrl+3: gradf       Ctrl+4: grect");
    helpWin.pos(20, 300);
    helpWin.mes("  Ctrl+5: gsquare     Ctrl+6: gcopy");
    helpWin.pos(20, 320);
    helpWin.mes("  Ctrl+7: gzoom       Ctrl+8: grotate");
    
    helpWin.color(255, 255, 255).pos(20, 350);
    helpWin.mes("【画像デモ - Shift + 数字キー】");
    helpWin.color(200, 200, 200).pos(20, 370);
    helpWin.mes("  Shift+1: bmpsave    Shift+2: picload");
    helpWin.pos(20, 390);
    helpWin.mes("  Shift+3: celload/celput");
    
    helpWin.color(255, 255, 255).pos(20, 420);
    helpWin.mes("【割り込みデモ - Alt + 数字キー】");
    helpWin.color(200, 200, 200).pos(20, 440);
    helpWin.mes("  Alt+1: onclick      Alt+2: onkey");
    helpWin.pos(20, 460);
    helpWin.mes("  Alt+3: onexit");
    
    helpWin.redraw(1);
}

// ═══════════════════════════════════════════════════════════════════
// カテゴリ名とデモ名を取得
// ═══════════════════════════════════════════════════════════════════

std::string getCategoryName() {
    switch (g_category) {
        case DemoCategory::Basic:     return "基本 (1-9)";
        case DemoCategory::Extended:  return "拡張 (Ctrl+1-8)";
        case DemoCategory::Image:     return "画像 (Shift+1-3)";
        case DemoCategory::Interrupt: return "割り込み (Alt+1-3)";
    }
    return "Unknown";
}

std::string getDemoName() {
    switch (g_category) {
        case DemoCategory::Basic:
            switch (static_cast<BasicDemo>(g_demoIndex)) {
                case BasicDemo::Line:   return "line (直線描画)";
                case BasicDemo::Circle: return "circle (円描画)";
                case BasicDemo::Pset:   return "pset/pget (点描画)";
                case BasicDemo::Boxf:   return "boxf (矩形塗りつぶし)";
                case BasicDemo::Cls:    return "cls (画面クリア)";
                case BasicDemo::Font:   return "font (フォント)";
                case BasicDemo::Title:  return "title (タイトル)";
                case BasicDemo::Width:  return "width (ウィンドウサイズ)";
                case BasicDemo::Groll:  return "groll (スクロール)";
                default: break;
            }
            break;
        case DemoCategory::Extended:
            switch (static_cast<ExtendedDemo>(g_demoIndex)) {
                case ExtendedDemo::Math:     return "Math Functions";
                case ExtendedDemo::Color:    return "Color Functions";
                case ExtendedDemo::Gradf:    return "gradf (グラデーション)";
                case ExtendedDemo::Grect:    return "grect (回転矩形)";
                case ExtendedDemo::Gsquare:  return "gsquare (任意四角形)";
                case ExtendedDemo::Gcopy:    return "gcopy (画面コピー)";
                case ExtendedDemo::Gzoom:    return "gzoom (変倍コピー)";
                case ExtendedDemo::Grotate:  return "grotate (回転コピー)";
                default: break;
            }
            break;
        case DemoCategory::Image:
            switch (static_cast<ImageDemo>(g_demoIndex)) {
                case ImageDemo::Bmpsave: return "bmpsave (BMP保存)";
                case ImageDemo::Picload: return "picload (画像ロード)";
                case ImageDemo::Celload: return "celload/celput";
                default: break;
            }
            break;
        case DemoCategory::Interrupt:
            switch (static_cast<InterruptDemo>(g_demoIndex)) {
                case InterruptDemo::OnClick: return "onclick (クリック割り込み)";
                case InterruptDemo::OnKey:   return "onkey (キー割り込み)";
                case InterruptDemo::OnExit:  return "onexit (終了割り込み)";
                default: break;
            }
            break;
    }
    return "Unknown";
}

// ═══════════════════════════════════════════════════════════════════
// キー入力処理
// ═══════════════════════════════════════════════════════════════════

void processInput(Screen& win) {
    bool ctrlPressed = getkey(VK::CONTROL) != 0;
    bool shiftPressed = getkey(VK::SHIFT) != 0;
    bool altPressed = getkey(VK::MENU) != 0;
    
    // モード切替（数字キー）
    for (int i = 1; i <= 9; i++) {
        if (getkey('0' + i)) {
            if (ctrlPressed && !shiftPressed && !altPressed) {
                // Ctrl + 数字: 拡張デモ
                if (i <= static_cast<int>(ExtendedDemo::COUNT)) {
                    g_category = DemoCategory::Extended;
                    g_demoIndex = i - 1;
                    await(200);
                }
            } else if (shiftPressed && !ctrlPressed && !altPressed) {
                // Shift + 数字: 画像デモ
                if (i <= static_cast<int>(ImageDemo::COUNT)) {
                    g_category = DemoCategory::Image;
                    g_demoIndex = i - 1;
                    await(200);
                }
            } else if (altPressed && !ctrlPressed && !shiftPressed) {
                // Alt + 数字: 割り込みデモ
                if (i <= static_cast<int>(InterruptDemo::COUNT)) {
                    g_category = DemoCategory::Interrupt;
                    g_demoIndex = i - 1;
                    await(200);
                }
            } else if (!ctrlPressed && !shiftPressed && !altPressed) {
                // 数字のみ: 基本デモ
                if (i <= static_cast<int>(BasicDemo::COUNT)) {
                    g_category = DemoCategory::Basic;
                    g_demoIndex = i - 1;
                    await(200);
                }
            }
        }
    }
    
    // モード別操作
    if (g_category == DemoCategory::Basic) {
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
    else if (g_category == DemoCategory::Extended) {
        switch (static_cast<ExtendedDemo>(g_demoIndex)) {
        case ExtendedDemo::Math:
            if (getkey('R')) {
                randomize();
                await(200);
            }
            break;
            
        case ExtendedDemo::Grect:
            if (getkey(VK::LEFT)) { g_angle -= 5.0; await(50); }
            if (getkey(VK::RIGHT)) { g_angle += 5.0; await(50); }
            break;
            
        case ExtendedDemo::Gcopy:
            if (getkey('C') && !g_bufferCreated) {
                // バッファを作成してコピーデモ
                auto buf = buffer({.width = 200, .height = 200});
                buf.color(255, 128, 0).boxf();
                buf.color(0, 0, 200);
                buf.circle(20, 20, 180, 180, 1);
                buf.color(255, 255, 255).pos(50, 90);
                buf.mes("Buffer");
                
                // メインウィンドウにコピー
                win.select();
                pos(50, 320);
                gmode(0, 100, 100);
                gcopy(buf.id(), 0, 0, 200, 200);
                
                g_bufferCreated = true;
                await(200);
            }
            break;
            
        default:
            break;
        }
    }
    else if (g_category == DemoCategory::Image) {
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
            
        default:
            break;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
// メインエントリーポイント
// ═══════════════════════════════════════════════════════════════════

int hspMain() {
    // メインウィンドウ作成
    auto win = screen({.width = 640, .height = 480, .title = "HSPPP Feature Demo - Press F1 for Help"});
    
    // ヘルプウィンドウ作成（初期は非表示）
    auto helpWin = screen({.width = 320, .height = 500, .mode = screen_hide, .title = "HSPPP Help"});
    
    // 割り込みハンドラ設定
    onclick([]() -> int {
        g_clickCount++;
        return 0;
    });
    
    onkey([]() -> int {
        g_keyCount++;
        g_lastKey = iparam();
        return 0;
    });
    
    hsppp::onexit([]() -> int {
        static int exitAttempts = 0;
        exitAttempts++;
        if (exitAttempts >= 2) {
            end(0);
        }
        return 0;
    });
    
    // メインループ
    while (true) {
        // F1でヘルプ表示切替
        if (getkey(VK::F1)) {
            g_helpVisible = !g_helpVisible;
            gsel(helpWin.id(), g_helpVisible ? 1 : -1);
            if (g_helpVisible) {
                drawHelpWindow(helpWin);
            }
            await(200);
        }
        
        // メインウィンドウの描画
        win.select();
        win.redraw(0);
        
        // 背景クリア
        if (g_category == DemoCategory::Basic && g_demoIndex == static_cast<int>(BasicDemo::Cls)) {
            win.cls(g_clsMode);
        } else {
            win.cls(0);
        }
        
        // タイトル
        win.font("MS Gothic", 16, 1);
        win.color(0, 0, 128).pos(20, 20);
        win.mes("=== HSPPP Feature Demo ===");
        
        // 現在のカテゴリとデモ名
        win.font("MS Gothic", 12, 0);
        win.color(0, 128, 0).pos(20, 45);
        win.mes("[" + getCategoryName() + "] " + getDemoName());
        
        win.font("MS Gothic", 14, 0);
        win.color(0, 0, 0).pos(20, 60);
        
        // デモ内容の描画
        switch (g_category) {
        case DemoCategory::Basic:
            drawBasicDemo(win);
            break;
        case DemoCategory::Extended:
            drawExtendedDemo(win);
            break;
        case DemoCategory::Image:
            drawImageDemo(win);
            break;
        case DemoCategory::Interrupt:
            drawInterruptDemo(win);
            break;
        }
        
        // フッター（ヘルプ表示案内）
        win.font("MS Gothic", 11, 0);
        win.color(128, 128, 128).pos(20, 455);
        win.mes("F1: ヘルプ表示  |  ESC: 終了  |  1-9: 基本  |  Ctrl+1-8: 拡張  |  Shift+1-3: 画像  |  Alt+1-3: 割り込み");
        
        win.redraw(1);
        
        // キー入力処理
        processInput(win);
        
        // ESCで終了
        if (stick() & 128) break;
        
        await(16);
    }
    
    return 0;
}
