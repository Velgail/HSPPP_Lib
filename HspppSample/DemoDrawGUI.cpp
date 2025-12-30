// HspppSample/DemoDrawGUI.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP GUIデモ - ボタン、入力、チェックボックス、コンボ・リストボックス
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
#include <memory>
import hsppp;
using namespace hsppp;

// ═══════════════════════════════════════════════════════════════════
// GUIデモ用グローバル変数の実体
// ═══════════════════════════════════════════════════════════════════

bool g_guiObjectsCreated = false;
int g_buttonClickCount = 0;
std::string g_inputText = "Hello HSPPP!";
int g_inputNumber = 42;
std::string g_mesboxText = "Line 1\nLine 2\nLine 3\nEdit me!";

// 選択系GUIコントロール用の状態変数（shared_ptrでライフタイム安全）
auto g_checkState = std::make_shared<int>(0);
auto g_comboxState = std::make_shared<int>(0);
auto g_listboxState = std::make_shared<int>(0);

// GUIオブジェクトID
static int g_btnId1 = -1;
static int g_btnId2 = -1;
static int g_inputStrId = -1;
static int g_inputIntId = -1;
static int g_mesboxId = -1;
static int g_chkId = -1;
static int g_comboxId = -1;
static int g_listboxId = -1;

// 現在のGUIデモの種類を追跡
static int g_currentGUIDemo = -1;

// ═══════════════════════════════════════════════════════════════════
// GUIデモの初期化（オブジェクト作成）
// ═══════════════════════════════════════════════════════════════════

static void initButtonInputDemo(Screen& win) {
    // 背景をクリア（初回のみ）
    win.color(255, 255, 255).cls();
    win.color(0, 0, 0);
    
    win.pos(10, 10);
    win.mes("=== Button & Input Demo (Ctrl+Shift+1) ===");
    win.mes("");
    win.mes("ボタンをクリックするとカウントアップします");
    
    win.pos(20, 80);
    objsize(120, 30);
    
    // ボタン1: カウントアップ
    g_btnId1 = button("Count Up", []() {
        g_buttonClickCount++;
        return 0;
    });
    
    // ボタン2: リセット
    g_btnId2 = button("Reset", []() {
        g_buttonClickCount = 0;
        g_inputText = "Hello HSPPP!";
        g_inputNumber = 42;
        objprm(g_inputStrId, g_inputText);
        objprm(g_inputIntId, g_inputNumber);
        return 0;
    });
    
    win.pos(20, 140);
    win.color(0, 0, 0).mes("文字列入力:");
    win.pos(20, 160);
    objsize(200, 25);
    g_inputStrId = input(g_inputText, 200, 25, 256);
    
    win.pos(20, 200);
    win.color(0, 0, 0).mes("数値入力:");
    win.pos(20, 220);
    objsize(150, 25);
    g_inputIntId = input(g_inputNumber, 150, 25, 10);
    
    win.pos(20, 260);
    win.color(0, 0, 0).mes("複数行テキスト (mesbox):");
    win.pos(20, 280);
    objsize(260, 60);
    g_mesboxId = mesbox(g_mesboxText, 260, 60, 1, 1000);
    
    // フッター説明
    win.pos(10, 455);
    win.color(128, 128, 128);
    win.mes("F1:ヘルプ ESC:終了 | 1-9:基本 Ctrl+0-9:拡張 Shift+1-4:画像 Alt+1-5:割り込み Ctrl+Shift+1-2:GUI");
    
    // 画面を確定（redraw 1相当）
    win.redraw(1);
}

static void initChoiceBoxDemo(Screen& win) {
    // 背景をクリア（初回のみ）
    win.color(255, 255, 255).cls();
    win.color(0, 0, 0);
    
    win.pos(10, 10);
    win.mes("=== Choice Controls Demo (Ctrl+Shift+2) ===");
    win.mes("");
    
    win.pos(20, 60);
    objsize(200, 24);
    
    // チェックボックス
    win.color(0, 0, 0).mes("チェックボックス:");
    win.pos(20, 80);
    g_chkId = chkbox("Enable Feature", g_checkState);
    
    // コンボボックス
    win.pos(20, 130);
    win.color(0, 0, 0).mes("コンボボックス:");
    win.pos(20, 150);
    objsize(180, 24);
    g_comboxId = combox(g_comboxState, 120, "Option A\nOption B\nOption C\nOption D");
    
    // リストボックス
    win.pos(20, 210);
    win.color(0, 0, 0).mes("リストボックス:");
    win.pos(20, 230);
    objsize(180, 80);
    g_listboxId = listbox(g_listboxState, 80, "Item 1\nItem 2\nItem 3\nItem 4\nItem 5");
    
    // 状態表示のラベル
    win.pos(250, 60);
    win.color(0, 0, 0);
    win.mes("現在の状態:");
    
    // フッター説明
    win.pos(10, 455);
    win.color(128, 128, 128);
    win.mes("F1:ヘルプ ESC:終了 | 1-9:基本 Ctrl+0-9:拡張 Shift+1-4:画像 Alt+1-5:割り込み Ctrl+Shift+1-2:GUI");
    
    // 画面を確定
    win.redraw(1);
}

// ═══════════════════════════════════════════════════════════════════
// GUIデモの更新描画（変化する部分のみ）
// ═══════════════════════════════════════════════════════════════════

static void updateButtonInputDemo(Screen& win) {
    // 変化する部分のみ再描画
    // カウント表示エリアをクリアして再描画
    win.redraw(0);  // 描画開始
    
    win.color(255, 255, 255);
    win.boxf(250, 160, 500, 270);  // 右側の表示エリアをクリア
    win.boxf(350, 75, 500, 110);   // カウント表示エリアをクリア
    
    win.pos(250, 180);
    win.color(0, 100, 0);
    win.mes("Current: " + g_inputText);
    
    win.pos(250, 220);
    win.mes("Current: " + std::to_string(g_inputNumber));
    
    win.pos(350, 80);
    win.mes("Count: " + std::to_string(g_buttonClickCount));
    
    win.redraw(1);  // 描画終了
}

static void updateChoiceBoxDemo(Screen& win) {
    // 変化する部分のみ再描画
    win.redraw(0);
    
    win.color(255, 255, 255);
    win.boxf(250, 75, 450, 270);  // 状態表示エリアをクリア
    
    win.pos(250, 80);
    win.color(0, 100, 0);
    win.mes("Check: " + std::string(*g_checkState ? "ON" : "OFF"));
    
    win.pos(250, 130);
    win.mes("Combo: " + std::to_string(*g_comboxState));
    
    win.pos(250, 200);
    win.mes("List: " + std::to_string(*g_listboxState));
    
    win.redraw(1);
}

// ═══════════════════════════════════════════════════════════════════
// GUIデモのクリーンアップ
// ═══════════════════════════════════════════════════════════════════

void clearGUIObjects() {
    clrobj();
    g_guiObjectsCreated = false;
    g_currentGUIDemo = -1;
    g_btnId1 = g_btnId2 = -1;
    g_inputStrId = g_inputIntId = g_mesboxId = -1;
    g_chkId = g_comboxId = g_listboxId = -1;
}

// ═══════════════════════════════════════════════════════════════════
// GUIデモのメイン描画関数
// ═══════════════════════════════════════════════════════════════════

void drawGUIDemo(Screen& win) {
    int demo = g_demoIndex;
    
    // デモが切り替わったらオブジェクトをクリアして再作成
    if (g_currentGUIDemo != demo) {
        if (g_guiObjectsCreated) {
            clearGUIObjects();
        }
    }
    
    // 初回のみオブジェクト作成
    if (!g_guiObjectsCreated) {
        switch (static_cast<GUIDemo>(demo)) {
            case GUIDemo::Button:
                initButtonInputDemo(win);
                break;
            case GUIDemo::ChoiceBox:
                initChoiceBoxDemo(win);
                break;
            default:
                break;
        }
        g_guiObjectsCreated = true;
        g_currentGUIDemo = demo;
    } else {
        // 更新描画（変化する部分のみ）
        switch (static_cast<GUIDemo>(demo)) {
            case GUIDemo::Button:
                updateButtonInputDemo(win);
                break;
            case GUIDemo::ChoiceBox:
                updateChoiceBoxDemo(win);
                break;
            default:
                break;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
// GUIデモのアクション処理
// ═══════════════════════════════════════════════════════════════════

void processGUIAction(Screen& win) {
    // Cキー: オブジェクトをクリア
    if (getkey('C')) {
        clearGUIObjects();
        g_actionLog = "All objects cleared";
    }
    
    // Rキー: オブジェクトを再作成
    if (getkey('R')) {
        clearGUIObjects();
        g_actionLog = "Objects will be recreated";
    }
}
