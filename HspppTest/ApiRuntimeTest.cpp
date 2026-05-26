// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppTest/ApiRuntimeTest.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP API ランタイムテスト
// APIがクラッシュせずに動作することを確認する
// ═══════════════════════════════════════════════════════════════════

import hsppp;
import <atomic>;
import <chrono>;
import <functional>;
import <thread>;

using namespace hsppp;

namespace hsppp_test {

    // テスト結果を追跡
    static int s_testsPassed = 0;
    static int s_testsFailed = 0;
    static int s_testsRun    = 0;
    static int s_firstFailedIndex = -1;
    static const char* s_firstFailedName = nullptr;

    // 簡易テストマクロ的な関数
    inline void check(bool condition, const char* testName) {
        ++s_testsRun;
        if (condition) {
            s_testsPassed++;
        } else {
            s_testsFailed++;
            if (s_firstFailedIndex < 0) {
                s_firstFailedIndex = s_testsRun;
                s_firstFailedName  = testName;
            }
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
        (void)screen(97, 200, 150, screen_hide);
        gsel(97);

        redraw(0);

        color(128, 128, 128);
        (void)boxf();

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
        (void)screen(96, 320, 240, screen_hide);
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
    // note系 / sendmsg / sysval テスト
    // ============================================================
    bool test_note_and_sendmsg() {
        // note系（ウィンドウ不要）
        {
            std::string note;
            std::string out;

            notesel(note);
            noteadd("A");
            noteadd("C");
            noteadd("B", 1);
            check(noteinfo(notemax) == 3, "noteinfo(notemax) == 3");

            noteget(out, 1);
            check(out == "B", "noteget index 1 == 'B'");
            check(notefind("A", notefind_match) == 0, "notefind match");
            check(notefind("B", notefind_first) == 1, "notefind first");
            check(notefind("C", notefind_instr) == 2, "notefind instr");

            notedel(1);
            check(noteinfo(notemax) == 2, "notedel reduces notemax");
            noteunsel();
        }

        // hwnd/sendmsg（ウィンドウが必要）
        {
            auto scr = screen({.width = 100, .height = 80, .mode = screen_hide});
            if (!scr.valid()) {
                return false;
            }
            scr.select();

            int64_t h = hwnd();
            check(h != 0, "hwnd() != 0");

            // WM_SETTEXT (0x000C)
            (void)sendmsg(h, 0x000C, 0, "HspppTest");
            [[maybe_unused]] int64_t inst = hinstance();
            [[maybe_unused]] int64_t dc = hdc();
        }

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
        
        // OOP版 width
        scr.width(300, 200);
        scr.width(-1, -1, 100, 100);  // 位置のみ変更

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
    // 入力関数テスト
    // ============================================================
    bool test_input_functions() {
        auto scr = screen({.width = 300, .height = 200, .mode = screen_hide});
        if (!scr.valid()) return false;

        // getkey テスト（クラッシュしないことを確認）
        // VK_SPACE = 32
        [[maybe_unused]] int keyState = getkey(32);
        check(keyState == 0 || keyState == 1, "getkey returns 0 or 1");

        // stick テスト（クラッシュしないことを確認）
        int stickResult = stick();
        check(stickResult >= 0, "stick returns non-negative value");

        // stick with non-trigger
        stickResult = stick(15);  // 矢印キー非トリガー
        check(stickResult >= 0, "stick with non-trigger");

        // stick with active check disabled
        stickResult = stick(0, 0);
        check(stickResult >= 0, "stick with active check disabled");

        // mousex/mousey テスト（グローバル版）
        [[maybe_unused]] int mx = mousex();
        [[maybe_unused]] int my = mousey();
        check(true, "mousex/mousey don't crash");

        // OOP版 mousex/mousey
        [[maybe_unused]] int mx2 = scr.mousex();
        [[maybe_unused]] int my2 = scr.mousey();
        check(true, "Screen::mousex/mousey don't crash");

        // mousew テスト
        [[maybe_unused]] int mw = mousew();
        check(true, "mousew doesn't crash");

        // mouse テスト（座標取得のみ、実際に動かすのは危険）
        // mouse();  // 省略時は現在位置
        check(true, "mouse function exists");

        // wait テスト（短い時間で）
        wait(1);  // 10ms
        check(true, "wait doesn't crash");

        return true;
    }

    // ============================================================
    // 文字列関数テスト（実行時検証）
    // ============================================================
    bool test_string_functions_runtime() {
        bool allPassed = true;

        // --- instr テスト ---
        // 基本検索
        check(instr("ABCDEF", "CD") == 2, "instr basic search");
        check(instr("ABCDEF", "AB") == 0, "instr at beginning");
        check(instr("ABCDEF", "EF") == 4, "instr at end");
        check(instr("ABCDEF", "XY") == -1, "instr not found");
        check(instr("ABCDEF", "") == 0, "instr empty search");
        
        // 開始位置指定（HSP仕様: 結果はp2を起点とした相対位置）
        check(instr("ABCABC", 3, "ABC") == 0, "instr with offset - relative position");
        // "ABCABC" の offset=1 から探索すると "BC" は絶対位置 1 にあるが、
        // p2 起点の相対位置では 0。HSP 仕様(結果はp2を起点とした相対位置) および
        // 実装 hsppp_string.inl L478 (return pos - p2) と整合。
        // ※ 旧期待値 == 1 はハーネスブロックで顕在化していなかった既存バグ。
        check(instr("ABCABC", 1, "BC") == 0, "instr with offset - found at relative 0");
        check(instr("ABCDEF", 2, "CD") == 0, "instr exact match at offset");
        check(instr("ABCDEF", 10, "AB") == -1, "instr offset beyond string");
        check(instr("ABCDEF", -1, "AB") == -1, "instr negative offset");

        // --- strmid テスト ---
        check(strmid("ABCDEF", 0, 3) == "ABC", "strmid from start");
        check(strmid("ABCDEF", 2, 3) == "CDE", "strmid from middle");
        check(strmid("ABCDEF", 4, 10) == "EF", "strmid beyond end");
        check(strmid("ABCDEF", -1, 3) == "DEF", "strmid right extract");
        check(strmid("AB", -1, 5) == "AB", "strmid right extract full");
        check(strmid("ABCDEF", 0, 0) == "", "strmid zero length");
        check(strmid("ABCDEF", -2, 3) == "", "strmid invalid negative");

        // --- strtrim テスト ---
        check(strtrim("  ABC  ", 0, ' ') == "ABC", "strtrim both ends");
        check(strtrim("  ABC  ", 1, ' ') == "ABC  ", "strtrim left only");
        check(strtrim("  ABC  ", 2, ' ') == "  ABC", "strtrim right only");
        check(strtrim(" A B C ", 3, ' ') == "ABC", "strtrim all");
        check(strtrim("XXABCXX", 0, 'X') == "ABC", "strtrim custom char");
        check(strtrim("ABC") == "ABC", "strtrim default (no spaces)");
        check(strtrim("   ") == "", "strtrim all spaces");
        check(strtrim("", 0, ' ') == "", "strtrim empty string");

        // --- strf テスト ---
        check(strf("Hello") == "Hello", "strf no args");
        check(strf("Value: %d", 123) == "Value: 123", "strf int");
        check(strf("Hex: %x", 255) == "Hex: ff", "strf hex");
        check(strf("Padded: %05d", 42) == "Padded: 00042", "strf padded");
        check(strf("Two: %d, %d", 1, 2) == "Two: 1, 2", "strf two ints");
        check(strf("Three: %d, %d, %d", 1, 2, 3) == "Three: 1, 2, 3", "strf three ints");

        // --- getpath テスト ---
        std::string testPath = "c:\\disk\\test.bmp";
        check(getpath(testPath, 0) == "c:\\disk\\test.bmp", "getpath copy");
        check(getpath(testPath, 1) == "c:\\disk\\test", "getpath remove ext");
        check(getpath(testPath, 2) == ".bmp", "getpath ext only");
        check(getpath(testPath, 8) == "test.bmp", "getpath remove dir");
        check(getpath(testPath, 8 + 1) == "test", "getpath file without ext");
        check(getpath(testPath, 32) == "c:\\disk\\", "getpath dir only");
        check(getpath("noext", 2) == "", "getpath no extension");
        check(getpath("file.txt", 32) == "", "getpath no directory");
        
        // Unix形式パス
        check(getpath("/home/user/file.txt", 8) == "file.txt", "getpath unix remove dir");
        check(getpath("/home/user/file.txt", 32) == "/home/user/", "getpath unix dir only");

        return allPassed;
    }

    // ============================================================
    // 非同期サブステートマシン runtime テスト
    // ============================================================
    enum class AsyncRuntimeParentState {
        Main,
        Done,
    };

    enum class AsyncRuntimeChildState {
        Running,
    };

    struct AsyncRuntimeMonitor {
        std::atomic<bool> launched = false;
        std::atomic<bool> update_entered = false;
        std::atomic<bool> stop_requested_seen = false;
        std::atomic<bool> done_entered = false;
        std::atomic<int> ticks = 0;
        std::atomic<std::size_t> count_after_leave = 99;
        std::atomic<std::size_t> main_thread_marker = 0;
        std::atomic<std::size_t> child_thread_marker = 0;
    };

    static std::size_t current_thread_marker() {
        return std::hash<std::thread::id>{}(std::this_thread::get_id());
    }

    bool test_async_submachine_runtime() {
        AsyncRuntimeMonitor monitor;
        StateGraph<AsyncRuntimeParentState> parent;

        parent.state(AsyncRuntimeParentState::Main)
          .on_enter([&]() {
              monitor.main_thread_marker.store(current_thread_marker(), std::memory_order_relaxed);

              AsyncSubMachineOptions options{};
              options.idle_wait_ms = 1;
              options.graphics = SubMachineGraphicsPolicy::none;

              parent.start_submachine<AsyncRuntimeChildState>(
                  AsyncRuntimeChildState::Running,
                  [&monitor](StateGraph<AsyncRuntimeChildState>& child) {
                      child.state(AsyncRuntimeChildState::Running)
                        .on_enter([&monitor]() {
                            monitor.child_thread_marker.store(current_thread_marker(), std::memory_order_relaxed);
                            monitor.launched.store(true, std::memory_order_relaxed);
                        })
                        .on_update([&monitor](StateGraph<AsyncRuntimeChildState>& child_sm) {
                            monitor.ticks.fetch_add(1, std::memory_order_relaxed);
                            monitor.update_entered.store(true, std::memory_order_relaxed);
                            for (int i = 0; i < 1000; ++i) {
                                if (submachine_stop_requested()) {
                                    monitor.stop_requested_seen.store(true, std::memory_order_relaxed);
                                    child_sm.quit();
                                    return;
                                }
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
                        });
                  },
                  options);
          })
          .on_update([&](StateGraph<AsyncRuntimeParentState>& sm) {
              if (monitor.launched.load(std::memory_order_relaxed) &&
                  monitor.update_entered.load(std::memory_order_relaxed)) {
                  sm.jump(AsyncRuntimeParentState::Done);
              }
          });

        parent.state(AsyncRuntimeParentState::Done)
          .on_enter([&]() {
              monitor.count_after_leave.store(parent.submachine_count(), std::memory_order_relaxed);
              monitor.done_entered.store(true, std::memory_order_relaxed);
              parent.quit();
          });

        parent.jump(AsyncRuntimeParentState::Main);
        for (int i = 0; i < 1000 && !monitor.done_entered.load(std::memory_order_relaxed); ++i) {
            parent.step();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        const auto main_marker = monitor.main_thread_marker.load(std::memory_order_relaxed);
        const auto child_marker = monitor.child_thread_marker.load(std::memory_order_relaxed);
        const bool thread_observed =
            main_marker != 0 &&
            child_marker != 0 &&
            main_marker != child_marker;

        check(monitor.launched.load(std::memory_order_relaxed), "async submachine launched");
        check(monitor.update_entered.load(std::memory_order_relaxed), "async submachine update entered");
        check(thread_observed, "async submachine runs on another thread");
        check(monitor.stop_requested_seen.load(std::memory_order_relaxed), "async submachine observes stop request");
        check(monitor.count_after_leave.load(std::memory_order_relaxed) == 0, "async submachine joined on parent leave");

        return monitor.launched.load(std::memory_order_relaxed) &&
               monitor.update_entered.load(std::memory_order_relaxed) &&
               thread_observed &&
               monitor.stop_requested_seen.load(std::memory_order_relaxed) &&
               monitor.count_after_leave.load(std::memory_order_relaxed) == 0;
    }

    // ============================================================
    // WM_DPICHANGED 後の m_pTargetBitmap 再生成検証 (TICKET-007 / R-B / K2)
    // ------------------------------------------------------------
    // 目的: WM_DPICHANGED を sendmsg で直接ディスパッチし、
    //   onDpiChanged 経路を通った後に
    //     (1) 描画コマンド (boxf) がクラッシュしない
    //     (2) pget が m_pTargetBitmap から正しい色を読める
    //     (3) bmpsave が m_pTargetBitmap を保存して例外を出さない
    //   ことを確認する。実機 DPI 変更 (モニタ移動) が困難な環境向けの代替。
    // 注: HspWindow::onDpiChanged は GetClientRect が同一サイズを返す前提でも
    //   SwapChain 再構築 + m_pTargetBitmap 防御的再生成を実行するため、
    //   隠しウィンドウ + lparam=0 (suggested RECT なし) でも経路網羅可能。
    // ============================================================
    bool test_dpi_changed_target_bitmap_rebind() {
        constexpr int  WM_DPICHANGED_MSG = 0x02E0;
        constexpr int  TestWindowId      = 95;
        constexpr int  TestWidth         = 200;
        constexpr int  TestHeight        = 150;
        constexpr int  ProbeX            = 100;
        constexpr int  ProbeY            =  60;
        constexpr int  PreR              = 123;
        constexpr int  PreG              =  45;
        constexpr int  PreB              =  67;
        constexpr int  PostR             =  11;
        constexpr int  PostG             =  22;
        constexpr int  PostB             =  33;

        (void)screen(TestWindowId, TestWidth, TestHeight, screen_hide);
        gsel(TestWindowId);

        // Pre-DPI: 単色塗り → pget で色を確認
        redraw(0);
        color(PreR, PreG, PreB);
        (void)boxf();
        redraw(1);

        pget(ProbeX, ProbeY);
        const int rPre = ginfo(16);
        const int gPre = ginfo(17);
        const int bPre = ginfo(18);
        check(rPre == PreR && gPre == PreG && bPre == PreB,
              "pre-DPI pget returns drawn color");

        // WM_DPICHANGED を直接ディスパッチ (192 DPI = 200%)
        // wParam = MAKEWPARAM(newDpiX, newDpiY) / lParam = 0 (suggested RECT は省略)
        const int64_t windowHwnd = hwnd();
        check(windowHwnd != 0, "hwnd() returns non-zero for active window");

        const int64_t wparam = (static_cast<int64_t>(192) << 16) | static_cast<int64_t>(192);
        (void)sendmsg(windowHwnd, WM_DPICHANGED_MSG, wparam, 0);

        // Post-DPI: 既存 m_pTargetBitmap 再生成 + CopyFromBitmap で内容引き継ぎが
        //   行われるため、pget は元の色を返すはず (案 A 採用判断、findings 参照)。
        pget(ProbeX, ProbeY);
        const int rPost1 = ginfo(16);
        const int gPost1 = ginfo(17);
        const int bPost1 = ginfo(18);
        check(rPost1 == PreR && gPost1 == PreG && bPost1 == PreB,
              "post-DPI pget preserves content (CopyFromBitmap hand-off)");

        // Post-DPI: 新規描画が問題なく機能することを確認
        redraw(0);
        color(PostR, PostG, PostB);
        (void)boxf(0, 0, TestWidth, TestHeight);
        redraw(1);

        pget(ProbeX, ProbeY);
        const int rPost2 = ginfo(16);
        const int gPost2 = ginfo(17);
        const int bPost2 = ginfo(18);
        check(rPost2 == PostR && gPost2 == PostG && bPost2 == PostB,
              "post-DPI boxf + pget on rebound bitmap works");

        // Post-DPI: bmpsave が m_pTargetBitmap を参照しても落ちない (副作用テスト)
        bmpsave("dpi_changed_target_bitmap.bmp");
        check(true, "post-DPI bmpsave does not crash");

        return rPre == PreR && gPre == PreG && bPre == PreB
            && rPost2 == PostR && gPost2 == PostG && bPost2 == PostB;
    }

    // ============================================================
    // anchor_pos / anchor_box / boxf(AnchorRect) - アンカー基準レイアウト
    // ============================================================
    bool test_anchor_layout() {
        bool allPassed = true;

        // --- (1) AnchorRect::resolve 単体: アスペクト比違いでも相対位置が保たれる ---
        // 右下から内側 10px に 50x30 の矩形を置くケース
        AnchorRect r{
            .h_anchor = ah_right,
            .v_anchor = av_bottom,
            .offset_x = -10,
            .offset_y = -10,
            .width    = 50,
            .height   = 30,
        };

        // 4:3 (640x480)
        RectI rc43 = r.resolve(640, 480);
        check(rc43.x2 == 640 - 10 && rc43.y2 == 480 - 10,
              "AnchorRect resolve 4:3 right-bottom edge anchored");
        check(rc43.width() == 50 && rc43.height() == 30,
              "AnchorRect resolve 4:3 preserves width/height");

        // 21:9 (2520x1080) — アスペクト比が変わっても右下からの距離は同一
        RectI rc219 = r.resolve(2520, 1080);
        check(rc219.x2 == 2520 - 10 && rc219.y2 == 1080 - 10,
              "AnchorRect resolve 21:9 right-bottom edge anchored");
        check(rc219.width() == 50 && rc219.height() == 30,
              "AnchorRect resolve 21:9 preserves size");

        // 左上アンカー
        AnchorRect rLT{
            .h_anchor = ah_left, .v_anchor = av_top,
            .offset_x = 5, .offset_y = 7,
            .width = 40, .height = 20,
        };
        RectI rcLT = rLT.resolve(800, 600);
        check(rcLT.x1 == 5 && rcLT.y1 == 7,
              "AnchorRect resolve left-top offset");

        // 中央アンカー（中心が画面中心）
        AnchorRect rC{
            .h_anchor = ah_center, .v_anchor = av_middle,
            .offset_x = 0, .offset_y = 0,
            .width = 100, .height = 60,
        };
        RectI rcC = rC.resolve(800, 600);
        check(rcC.x1 == 800/2 - 50 && rcC.y1 == 600/2 - 30,
              "AnchorRect resolve center anchor centers the box");
        check(rcC.x2 - rcC.x1 == 100 && rcC.y2 - rcC.y1 == 60,
              "AnchorRect resolve center preserves size");

        // --- (2) Screen 経由で API がクラッシュせず動作することを確認 ---
        {
            auto scr = screen({.width = 400, .height = 300, .mode = screen_hide});
            check(scr.valid(), "anchor: screen created");

            scr.redraw(0);
            scr.color(0, 0, 0);
            (void)scr.boxf();

            // HSP 互換命令: 右下から内側 10px に anchor_pos
            scr.color(255, 0, 0);
            scr.anchor_pos(ah_right, av_bottom, -10, -10);

            // HSP 互換命令: 右下に 50x30 の矩形
            scr.anchor_box(ah_right, av_bottom, 0, 0, 50, 30);

            // OOP: AnchorRect で左上に 20x20
            scr.boxf(AnchorRect{
                .h_anchor = ah_left, .v_anchor = av_top,
                .offset_x = 0, .offset_y = 0,
                .width = 20, .height = 20,
            });

            scr.redraw(1);
            check(true, "Screen anchor_pos/anchor_box/boxf(AnchorRect) run without crash");
            allPassed &= scr.valid();
        }

        // --- (3) グローバル命令版（HSP 互換）も動くか ---
        {
            (void)screen(96, 400, 300, screen_hide);
            gsel(96);
            redraw(0);
            color(255, 255, 255);
            (void)boxf();
            color(0, 0, 255);
            anchor_pos(ah_center, av_middle, 0, 0);
            anchor_box(ah_left, av_bottom, 5, -25, 30, 20);
            boxf(AnchorRect{
                .h_anchor = ah_right, .v_anchor = av_top,
                .offset_x = -25, .offset_y = 5,
                .width = 20, .height = 20,
            });
            redraw(1);
            check(true, "global anchor_pos/anchor_box/boxf(AnchorRect) run without crash");
        }

        // --- (4) 描画結果検証: 4:3 と 16:9 で「右下から内側 10px」が一致した相対挙動になる ---
        // バッファ (buffer) を使い、pget で塗り色を検証する。
        auto verifyRightBottom = [&](int id, int w, int h, const char* label) {
            (void)screen(id, w, h, screen_hide);
            gsel(id);
            redraw(0);
            color(255, 255, 255);
            (void)boxf();
            color(200, 50, 75);
            // 右下を起点に 10x10 の塗り (offsetX=-10, offsetY=-10, w=10, h=10)
            anchor_box(ah_right, av_bottom, -10, -10, 10, 10);
            redraw(1);

            // anchor_box(ah_right, av_bottom, -10, -10, 10, 10) は
            //   AnchorRect 解決: baseX = bufferW - width(10) = w-10, x1 = w-10 + (-10) = w-20
            //   → 塗り範囲は (w-20, h-20) ~ (w-10, h-10)
            // 範囲内 (w-15, h-15) は塗られているはず
            pget(w - 15, h - 15);
            const int r = ginfo(16);
            const int g = ginfo(17);
            const int b = ginfo(18);
            check(r == 200 && g == 50 && b == 75,
                  label);
        };
        verifyRightBottom(91, 400, 300, "anchor 4:3 right-bottom interior pixel colored");
        verifyRightBottom(92, 640, 360, "anchor 16:9 right-bottom interior pixel colored");
        verifyRightBottom(93, 840, 360, "anchor 21:9 right-bottom interior pixel colored");

        return allPassed;
    }

    // ============================================================
    // 公開テスト関数
    // ============================================================

    /// @brief すべてのランタイムテストを実行
    /// @return 成功したテスト数
    int run_runtime_tests() {
        s_testsPassed = 0;
        s_testsFailed = 0;
        s_testsRun    = 0;
        s_firstFailedIndex = -1;
        s_firstFailedName  = nullptr;

        test_screen_creation();
        test_buffer_creation();
        test_drawing_commands();
        test_global_functions();
        test_ginfo();
        test_copy_functions();
        test_font_functions();
        test_title_width_functions();
        test_method_chaining();
        test_input_functions();
        test_string_functions_runtime();
        test_note_and_sendmsg();
        test_async_submachine_runtime();
        test_dpi_changed_target_bitmap_rebind();
        test_anchor_layout();

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

    int get_first_failed_index() { return s_firstFailedIndex; }
    const char* get_first_failed_name() { return s_firstFailedName ? s_firstFailedName : ""; }

}  // namespace hsppp_test
