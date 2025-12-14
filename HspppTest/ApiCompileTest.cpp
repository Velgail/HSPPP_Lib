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
        [[maybe_unused]] int len1 = hsppp::strlen("Hello");
        [[maybe_unused]] int len2 = hsppp::strlen("");
        [[maybe_unused]] int len3 = hsppp::strlen(std::string("日本語"));  // マルチバイト
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
        compile_test::test_math_constants();
        // compile_test::test_end_function_signature(); // end()は呼ばない

        return true;
    }

}  // namespace hsppp_test
