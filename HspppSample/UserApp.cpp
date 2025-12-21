// HspppSample/UserApp.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP 機能デモアプリケーション
// 
// 操作方法:
//   F1: ヘルプウィンドウの表示/非表示
//   1-9: 基本デモの選択
//   Ctrl + 0-9/-/=: 拡張デモの選択
//   Shift + 1-4: 画像関連デモの選択
//   Alt + 1-5: 割り込みデモの選択
//   ESC: 終了
//
// 各デモ画面でのアクション:
//   アクションはデモごとに異なり、修飾キーなしの文字キーで実行
//   (画面遷移に使うキーとは別)
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
import <format>;
import <optional>;
using namespace hsppp;

// ═══════════════════════════════════════════════════════════════════
// グローバル状態（実体定義）
// ═══════════════════════════════════════════════════════════════════

// 現在のデモ状態
DemoCategory g_category = DemoCategory::Basic;
int g_demoIndex = static_cast<int>(BasicDemo::Line);
DemoCategory g_prevCategory = DemoCategory::Basic;
int g_prevDemoIndex = static_cast<int>(BasicDemo::Line);

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
bool g_bgscrVisible = false;

// バッファ用変数
bool g_bufferCreated = false;
hsppp::OptInt g_srcBufferId = hsppp::omit;

// 割り込みデモ用変数
int g_clickCount = 0;
int g_keyCount = 0;
int g_lastKey = 0;
int g_cmdMessageCount = 0;
int g_lastCmdMessage = 0;

// onerror デモ用変数
bool g_errorHandlerEnabled = false;
int g_lastErrorCode = 0;
std::string g_lastErrorMessage = "";

// アクション実行結果表示用
std::string g_actionLog = "";

// ═══════════════════════════════════════════════════════════════════
// デモ切り替え時のリセット処理
// ═══════════════════════════════════════════════════════════════════

void onDemoChanged(Screen& win) {
    // 前のデモがWidth/Grollだった場合、ウィンドウサイズとスクロールをリセット
    if (g_prevCategory == DemoCategory::Basic) {
        BasicDemo prev = static_cast<BasicDemo>(g_prevDemoIndex);
        if (prev == BasicDemo::Width || prev == BasicDemo::Groll) {
            win.width(640, 480);
            win.groll(0, 0);
            g_scrollX = 0;
            g_scrollY = 0;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
// ヘルプウィンドウの描画
// ═══════════════════════════════════════════════════════════════════

void drawHelpWindow(Screen& helpWin) {
    helpWin.redraw(0);
    helpWin.cls(4);  // 黒背景
    
    helpWin.font("MS Gothic", 14, 1);
    helpWin.color(255, 255, 0).pos(20, 10);
    helpWin.mes("=== HSPPP 操作ガイド ===");
    
    helpWin.font("MS Gothic", 11, 0);
    helpWin.color(255, 255, 255).pos(20, 35);
    helpWin.mes("【基本操作】");
    helpWin.color(200, 200, 200).pos(20, 52);
    helpWin.mes("  F1: このヘルプの表示/非表示");
    helpWin.pos(20, 68);
    helpWin.mes("  ESC: プログラム終了");
    
    helpWin.color(255, 255, 255).pos(20, 92);
    helpWin.mes("【デモ選択 - 数字キー 1-9】");
    helpWin.color(200, 200, 200).pos(20, 109);
    helpWin.mes("  1: line (直線)     2: circle (円)");
    helpWin.pos(20, 125);
    helpWin.mes("  3: pset/pget (点)  4: boxf (矩形)");
    helpWin.pos(20, 141);
    helpWin.mes("  5: cls (クリア)    6: font (フォント)");
    helpWin.pos(20, 157);
    helpWin.mes("  7: title (タイトル) 8: width (サイズ)");
    helpWin.pos(20, 173);
    helpWin.mes("  9: groll (スクロール)");
    
    helpWin.color(255, 255, 255).pos(20, 197);
    helpWin.mes("【拡張デモ - Ctrl + キー】");
    helpWin.color(200, 200, 200).pos(20, 214);
    helpWin.mes("  Ctrl+1: 数学関数    Ctrl+2: 色関数");
    helpWin.pos(20, 230);
    helpWin.mes("  Ctrl+3: gradf       Ctrl+4: grect");
    helpWin.pos(20, 246);
    helpWin.mes("  Ctrl+5: gsquare     Ctrl+6: gcopy");
    helpWin.pos(20, 262);
    helpWin.mes("  Ctrl+7: gzoom       Ctrl+8: grotate");
    helpWin.pos(20, 278);
    helpWin.mes("  Ctrl+9: 文字列操作  Ctrl+0: システム情報");
    helpWin.pos(20, 294);
    helpWin.mes("  Ctrl+-: ファイル操作 Ctrl+=: マウス入力");
    helpWin.pos(20, 310);
    helpWin.mes("  Ctrl+[: イージング  Ctrl+]: ソート");
    
    helpWin.color(255, 255, 255).pos(20, 334);
    helpWin.mes("【画像デモ - Shift + 数字キー】");
    helpWin.color(200, 200, 200).pos(20, 351);
    helpWin.mes("  Shift+1: bmpsave    Shift+2: picload");
    helpWin.pos(20, 367);
    helpWin.mes("  Shift+3: celload    Shift+4: bgscr");
    
    helpWin.color(255, 255, 255).pos(20, 391);
    helpWin.mes("【割り込みデモ - Alt + 数字キー】");
    helpWin.color(200, 200, 200).pos(20, 408);
    helpWin.mes("  Alt+1: onclick      Alt+2: onkey");
    helpWin.pos(20, 424);
    helpWin.mes("  Alt+3: onexit       Alt+4: oncmd");
    helpWin.pos(20, 440);
    helpWin.mes("  Alt+5: onerror");
    
    helpWin.color(255, 200, 0).pos(20, 471);
    helpWin.mes("※修飾キー(Ctrl/Alt/Shift)押下中はアクション無効");
    
    helpWin.redraw(1);
}

// ═══════════════════════════════════════════════════════════════════
// カテゴリ名とデモ名を取得
// ═══════════════════════════════════════════════════════════════════

std::string getCategoryName() {
    switch (g_category) {
        case DemoCategory::Basic:     return "基本 (1-9)";
        case DemoCategory::Extended:  return "拡張 (Ctrl+0-9,-,=,[,])";
        case DemoCategory::Image:     return "画像 (Shift+1-3)";
        case DemoCategory::Interrupt: return "割り込み (Alt+1-5)";
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
                case ExtendedDemo::Math:       return "Math Functions";
                case ExtendedDemo::Color:      return "Color Functions";
                case ExtendedDemo::Gradf:      return "gradf (グラデーション)";
                case ExtendedDemo::Grect:      return "grect (回転矩形)";
                case ExtendedDemo::Gsquare:    return "gsquare (任意四角形)";
                case ExtendedDemo::Gcopy:      return "gcopy (画面コピー)";
                case ExtendedDemo::Gzoom:      return "gzoom (変倍コピー)";
                case ExtendedDemo::Grotate:    return "grotate (回転コピー)";
                case ExtendedDemo::StringFunc: return "String Functions (文字列操作)";
                case ExtendedDemo::SystemInfo: return "System Info (sysinfo/dirinfo/peek/poke)";
                case ExtendedDemo::FileOps:    return "File Operations (exist/dirlist/bload/bsave/exec/dialog)";
                case ExtendedDemo::InputMouse: return "Mouse Input (mouse/mousex/mousey/mousew)";
                case ExtendedDemo::Easing:     return "Easing Functions (setease/getease/geteasef)";
                case ExtendedDemo::Sorting:    return "Sort Functions (sortval/sortstr/sortnote/sortget)";
                default: break;
            }
            break;
        case DemoCategory::Image:
            switch (static_cast<ImageDemo>(g_demoIndex)) {
                case ImageDemo::Bmpsave: return "bmpsave (BMP保存)";
                case ImageDemo::Picload: return "picload (画像ロード)";
                case ImageDemo::Celload: return "celload/celput/loadCel";
                case ImageDemo::Bgscr:   return "bgscr (枠なしウィンドウ)";
                default: break;
            }
            break;
        case DemoCategory::Interrupt:
            switch (static_cast<InterruptDemo>(g_demoIndex)) {
                case InterruptDemo::OnClick: return "onclick (クリック割り込み)";
                case InterruptDemo::OnKey:   return "onkey (キー割り込み)";
                case InterruptDemo::OnExit:  return "onexit (終了割り込み)";
                case InterruptDemo::OnCmd:   return "oncmd (Windowsメッセージ割り込み)";
                case InterruptDemo::OnError: return "onerror (エラーハンドリング)";
                default: break;
            }
            break;
    }
    return "Unknown";
}

// ═══════════════════════════════════════════════════════════════════
// 画面遷移処理（デモ選択のみ）
// ═══════════════════════════════════════════════════════════════════

void processDemoSelection(Screen& win) {
    bool ctrlPressed = getkey(VK::CONTROL) != 0;
    bool shiftPressed = getkey(VK::SHIFT) != 0;
    bool altPressed = getkey(VK::MENU) != 0;
    
    DemoCategory newCategory = g_category;
    int newIndex = g_demoIndex;
    bool changed = false;
    
    // Ctrl + - : ファイル操作デモ
    if (ctrlPressed && !shiftPressed && !altPressed && getkey(0xBD)) {
        newCategory = DemoCategory::Extended;
        newIndex = static_cast<int>(ExtendedDemo::FileOps);
        changed = true;
    }
    
    // Ctrl + ; (または =) : マウス入力デモ
    // 日本語キーボードでは ; が 0xBB、USキーボードでは = が 0xBB
    if (ctrlPressed && !shiftPressed && !altPressed && (getkey(0xBB) || getkey(0xBA))) {
        newCategory = DemoCategory::Extended;
        newIndex = static_cast<int>(ExtendedDemo::InputMouse);
        changed = true;
    }
    
    // Ctrl + [ : イージングデモ (VK_OEM_4 = 0xDB)
    if (ctrlPressed && !shiftPressed && !altPressed && getkey(0xDB)) {
        newCategory = DemoCategory::Extended;
        newIndex = static_cast<int>(ExtendedDemo::Easing);
        changed = true;
    }
    
    // Ctrl + ] : ソートデモ (VK_OEM_6 = 0xDD)
    if (ctrlPressed && !shiftPressed && !altPressed && getkey(0xDD)) {
        newCategory = DemoCategory::Extended;
        newIndex = static_cast<int>(ExtendedDemo::Sorting);
        changed = true;
    }
    
    // モード切替（数字キー）
    for (int i = 0; i <= 9; i++) {
        if (getkey('0' + i)) {
            if (ctrlPressed && !shiftPressed && !altPressed) {
                // Ctrl + 数字: 拡張デモ (0-9)
                int index = (i == 0) ? 9 : (i - 1);
                if (index < static_cast<int>(ExtendedDemo::COUNT)) {
                    newCategory = DemoCategory::Extended;
                    newIndex = index;
                    changed = true;
                }
            } else if (i >= 1) {
                if (shiftPressed && !ctrlPressed && !altPressed) {
                    // Shift + 数字: 画像デモ
                    if (i <= static_cast<int>(ImageDemo::COUNT)) {
                        newCategory = DemoCategory::Image;
                        newIndex = i - 1;
                        changed = true;
                    }
                } else if (altPressed && !ctrlPressed && !shiftPressed) {
                    // Alt + 数字: 割り込みデモ
                    if (i <= static_cast<int>(InterruptDemo::COUNT)) {
                        newCategory = DemoCategory::Interrupt;
                        newIndex = i - 1;
                        changed = true;
                    }
                } else if (!ctrlPressed && !shiftPressed && !altPressed) {
                    // 数字のみ: 基本デモ
                    if (i <= static_cast<int>(BasicDemo::COUNT)) {
                        newCategory = DemoCategory::Basic;
                        newIndex = i - 1;
                        changed = true;
                    }
                }
            }
        }
    }
    
    // デモが変更された場合
    if (changed && (newCategory != g_category || newIndex != g_demoIndex)) {
        g_prevCategory = g_category;
        g_prevDemoIndex = g_demoIndex;
        g_category = newCategory;
        g_demoIndex = newIndex;
        onDemoChanged(win);
        await(200);
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
        win.font("MS Gothic", 10, 0);
        win.color(128, 128, 128).pos(20, 455);
        win.mes("F1: ヘルプ  |  ESC: 終了  |  1-9: 基本  |  Ctrl+0-9/-/=: 拡張  |  Shift+1-4: 画像  |  Alt+1-5: 割り込み");
        
        win.redraw(1);
        
        // デモ選択処理（画面遷移）
        processDemoSelection(win);
        
        // 各デモのアクション処理
        switch (g_category) {
        case DemoCategory::Basic:
            processBasicAction(win);
            break;
        case DemoCategory::Extended:
            processExtendedAction(win);
            break;
        case DemoCategory::Image:
            processImageAction(win);
            break;
        case DemoCategory::Interrupt:
            processInterruptAction(win);
            break;
        }
        
        // ESCで終了
        if (stick() & 128) break;
        
        await(16);
    }
    
    return 0;
}
