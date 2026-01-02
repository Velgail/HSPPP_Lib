// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

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
    
    case InterruptDemo::OnCmd:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("oncmd: Windowsメッセージ割り込み");
        
        win.color(0, 0, 128).pos(50, 130);
        win.mes("=== Windowsメッセージ割り込み状態 ===");
        
        win.color(0, 0, 0).pos(50, 160);
        win.mes("受信メッセージ数: " + str(g_cmdMessageCount));
        win.pos(50, 180);
        win.mes(std::format("最後のメッセージ: 0x{:04X}", g_lastCmdMessage));
        
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 0).pos(50, 220);
        win.mes("oncmd 使用方法:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 245);
        win.mes("oncmd(handler, WM_PAINT) - WM_PAINTを監視");
        win.pos(50, 265);
        win.mes("oncmd(0, WM_PAINT) - 監視停止");
        win.pos(50, 285);
        win.mes("oncmd(1, WM_PAINT) - 監視再開");
        
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 0).pos(50, 320);
        win.mes("主要なWindowsメッセージ:");
        win.font("MS Gothic", 10, 0);
        win.color(0, 0, 0).pos(50, 345);
        win.mes("WM_PAINT=0x000F, WM_TIMER=0x0113, WM_SIZE=0x0005");
        win.pos(50, 360);
        win.mes("WM_MOVE=0x0003, WM_ACTIVATE=0x0006");
        break;
    
    case InterruptDemo::OnError:
        win.color(0, 0, 0).pos(20, 85);
        win.mes("onerror: 致命的エラー時の処理 - hspMainの外側で最終処理を行う");
        
        win.font("MS Gothic", 12, 1);
        win.color(255, 0, 0).pos(50, 120);
        win.mes("【重要】onerrorの動作原理:");
        win.font("MS Gothic", 11, 0);
        win.color(0, 0, 0).pos(50, 145);
        win.mes("1. HspErrorが発生すると、hspMainの外側でキャッチされる");
        win.pos(50, 163);
        win.mes("2. onerrorハンドラが設定されていれば、それが呼ばれる");
        win.pos(50, 181);
        win.mes("3. ハンドラ実行後、自動的にend(1)でプログラム終了");
        win.pos(50, 199);
        win.mes("4. ハンドラがなければ、エラーダイアログ表示後に終了");
        
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(50, 235);
        win.mes("アクション:");
        win.font("MS Gothic", 12, 0);
        win.color(0, 0, 0).pos(50, 260);
        win.mes("T: テストエラーを発生させる（プログラムは終了します）");
        
        win.font("MS Gothic", 12, 1);
        win.color(0, 128, 0).pos(350, 120);
        win.mes("onerror 使用例:");
        win.font("MS Gothic", 10, 0);
        win.color(0, 0, 0).pos(350, 145);
        win.mes("// hspMain() の前に設定");
        win.pos(350, 162);
        win.mes("onerror([](const HspError& e) {");
        win.pos(350, 179);
        win.mes("  // エラーログをファイルに保存");
        win.pos(350, 196);
        win.mes("  // クリーンアップ処理");
        win.pos(350, 213);
        win.mes("  return 0;");
        win.pos(350, 230);
        win.mes("});");
        
        win.font("MS Gothic", 12, 1);
        win.color(128, 0, 0).pos(350, 260);
        win.mes("エラーコード定数:");
        win.font("MS Gothic", 10, 0);
        win.color(0, 0, 0).pos(350, 285);
        win.mes("ERR_OUT_OF_RANGE=7  (範囲外)");
        win.pos(350, 300);
        win.mes("ERR_FILE_IO=12  (ファイルI/O)");
        win.pos(350, 315);
        win.mes("ERR_INVALID_HANDLE=14  (無効ハンドル)");
        
        win.font("MS Gothic", 12, 1);
        win.color(0, 0, 128).pos(50, 320);
        win.mes("HspError プロパティ:");
        win.font("MS Gothic", 10, 0);
        win.color(0, 0, 0).pos(50, 345);
        win.mes("e.error_code() - エラーコード");
        win.pos(50, 360);
        win.mes("e.message() - エラーメッセージ");
        win.pos(50, 375);
        win.mes("e.file_name() - 発生ファイル名");
        win.pos(50, 390);
        win.mes("e.line_number() - 発生行番号");
        win.pos(50, 405);
        win.mes("e.function_name() - 発生関数名");
        break;
        
    default:
        break;
    }
}

// ═══════════════════════════════════════════════════════════════════
// 割り込みデモのアクション処理
// ═══════════════════════════════════════════════════════════════════

void processInterruptAction(Screen& win) {
    // 修飾キーが押されている場合はアクション無効
    if (isModifierKeyPressed()) return;
    
    switch (static_cast<InterruptDemo>(g_demoIndex)) {
    case InterruptDemo::OnError:
        if (getkey('T')) {
            // 致命的エラーを発生させる
            // これはhspMainの外側でキャッチされ、onerrorハンドラ（設定されていれば）が呼ばれ、
            // その後プログラムは終了する
            color(300, 0, 0);  // 範囲外の値→HspErrorがスロー
            // ↑ここから先は実行されない（例外がスローされるため）
        }
        break;
        
    default:
        break;
    }
}
