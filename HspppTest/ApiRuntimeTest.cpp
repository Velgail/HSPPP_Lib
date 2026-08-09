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

// 仮想画面リサイズ回帰テスト (test_virtual_resize_no_max_track_clamp) で
// 実 HWND の物理クライアントサイズを Win32 GetClientRect で直接観測する必要があるため、
// Windows.h を取り込む。NOMINMAX / WIN32_LEAN_AND_MEAN で hsppp との衝突を回避。
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

import hsppp;
import hsppp_testing;   // LogicalRenderContext 白箱テスト用シム
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
    // HSP互換のカレント画面・ウィンドウ終了契機
    // ============================================================
    bool test_current_screen_and_window_lifecycle() {
        constexpr int ScreenId = 201;
        constexpr int BufferId = 202;
        constexpr int BackgroundId = 203;
        constexpr int RecreateId = 204;
        constexpr int ConvertId = 205;

        (void)screen(ScreenId, 80, 60, screen_hide);
        check(ginfo(3) == ScreenId, "screen makes its ID current");
        (void)buffer(BufferId, 16, 16);
        check(ginfo(3) == BufferId, "buffer makes its ID current");
        (void)bgscr(BackgroundId, 80, 60, screen_hide);
        check(ginfo(3) == BackgroundId, "bgscr makes its ID current");

        MSG msg{};
        while (PeekMessageW(&msg, nullptr, WM_QUIT, WM_QUIT, PM_REMOVE)) {}
        (void)screen(RecreateId, 80, 60, screen_hide);
        (void)screen(RecreateId, 96, 72, screen_hide);
        const bool queuedQuit = PeekMessageW(&msg, nullptr, WM_QUIT, WM_QUIT, PM_REMOVE) != 0;
        check(!queuedQuit, "same-ID screen recreation does not enqueue WM_QUIT");

        // OpenHSPのMakeBmscr分岐: screen→bufferは可能。いったんbufferになったIDを
        // screen/bgscrで再初期化してもウィンドウ型へは戻らない。
        (void)screen(ConvertId, 30, 20, screen_hide);
        check(hwnd() != 0, "screen ID owns a native window before buffer conversion");
        (void)buffer(ConvertId, 17, 19);
        check(ginfo(3) == ConvertId && hwnd() == 0 && ginfo(26) == 17 && ginfo(27) == 19,
              "screen ID can be reinitialized as buffer");
        (void)screen(ConvertId, 22, 23, screen_hide);
        check(hwnd() == 0 && ginfo(26) == 22 && ginfo(27) == 23,
              "screen command keeps an existing buffer ID as buffer");
        (void)bgscr(ConvertId, 24, 25, screen_hide);
        check(hwnd() == 0 && ginfo(26) == 24 && ginfo(27) == 25,
              "bgscr command keeps an existing buffer ID as buffer");

        int exitWindowId = -1;
        int exitReason = -1;
        onexit([&] {
            exitWindowId = static_cast<int>(wparam());
            exitReason = iparam();
        });
        gsel(ScreenId);
        HWND target = reinterpret_cast<HWND>(static_cast<intptr_t>(hwnd()));
        SendMessageW(target, WM_CLOSE, 0, 0);
        (void)internal::processPendingInterrupt();
        check(exitWindowId == ScreenId, "WM_CLOSE on any screen reaches onexit with its window ID");
        check(IsWindow(target) != FALSE, "onexit intercept keeps the closed screen alive");

        exitWindowId = -1;
        exitReason = -1;
        const LRESULT queryResult = SendMessageW(target, WM_QUERYENDSESSION, 0, 0);
        (void)internal::processPendingInterrupt();
        check(queryResult == FALSE, "onexit intercept vetoes WM_QUERYENDSESSION until end");
        check(exitWindowId == ScreenId && exitReason == 1,
              "WM_QUERYENDSESSION reaches onexit with reason and window ID");
        onexit(InterruptHandler{});
        return !queuedQuit && exitWindowId == ScreenId;
    }

    // ============================================================
    // gmode 0〜7のHSP整数RGB演算とgzoom既定補間
    // ============================================================
    bool test_hsp_gmode_pixels() {
        constexpr int SrcId = 210;
        constexpr int DestId = 211;
        (void)buffer(SrcId, 2, 1);
        redraw(0);
        color(101, 151, 201); (void)boxf(0, 0, 1, 1);
        color(64, 128, 255); (void)boxf(1, 0, 2, 1);
        redraw(1);

        (void)buffer(DestId, 3, 1);
        auto fillDest = [&](int r, int g, int b) {
            gsel(DestId);
            redraw(0);
            color(r, g, b);
            (void)boxf();
            redraw(1);
            pos(0, 0);
        };
        auto pixel = [&](int x) {
            gsel(DestId);
            pget(x, 0);
            return (ginfo_r() << 16) | (ginfo_g() << 8) | ginfo_b();
        };
        auto rgb = [](int r, int g, int b) { return (r << 16) | (g << 8) | b; };

        fillDest(21, 41, 81);
        gmode(gmode_gdi, 1, 1, 128);
        gcopy(SrcId, 0, 0, 1, 1);
        check(pixel(0) == rgb(101, 151, 201), "gmode 0 normal copy pixel");

        fillDest(21, 41, 81);
        gmode(gmode_rgb0, 1, 1, 128);
        gcopy(SrcId, 0, 0, 1, 1);
        check(pixel(0) == rgb(101, 151, 201), "gmode 2 non-black pixel copy");

        fillDest(21, 41, 81);
        gmode(gmode_alpha, 1, 1, 128);
        gcopy(SrcId, 0, 0, 1, 1);
        check(pixel(0) == rgb(60, 95, 140), "gmode 3 uses HSP separate integer shifts");

        fillDest(21, 41, 81);
        color(101, 151, 201);
        gmode(gmode_rgb0alpha, 1, 1, 128);
        gcopy(SrcId, 0, 0, 1, 1);
        check(pixel(0) == rgb(21, 41, 81), "gmode 4 current-color key is transparent");

        fillDest(21, 41, 81);
        gmode(gmode_add, 1, 1, 128);
        gcopy(SrcId, 0, 0, 1, 1);
        check(pixel(0) == rgb(71, 116, 181), "gmode 5 additive integer blend");

        fillDest(200, 210, 220);
        gmode(gmode_sub, 1, 1, 128);
        gcopy(SrcId, 0, 0, 1, 1);
        check(pixel(0) == rgb(150, 135, 120), "gmode 6 subtractive integer blend");

        fillDest(20, 40, 80);
        gmode(gmode_pixela, 1, 1, 0);
        gcopy(SrcId, 0, 0, 1, 1);
        check(pixel(0) == rgb(39, 94, 201), "gmode 7 adjacent RGB mask integer blend");

        // コピー先が左にはみ出した時、HSPはコピー元も同じ量だけ進めてクリップする。
        fillDest(1, 2, 3);
        pos(-1, 0);
        gmode(gmode_alpha, 2, 1, 256);
        gcopy(SrcId, 0, 0, 2, 1);
        check(pixel(0) == rgb(64, 128, 255), "gcopy clips destination and advances source together");

        // gzoomのp8省略はgmode_interp拡張値ではなくHSP既定の0（nearest）。
        fillDest(0, 0, 0);
        gmode_interp(1);
        pos(0, 0);
        gzoom(3, 1, SrcId, 0, 0, 2, 1);
        const int middle = pixel(1);
        check(middle == rgb(101, 151, 201) || middle == rgb(64, 128, 255),
              "gzoom omitted p8 remains unfiltered HSP copy");

        return true;
    }

    // ============================================================
    // GUIオブジェクト: ID名前空間・再利用・objmode配置時反映
    // ============================================================
    bool test_hsp_gui_object_state() {
        auto first = screen(220, 240, 160, screen_hide);
        auto second = screen(221, 240, 160, screen_hide);
        first.pos(4, 4);
        second.pos(4, 4);
        const int firstId = first.button("first", [] {});
        const int secondId = second.button("second", [] {});
        check(firstId == 0 && secondId == 0, "object IDs are local to each screen");

        first.select();
        clrobj(0, 0);
        first.pos(4, 4);
        const int reusedId = first.button("reused", [] {});
        check(reusedId == 0, "clrobj deleted object ID is reused");

        auto styled = screen(222, 260, 140, screen_hide);
        styled.select();
        font("Arial", 22, 1);
        color(10, 20, 30);
        objcolor(200, 210, 220);
        objmode(objmode_usefont + objmode_usecolor, 0);
        auto value = std::make_shared<std::string>("HSP++");
        const int inputId = input(value, 180, 32, 32);
        check(inputId == 0, "styled input receives first object ID");

        HWND parent = reinterpret_cast<HWND>(static_cast<intptr_t>(hwnd()));
        HWND control = GetWindow(parent, GW_CHILD);
        HFONT controlFont = control ? reinterpret_cast<HFONT>(SendMessageW(control, WM_GETFONT, 0, 0)) : nullptr;
        LOGFONTW fontInfo{};
        const bool fontApplied = controlFont && GetObjectW(controlFont, sizeof(fontInfo), &fontInfo) == sizeof(fontInfo);
        check(fontApplied && std::wstring_view(fontInfo.lfFaceName) == L"Arial",
              "objmode_usefont snapshots current font at object creation");

        HDC dc = control ? GetDC(control) : nullptr;
        LRESULT brush = dc ? SendMessageW(parent, WM_CTLCOLOREDIT,
                                           reinterpret_cast<WPARAM>(dc), reinterpret_cast<LPARAM>(control)) : 0;
        const bool colorsApplied = dc && brush != 0 &&
            GetBkColor(dc) == RGB(10, 20, 30) && GetTextColor(dc) == RGB(200, 210, 220);
        check(colorsApplied, "objmode_usecolor maps color to background and objcolor to text");
        if (dc) ReleaseDC(control, dc);
        return firstId == 0 && secondId == 0 && reusedId == 0 && fontApplied && colorsApplied;
    }

    // ============================================================
    // picload/celload/celdiv/celputのHSP画面ID経路
    // ============================================================
    bool test_hsp_image_id_commands() {
        const std::string ImagePath = "hsppp_image_command_probe.bmp";
        (void)buffer(230, 2, 3);
        redraw(0);
        color(12, 34, 56); (void)boxf();
        color(101, 151, 201); (void)boxf(0, 0, 1, 1);
        redraw(1);
        bmpsave(ImagePath);

        (void)screen(231, 9, 8, screen_hide);
        picload(ImagePath, 0);
        check(ginfo(3) == 231 && ginfo(26) == 2 && ginfo(27) == 3,
              "picload mode 0 reinitializes current screen to image size");

        (void)screen(232, 9, 8, screen_hide);
        picload(ImagePath, 1);
        check(ginfo(26) == 9 && ginfo(27) == 8,
              "picload mode 1 overlays without resizing screen");

        const int loadedId = celload(ImagePath, 233, 0);
        check(loadedId == 233 && ginfo(3) == 233, "celload loads into requested screen ID and selects it");
        celdiv(loadedId, 1, 1, 0, 0);
        (void)buffer(234, 2, 1);
        redraw(0); color(0, 0, 0); (void)boxf(); redraw(1);
        pos(0, 0);
        gmode(gmode_gdi, 1, 1, 256);
        celput(loadedId, 0);
        pget(0, 0);
        check(ginfo_r() == 101 && ginfo_g() == 151 && ginfo_b() == 201,
              "celput copies selected cell through HSP gmode path");
        check(ginfo(22) == 1, "celput advances current X by unscaled cell width");

        const int reused = celload(ImagePath);
        const int reusedAgain = celload(ImagePath);
        check(reused == reusedAgain, "celload omitted ID reuses previously loaded image");
        const int alwaysNew = celload(ImagePath, celid_auto);
        check(alwaysNew != reused, "celload ID -1 always allocates a new screen ID");
        deletefile(ImagePath);
        return true;
    }

    // ============================================================
    // mouse/ginfoはデスクトップ座標、mousex/mouseyはカレント画面座標
    // ============================================================
    bool test_hsp_mouse_coordinate_spaces() {
        POINT original{};
        if (!GetCursorPos(&original)) return false;
        (void)screen(240, 80, 60, screen_hide);
        mouse(original.x, original.y, 1);
        check(ginfo(0) == original.x && ginfo(1) == original.y,
              "mouse and ginfo(0/1) use desktop coordinates");
        [[maybe_unused]] const int localX = mousex();
        [[maybe_unused]] const int localY = mousey();
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
    // WM_DPICHANGED 後の m_pTargetBitmap 再生成検証 (R-B / K2)
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
    // 仮想画面 公開API: Screen::physToLogical / logicalToPhys / letterboxColor
    // ============================================================
    bool test_virtual_screen_public_api() {
        bool allPassed = true;
        auto absi = [](int v) -> int { return v < 0 ? -v : v; };

        // (1) 仮想画面 OFF: 恒等変換となること
        {
            auto scrOff = screen({
                .width = 320, .height = 240, .mode = screen_hide,
                .virtual_resolution = false,
            });
            check(scrOff.valid(), "virtual API: OFF screen created");

            int lx = -1, ly = -1;
            scrOff.physToLogical(100, 50, lx, ly);
            check(lx == 100 && ly == 50, "virtual API: physToLogical identity on OFF");

            int px = -1, py = -1;
            scrOff.logicalToPhys(123, 45, px, py);
            check(px == 123 && py == 45, "virtual API: logicalToPhys identity on OFF");

            // letterboxColor は no-op で良いが、クラッシュせず呼べること
            auto& ref = scrOff.letterboxColor(0, 128, 255);
            check(&ref == &scrOff, "virtual API: letterboxColor returns *this for chaining");

            allPassed &= scrOff.valid();
        }

        // (2) 仮想画面 ON: 物理 = 論理 = 320x240 のとき恒等になることを確認
        //     hidden mode では client サイズが width/height で初期化されるため
        //     uniform スケール = 1.0, offset = 0 となる。
        {
            auto scrOn = screen({
                .width = 320, .height = 240, .mode = screen_hide,
                .virtual_resolution = true,
            });
            check(scrOn.valid(), "virtual API: ON screen created");

            // letterboxColor は範囲外もクランプして受理されること
            scrOn.letterboxColor(255, 0, 0);
            scrOn.letterboxColor(-10, 999, 64);  // クランプされる想定（クラッシュしないこと）
            check(true, "virtual API: letterboxColor accepts out-of-range without crash");

            // 物理 → 論理 → 物理 の往復一貫性
            // hidden mode で physClient = (width,height) のとき scale=1.0 / offset=0
            // → 往復差分は 0 で安定する。
            int totalDiff = 0;
            const int samples[][2] = { {0,0}, {1,1}, {160,120}, {319,239}, {50,200} };
            for (auto& s : samples) {
                int lx = 0, ly = 0;
                scrOn.physToLogical(s[0], s[1], lx, ly);
                int rx = 0, ry = 0;
                scrOn.logicalToPhys(lx, ly, rx, ry);
                totalDiff += absi(rx - s[0]) + absi(ry - s[1]);
            }
            check(totalDiff == 0,
                  "virtual API: phys->log->phys round-trip is exact at scale=1.0");

            // 逆向き: 論理 → 物理 → 論理 の往復一貫性
            int totalDiff2 = 0;
            const int samples2[][2] = { {0,0}, {10,10}, {160,120}, {319,239} };
            for (auto& s : samples2) {
                int px = 0, py = 0;
                scrOn.logicalToPhys(s[0], s[1], px, py);
                int lx = 0, ly = 0;
                scrOn.physToLogical(px, py, lx, ly);
                totalDiff2 += absi(lx - s[0]) + absi(ly - s[1]);
            }
            check(totalDiff2 == 0,
                  "virtual API: log->phys->log round-trip is exact at scale=1.0");

            allPassed &= scrOn.valid();
        }

        // (3) 無効ハンドルでも安全に呼べる（恒等フォールバック / no-op）
        {
            Screen invalid;
            int lx = 7, ly = 9;
            invalid.physToLogical(11, 13, lx, ly);
            check(lx == 11 && ly == 13, "virtual API: physToLogical fallback on invalid handle");
            int px = 0, py = 0;
            invalid.logicalToPhys(11, 13, px, py);
            check(px == 11 && py == 13, "virtual API: logicalToPhys fallback on invalid handle");
            invalid.letterboxColor(64, 64, 64);
            check(true, "virtual API: letterboxColor no-op on invalid handle");
        }

        return allPassed;
    }

    // ============================================================
    // 仮想画面リサイズ回帰テスト
    // ------------------------------------------------------------
    // virtual_resolution=true なウィンドウに対し Screen::width(physW, physH)
    // で論理バッファサイズを超える物理クライアントサイズを要求した際に、
    // OS の WM_GETMINMAXINFO クランプによって SetWindowPos が論理サイズへ
    // 切り詰められないことを保証する。Window.cpp の WM_GETMINMAXINFO ハンドラ
    // が m_virtualEnabled を考慮するよう修正したことの回帰検証。
    //
    // 実装メモ:
    //   - screen_hide で生成した不可視ウィンドウでも CreateWindowExW 後に
    //     SetWindowPos は WM_GETMINMAXINFO を発火するため、本バグは
    //     hidden mode でも決定的に再現／検証可能（test artifact / 作業ログ参照）。
    //   - Screen は HWND を直接公開しないため、scr.select() で current 化し
    //     hsppp::hwnd() 経由で取得した HWND に GetClientRect を直接適用する。
    // ============================================================
    bool test_virtual_resize_no_max_track_clamp() {
        bool allPassed = true;

        constexpr int kBufW    = 640;
        constexpr int kBufH    = 480;
        constexpr int kPhysW   = 1088;   // バッファ 640 を超える物理サイズ
        constexpr int kPhysH   = 816;

        auto scrOn = screen({
            .width = kBufW, .height = kBufH,
            .mode = screen_hide,
            .title = "HSPPP T16 Regression (virtual)",
            .virtual_resolution = true,
        });
        check(scrOn.valid(), "T16: virtual=ON hidden screen created");

        // 旧実装では SetWindowPos が ptMaxTrackSize により 640x480 相当へ
        // 切り詰められ、GetClientRect も論理バッファサイズに留まっていた。
        scrOn.width(kPhysW, kPhysH);
        scrOn.select();

        HWND hOn = reinterpret_cast<HWND>(static_cast<intptr_t>(hwnd()));
        check(hOn != nullptr, "T16: hwnd() returns non-null for virtual=ON screen");

        RECT rcOn{};
        BOOL okOn = ::GetClientRect(hOn, &rcOn);
        check(okOn != 0, "T16: GetClientRect succeeds on virtual=ON HWND");

        const int physWOn = rcOn.right - rcOn.left;
        const int physHOn = rcOn.bottom - rcOn.top;
        check(physWOn == kPhysW,
              "T16 regression: virtual=ON physical client width follows requested size beyond buffer");
        check(physHOn == kPhysH,
              "T16 regression: virtual=ON physical client height follows requested size beyond buffer");
        if (!(physWOn == kPhysW && physHOn == kPhysH)) {
            allPassed = false;
        }

        return allPassed;
    }

    // ============================================================
    // LogicalRenderContext (hsppp_testing module) 白箱テスト
    // DPI 100/150/200% × 仮想 ON/OFF × 3 API (compute_present_mapping
    //   / phys_to_logical / logical_to_phys) の純関数シム経由検証。
    // 直接の Internal-Public 分離破壊を伴わず、export された自由関数シムのみ使用。
    // ============================================================
    static inline bool lrc_nearly_eq(float a, float b) {
        float d = a - b;
        if (d < 0.0f) d = -d;
        return d <= 0.001f;
    }

    static inline bool lrc_mapping_equal(const ::hsppp::testing::PresentMappingView& m,
                                         float scale, float ox, float oy, float dw, float dh) {
        return lrc_nearly_eq(m.scale,   scale)
            && lrc_nearly_eq(m.offsetX, ox)
            && lrc_nearly_eq(m.offsetY, oy)
            && lrc_nearly_eq(m.destW,   dw)
            && lrc_nearly_eq(m.destH,   dh);
    }

    // --- compute_present_mapping: 6 ケース ---
    // 仮想 OFF: physClient = logical * (DPI/96) 前提で scale = DPI/96, offset=(0,0)
    void test_lrc_cpm_dpi100_virt_off() {
        auto m = ::hsppp::testing::compute_present_mapping(640, 480, 640, 480, 96u, false);
        check(lrc_mapping_equal(m, 1.0f, 0.0f, 0.0f, 640.0f, 480.0f),
              "test_lrc_cpm_dpi100_virt_off");
    }
    void test_lrc_cpm_dpi150_virt_off() {
        auto m = ::hsppp::testing::compute_present_mapping(640, 480, 960, 720, 144u, false);
        check(lrc_mapping_equal(m, 1.5f, 0.0f, 0.0f, 960.0f, 720.0f),
              "test_lrc_cpm_dpi150_virt_off");
    }
    void test_lrc_cpm_dpi200_virt_off() {
        auto m = ::hsppp::testing::compute_present_mapping(640, 480, 1280, 960, 192u, false);
        check(lrc_mapping_equal(m, 2.0f, 0.0f, 0.0f, 1280.0f, 960.0f),
              "test_lrc_cpm_dpi200_virt_off");
    }
    // 仮想 ON: uniform = min(sx, sy), 余白を中央寄せ
    void test_lrc_cpm_dpi100_virt_on_letterbox_x() {
        // log 640x480, phys 1024x600 → sx=1.6, sy=1.25 → scale=1.25
        // destW=800, destH=600, offsetX=(1024-800)/2=112, offsetY=0
        auto m = ::hsppp::testing::compute_present_mapping(640, 480, 1024, 600, 96u, true);
        check(lrc_mapping_equal(m, 1.25f, 112.0f, 0.0f, 800.0f, 600.0f),
              "test_lrc_cpm_dpi100_virt_on_letterbox_x");
    }
    void test_lrc_cpm_dpi150_virt_on_letterbox_x() {
        // log 640x480, phys 1920x1080 → sx=3.0, sy=2.25 → scale=2.25
        // destW=1440, destH=1080, offsetX=(1920-1440)/2=240, offsetY=0
        auto m = ::hsppp::testing::compute_present_mapping(640, 480, 1920, 1080, 144u, true);
        check(lrc_mapping_equal(m, 2.25f, 240.0f, 0.0f, 1440.0f, 1080.0f),
              "test_lrc_cpm_dpi150_virt_on_letterbox_x");
    }
    void test_lrc_cpm_dpi200_virt_on_letterbox_y() {
        // log 640x480, phys 1280x1024 → sx=2.0, sy=2.1333 → scale=2.0
        // destW=1280, destH=960, offsetX=0, offsetY=(1024-960)/2=32
        auto m = ::hsppp::testing::compute_present_mapping(640, 480, 1280, 1024, 192u, true);
        check(lrc_mapping_equal(m, 2.0f, 0.0f, 32.0f, 1280.0f, 960.0f),
              "test_lrc_cpm_dpi200_virt_on_letterbox_y");
    }

    // --- phys_to_logical: 3 ケース ---
    void test_lrc_p2l_dpi100_virt_off() {
        int lx = -1, ly = -1;
        ::hsppp::testing::phys_to_logical(640, 480, 640, 480, 96u, false, 320, 240, lx, ly);
        check(lx == 320 && ly == 240, "test_lrc_p2l_dpi100_virt_off");
    }
    void test_lrc_p2l_dpi150_virt_off() {
        // scale=1.5: phys(450,75) → (300,50)
        int lx = -1, ly = -1;
        ::hsppp::testing::phys_to_logical(640, 480, 960, 720, 144u, false, 450, 75, lx, ly);
        check(lx == 300 && ly == 50, "test_lrc_p2l_dpi150_virt_off");
    }
    void test_lrc_p2l_dpi150_virt_on_letterbox() {
        // scale=2.25, offsetX=240, offsetY=0; phys(465,225) → ((465-240)/2.25, 225/2.25) = (100,100)
        int lx = -1, ly = -1;
        ::hsppp::testing::phys_to_logical(640, 480, 1920, 1080, 144u, true, 465, 225, lx, ly);
        check(lx == 100 && ly == 100, "test_lrc_p2l_dpi150_virt_on_letterbox");
    }

    // --- logical_to_phys: 3 ケース ---
    void test_lrc_l2p_dpi100_virt_off() {
        int px = -1, py = -1;
        ::hsppp::testing::logical_to_phys(640, 480, 640, 480, 96u, false, 320, 240, px, py);
        check(px == 320 && py == 240, "test_lrc_l2p_dpi100_virt_off");
    }
    void test_lrc_l2p_dpi200_virt_off() {
        // scale=2.0: log(100,100) → phys(200,200)
        int px = -1, py = -1;
        ::hsppp::testing::logical_to_phys(640, 480, 1280, 960, 192u, false, 100, 100, px, py);
        check(px == 200 && py == 200, "test_lrc_l2p_dpi200_virt_off");
    }
    void test_lrc_l2p_dpi150_virt_on_letterbox() {
        // scale=2.25, offsetX=240; log(100,100) → (240+225, 0+225) = (465, 225)
        int px = -1, py = -1;
        ::hsppp::testing::logical_to_phys(640, 480, 1920, 1080, 144u, true, 100, 100, px, py);
        check(px == 465 && py == 225, "test_lrc_l2p_dpi150_virt_on_letterbox");
    }

    // --- 往復一貫性 (logical → phys → logical) ---
    void test_lrc_roundtrip_dpi200_virt_on_letterbox_y() {
        // log 640x480, phys 1280x1024 (DPI200 virt ON): scale=2.0, offsetY=32
        // log(50,80) → phys(0+100, 32+160) = (100, 192) → back to (50,80)
        int px = -1, py = -1;
        ::hsppp::testing::logical_to_phys(640, 480, 1280, 1024, 192u, true, 50, 80, px, py);
        int lx = -1, ly = -1;
        ::hsppp::testing::phys_to_logical(640, 480, 1280, 1024, 192u, true, px, py, lx, ly);
        check(px == 100 && py == 192 && lx == 50 && ly == 80,
              "test_lrc_roundtrip_dpi200_virt_on_letterbox_y");
    }

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
        test_current_screen_and_window_lifecycle();
        test_hsp_gmode_pixels();
        test_hsp_gui_object_state();
        test_hsp_image_id_commands();
        test_hsp_mouse_coordinate_spaces();
        test_font_functions();
        test_title_width_functions();
        test_method_chaining();
        test_input_functions();
        test_string_functions_runtime();
        test_note_and_sendmsg();
        test_async_submachine_runtime();
        test_dpi_changed_target_bitmap_rebind();
        test_anchor_layout();
        test_virtual_screen_public_api();
        test_virtual_resize_no_max_track_clamp();

        // LogicalRenderContext 白箱テスト (hsppp_testing module 経由)
        test_lrc_cpm_dpi100_virt_off();
        test_lrc_cpm_dpi150_virt_off();
        test_lrc_cpm_dpi200_virt_off();
        test_lrc_cpm_dpi100_virt_on_letterbox_x();
        test_lrc_cpm_dpi150_virt_on_letterbox_x();
        test_lrc_cpm_dpi200_virt_on_letterbox_y();
        test_lrc_p2l_dpi100_virt_off();
        test_lrc_p2l_dpi150_virt_off();
        test_lrc_p2l_dpi150_virt_on_letterbox();
        test_lrc_l2p_dpi100_virt_off();
        test_lrc_l2p_dpi200_virt_off();
        test_lrc_l2p_dpi150_virt_on_letterbox();
        test_lrc_roundtrip_dpi200_virt_on_letterbox_y();

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
