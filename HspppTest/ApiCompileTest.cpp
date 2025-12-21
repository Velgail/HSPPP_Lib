// HspppTest/ApiCompileTest.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP API コンパイルテスト
// すべてのAPIが正常にコンパイルできることを確認する
// ═══════════════════════════════════════════════════════════════════

import hsppp;
#include <vector>    // テスト用
#include <cctype>    // std::toupper
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

        // cls
        scr.cls();
        scr.cls(0);
        scr.cls(1);
        scr.cls(2);
        scr.cls(3);
        scr.cls(4);

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

        // 拡張描画命令（OOP版）
        scr.gradf(0, 0, 100, 100, 0, 0xFF0000, 0x0000FF);
        scr.grect(100, 100, 0.5, 50, 30);

        // 画像操作
        scr.picload("test.bmp");
        scr.picload("test.png", 0);
        scr.picload("test.jpg", 1);
        scr.picload("test.bmp", 2);
        scr.bmpsave("output.bmp");

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
        scr.width(100, 100);
        scr.width(100, 100, 50, 50);
        scr.width(-1, -1, 100, 100, 0);
        scr.width(-1, -1, -100, -100, 1);  // マルチモニタ対応

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
        // cls
        cls();
        cls(0);
        cls(1);
        cls(2);
        cls(3);
        cls(4);
        cls(omit);

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

        // gradf
        gradf();
        gradf(0, 0);
        gradf(0, 0, 100, 100);
        gradf(0, 0, 100, 100, 0);
        gradf(0, 0, 100, 100, 1, 0xFF0000, 0x0000FF);
        gradf(omit, omit, omit, omit, omit, omit, omit);

        // grect
        grect();
        grect(100, 100);
        grect(100, 100, 0.5);
        grect(100, 100, 0.5, 50, 30);
        grect(omit, omit, omit, omit, omit);

        // grotate
        grotate();
        grotate(0);
        grotate(0, 0, 0);
        grotate(0, 0, 0, 0.0);
        grotate(0, 0, 0, 0.0, 64, 64);
        grotate(omit, omit, omit, omit, omit, omit);

        // gsquare - 構造体版API
        Quad dst = {{0, 0}, {100, 0}, {100, 100}, {0, 100}};
        QuadUV src = {{0, 0}, {32, 0}, {32, 32}, {0, 32}};
        QuadColors colors = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
        gsquare(-1, dst);                      // 単色塗りつぶし
        gsquare(0, dst, src);                  // 画像コピー
        gsquare(gsquare_grad, dst, colors);    // グラデーション

        // print (mes互換)
        print("Test message");
        print("Test", 1);  // 改行なし
        print("Shadow", 2);  // 影付き

        // gettime
        [[maybe_unused]] int year = gettime(0);
        [[maybe_unused]] int month = gettime(1);
        [[maybe_unused]] int dayOfWeek = gettime(2);
        [[maybe_unused]] int day = gettime(3);
        [[maybe_unused]] int hour = gettime(4);
        [[maybe_unused]] int minute = gettime(5);
        [[maybe_unused]] int second = gettime(6);
        [[maybe_unused]] int millisec = gettime(7);
    }

    // ============================================================
    // 画像操作関数のテスト
    // ============================================================
    void test_image_functions() {
        // picload - HSP互換
        picload("test.bmp");
        picload("test.png", 0);
        picload("test.jpg", 1);
        picload("test.bmp", 2);
        picload("test.bmp", {});
        picload("test.bmp", omit);

        // bmpsave - HSP互換
        bmpsave("output.bmp");

        // celload - HSP互換
        [[maybe_unused]] int celId1 = celload("sprite.png");
        [[maybe_unused]] int celId2 = celload("sprite.png", 1);
        [[maybe_unused]] int celId3 = celload("sprite.png", {});
        [[maybe_unused]] int celId4 = celload("sprite.png", omit);

        // celdiv - HSP互換
        celdiv(1, 8, 8);
        celdiv(celId1, 4, 4);

        // celput - HSP互換
        celput(1, 0);
        celput(1, 0, 100, 100);
        celput(1, 0, {}, {});
        celput(1, 0, omit, omit);
        celput(celId1, 5);
        celput(celId1, 5, 200);
        celput(celId1, 5, 200, 150);
    }

    // ============================================================
    // Celクラスのテスト
    // ============================================================
    void test_cel_class() {
        // loadCel - OOP版ファクトリー
        [[maybe_unused]] Cel cel1 = loadCel("sprite.png");
        [[maybe_unused]] Cel cel2 = loadCel("sprite.png", 10);
        [[maybe_unused]] Cel cel3 = loadCel("sprite.png", {});
        [[maybe_unused]] Cel cel4 = loadCel("sprite.png", omit);

        // 有効性チェック
        [[maybe_unused]] bool valid = cel1.valid();
        [[maybe_unused]] bool b = static_cast<bool>(cel1);
        [[maybe_unused]] int id = cel1.id();

        // サイズ取得
        [[maybe_unused]] int w = cel1.width();
        [[maybe_unused]] int h = cel1.height();

        // メソッドチェーン
        cel1.divide(8, 8);
        cel1.put(0);
        cel1.put(0, 100, 100);
        cel1.put(0, {}, {});
        cel1.put(0, omit, omit);
        cel1.divide(4, 4).put(5, 200, 150);

        // コピー・ムーブ
        [[maybe_unused]] Cel cel5 = cel1;
        [[maybe_unused]] Cel cel6 = std::move(cel2);
        cel3 = cel4;
        cel4 = std::move(cel5);
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
    int test_error_handler(const HspError& error) {
        [[maybe_unused]] int code = error.error_code();
        [[maybe_unused]] int line = error.line_number();
        return 0;
    }
    
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
        
        // onerror (ErrorHandler型 - HspError引数を受け取る)
        hsppp::onerror(test_error_handler);
        hsppp::onerror([](const HspError& e) { return 0; });  // ラムダ式
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

    // ============================================================
    // 数学関数のテスト
    // ============================================================
    void test_math_functions() {
        // abs - std::abs で int/double 両対応
        [[maybe_unused]] int a1 = hsppp::abs(-10);
        [[maybe_unused]] int a2 = hsppp::abs(10);
        [[maybe_unused]] double a3 = hsppp::abs(-3.14);
        [[maybe_unused]] double a4 = hsppp::abs(3.14);

        // 三角関数 - ラジアン単位
        [[maybe_unused]] double s1 = hsppp::sin(0.0);
        [[maybe_unused]] double s2 = hsppp::sin(M_PI / 2);
        [[maybe_unused]] double c1 = hsppp::cos(0.0);
        [[maybe_unused]] double c2 = hsppp::cos(M_PI);
        [[maybe_unused]] double t1 = hsppp::tan(0.0);
        [[maybe_unused]] double t2 = hsppp::tan(M_PI / 4);

        // 度数法対応: deg2rad() で変換
        [[maybe_unused]] double s3 = hsppp::sin(hsppp::deg2rad(45.0));   // 45度
        [[maybe_unused]] double c3 = hsppp::cos(hsppp::deg2rad(90.0));   // 90度

        // アークタンジェント - std::atan2
        [[maybe_unused]] double at1 = hsppp::atan2(1.0, 1.0);
        [[maybe_unused]] double at2 = hsppp::atan2(0.0, 1.0);
        [[maybe_unused]] double at3 = hsppp::atan2(1.0, 0.0);

        // sqrt / pow / exp / log - C++標準関数
        [[maybe_unused]] double sq1 = hsppp::sqrt(4.0);
        [[maybe_unused]] double sq2 = hsppp::sqrt(2.0);
        [[maybe_unused]] double pw1 = hsppp::pow(2.0, 3.0);   // 2^3 = 8
        [[maybe_unused]] double pw2 = hsppp::pow(10.0, 2.0);  // 10^2 = 100
        [[maybe_unused]] double ex1 = hsppp::exp(1.0);        // e^1
        [[maybe_unused]] double ex2 = hsppp::exp(0.0);        // e^0 = 1
        [[maybe_unused]] double lg1 = hsppp::log(M_E);        // log(e) = 1
        [[maybe_unused]] double lg2 = hsppp::log(1.0);        // log(1) = 0

        // 乱数
        [[maybe_unused]] int r1 = hsppp::rnd(100);
        [[maybe_unused]] int r2 = hsppp::rnd(10);
        hsppp::randomize();                // 時刻ベース
        hsppp::randomize(12345);           // 指定シード
        hsppp::randomize(omit);            // 省略
        [[maybe_unused]] int r3 = hsppp::rnd(1000);

        // limit / limitf
        [[maybe_unused]] int lm1 = hsppp::limit(50, 0, 100);       // 50
        [[maybe_unused]] int lm2 = hsppp::limit(-10, 0, 100);      // 0
        [[maybe_unused]] int lm3 = hsppp::limit(200, 0, 100);      // 100
        [[maybe_unused]] int lm4 = hsppp::limit(50);               // 両方省略
        [[maybe_unused]] int lm5 = hsppp::limit(50, 10);           // 最大省略
        [[maybe_unused]] int lm6 = hsppp::limit(50, omit, 40);     // 最小省略
        [[maybe_unused]] double lf1 = hsppp::limitf(0.5, 0.0, 1.0);
        [[maybe_unused]] double lf2 = hsppp::limitf(-0.5, 0.0, 1.0);
        [[maybe_unused]] double lf3 = hsppp::limitf(1.5, 0.0, 1.0);
        [[maybe_unused]] double lf4 = hsppp::limitf(0.5);
        [[maybe_unused]] double lf5 = hsppp::limitf(0.5, 0.0);
        [[maybe_unused]] double lf6 = hsppp::limitf(0.5, omit, 0.3);
    }

    // ============================================================
    // 型変換関数のテスト
    // ============================================================
    void test_conversion_functions() {
        // toInt
        [[maybe_unused]] int i1 = hsppp::toInt(3.14);      // 3
        [[maybe_unused]] int i2 = hsppp::toInt(3.99);      // 3（切り捨て）
        [[maybe_unused]] int i3 = hsppp::toInt(-2.5);      // -2
        [[maybe_unused]] int i4 = hsppp::toInt(std::string("123"));
        [[maybe_unused]] int i5 = hsppp::toInt(std::string("abc"));  // 0

        // toDouble
        [[maybe_unused]] double d1 = hsppp::toDouble(42);
        [[maybe_unused]] double d2 = hsppp::toDouble(-10);
        [[maybe_unused]] double d3 = hsppp::toDouble(std::string("3.14"));
        [[maybe_unused]] double d4 = hsppp::toDouble(std::string("abc"));  // 0.0

        // str (int版は既存、double版を追加テスト)
        [[maybe_unused]] std::string s1 = hsppp::str(123);       // int版
        [[maybe_unused]] std::string s2 = hsppp::str(3.14);      // double版
        [[maybe_unused]] std::string s3 = hsppp::str(-42);
        [[maybe_unused]] std::string s4 = hsppp::str(0.0);

        // strlen
        [[maybe_unused]] int64_t len1 = hsppp::strlen("Hello");
        [[maybe_unused]] int64_t len2 = hsppp::strlen("");
        [[maybe_unused]] int64_t len3 = hsppp::strlen(std::string("日本語"));  // マルチバイト
    }

    // ============================================================
    // 色関連関数のテスト
    // ============================================================
    void test_color_functions() {
        // hsvcolor
        hsppp::hsvcolor(0, 255, 255);      // 赤
        hsppp::hsvcolor(64, 255, 255);     // 緑
        hsppp::hsvcolor(128, 255, 255);    // 青
        hsppp::hsvcolor(0, 0, 255);        // 白（彩度0）
        hsppp::hsvcolor(0, 255, 0);        // 黒（明度0）

        // rgbcolor
        hsppp::rgbcolor(0xFF0000);         // 赤
        hsppp::rgbcolor(0x00FF00);         // 緑
        hsppp::rgbcolor(0x0000FF);         // 青
        hsppp::rgbcolor(0xFFFFFF);         // 白
        hsppp::rgbcolor(0x000000);         // 黒
        hsppp::rgbcolor(0x808080);         // グレー

        // syscolor
        hsppp::syscolor(0);   // スクロールバー
        hsppp::syscolor(1);   // デスクトップ
        hsppp::syscolor(5);   // ウィンドウ背景
        hsppp::syscolor(8);   // ウィンドウテキスト
        hsppp::syscolor(15);  // 3D表面
    }

    // ============================================================
    // 文字列操作関数のテスト
    // ============================================================
    void test_string_functions() {
        // instr - 文字列の検索
        [[maybe_unused]] int64_t pos1 = hsppp::instr("ABCDEF", "CD");          // 2
        [[maybe_unused]] int64_t pos2 = hsppp::instr("ABCDEF", 2, "CD");       // 0 (2を起点)
        [[maybe_unused]] int64_t pos3 = hsppp::instr("ABCDEF", "XY");          // -1
        [[maybe_unused]] int64_t pos4 = hsppp::instr("ABCDEF", -1, "CD");      // -1 (負のインデックス)
        [[maybe_unused]] int64_t pos5 = hsppp::instr("", "ABC");               // -1
        [[maybe_unused]] int64_t pos6 = hsppp::instr("ABCABC", 3, "ABC");      // 0 (3を起点)
        [[maybe_unused]] int64_t pos7 = hsppp::instr("Hello World", 0, "World"); // 6
        [[maybe_unused]] int64_t pos8 = hsppp::instr("ABC", "");               // 0 (空文字検索)

        // strmid - 文字列の一部を取り出す
        [[maybe_unused]] std::string mid1 = hsppp::strmid("ABCDEF", 1, 3);   // "BCD"
        [[maybe_unused]] std::string mid2 = hsppp::strmid("ABCDEF", -1, 3);  // "DEF" (右から3文字)
        [[maybe_unused]] std::string mid3 = hsppp::strmid("ABCDEF", 0, 100); // "ABCDEF" (超過は実際の長さまで)
        [[maybe_unused]] std::string mid4 = hsppp::strmid("ABCDEF", 10, 3);  // "" (範囲外)
        [[maybe_unused]] std::string mid5 = hsppp::strmid("", 0, 3);         // ""
        [[maybe_unused]] std::string mid6 = hsppp::strmid("ABCDEF", 0, 0);   // ""
        [[maybe_unused]] std::string mid7 = hsppp::strmid("AB", -1, 5);      // "AB" (右から5文字、実際は2文字)

        // strtrim - 指定した文字だけを取り除く
        [[maybe_unused]] std::string trim1 = hsppp::strtrim("  ABC  ", 0, ' ');    // "ABC" (両端)
        [[maybe_unused]] std::string trim2 = hsppp::strtrim("  ABC  ", 1, ' ');    // "ABC  " (左端)
        [[maybe_unused]] std::string trim3 = hsppp::strtrim("  ABC  ", 2, ' ');    // "  ABC" (右端)
        [[maybe_unused]] std::string trim4 = hsppp::strtrim(" A B C ", 3, ' ');    // "ABC" (全て)
        [[maybe_unused]] std::string trim5 = hsppp::strtrim("ABC");                // "ABC" (デフォルト: 両端のスペース)
        [[maybe_unused]] std::string trim6 = hsppp::strtrim("XXABCXX", 0, 'X');    // "ABC"
        [[maybe_unused]] std::string trim7 = hsppp::strtrim("", 0, ' ');           // ""

        // strf - 書式付き文字列を変換
        [[maybe_unused]] std::string fmt1 = hsppp::strf("Hello");                      // "Hello"
        [[maybe_unused]] std::string fmt2 = hsppp::strf("Value: %d", 123);             // "Value: 123"
        [[maybe_unused]] std::string fmt3 = hsppp::strf("Hex: %x", 255);               // "Hex: ff"
        [[maybe_unused]] std::string fmt4 = hsppp::strf("Float: %f", 3.14);            // "Float: 3.140000"
        [[maybe_unused]] std::string fmt5 = hsppp::strf("Padded: %05d", 42);           // "Padded: 00042"
        [[maybe_unused]] std::string fmt6 = hsppp::strf("String: %s", std::string("test")); // "String: test"
        [[maybe_unused]] std::string fmt7 = hsppp::strf("Two: %d, %d", 1, 2);          // "Two: 1, 2"
        [[maybe_unused]] std::string fmt8 = hsppp::strf("Mix: %d, %f", 10, 2.5);       // "Mix: 10, 2.500000"
        [[maybe_unused]] std::string fmt9 = hsppp::strf("Three: %d, %d, %d", 1, 2, 3); // "Three: 1, 2, 3"

        // getpath - パスの一部を取得
        std::string testPath = "c:\\disk\\test.bmp";
        [[maybe_unused]] std::string path1 = hsppp::getpath(testPath, 0);        // "c:\\disk\\test.bmp" (そのまま)
        [[maybe_unused]] std::string path2 = hsppp::getpath(testPath, 1);        // "c:\\disk\\test" (拡張子除去)
        [[maybe_unused]] std::string path3 = hsppp::getpath(testPath, 2);        // ".bmp" (拡張子のみ)
        [[maybe_unused]] std::string path4 = hsppp::getpath(testPath, 8);        // "test.bmp" (ディレクトリ除去)
        [[maybe_unused]] std::string path5 = hsppp::getpath(testPath, 8 + 1);    // "test" (ディレクトリ+拡張子除去)
        [[maybe_unused]] std::string path6 = hsppp::getpath(testPath, 16);       // "c:\\disk\\test.bmp" (小文字)
        [[maybe_unused]] std::string path7 = hsppp::getpath(testPath, 32);       // "c:\\disk\\" (ディレクトリのみ)
        [[maybe_unused]] std::string path8 = hsppp::getpath("", 0);              // ""
        [[maybe_unused]] std::string path9 = hsppp::getpath("noext", 2);         // "" (拡張子なし)
        [[maybe_unused]] std::string pathA = hsppp::getpath("file.txt", 32);     // "" (ディレクトリなし)

        // Unix形式パスも対応
        std::string unixPath = "/home/user/file.txt";
        [[maybe_unused]] std::string pathB = hsppp::getpath(unixPath, 8);        // "file.txt"
        [[maybe_unused]] std::string pathC = hsppp::getpath(unixPath, 32);       // "/home/user/"

        // strrep - 文字列の置換
        std::string repStr = "aaa bbb aaa ccc";
        [[maybe_unused]] int64_t repCount1 = hsppp::strrep(repStr, "aaa", "XXX");    // 2 (置換回数)
        // repStr は "XXX bbb XXX ccc" になっている
        std::string repStr2 = "ABCABC";
        [[maybe_unused]] int64_t repCount2 = hsppp::strrep(repStr2, "ABC", "X");     // 2
        std::string repStr3 = "Hello";
        [[maybe_unused]] int64_t repCount3 = hsppp::strrep(repStr3, "XYZ", "");      // 0 (見つからない)
        std::string repStr4 = "";
        [[maybe_unused]] int64_t repCount4 = hsppp::strrep(repStr4, "A", "B");       // 0 (空文字列)

        // getstr - バッファから文字列読み出し
        std::string strBuf = "ABC,DEF,GHI";
        std::string destStr;
        [[maybe_unused]] int64_t len1 = hsppp::getstr(destStr, strBuf, 0, ',');      // "ABC", 4
        [[maybe_unused]] int64_t len2 = hsppp::getstr(destStr, strBuf, 4, ',');      // "DEF", 4
        
        std::string multiLine = "Line1\nLine2\nLine3";
        [[maybe_unused]] int64_t len3 = hsppp::getstr(destStr, multiLine, 0);        // "Line1", 6
        [[maybe_unused]] int64_t len4 = hsppp::getstr(destStr, multiLine, 6);        // "Line2", 6
        
        // getstr with vector<uint8_t>
        std::vector<uint8_t> vecBuf = {'A', 'B', 'C', ',', 'D', 'E', 'F', 0};
        [[maybe_unused]] int64_t vlen1 = hsppp::getstr(destStr, vecBuf, 0, ',');     // "ABC", 4

        // split - 文字列を分割
        std::vector<std::string> result1 = hsppp::split("12,34,56", ",");        // {"12", "34", "56"}
        [[maybe_unused]] size_t splitCount = result1.size();                      // 3
        std::vector<std::string> result2 = hsppp::split("Hello", ",");           // {"Hello"}
        std::vector<std::string> result3 = hsppp::split("A::B::C", "::");        // {"A", "B", "C"}
        std::vector<std::string> result4 = hsppp::split("", ",");                // {""}
        std::vector<std::string> result5 = hsppp::split("A,B,", ",");            // {"A", "B", ""}
        
        // std::stringとの相互変換
        std::string stdStr = "standard";
        hsppp::string fromStd = stdStr;                                // std::stringから変換
        std::string toStd = fromStd;                                   // std::stringへ暗黙変換
    }

    // ============================================================
    // 数学定数のテスト
    // ============================================================
    void test_math_constants() {
        [[maybe_unused]] double pi = M_PI;
        [[maybe_unused]] double e = M_E;
        [[maybe_unused]] double log2e = M_LOG2E;
        [[maybe_unused]] double log10e = M_LOG10E;
        [[maybe_unused]] double ln2 = M_LN2;
        [[maybe_unused]] double ln10 = M_LN10;
        [[maybe_unused]] double sqrt2 = M_SQRT2;
        [[maybe_unused]] double sqrt3 = M_SQRT3;
        [[maybe_unused]] double sqrtpi = M_SQRTPI;
    }

    // ============================================================
    // C++標準ライブラリエクスポートのテスト
    // ============================================================
    void test_cpp_stdlib_exports() {
        // --- std::format (C++20) ---
        [[maybe_unused]] auto fmt1 = hsppp::format("Hello, {}!", "World");
        [[maybe_unused]] auto fmt2 = hsppp::format("{:05d}", 42);           // "00042"
        [[maybe_unused]] auto fmt3 = hsppp::format("{:.2f}", 3.14159);      // "3.14"
        [[maybe_unused]] auto fmt4 = hsppp::format("{0} + {0} = {1}", 2, 4);// "2 + 2 = 4"
        [[maybe_unused]] auto fmt5 = hsppp::format("{:#x}", 255);           // "0xff"

        // vformat (動的引数) - make_format_argsは左辺値参照が必要
        int val = 42;
        [[maybe_unused]] auto vfmt = hsppp::vformat("Value: {}", hsppp::make_format_args(val));

        // --- 文字列型 ---
        [[maybe_unused]] hsppp::string str1 = "Hello";
        [[maybe_unused]] hsppp::wstring wstr1 = L"Wide";
        [[maybe_unused]] hsppp::u8string u8str1 = u8"UTF-8";
        [[maybe_unused]] hsppp::u16string u16str1 = u"UTF-16";
        [[maybe_unused]] hsppp::u32string u32str1 = U"UTF-32";

        // --- 文字列ビュー ---
        [[maybe_unused]] hsppp::string_view sv1 = "View";
        [[maybe_unused]] hsppp::wstring_view wsv1 = L"WideView";
        [[maybe_unused]] hsppp::u8string_view u8sv1 = u8"UTF-8 View";

        // --- 文字列変換 ---
        [[maybe_unused]] hsppp::string s_from_int = hsppp::to_string(42);
        [[maybe_unused]] hsppp::string s_from_dbl = hsppp::to_string(3.14);
        [[maybe_unused]] hsppp::wstring ws_from_int = hsppp::to_wstring(42);

        [[maybe_unused]] int i1 = hsppp::stoi("123");
        [[maybe_unused]] long l1 = hsppp::stol("123456");
        [[maybe_unused]] int64_t ll1 = hsppp::stoll("123456789012");
        [[maybe_unused]] unsigned long ul1 = hsppp::stoul("12345");
        [[maybe_unused]] uint64_t ull1 = hsppp::stoull("1234567890");
        [[maybe_unused]] float f1 = hsppp::stof("3.14");
        [[maybe_unused]] double d1 = hsppp::stod("3.14159");
        [[maybe_unused]] long double ld1 = hsppp::stold("3.14159265358979");

        // --- アルゴリズム ---
        hsppp::string alg_str = "hello";
        hsppp::transform(alg_str.begin(), alg_str.end(), alg_str.begin(), 
                        [](char c) { return static_cast<char>(std::toupper(c)); });
        
        std::vector<int> vec = {1, 2, 3, 4, 5};
        [[maybe_unused]] auto it1 = hsppp::find(vec.begin(), vec.end(), 3);
        [[maybe_unused]] auto it2 = hsppp::find_if(vec.begin(), vec.end(), [](int x) { return x > 3; });
        [[maybe_unused]] auto cnt = hsppp::count(vec.begin(), vec.end(), 3);
        [[maybe_unused]] bool all = hsppp::all_of(vec.begin(), vec.end(), [](int x) { return x > 0; });
        [[maybe_unused]] bool any = hsppp::any_of(vec.begin(), vec.end(), [](int x) { return x == 3; });
        [[maybe_unused]] bool none = hsppp::none_of(vec.begin(), vec.end(), [](int x) { return x < 0; });
        
        hsppp::sort(vec.begin(), vec.end());
        hsppp::reverse(vec.begin(), vec.end());

        // --- optional ---
        [[maybe_unused]] hsppp::optional<int> opt1;
        [[maybe_unused]] hsppp::optional<int> opt2 = hsppp::nullopt;
        [[maybe_unused]] hsppp::optional<int> opt3 = hsppp::make_optional(42);

        // --- vector ---
        [[maybe_unused]] hsppp::vector<int> vec1;
        [[maybe_unused]] hsppp::vector<uint8_t> vec2 = {1, 2, 3, 4};

        // --- functional ---
        [[maybe_unused]] hsppp::function<int(int, int)> fn = [](int a, int b) { return a + b; };
        [[maybe_unused]] int result = hsppp::invoke(fn, 1, 2);
        
        auto add_five = hsppp::bind_front([](int a, int b) { return a + b; }, 5);
        [[maybe_unused]] int r2 = add_five(3);  // 5 + 3 = 8
    }

    // ============================================================
    // システム情報関数のテスト
    // ============================================================
    void test_sysinfo_functions() {
        // sysinfo_str - 文字列を返すシステム情報
        [[maybe_unused]] std::string osName = sysinfo_str(0);       // OS名
        [[maybe_unused]] std::string userName = sysinfo_str(1);     // ユーザー名
        [[maybe_unused]] std::string compName = sysinfo_str(2);     // コンピュータ名

        // sysinfo_int - 整数を返すシステム情報（int64_t対応）
        [[maybe_unused]] int64_t lang = sysinfo_int(3);         // 言語（常に1=日本語）
        [[maybe_unused]] int64_t cpuType = sysinfo_int(16);     // CPU種類
        [[maybe_unused]] int64_t cpuCount = sysinfo_int(17);    // CPU数
        [[maybe_unused]] int64_t memLoad = sysinfo_int(33);     // メモリ使用率(%)
        [[maybe_unused]] int64_t totalPhys = sysinfo_int(34);   // 全物理メモリ(MB)
        [[maybe_unused]] int64_t availPhys = sysinfo_int(35);   // 空き物理メモリ(MB)
        [[maybe_unused]] int64_t totalSwap = sysinfo_int(36);   // スワップファイル合計(MB)
        [[maybe_unused]] int64_t availSwap = sysinfo_int(37);   // スワップファイル空き(MB)
        [[maybe_unused]] int64_t totalVirt = sysinfo_int(38);   // 仮想メモリ合計(MB)
        [[maybe_unused]] int64_t availVirt = sysinfo_int(39);   // 仮想メモリ空き(MB)
    }

    // ============================================================
    // ディレクトリ情報関数のテスト
    // ============================================================
    void test_dirinfo_functions() {
        // dirinfo
        [[maybe_unused]] std::string curDir = dirinfo(0);       // カレントディレクトリ
        [[maybe_unused]] std::string exeDir = dirinfo(1);       // 実行ファイルディレクトリ
        [[maybe_unused]] std::string winDir = dirinfo(2);       // Windowsディレクトリ
        [[maybe_unused]] std::string sysDir = dirinfo(3);       // システムディレクトリ
        [[maybe_unused]] std::string cmdLine = dirinfo(4);      // コマンドライン
        [[maybe_unused]] std::string tvDir = dirinfo(5);        // HSPTVディレクトリ（空）
        [[maybe_unused]] std::string desktop = dirinfo(0x10000); // デスクトップ(CSIDL_DESKTOP)
        [[maybe_unused]] std::string mydoc = dirinfo(0x10005);  // マイドキュメント(CSIDL_PERSONAL)

        // dir_* 関数
        [[maybe_unused]] std::string d_cur = dir_cur();
        [[maybe_unused]] std::string d_exe = dir_exe();
        [[maybe_unused]] std::string d_win = dir_win();
        [[maybe_unused]] std::string d_sys = dir_sys();
        [[maybe_unused]] std::string d_cmd = dir_cmdline();
        [[maybe_unused]] std::string d_desk = dir_desktop();
        [[maybe_unused]] std::string d_mydoc = dir_mydoc();
    }



    // ============================================================
    // ファイル操作関数のテスト
    // ============================================================
    void test_file_functions() {
        // exec実行モード定数
        [[maybe_unused]] int m1 = exec_normal;
        [[maybe_unused]] int m2 = exec_minimized;
        [[maybe_unused]] int m3 = exec_shellexec;
        [[maybe_unused]] int m4 = exec_print;

        // dialog タイプ定数
        [[maybe_unused]] int d1 = dialog_info;
        [[maybe_unused]] int d2 = dialog_warning;
        [[maybe_unused]] int d3 = dialog_yesno;
        [[maybe_unused]] int d4 = dialog_yesno_w;
        [[maybe_unused]] int d5 = dialog_open;
        [[maybe_unused]] int d6 = dialog_save;
        [[maybe_unused]] int d7 = dialog_color;
        [[maybe_unused]] int d8 = dialog_colorex;

        // exec - ファイル実行（シグネチャのみ確認）
        [[maybe_unused]] int res1 = exec("notepad", 0, "");              // 実際には実行しない
        [[maybe_unused]] int res2 = exec("file.txt", exec_shellexec);    // 実際には実行しない

        // exist - ファイルサイズ取得
        [[maybe_unused]] int64_t size1 = exist("nonexistent_file_12345.txt");  // -1が返る

        // dirlist - ディレクトリ一覧取得
        [[maybe_unused]] std::vector<std::string> list1 = dirlist("*.*");       // すべてのファイル
        [[maybe_unused]] std::vector<std::string> list2 = dirlist("*.txt", 0);  // .txtファイル
        [[maybe_unused]] std::vector<std::string> list3 = dirlist("*", 1);      // ディレクトリ除外
        [[maybe_unused]] std::vector<std::string> list4 = dirlist("*", 5);      // ディレクトリのみ

        // dialog - ダイアログ（シグネチャのみ確認）
        [[maybe_unused]] DialogResult res3 = dialog("test", 0);
        [[maybe_unused]] std::string res4 = dialog("txt", 16);
        [[maybe_unused]] int res5 = dialog("yesno", dialog_yesno);

        // bload/bsave（シグネチャのみ確認）
        std::string strBuf(64, '\0');
        std::vector<uint8_t> vecBuf(64, 0);
        
        // bload シグネチャ
        [[maybe_unused]] int64_t bl1 = bload("test.bin", strBuf);
        [[maybe_unused]] int64_t bl2 = bload("test.bin", strBuf, 32);
        [[maybe_unused]] int64_t bl3 = bload("test.bin", strBuf, 32, 0);
        [[maybe_unused]] int64_t bl4 = bload("test.bin", vecBuf);
        [[maybe_unused]] int64_t bl5 = bload("test.bin", vecBuf, 32);
        [[maybe_unused]] int64_t bl6 = bload("test.bin", vecBuf, 32, 0);

        // bsave シグネチャ
        [[maybe_unused]] int64_t bs1 = bsave("test.bin", strBuf);
        [[maybe_unused]] int64_t bs2 = bsave("test.bin", strBuf, 32);
        [[maybe_unused]] int64_t bs3 = bsave("test.bin", strBuf, 32, 0);
        [[maybe_unused]] int64_t bs4 = bsave("test.bin", vecBuf);
        [[maybe_unused]] int64_t bs5 = bsave("test.bin", vecBuf, 32);
        [[maybe_unused]] int64_t bs6 = bsave("test.bin", vecBuf, 32, 0);

        // dialog（シグネチャのみ確認）
        dialog("メッセージ");
        dialog("メッセージ", 0);
        dialog("メッセージ", 0, "タイトル");
        dialog("メッセージ", dialog_yesno, "確認");
        dialog("txt", dialog_open, "テキストファイル");
        dialog("", dialog_color);
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
        compile_test::test_image_functions();
        compile_test::test_cel_class();
        compile_test::test_window_control_functions();
        compile_test::test_control_functions();
        compile_test::test_font_window_functions();
        compile_test::test_input_functions();
        compile_test::test_interrupt_functions();
        compile_test::test_math_functions();
        compile_test::test_conversion_functions();
        compile_test::test_color_functions();
        compile_test::test_string_functions();
        compile_test::test_math_constants();
        compile_test::test_cpp_stdlib_exports();
        compile_test::test_sysinfo_functions();
        compile_test::test_dirinfo_functions();
        compile_test::test_file_functions();
        // compile_test::test_end_function_signature(); // end()は呼ばない

        return true;
    }

}  // namespace hsppp_test
