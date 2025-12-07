// HspppSample/UserApp.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモ: 割り込みハンドラ＆エラーハンドリングテスト
// ═══════════════════════════════════════════════════════════════════

import hsppp;
using namespace hsppp;

// グローバル変数（割り込みハンドラからアクセス）
int g_clickCount = 0;
int g_lastKeyCode = 0;
bool g_confirmExit = false;
bool g_errorOccurred = false;
int g_errorCode = 0;
int g_errorLine = 0;

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

// onerror ハンドラ: エラー発生時に呼び出される
// HspErrorオブジェクトからエラー情報を取得
int onErrorHandler(const HspError& error) {
    g_errorOccurred = true;
    g_errorCode = error.error_code();    // HspErrorから取得
    g_errorLine = error.line_number();   // HspErrorから取得

    // ここで独自のエラー処理を記述できます
    // 例: 画面にエラー情報を描画
    color(255, 0, 0);
    pos(10, 10);
    mes("Error occurred!");
    pos(10, 30);
    mes("Error code: " + str(error.error_code()));
    pos(10, 50);
    mes("Line: " + str(error.line_number()));

    // 画面を更新して表示
    redraw(1);

    // ユーザーがウィンドウを閉じるまで待機
    // （実際にはstop()を呼べば良いが、今は簡略化のため省略）

    // この関数が終了すると自動的に end() が呼ばれる（HSP仕様）
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
    hsppp::onerror(onErrorHandler);   // エラー割り込み
    hsppp::onerror(0);

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
        mw.mes("Press 'E' to trigger error (tests onerror handler)");
        mw.pos(20, 120);
        mw.mes("Press ESC or click X to exit (with confirmation)");

        // mes オプションデモ
        mw.color(100, 100, 100).pos(20, 160);
        mw.mes("Text without newline:", 1);  // mesopt_nocr: 改行しない
        mw.mes("continued!");

        mw.color(0, 100, 200).pos(20, 190);
        mw.mes("Shadow text:", 2);  // mesopt_shadow: 影付き

        mw.color(200, 100, 0).pos(20, 220);
        mw.mes("Outline text:", 4);  // mesopt_outline: 縁取り
        
        // クリックカウント表示
        mw.color(0, 0, 200).pos(20, 150);
        mw.mes("Click count:");
        mw.pos(150, 150);
        mw.mes(str(g_clickCount));

        // 最後に押されたキー表示
        mw.color(0, 128, 0).pos(20, 180);
        mw.mes("Last key code:");
        mw.pos(150, 180);
        mw.mes(str(g_lastKeyCode));
        
        if (g_lastKeyCode > 0) {
            mw.pos(200, 180);
            if (g_lastKeyCode >= 32 && g_lastKeyCode < 127) {
                std::string charDisplay = "(" + std::string(1, static_cast<char>(g_lastKeyCode)) + ")";
                mw.mes(charDisplay);
            }
        }

        // システム変数表示
        mw.color(128, 0, 128).pos(20, 220);
        mw.mes("iparam/wparam/lparam:");
        mw.pos(20, 240);
        std::string sysVars = "ip=" + str(iparam()) + " wp=" + str(wparam()) + " lp=" + str(lparam());
        mw.mes(sysVars);
        
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

        // Eキーでエラーを発生（自動的にonerrorハンドラが呼ばれる）
        if (getkey('E')) {
            // 範囲外の値でエラーを発生させる
            color(999, -50, 300);  // ERR_OUT_OF_RANGE が発生 → onerrorハンドラへ
        }

        mw.redraw(1);  // 描画終了
        await(16);     // 約60fps
    }
    
    return 0;
}
