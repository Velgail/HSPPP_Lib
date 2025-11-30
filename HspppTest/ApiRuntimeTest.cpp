// HspppTest/ApiRuntimeTest.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP API ランタイムテスト
// APIがクラッシュせずに動作することを確認する
// ═══════════════════════════════════════════════════════════════════

import hsppp;
using namespace hsppp;

namespace hsppp_test {

    // テスト結果を追跡
    static int s_testsPassed = 0;
    static int s_testsFailed = 0;

    // 簡易テストマクロ的な関数
    inline void check(bool condition, const char* testName) {
        if (condition) {
            s_testsPassed++;
        } else {
            s_testsFailed++;
            // デバッグ出力（OutputDebugStringA は Windows.h が必要なので省略）
        }
    }

    // ============================================================
    // Screen 作成テスト
    // ============================================================
    bool test_screen_creation() {
        bool allPassed = true;

        // OOP版 screen()
        {
            auto scr = screen({.width = 200, .height = 150, .mode = screen_hide});
            check(scr.valid(), "screen() OOP returns valid handle");
            check(scr.width() == 200, "screen() width matches");
            check(scr.height() == 150, "screen() height matches");
            allPassed &= scr.valid();
        }

        // HSP互換版 screen(id, ...)
        {
            auto scr = screen(99, 300, 200, screen_hide);
            check(scr.valid(), "screen(id) returns valid handle");
            check(scr.id() == 99, "screen(id) ID matches");
            check(scr.width() == 300, "screen(id) width matches");
            allPassed &= scr.valid();
        }

        return allPassed;
    }

    // ============================================================
    // Buffer 作成テスト
    // ============================================================
    bool test_buffer_creation() {
        bool allPassed = true;

        // OOP版
        {
            auto buf = buffer({.width = 128, .height = 128});
            check(buf.valid(), "buffer() OOP returns valid handle");
            check(buf.width() == 128, "buffer() width matches");
            allPassed &= buf.valid();
        }

        // HSP互換版
        {
            auto buf = buffer(98, 256, 256);
            check(buf.valid(), "buffer(id) returns valid handle");
            check(buf.id() == 98, "buffer(id) ID matches");
            allPassed &= buf.valid();
        }

        return allPassed;
    }

    // ============================================================
    // 描画命令テスト（クラッシュしないことを確認）
    // ============================================================
    bool test_drawing_commands() {
        auto scr = screen({.width = 400, .height = 300, .mode = screen_hide});
        if (!scr.valid()) return false;

        // 描画設定
        scr.color(255, 255, 255);
        scr.pos(10, 10);

        // redraw(0) で描画開始
        scr.redraw(0);

        // 矩形
        scr.color(255, 0, 0);
        scr.boxf(0, 0, 100, 100);
        scr.boxf();

        // 直線
        scr.color(0, 255, 0);
        scr.line(50, 50);
        scr.line(100, 100, 50, 50);

        // 円
        scr.color(0, 0, 255);
        scr.circle(150, 50, 250, 150, 1);
        scr.circle(150, 50, 250, 150, 0);

        // 点
        scr.color(255, 255, 0);
        scr.pset(200, 200);
        scr.pos(210, 210);
        scr.pset();

        // 文字
        scr.color(0, 0, 0);
        scr.pos(10, 250);
        scr.mes("Test drawing");

        // redraw(1) で画面更新
        scr.redraw(1);

        return true;
    }

    // ============================================================
    // グローバル関数テスト
    // ============================================================
    bool test_global_functions() {
        // 隠しウィンドウで実行
        screen(97, 200, 150, screen_hide);
        gsel(97);

        redraw(0);

        color(128, 128, 128);
        boxf();

        color(255, 255, 255);
        pos(10, 10);
        mes("Global test");

        line(50, 50, 10, 10);
        circle(60, 60, 120, 120, 1);
        pset(100, 100);

        redraw(1);

        return true;
    }

    // ============================================================
    // ginfo テスト
    // ============================================================
    bool test_ginfo() {
        screen(96, 320, 240, screen_hide);
        gsel(96);

        // 各種情報取得（値の妥当性はGUI環境依存なので、クラッシュしないことを確認）
        [[maybe_unused]] int mouseX = ginfo(0);
        [[maybe_unused]] int mouseY = ginfo(1);
        [[maybe_unused]] int activeId = ginfo(2);
        [[maybe_unused]] int currentId = ginfo(3);

        color(100, 150, 200);
        int r = ginfo(16);
        int g = ginfo(17);
        int b = ginfo(18);
        check(r == 100, "ginfo(16) R matches");
        check(g == 150, "ginfo(17) G matches");
        check(b == 200, "ginfo(18) B matches");

        check(ginfo_r() == r, "ginfo_r() matches ginfo(16)");
        check(ginfo_g() == g, "ginfo_g() matches ginfo(17)");
        check(ginfo_b() == b, "ginfo_b() matches ginfo(18)");

        [[maybe_unused]] int deskW = ginfo(20);
        [[maybe_unused]] int deskH = ginfo(21);
        check(deskW > 0, "ginfo(20) desktop width > 0");
        check(deskH > 0, "ginfo(21) desktop height > 0");

        pos(50, 50);
        check(ginfo(22) == 50, "ginfo(22) current X");
        check(ginfo(23) == 50, "ginfo(23) current Y");

        check(ginfo(26) == 320, "ginfo(26) init width");
        check(ginfo(27) == 240, "ginfo(27) init height");

        return true;
    }

    // ============================================================
    // gcopy/gzoom テスト
    // ============================================================
    bool test_copy_functions() {
        // バッファを作成してパターンを描画
        auto src = buffer({.width = 64, .height = 64});
        src.redraw(0);
        src.color(255, 0, 0).boxf(0, 0, 32, 32);
        src.color(0, 255, 0).boxf(32, 0, 64, 32);
        src.color(0, 0, 255).boxf(0, 32, 32, 64);
        src.color(255, 255, 0).boxf(32, 32, 64, 64);
        src.redraw(1);

        // コピー先ウィンドウ
        auto dest = screen({.width = 256, .height = 256, .mode = screen_hide});
        dest.redraw(0);
        dest.color(128, 128, 128).boxf();

        // gcopy
        gsel(dest.id());
        pos(10, 10);
        gmode(0, 64, 64);
        gcopy(src.id(), 0, 0, 64, 64);

        // gzoom
        pos(100, 10);
        gzoom(128, 128, src.id(), 0, 0, 64, 64, 0);

        dest.redraw(1);

        return true;
    }

    // ============================================================
    // font/sysfont テスト
    // ============================================================
    bool test_font_functions() {
        auto scr = screen({.width = 300, .height = 200, .mode = screen_hide});
        if (!scr.valid()) return false;

        scr.redraw(0);
        scr.color(255, 255, 255).boxf();

        // HSP互換版 font
        font("MS Gothic", 12, 0);
        scr.color(0, 0, 0).pos(10, 10);
        scr.mes("Normal 12pt");

        font("MS Gothic", 16, 1);  // 太字
        scr.pos(10, 30);
        scr.mes("Bold 16pt");

        font("MS Gothic", 14, 2);  // イタリック
        scr.pos(10, 55);
        scr.mes("Italic 14pt");

        // OOP版 font
        scr.font("Arial", 10, 0);
        scr.pos(10, 80);
        scr.mes("Arial 10pt");

        // sysfont
        sysfont(0);  // HSP標準
        scr.pos(10, 100);
        scr.mes("sysfont(0)");

        scr.sysfont(17);  // デフォルトGUI
        scr.pos(10, 120);
        scr.mes("sysfont(17)");

        scr.redraw(1);
        return true;
    }

    // ============================================================
    // title/width テスト
    // ============================================================
    bool test_title_width_functions() {
        auto scr = screen({.width = 400, .height = 300, .mode = screen_hide});
        if (!scr.valid()) return false;

        // HSP互換版 title
        scr.select();
        title("Test Title 1");

        // OOP版 title
        scr.title("Test Title 2");

        // HSP互換版 width（サイズ変更）
        width(350, 250);
        
        // OOP版 windowSize
        scr.windowSize(300, 200);
        scr.windowSize(-1, -1, 100, 100);  // 位置のみ変更

        return true;
    }

    // ============================================================
    // メソッドチェーンテスト
    // ============================================================
    bool test_method_chaining() {
        auto scr = screen({.width = 200, .height = 200, .mode = screen_hide});
        
        scr.redraw(0)
           .color(255, 255, 255)
           .boxf()
           .color(255, 0, 0)
           .pos(10, 10)
           .mes("Chain")
           .line(50, 50)
           .line(100, 100, 50, 50)
           .color(0, 255, 0)
           .circle(50, 50, 150, 150, 0)
           .color(0, 0, 255)
           .pset(100, 100)
           .redraw(1);

        return true;
    }

    // ============================================================
    // 公開テスト関数
    // ============================================================

    /// @brief すべてのランタイムテストを実行
    /// @return 成功したテスト数
    int run_runtime_tests() {
        s_testsPassed = 0;
        s_testsFailed = 0;

        test_screen_creation();
        test_buffer_creation();
        test_drawing_commands();
        test_global_functions();
        test_ginfo();
        test_copy_functions();
        test_font_functions();
        test_title_width_functions();
        test_method_chaining();

        return s_testsPassed;
    }

    /// @brief 失敗したテスト数を取得
    int get_failed_count() {
        return s_testsFailed;
    }

    /// @brief 成功したテスト数を取得
    int get_passed_count() {
        return s_testsPassed;
    }

}  // namespace hsppp_test
