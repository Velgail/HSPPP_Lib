// HspppTest/ApiCompileTest.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP API コンパイルテスト
// すべてのAPIが正常にコンパイルできることを確認する
// ═══════════════════════════════════════════════════════════════════

import hsppp;
using namespace hsppp;

// ============================================================
// コンパイルテスト: このファイルがコンパイルできれば、
// すべてのAPIのシグネチャが正しく定義されている
// ============================================================

namespace compile_test {

    // ============================================================
    // 型・定数のテスト
    // ============================================================
    void test_types_and_constants() {
        // OptInt
        [[maybe_unused]] OptInt opt1;
        [[maybe_unused]] OptInt opt2 = omit;
        [[maybe_unused]] OptInt opt3 = 42;
        [[maybe_unused]] OptInt opt4 = {};
        [[maybe_unused]] bool b1 = opt1.is_default();
        [[maybe_unused]] int v1 = opt1.value_or(0);

        // OptDouble
        [[maybe_unused]] OptDouble optd1;
        [[maybe_unused]] OptDouble optd2 = omit;
        [[maybe_unused]] OptDouble optd3 = 3.14;
        [[maybe_unused]] OptDouble optd4 = 42;  // int -> double
        [[maybe_unused]] bool b2 = optd1.is_default();
        [[maybe_unused]] double v2 = optd1.value_or(0.0);

        // Screen mode flags
        [[maybe_unused]] int m1 = screen_normal;
        [[maybe_unused]] int m2 = screen_palette;
        [[maybe_unused]] int m3 = screen_hide;
        [[maybe_unused]] int m4 = screen_fixedsize;
        [[maybe_unused]] int m5 = screen_tool;
        [[maybe_unused]] int m6 = screen_frame;
        [[maybe_unused]] int m7 = screen_offscreen;
        [[maybe_unused]] int m8 = screen_usergcopy;
        [[maybe_unused]] int m9 = screen_fullscreen;
    }

    // ============================================================
    // パラメータ構造体のテスト
    // ============================================================
    void test_param_structs() {
        // ScreenParams - designated initializers
        [[maybe_unused]] ScreenParams sp1;
        [[maybe_unused]] ScreenParams sp2 = {.width = 800};
        [[maybe_unused]] ScreenParams sp3 = {.width = 800, .height = 600};
        [[maybe_unused]] ScreenParams sp4 = {.width = 800, .height = 600, .mode = screen_hide};
        [[maybe_unused]] ScreenParams sp5 = {.title = "Test"};

        // BufferParams
        [[maybe_unused]] BufferParams bp1;
        [[maybe_unused]] BufferParams bp2 = {.width = 256, .height = 256};

        // BgscrParams
        [[maybe_unused]] BgscrParams bgp1;
        [[maybe_unused]] BgscrParams bgp2 = {.width = 320, .height = 240, .pos_x = 100, .pos_y = 100};
    }

    // ============================================================
    // Screen クラスのテスト（メソッドシグネチャ確認）
    // ============================================================
    void test_screen_class(Screen& scr) {
        // 有効性チェック
        [[maybe_unused]] bool v = scr.valid();
        [[maybe_unused]] bool b = static_cast<bool>(scr);
        [[maybe_unused]] int id = scr.id();

        // 描画設定（メソッドチェーン対応）
        scr.color(255, 255, 255);
        scr.pos(10, 10);
        scr.color(0, 0, 0).pos(20, 20);

        // 描画命令（メソッドチェーン対応）
        scr.mes("Hello");
        scr.boxf(0, 0, 100, 100);
        scr.boxf();
        scr.line(100, 100);
        scr.line(200, 200, 100, 100);
        scr.circle(0, 0, 100, 100);
        scr.circle(0, 0, 100, 100, 0);
        scr.pset(50, 50);
        scr.pset();
        scr.pget(50, 50);
        scr.pget();

        // 制御
        scr.redraw(0);
        scr.redraw(1);
        scr.redraw();
        scr.select();

        // プロパティ
        [[maybe_unused]] int w = scr.width();
        [[maybe_unused]] int h = scr.height();

        // フォント設定（メソッドチェーン対応）
        scr.font("MS Gothic", 12, 0);
        scr.font("Arial", 16, 1);  // 太字
        scr.font("MS Gothic", 14, 2);  // イタリック
        scr.font("MS Gothic", 18, 3);  // 太字+イタリック
        scr.sysfont(0);   // HSP標準
        scr.sysfont(17);  // デフォルトGUI

        // タイトル設定
        scr.title("Test Title");

        // ウィンドウサイズ設定
        scr.windowSize(100, 100);
        scr.windowSize(100, 100, 50, 50);
        scr.windowSize(-1, -1, 100, 100, 0);
        scr.windowSize(-1, -1, -100, -100, 1);  // マルチモニタ対応

        // メソッドチェーン
        scr.color(255, 0, 0)
           .pos(0, 0)
           .mes("Chain test")
           .boxf(10, 10, 50, 50)
           .line(100, 100)
           .circle(200, 200, 300, 300)
           .pset(400, 400);
    }

    // ============================================================
    // OOP版関数のテスト
    // ============================================================
    void test_oop_functions() {
        // screen (OOP)
        [[maybe_unused]] Screen s1 = screen();
        [[maybe_unused]] Screen s2 = screen(ScreenParams{});
        [[maybe_unused]] Screen s3 = screen({.width = 800, .height = 600});

        // buffer (OOP)
        [[maybe_unused]] Screen b1 = buffer();
        [[maybe_unused]] Screen b2 = buffer(BufferParams{});
        [[maybe_unused]] Screen b3 = buffer({.width = 256, .height = 256});

        // bgscr (OOP)
        [[maybe_unused]] Screen bg1 = bgscr();
        [[maybe_unused]] Screen bg2 = bgscr(BgscrParams{});
        [[maybe_unused]] Screen bg3 = bgscr({.width = 320, .height = 240});
    }

    // ============================================================
    // HSP互換版関数のテスト
    // ============================================================
    void test_hsp_compat_functions() {
        // screen (HSP互換)
        [[maybe_unused]] auto s1 = screen(0);
        [[maybe_unused]] auto s2 = screen(1, 800, 600);
        [[maybe_unused]] auto s3 = screen(2, omit, omit, screen_hide);
        [[maybe_unused]] auto s4 = screen(3, {}, {}, {}, 100, 100);
        [[maybe_unused]] auto s5 = screen(4, 640, 480, 0, -1, -1, 0, 0, "Title");

        // buffer (HSP互換)
        [[maybe_unused]] auto b1 = buffer(10);
        [[maybe_unused]] auto b2 = buffer(11, 256, 256);
        [[maybe_unused]] auto b3 = buffer(12, omit, omit, 0);

        // bgscr (HSP互換)
        [[maybe_unused]] auto bg1 = bgscr(20);
        [[maybe_unused]] auto bg2 = bgscr(21, 320, 240);
        [[maybe_unused]] auto bg3 = bgscr(22, 640, 480, 0, 100, 100);
    }

    // ============================================================
    // グローバル描画関数のテスト
    // ============================================================
    void test_global_drawing_functions() {
        // 基本描画設定
        color(255, 255, 255);
        pos(10, 10);

        // 描画命令
        mes("Test message");
        boxf(0, 0, 100, 100);
        boxf();

        // line
        line();
        line(100, 100);
        line(200, 200, 100, 100);
        line(omit, omit, 50, 50);

        // circle
        circle();
        circle(0, 0, 100, 100);
        circle(0, 0, 100, 100, 0);
        circle(0, 0, 100, 100, 1);
        circle(omit, omit, omit, omit, 0);

        // pset
        pset();
        pset(50, 50);
        pset(omit, omit);

        // pget
        pget();
        pget(50, 50);
        pget(omit, omit);
    }

    // ============================================================
    // ウィンドウ制御関数のテスト
    // ============================================================
    void test_window_control_functions() {
        // gsel
        gsel();
        gsel(0);
        gsel(0, 1);
        gsel(omit, 2);

        // gmode
        gmode();
        gmode(0);
        gmode(0, 32, 32);
        gmode(3, 64, 64, 128);
        gmode(omit, omit, omit, 256);

        // gcopy
        gcopy();
        gcopy(0);
        gcopy(0, 10, 10);
        gcopy(0, 0, 0, 100, 100);
        gcopy(omit, omit, omit, 64, 64);

        // gzoom
        gzoom();
        gzoom(200, 200);
        gzoom(200, 200, 0);
        gzoom(200, 200, 0, 0, 0);
        gzoom(200, 200, 0, 0, 0, 100, 100);
        gzoom(200, 200, 0, 0, 0, 100, 100, 1);
    }

    // ============================================================
    // 制御・情報取得関数のテスト
    // ============================================================
    void test_control_functions() {
        // redraw
        redraw();
        redraw(0);
        redraw(1);

        // await
        await(0);
        await(16);

        // ginfo
        [[maybe_unused]] int g0 = ginfo(0);   // マウスX
        [[maybe_unused]] int g1 = ginfo(1);   // マウスY
        [[maybe_unused]] int g2 = ginfo(2);   // アクティブウィンドウID
        [[maybe_unused]] int g3 = ginfo(3);   // 操作先ウィンドウID
        [[maybe_unused]] int g16 = ginfo(16); // R
        [[maybe_unused]] int g17 = ginfo(17); // G
        [[maybe_unused]] int g18 = ginfo(18); // B
        [[maybe_unused]] int g20 = ginfo(20); // デスクトップ幅
        [[maybe_unused]] int g21 = ginfo(21); // デスクトップ高さ
        [[maybe_unused]] int g22 = ginfo(22); // カレントX
        [[maybe_unused]] int g23 = ginfo(23); // カレントY
        [[maybe_unused]] int g26 = ginfo(26); // 画面幅
        [[maybe_unused]] int g27 = ginfo(27); // 画面高さ

        // ginfo helpers
        [[maybe_unused]] int r = ginfo_r();
        [[maybe_unused]] int g = ginfo_g();
        [[maybe_unused]] int b = ginfo_b();
    }

    // ============================================================
    // フォント・ウィンドウ制御関数のテスト
    // ============================================================
    void test_font_window_functions() {
        // font
        font("MS Gothic");
        font("MS Gothic", 12);
        font("MS Gothic", 12, 0);
        font("MS Gothic", 12, 1);  // 太字
        font("MS Gothic", 12, 2);  // イタリック
        font("MS Gothic", 12, 3);  // 太字+イタリック
        font("MS Gothic", 12, 16); // アンチエイリアス
        font("MS Gothic", 12, 0, 1); // decorationWidth
        font("Arial", omit, omit, omit);

        // sysfont
        sysfont();
        sysfont(0);   // HSP標準
        sysfont(10);  // OEM固定幅
        sysfont(11);  // Windows固定幅
        sysfont(12);  // Windows可変幅
        sysfont(13);  // 標準システム
        sysfont(17);  // デフォルトGUI
        sysfont(omit);

        // title
        title("Window Title");
        title("");

        // width
        width();
        width(100);
        width(100, 100);
        width(100, 100, 50, 50);
        width(100, 100, 50, 50, 0);
        width(100, 100, 50, 50, 1);  // マルチモニタ対応
        width(-1, -1, 100, 100);
        width(omit, omit, omit, omit, omit);
    }

    // ============================================================
    // 終了関数（呼び出さない、シグネチャ確認のみ）
    // ============================================================
    void test_end_function_signature() {
        // end() は [[noreturn]] なので実際には呼ばない
        // シグネチャの確認のみ
        [[maybe_unused]] auto end_ptr = &end;
        (void)end_ptr;  // 使用したことにする
    }

    // ============================================================
    // 入力関数のテスト
    // ============================================================
    void test_input_functions() {
        // stick
        [[maybe_unused]] int s1 = stick();
        [[maybe_unused]] int s2 = stick(0);
        [[maybe_unused]] int s3 = stick(15);  // 矢印キー非トリガー
        [[maybe_unused]] int s4 = stick(15, 0);  // アクティブチェックなし
        [[maybe_unused]] int s5 = stick(15, 1);  // アクティブチェックあり
        [[maybe_unused]] int s6 = stick(omit, omit);

        // getkey
        [[maybe_unused]] int k1 = getkey(32);   // スペース
        [[maybe_unused]] int k2 = getkey(13);   // Enter
        [[maybe_unused]] int k3 = getkey(27);   // ESC
        [[maybe_unused]] int k4 = getkey(37);   // ←
        [[maybe_unused]] int k5 = getkey(38);   // ↑
        [[maybe_unused]] int k6 = getkey(39);   // →
        [[maybe_unused]] int k7 = getkey(40);   // ↓
        [[maybe_unused]] int k8 = getkey(1);    // 左クリック
        [[maybe_unused]] int k9 = getkey(2);    // 右クリック

        // mouse
        mouse();
        mouse(100);
        mouse(100, 100);
        mouse(100, 100, 0);
        mouse(100, 100, -1);
        mouse(100, 100, 1);
        mouse(100, 100, 2);
        mouse(omit, omit, omit);

        // mousex, mousey, mousew
        [[maybe_unused]] int mx = mousex();
        [[maybe_unused]] int my = mousey();
        [[maybe_unused]] int mw = mousew();

        // wait
        // wait();  // デフォルト値で待機（時間がかかるためコメントアウト）
        // wait(1);  // 10ms
        // wait(omit);
    }

    // Screen クラスの入力関数テスト
    void test_screen_input_functions(Screen& scr) {
        [[maybe_unused]] int mx = scr.mousex();
        [[maybe_unused]] int my = scr.mousey();
    }

    // ============================================================
    // 割り込みハンドラのテスト
    // ============================================================
    
    // テスト用ハンドラ
    int test_interrupt_handler() { return 0; }
    
    void test_interrupt_functions() {
        // onclick
        hsppp::onclick(test_interrupt_handler);
        hsppp::onclick(nullptr);  // 解除
        hsppp::onclick(0);   // 一時停止
        hsppp::onclick(1);   // 再開
        
        // oncmd
        hsppp::oncmd(test_interrupt_handler, 0x0001);  // WM_CREATE
        hsppp::oncmd(nullptr, 0x0001);  // 解除
        hsppp::oncmd(0, 0x0001);   // 特定メッセージ停止
        hsppp::oncmd(1, 0x0001);   // 特定メッセージ再開
        hsppp::oncmd(0);   // 全体停止
        hsppp::oncmd(1);   // 全体再開
        
        // onerror
        hsppp::onerror(test_interrupt_handler);
        hsppp::onerror(nullptr);
        hsppp::onerror(0);
        hsppp::onerror(1);
        
        // onexit
        hsppp::onexit(test_interrupt_handler);
        hsppp::onexit(nullptr);
        hsppp::onexit(0);
        hsppp::onexit(1);
        
        // onkey
        hsppp::onkey(test_interrupt_handler);
        hsppp::onkey(nullptr);
        hsppp::onkey(0);
        hsppp::onkey(1);
        
        // システム変数
        [[maybe_unused]] int ip = hsppp::iparam();
        [[maybe_unused]] int wp = hsppp::wparam();
        [[maybe_unused]] int lp = hsppp::lparam();
        [[maybe_unused]] const InterruptParams& params = hsppp::getInterruptParams();
        (void)params;
    }

    // Screen クラスの割り込みハンドラテスト
    void test_screen_interrupt_functions(Screen& scr) {
        scr.onclick(test_interrupt_handler)
           .onkey(test_interrupt_handler)
           .oncmd(test_interrupt_handler, 0x0001);
    }

}  // namespace compile_test

// ============================================================
// テスト実行関数
// ============================================================
namespace hsppp_test {

    /// @brief すべてのコンパイルテストを実行
    /// @return テストが成功したら true
    bool run_compile_tests() {
        // コンパイルが通った時点で成功
        // 実行時はクラッシュしないことを確認

        compile_test::test_types_and_constants();
        compile_test::test_param_structs();

        // 実際にウィンドウを作成してテスト
        auto testScreen = screen({.width = 100, .height = 100, .mode = screen_hide});
        if (testScreen.valid()) {
            compile_test::test_screen_class(testScreen);
            compile_test::test_screen_input_functions(testScreen);
            compile_test::test_screen_interrupt_functions(testScreen);
        }

        // 残りのテスト（ウィンドウ作成を伴うもの）
        // compile_test::test_oop_functions();      // 多数のウィンドウを作成
        // compile_test::test_hsp_compat_functions(); // 多数のウィンドウを作成

        compile_test::test_global_drawing_functions();
        compile_test::test_window_control_functions();
        compile_test::test_control_functions();
        compile_test::test_font_window_functions();
        compile_test::test_input_functions();
        compile_test::test_interrupt_functions();
        // compile_test::test_end_function_signature(); // end()は呼ばない

        return true;
    }

}  // namespace hsppp_test
