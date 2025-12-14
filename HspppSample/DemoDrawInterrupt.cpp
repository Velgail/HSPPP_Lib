// HspppSample/DemoDrawInterrupt.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 割り込みデモ描画
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
import <format>;
using namespace hsppp;

void drawInterruptDemo(Screen& win) {
    switch (static_cast<InterruptDemo>(g_demoIndex)) {
    case InterruptDemo::OnClick:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("onclick: マウスクリック時の割り込み");
        
        win.color(0, 0, 128).pos(50, 130);
        win.mes("=== クリック割り込み状態 ===");
        
        win.color(0, 0, 0).pos(50, 160);
        win.mes("クリック回数: " + str(g_clickCount));
        win.pos(50, 180);
        win.mes("画面内でクリックしてカウンターを増やしてください");
        
        // クリック検出エリアの表示
        win.color(200, 200, 255);
        win.boxf(50, 220, 590, 400);
        win.color(0, 0, 128).pos(250, 300);
        win.mes("Click Area");
        break;
        
    case InterruptDemo::OnKey:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("onkey: キー入力時の割り込み");
        
        win.color(0, 0, 128).pos(50, 130);
        win.mes("=== キー割り込み状態 ===");
        
        win.color(0, 0, 0).pos(50, 160);
        win.mes("キー入力回数: " + str(g_keyCount));
        win.pos(50, 180);
        win.mes(std::format("最後のキーコード: {} (0x{:x})", g_lastKey, g_lastKey));
        
        win.color(0, 128, 0).pos(50, 220);
        win.mes("任意のキーを押してください（モード切替キー以外）");
        break;
        
    case InterruptDemo::OnExit:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("onexit: 終了ボタン押下時の割り込み");
        
        win.color(0, 0, 128).pos(50, 130);
        win.mes("=== 終了割り込み設定中 ===");
        
        win.color(0, 0, 0).pos(50, 160);
        win.mes("ウィンドウの×ボタンを押しても、end()を呼ぶまで終了しません");
        win.pos(50, 180);
        win.mes("2回連続で×ボタンを押すと終了します");
        
        win.color(128, 0, 0).pos(50, 220);
        win.mes("=== 割り込みパラメータ (システム変数) ===");
        win.color(0, 0, 0).pos(50, 250);
        win.mes("iparam() = " + str(iparam()));
        win.pos(50, 270);
        win.mes("wparam() = " + str(wparam()));
        win.pos(50, 290);
        win.mes("lparam() = " + str(lparam()));
        break;
        
    default:
        break;
    }
}
