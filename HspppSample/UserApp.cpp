// HspppSample/UserApp.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモ: OOP版 screen/buffer/bgscr のテスト
// ═══════════════════════════════════════════════════════════════════

import hsppp;
using namespace hsppp;

// ユーザーのエントリーポイント
int hspMain() {

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  OOP版 screen - ID自動採番でウィンドウ作成                   ║
    // ╚═══════════════════════════════════════════════════════════════╝

    // OOP版: 構造体パラメータでウィンドウ作成（IDは自動採番）
    auto mainWin = screen({.width = 640, .height = 480, .title = "OOP Test - Main Window"});

    // メソッドチェーンで描画
    mainWin.color(200, 200, 200).boxf();
    mainWin.color(0, 0, 0).pos(20, 20);
    mainWin.mes("=== OOP Style API Test ===");
    mainWin.pos(20, 50);
    mainWin.mes("Main window created with auto ID");

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  OOP版 buffer - ID自動採番で仮想画面作成                     ║
    // ╚═══════════════════════════════════════════════════════════════╝

    // OOP版: 仮想画面を作成（IDは自動採番）
    auto offscreen = buffer({.width = 200, .height = 200});

    // 仮想画面に描画
    offscreen.redraw(0);
    offscreen.color(255, 0, 0).boxf();              // 赤で全体塗りつぶし
    offscreen.color(0, 255, 0).boxf(50, 50, 150, 150);  // 緑の矩形
    offscreen.color(0, 0, 255).boxf(75, 75, 125, 125);  // 青の矩形
    offscreen.redraw(1);

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  OOP版 Screen::select() で描画先切り替え                     ║
    // ╚═══════════════════════════════════════════════════════════════╝

    // メインウィンドウに戻って描画
    mainWin.select();  // gsel(mainId) と同等
    
    mainWin.pos(20, 80);
    mainWin.mes("Buffer created with auto ID");

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  gcopy/gzoom - Screen::id()を使ってバッファからコピー        ║
    // ╚═══════════════════════════════════════════════════════════════╝

    mainWin.pos(20, 120);
    mainWin.mes("gcopy from buffer:");
    pos(20, 140);
    gcopy(offscreen.id(), 0, 0, 100, 100);  // Screen::id()でIDを取得してコピー

    mainWin.pos(150, 120);
    mainWin.mes("gzoom (2x):");
    pos(150, 140);
    gzoom(200, 200, offscreen.id(), 0, 0, 100, 100, 0);  // 拡大コピー

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  OOP版 bgscr - ID自動採番で枠なしウィンドウ作成              ║
    // ╚═══════════════════════════════════════════════════════════════╝

    // OOP版: 枠なしウィンドウを作成（IDは自動採番）
    auto popup = bgscr({.width = 200, .height = 150, .pos_x = 500, .pos_y = 400});

    popup.color(50, 50, 100).boxf();
    popup.color(255, 255, 255).pos(20, 20);
    popup.mes("Borderless (OOP)");
    popup.pos(20, 50);
    popup.mes("Auto ID assigned");
    popup.pos(20, 80);
    popup.mes("No frame!");

    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  最終確認 - メインウィンドウに結果表示                       ║
    // ╚═══════════════════════════════════════════════════════════════╝

    mainWin.select();
    mainWin.color(0, 128, 0).pos(20, 360);
    mainWin.mes("=== Test Complete! ===");
    mainWin.mes("OOP API working: screen(), buffer(), bgscr()");
    mainWin.mes("gcopy/gzoom work with Screen::id()");

    // ═══════════════════════════════════════════════════════════════
    // 自動終了（デバッグ用）
    // ═══════════════════════════════════════════════════════════════
    
    await(5000);  // 5秒待機
    end(0);       // 正常終了

    return 0;
}
