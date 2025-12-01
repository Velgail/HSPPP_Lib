// HspppSample/UserApp.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモ: 割り込みハンドラテスト（onclick, onkey, onexit）
// ═══════════════════════════════════════════════════════════════════

import hsppp;
using namespace hsppp;

// グローバル変数（割り込みハンドラからアクセス）
int g_clickCount = 0;
int g_lastKeyCode = 0;
bool g_confirmExit = false;

// onclick ハンドラ: マウスクリック時に呼び出される
int onClickHandler() {
    g_clickCount++;
    return 0;  // 戻り値（現在は無視）
}

// onkey ハンドラ: キー入力時に呼び出される
int onKeyHandler() {
    g_lastKeyCode = iparam();  // 押されたキーコード
    return 0;
}

// onexit ハンドラ: ウィンドウを閉じようとした時に呼び出される
int onExitHandler() {
    g_confirmExit = true;
    return 0;
}

// ユーザーのエントリーポイント
int hspMain() {
    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  メインウィンドウ作成                                         ║
    // ╚═══════════════════════════════════════════════════════════════╝
    auto mw = screen({.width = 640, .height = 480, .title = "HSPPP Interrupt Demo"});
    
    // ╔═══════════════════════════════════════════════════════════════╗
    // ║  割り込みハンドラを登録                                       ║
    // ╚═══════════════════════════════════════════════════════════════╝
    hsppp::onclick(onClickHandler);   // クリック割り込み
    hsppp::onkey(onKeyHandler);       // キー割り込み
    hsppp::onexit(onExitHandler);     // 終了割り込み

    // メインループ
    while (true) {
        mw.redraw(0);  // 描画開始
        
        // 背景クリア
        mw.color(240, 240, 240).boxf();
        
        // タイトル
        mw.sysfont(0);
        mw.color(0, 0, 0).pos(20, 20);
        mw.mes("=== HSPPP Interrupt Handler Demo ===");
        
        // 説明
        mw.pos(20, 60);
        mw.mes("Click anywhere to increment counter");
        mw.pos(20, 80);
        mw.mes("Press any key to display keycode");
        mw.pos(20, 100);
        mw.mes("Press ESC or click X to exit (with confirmation)");
        
        // クリックカウント表示
        mw.color(0, 0, 200).pos(20, 150);
        mw.mes("Click count:");
        mw.pos(150, 150);
        // 数値表示のための簡易的な方法
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", g_clickCount);
        mw.mes(buf);
        
        // 最後に押されたキー表示
        mw.color(0, 128, 0).pos(20, 180);
        mw.mes("Last key code:");
        mw.pos(150, 180);
        snprintf(buf, sizeof(buf), "%d", g_lastKeyCode);
        mw.mes(buf);
        
        if (g_lastKeyCode > 0) {
            mw.pos(200, 180);
            char charBuf[8] = {};
            if (g_lastKeyCode >= 32 && g_lastKeyCode < 127) {
                charBuf[0] = '(';
                charBuf[1] = static_cast<char>(g_lastKeyCode);
                charBuf[2] = ')';
                mw.mes(charBuf);
            }
        }
        
        // システム変数表示
        mw.color(128, 0, 128).pos(20, 220);
        mw.mes("iparam/wparam/lparam:");
        mw.pos(20, 240);
        snprintf(buf, sizeof(buf), "ip=%d wp=%d lp=%d", iparam(), wparam(), lparam());
        mw.mes(buf);
        
        // 終了確認ダイアログ（簡易実装）
        if (g_confirmExit) {
            // 確認メッセージ表示
            mw.color(255, 200, 200).boxf(150, 200, 490, 280);
            mw.color(0, 0, 0).pos(180, 220);
            mw.mes("Exit confirmation");
            mw.pos(180, 240);
            mw.mes("Press Y to exit, N to cancel");
            
            // Y/N入力チェック
            if (getkey('Y')) {
                end(0);  // 終了
            }
            if (getkey('N')) {
                g_confirmExit = false;  // キャンセル
            }
        }
        
        // ESCキーで終了確認
        if (stick() & 128) {
            g_confirmExit = true;
        }
        
        mw.redraw(1);  // 描画終了
        await(16);     // 約60fps
    }
    
    return 0;
}
