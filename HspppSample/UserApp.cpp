// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

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

// 表示系デモ (Display) 実体
Screen      g_virtScalingScreen;
bool        g_displaySubVisible    = false;
int         g_dpiChangeCount       = 0;
int         g_dpiLastReported      = 0;
std::string g_dpiChangeLog         = "";
int         g_virtPresetIndex      = 0;  // 既定: 1920x1080 (FHD, uniform scale=2.25)
int         g_virtScaleModeIndex   = 1;  // 既定: linear
int         g_virtLetterColorIndex = 1;  // 既定: 濃シアン (letterbox 視認性確保)
int         g_virtInterpIndex      = 1;  // 既定: linear (gmode_interp: 0=nearest / 1=linear / 2=aniso)

// Anchor Playground サブデモ実体
Screen      g_anchorPlaygroundScreen;
bool        g_anchorPlaygroundVisible = false;
int         g_anchorAspectIndex       = 2;     // 既定: 960x540 (16:9)
bool        g_anchorShowFixed         = true;  // 既定: 比較対照 ON
bool        g_anchorShowGrid          = false; // 既定: リファレンスグリッド OFF
bool        g_anchorShowGuides        = true;  // 既定: ガイド矢印 + 実測値 ON

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
    
    // 前のデモがGUIだった場合、オブジェクトをクリア
    if (g_prevCategory == DemoCategory::GUI) {
        clrobj();
        g_guiObjectsCreated = false;
    }

    // 前のデモが Display で、現在 Display 以外の場合、サブウィンドウを隠す
    if (g_prevCategory == DemoCategory::Display && g_category != DemoCategory::Display) {
        onDisplayDemoLeft();
        win.select();
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
    
    helpWin.color(255, 255, 255).pos(20, 464);
    helpWin.mes("【GUIデモ - Ctrl+Shift + 数字キー】");
    helpWin.color(200, 200, 200).pos(20, 481);
    helpWin.mes("  Ctrl+Shift+1: button/input  Ctrl+Shift+2: chkbox/combox");

    helpWin.color(255, 255, 255).pos(20, 505);
    helpWin.mes("【表示系デモ - Alt+Shift + 数字キー】");
    helpWin.color(200, 200, 200).pos(20, 522);
    helpWin.mes("  Alt+Shift+1: HiDPI/DPI    Alt+Shift+2: 仮想画面  Alt+Shift+3: アンカー");
    helpWin.color(180, 180, 180).pos(20, 538);
    helpWin.mes("  ※仮想画面: S=拡大率/物理サイズ循環 M=vscalemode C=letter色 V=表示 R=既定復帰");
    helpWin.color(180, 180, 180).pos(20, 552);
    helpWin.mes("  ※アンカー: A/Shift+A=比率 G=リファレンスグリッド C=Fixed列 H=ガイド R=既定 V=表示");

    helpWin.color(255, 200, 0).pos(20, 568);
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
        case DemoCategory::GUI:       return "GUI (Ctrl+Shift+1-2)";
        case DemoCategory::Media:     return "マルチメディア (Ctrl+Alt+1)";
        case DemoCategory::Display:   return "表示系 (Alt+Shift+1-3)";
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
        case DemoCategory::GUI:
            switch (static_cast<GUIDemo>(g_demoIndex)) {
                case GUIDemo::Button:    return "button/input/mesbox (ボタン・入力)";
                case GUIDemo::ChoiceBox: return "chkbox/combox/listbox (選択系)";
                default: break;
            }
            break;
        case DemoCategory::Media:
            switch (static_cast<MediaDemo>(g_demoIndex)) {
                case MediaDemo::AudioPlayback: return "mmload/mmplay/mmstop (音声再生)";
                default: break;
            }
            break;
        case DemoCategory::Display:
            switch (static_cast<DisplayDemo>(g_demoIndex)) {
                case DisplayDemo::HiDPI:   return "HiDPI awareness / WM_DPICHANGED ログ";
                case DisplayDemo::Virtual: return "screen_mode_virtual ON/OFF 比較";
                case DisplayDemo::Anchor:  return "AnchorRect / anchor_box / anchor_pos";
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
    bool winPressed = getkey(VK::LWIN) != 0 || getkey(VK::RWIN) != 0;
    
    DemoCategory newCategory = g_category;
    int newIndex = g_demoIndex;
    bool changed = false;
    
    // Ctrl+Shift + 数字: GUIデモ
    if (ctrlPressed && shiftPressed && !altPressed) {
        for (int i = 1; i <= 2; i++) {
            if (getkey('0' + i)) {
                if (i <= static_cast<int>(GUIDemo::COUNT)) {
                    newCategory = DemoCategory::GUI;
                    newIndex = i - 1;
                    changed = true;
                }
            }
        }
    }
    
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
    
    // Alt+Shift + 数字: 表示系デモ (HiDPI/Virtual/Anchor)
    if (altPressed && shiftPressed && !ctrlPressed && !winPressed) {
        for (int i = 1; i <= 3; i++) {
            if (getkey('0' + i)) {
                if (i <= static_cast<int>(DisplayDemo::COUNT)) {
                    newCategory = DemoCategory::Display;
                    newIndex = i - 1;
                    changed = true;
                }
            }
        }
    }

    // Ctrl+Alt + 数字: マルチメディアデモ
    if (altPressed && ctrlPressed && !shiftPressed) {
        for (int i = 1; i <= 1; i++) {
            if (getkey('0' + i)) {
                if (i <= static_cast<int>(MediaDemo::COUNT)) {
                    newCategory = DemoCategory::Media;
                    newIndex = i - 1;
                    changed = true;
                }
            }
        }
    }
    
    // モード切替（数字キー）
    for (int i = 0; i <= 9; i++) {
        if (getkey('0' + i)) {
            if (ctrlPressed && !shiftPressed && !altPressed && !winPressed) {
                // Ctrl + 数字: 拡張デモ (0-9)
                int index = (i == 0) ? 9 : (i - 1);
                if (index < static_cast<int>(ExtendedDemo::COUNT)) {
                    newCategory = DemoCategory::Extended;
                    newIndex = index;
                    changed = true;
                }
            } else if (i >= 1) {
                if (shiftPressed && !ctrlPressed && !altPressed && !winPressed) {
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
                } else if (!ctrlPressed && !shiftPressed && !altPressed
                           && g_category != DemoCategory::Display) {
                    // 数字のみ: 基本デモ
                    // Display カテゴリ表示中はサブデモ側 (processDisplayAction) が
                    // 文字キー (Virtual: S/M/C/V/R, Anchor: A/G/C/H/R/V) を専用ホットキーに
                    // 使用するため、ここでの修飾なし数字キーによる Basic カテゴリ強制遷移は
                    // ユーザーの誤操作で表示系デモから抜けてしまうのを抑止するために無効化する。
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
        // GUIカテゴリから離れる場合はオブジェクトをクリア
        if (g_category == DemoCategory::GUI && newCategory != DemoCategory::GUI) {
            clearGUIObjects();
        }
        
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

void hspMain() {
    // メインウィンドウ作成
    auto win = screen({.width = 640, .height = 480, .title = "HSPPP Feature Demo - Press F1 for Help"});
    
    // ヘルプウィンドウ作成（初期は非表示）
    auto helpWin = screen({.width = 320, .height = 600, .mode = screen_hide, .title = "HSPPP Help"});

    // 表示系デモ用 仮想画面 Scaling サブウィンドウ
    //   - 論理 640x480 / virtual_resolution=true / vscale_linear（既定）
    //   - Display::Virtual サブデモ突入時に gsel で可視化し、ホットキーで
    //     物理クライアントサイズ・vscalemode・letterboxColor を実行時切替する。
    //   - 起動直後は非表示。letterboxColor は視認性の高い濃シアンを既定とする。
    g_virtScalingScreen = screen({
        .width = 640, .height = 480, .mode = screen_hide,
        .title = "Virtual Scaling Demo (HSPPP Display Demo)",
        .virtual_resolution = true,
    });
    g_virtScalingScreen.letterboxColor(0, 96, 128);  // 既定: 濃シアン

    // 表示系デモ用 Anchor Playground サブウィンドウ
    //   - 物理クライアントサイズ 960x540 / virtual_resolution=false（バッファ=物理クライアント）
    //   - Display::Anchor 突入時に gsel で可視化し、A/Shift+A でアスペクト比プリセットを実時間切替する。
    //   - 仮想画面を使わないことで、物理リサイズに anchor_box が直接追従する様子を訴求できる。
    //   - 起動直後は非表示。
    g_anchorPlaygroundScreen = screen({
        .width = 960, .height = 540, .mode = screen_hide,
        .title = "Anchor Playground (HSPPP Display Demo)",
        .virtual_resolution = false,
    });

    // メインウィンドウへフォーカスを戻す
    win.select();

    // 割り込みハンドラ設定
    onclick([]() {
        g_clickCount++;
    });
    
    onkey([]() {
        g_keyCount++;
        g_lastKey = iparam();
    });
    
    hsppp::onexit([]() {
        static int exitAttempts = 0;
        exitAttempts++;
        if (exitAttempts >= 2) {
            end(0);
        }
    });

    // WM_DPICHANGED (0x02E0) を捕捉して HiDPI デモ用ログを蓄積
    //   - wparam の LOWORD に新 DPI が入る
    //   - lparam は OS 推奨ウィンドウ矩形 RECT*（ライブラリ側で適切に処理済み）
    constexpr int WM_DPICHANGED_ID = 0x02E0;
    oncmd([]() {
        const std::int64_t wp = wparam();
        const int newDpi = wp & 0xFFFF;  // LOWORD
        g_dpiChangeCount++;
        g_dpiLastReported = newDpi;
        // ログ末尾 8 件保持: 単純に行数で切り詰める
        std::string line = "[#" + std::to_string(g_dpiChangeCount) + "] DPI -> "
                         + std::to_string(newDpi) + "  (scale "
                         + std::to_string(newDpi * 100 / 96) + "%)\n";
        g_dpiChangeLog += line;
        // 8 行を超えたら先頭から削除
        int lineCount = 0;
        for (char c : g_dpiChangeLog) if (c == '\n') ++lineCount;
        while (lineCount > 8) {
            auto pos = g_dpiChangeLog.find('\n');
            if (pos == std::string::npos) break;
            g_dpiChangeLog.erase(0, pos + 1);
            --lineCount;
        }
    }, WM_DPICHANGED_ID);
    
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
        
        // GUIデモは自身で描画を管理するため、ここでは何もしない
        if (g_category == DemoCategory::GUI) {
            drawGUIDemo(win);
            
            // デモ選択処理（画面遷移）
            processDemoSelection(win);
            processGUIAction(win);
            
            // ESCで終了
            if (stick() & 128) break;
            
            await(16);
            continue;
        }
        
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
        case DemoCategory::GUI:
            drawGUIDemo(win);
            break;
        case DemoCategory::Media:
            drawMediaDemo(win);
            break;
        case DemoCategory::Display:
            drawDisplayDemo(win);
            break;
        }
        
        // フッター（ヘルプ表示案内）
        win.font("MS Gothic", 10, 0);
        win.color(128, 128, 128).pos(10, 445);
        win.mes("F1:ヘルプ ESC:終了 | 1-9:基本 Ctrl+0-9:拡張 Shift+1-4:画像 Alt+1-5:割込");
        win.pos(10, 461);
        win.mes("Ctrl+Shift+1-2:GUI Ctrl+Alt+1:メディア Alt+Shift+1-3:表示系");
        
        win.redraw(1);
        
        // デモ選択処理（画面遷移）
        if (!g_videoMode) {
            processDemoSelection(win);
        }
        
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
        case DemoCategory::GUI:
            processGUIAction(win);
            break;
        case DemoCategory::Media:
            processMediaAction(win);
            break;
        case DemoCategory::Display:
            processDisplayAction(win);
            break;
        }
        
        // ESC: 動画再生中は停止、それ以外は終了
        if (stick() & 128) {
            if (g_videoMode) {
                mmstop(0);
                g_mediaIsPlaying = false;
                g_videoMode = false;
            } else {
                break;
            }
        }
        
        await(16);
    }
}
