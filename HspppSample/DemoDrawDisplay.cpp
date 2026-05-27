// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppSample/DemoDrawDisplay.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 表示系デモ
//   - HiDPI awareness / WM_DPICHANGED ログ / 現在 DPI 表示
//   - 仮想画面 (screen_mode_virtual=128) ON/OFF 比較
//   - アンカー基準レイアウト (anchor_box / anchor_pos / AnchorRect)
//
// 設計方針:
//   200% DPI 実機モニタを所持しない環境でも「相対比較」できることを優先する。
//     - HiDPI: 起動時の推定 DPI（ginfo(12)*96/ginfo(26)）と WM_DPICHANGED 受信ログを併記
//     - 仮想画面: 同一描画を ON/OFF の 2 サブウィンドウに並置。手動リサイズで letterbox/scale を視認
//     - アンカー: 1 / 3 / 5 キーで疑似バッファサイズを切替え、anchor 矩形の位置追従を可視化
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
import <format>;
import <string>;
using namespace hsppp;

// ═══════════════════════════════════════════════════════════════════
// グローバル状態（DemoDrawDisplay 専用 / 実体は UserApp.cpp）
// ═══════════════════════════════════════════════════════════════════

// 仮想画面比較用サブウィンドウ（hspMain で生成）
extern Screen g_virtOffScreen;
extern Screen g_virtOnScreen;
extern bool   g_displaySubVisible;   // 仮想画面比較ウィンドウが表示中か

// HiDPI 状態
extern int         g_dpiChangeCount;   // WM_DPICHANGED 受信回数
extern int         g_dpiLastReported;  // 最後に通知された DPI (0=未受信)
extern std::string g_dpiChangeLog;     // WM_DPICHANGED ログ（末尾 8 件）

// アンカーデモ用
extern int g_anchorPresetIndex;        // 0=320x240, 1=480x320, 2=600x420

namespace {

constexpr int kAnchorPresets[3][2] = {
    {320, 240},
    {480, 320},
    {600, 420},
};

// 仮想画面比較ウィンドウへの共通テストパターン描画
void drawVirtualTestPattern(Screen& w, const char* label) {
    w.select();
    w.redraw(0);
    w.cls(4);

    // 色付きグリッド（16x12）
    const int gw = 320 / 16;
    const int gh = 240 / 12;
    for (int y = 0; y < 12; ++y) {
        for (int x = 0; x < 16; ++x) {
            int r = (x * 16) & 0xFF;
            int g = (y * 24) & 0xFF;
            int b = ((x + y) * 12) & 0xFF;
            w.color(r, g, b).boxf(x * gw, y * gh, (x + 1) * gw - 1, (y + 1) * gh - 1);
        }
    }

    // 細線（ピクセル整合の視認用）
    w.color(255, 255, 255);
    for (int i = 0; i < 320; i += 8) {
        w.line(i, 0, i, 4);
    }
    for (int j = 0; j < 240; j += 8) {
        w.line(0, j, 4, j);
    }

    // ラベル
    w.color(0, 0, 0).boxf(0, 0, 320, 18);
    w.color(255, 255, 0).pos(6, 2);
    w.font("MS Gothic", 12, 1);
    w.mes(label);

    w.redraw(1);
}

void ensureVirtualSubWindows(bool visible) {
    if (visible == g_displaySubVisible) return;
    if (g_virtOffScreen.valid()) {
        gsel(g_virtOffScreen.id(), visible ? 1 : -1);
    }
    if (g_virtOnScreen.valid()) {
        gsel(g_virtOnScreen.id(), visible ? 1 : -1);
    }
    g_displaySubVisible = visible;

    if (visible) {
        // 初回 / 再表示時に共通パターンを再描画しておく
        if (g_virtOffScreen.valid()) drawVirtualTestPattern(g_virtOffScreen, "screen_mode_virtual = OFF");
        if (g_virtOnScreen.valid())  drawVirtualTestPattern(g_virtOnScreen,  "screen_mode_virtual = ON  (vscale_linear)");
    }
}

void drawHiDPIDemo(Screen& win) {
    const int initLogicalW  = ginfo(ginfo_type_sx);     // 26: 初期化 X (= screen() 指定の論理 px)
    const int initLogicalH  = ginfo(ginfo_type_sy);     // 27
    const int clientPhysW   = ginfo(ginfo_type_mesx);   // 12: クライアント X (仮想画面 OFF 時は物理 px)
    const int clientPhysH   = ginfo(ginfo_type_mesy);   // 13
    const int desktopW      = ginfo(ginfo_type_dispx);  // 20
    const int desktopH      = ginfo(ginfo_type_dispy);  // 21
    const int mouseLogicalX = ginfo(ginfo_type_mx);     // 0  (HSP 仕様: 論理 px)
    const int mouseLogicalY = ginfo(ginfo_type_my);     // 1

    // 推定 DPI: 物理クライアント幅 / 論理初期化幅 × 96
    int estimatedDpi = 96;
    if (initLogicalW > 0) {
        estimatedDpi = clientPhysW * 96 / initLogicalW;
    }

    win.color(0, 0, 0).pos(20, 85);
    win.font("MS Gothic", 14, 1);
    win.mes("[HiDPI] DPI awareness / WM_DPICHANGED 目視デモ");

    win.font("MS Gothic", 12, 0);
    win.color(0, 0, 0).pos(20, 110);
    win.mes(std::format("  初期化サイズ (論理 px):     {} x {}", initLogicalW, initLogicalH));
    win.pos(20, 128);
    win.mes(std::format("  現クライアント (物理 px):   {} x {}", clientPhysW, clientPhysH));
    win.pos(20, 146);
    win.mes(std::format("  推定 DPI (初期表示時):       {}  (= 100% は 96, 150% は 144, 200% は 192)", estimatedDpi));
    win.pos(20, 164);
    win.mes(std::format("  デスクトップ全体:           {} x {}", desktopW, desktopH));
    win.pos(20, 182);
    win.mes(std::format("  マウス座標 (論理 px):       ({}, {})", mouseLogicalX, mouseLogicalY));

    win.color(0, 0, 128).pos(20, 210);
    win.mes(std::format("WM_DPICHANGED 受信回数: {}   最終通知 DPI: {}",
                        g_dpiChangeCount,
                        g_dpiLastReported == 0 ? std::string("（未受信）") : std::to_string(g_dpiLastReported)));

    win.color(64, 64, 64).pos(20, 232);
    win.mes("WM_DPICHANGED ログ（末尾 8 件）:");

    // 黒背景のログ枠
    win.color(32, 32, 32).boxf(20, 250, 620, 380);
    win.color(0, 255, 0).pos(28, 256);
    win.font("MS Gothic", 11, 0);
    if (g_dpiChangeLog.empty()) {
        win.mes("(まだ受信していません。ウィンドウを別 DPI モニタへドラッグすると発火します)");
    } else {
        win.mes(g_dpiChangeLog);
    }

    win.font("MS Gothic", 11, 0);
    win.color(0, 0, 128).pos(20, 395);
    win.mes("操作: マウスを動かすと論理座標が更新 / 200% DPI モニタへドラッグで WM_DPICHANGED 発火");
    win.color(128, 0, 0).pos(20, 412);
    win.mes("注意: 物理画面の DPI が 96 でない場合、起動直後の『推定 DPI』が 96 以外になります。");
}

void drawVirtualScreenDemo(Screen& win) {
    ensureVirtualSubWindows(true);

    win.color(0, 0, 0).pos(20, 85);
    win.font("MS Gothic", 14, 1);
    win.mes("[仮想画面] screen_mode_virtual = 128 の ON/OFF 比較デモ");

    win.font("MS Gothic", 12, 0);
    win.color(0, 0, 0).pos(20, 112);
    win.mes("  左サブウィンドウ : 仮想画面 OFF（物理 px = 論理 px、リサイズはバッファでクランプ）");
    win.pos(20, 130);
    win.mes("  右サブウィンドウ : 仮想画面 ON  （論理→物理 自動拡縮、vscale_linear）");

    win.color(0, 0, 128).pos(20, 158);
    win.mes(std::format("  サブウィンドウ表示状態:  {}",
                        g_displaySubVisible ? "表示中" : "非表示"));

    win.color(64, 64, 64).pos(20, 180);
    win.mes("操作:");
    win.pos(40, 198);
    win.mes("  v : 仮想画面比較サブウィンドウの表示 / 非表示トグル");
    win.pos(40, 216);
    win.mes("  r : サブウィンドウへテストパターン再描画");

    win.color(0, 0, 0).pos(20, 244);
    win.mes("見方:");
    win.pos(40, 262);
    win.mes("  両ウィンドウを手動でドラッグリサイズすると差分が視認できます。");
    win.pos(40, 280);
    win.mes("    OFF 側: 描画は 320x240 物理 px のまま、超過分は灰色帯 / クランプ");
    win.pos(40, 298);
    win.mes("    ON  側: 内容全体が物理クライアントに合わせ自動拡縮（letterbox 動作）");

    // 論理↔物理 変換ログ（マウス座標）
    const int mouseLogicalX = ginfo(ginfo_type_mx);
    const int mouseLogicalY = ginfo(ginfo_type_my);
    const int clientPhysW   = ginfo(ginfo_type_mesx);
    const int clientPhysH   = ginfo(ginfo_type_mesy);
    const int initLogicalW  = ginfo(ginfo_type_sx);
    const int initLogicalH  = ginfo(ginfo_type_sy);

    win.color(0, 64, 0).pos(20, 326);
    win.mes(std::format("論理↔物理 変換ログ (メインウィンドウ):"));
    win.pos(40, 344);
    win.mes(std::format("  ginfo(mx,my)= ({}, {}) 論理 px",
                        mouseLogicalX, mouseLogicalY));
    win.pos(40, 362);
    win.mes(std::format("  クライアント物理 px= {}x{}  / 初期化論理 px= {}x{}",
                        clientPhysW, clientPhysH, initLogicalW, initLogicalH));

    win.color(128, 0, 0).pos(20, 395);
    win.mes("注意: 仮想画面 ON/OFF は生成時パラメータです（screen() の virtual_resolution）。");
    win.color(128, 0, 0).pos(20, 412);
    win.mes("       本デモは『起動時に両方生成済の 2 ウィンドウ』を比較表示します。");
}

void drawAnchorDemo(Screen& win) {
    // メイン画面でのアンカー実例:
    //   現在のバッファ (= 640x480 論理 px) に対し anchor_box を使う
    win.color(0, 0, 0).pos(20, 85);
    win.font("MS Gothic", 14, 1);
    win.mes("[アンカー] AnchorRect / anchor_box / anchor_pos 目視デモ");

    win.font("MS Gothic", 12, 0);
    win.color(64, 64, 64).pos(20, 110);
    win.mes("(a) メインバッファ (640x480) に対するアンカー矩形:");

    // 9 アンカー組合せの矩形を本物の anchor_box で描画
    struct A { int h; int v; int r; int g; int b; const char* name; };
    constexpr A kAnchors[] = {
        { ah_left,   av_top,    255,   0,   0, "LT" },
        { ah_center, av_top,      0, 200,   0, "CT" },
        { ah_right,  av_top,      0,   0, 255, "RT" },
        { ah_left,   av_middle, 255, 128,   0, "LM" },
        { ah_center, av_middle, 255, 255,   0, "CM" },
        { ah_right,  av_middle,   0, 255, 255, "RM" },
        { ah_left,   av_bottom, 255,   0, 255, "LB" },
        { ah_center, av_bottom, 128, 128, 128, "CB" },
        { ah_right,  av_bottom,  64, 128, 192, "RB" },
    };
    constexpr int boxW = 60;
    constexpr int boxH = 28;
    for (const auto& a : kAnchors) {
        // 描画領域 (20,130) - (620,460) 内に押し込めるため offset で内側オフセット
        int ox = (a.h == ah_left)  ?  20 : (a.h == ah_right)  ? -20 : 0;
        int oy = (a.v == av_top)   ? 130 : (a.v == av_bottom) ? -20 : 55; // 中段は +55 で 130～ 領域中央寄り
        win.color(a.r, a.g, a.b).anchor_box(a.h, a.v, ox, oy, boxW, boxH);
        win.color(0, 0, 0).anchor_pos(a.h, a.v, ox + 4, oy + 6);
        win.font("MS Gothic", 11, 1);
        win.mes(a.name);
    }

    // (b) 疑似バッファサイズ切替: AnchorRect::resolve を使い、別矩形枠内に並置
    win.font("MS Gothic", 12, 0);
    int presetW = kAnchorPresets[g_anchorPresetIndex][0];
    int presetH = kAnchorPresets[g_anchorPresetIndex][1];

    win.color(64, 64, 64).pos(20, 410);
    win.mes(std::format("(b) 疑似バッファ {}x{} に対する AnchorRect 解決結果（1 / 3 / 5 キーで切替）",
                        presetW, presetH));
    // 枠
    constexpr int frameX = 250;
    constexpr int frameY = 430;
    const int frameW = (presetW * 200) / 600; // 最大幅 200px に正規化表示
    const int frameH = (presetH * 200) / 600;
    win.color(180, 180, 180).boxf(frameX, frameY, frameX + frameW, frameY + 1);
    win.color(180, 180, 180).boxf(frameX, frameY, frameX + 1,        frameY + frameH);
    win.color(180, 180, 180).boxf(frameX + frameW - 1, frameY, frameX + frameW, frameY + frameH);
    win.color(180, 180, 180).boxf(frameX, frameY + frameH - 1, frameX + frameW, frameY + frameH);

    // 4 角 + 中央のアンカー矩形（small）
    struct A2 { AnchorH h; AnchorV v; int r; int g; int b; };
    constexpr A2 kAnchors2[] = {
        { ah_left,   av_top,    255,   0,   0 },
        { ah_right,  av_top,      0,   0, 255 },
        { ah_left,   av_bottom, 255, 128,   0 },
        { ah_right,  av_bottom,   0, 200,   0 },
        { ah_center, av_middle, 255, 255,   0 },
    };
    for (const auto& a : kAnchors2) {
        AnchorRect rect{ a.h, a.v, 0, 0, 16, 10 };
        RectI r = rect.resolve(presetW, presetH);
        // 表示は presetW x presetH を frameW x frameH に縮小投影
        const int rx1 = frameX + (r.x1 * frameW) / presetW;
        const int ry1 = frameY + (r.y1 * frameH) / presetH;
        const int rx2 = frameX + (r.x2 * frameW) / presetW;
        const int ry2 = frameY + (r.y2 * frameH) / presetH;
        win.color(a.r, a.g, a.b).boxf(rx1, ry1, rx2, ry2);
    }

    win.font("MS Gothic", 11, 0);
    win.color(0, 0, 128).pos(20, 440);
    win.mes("(a) 実際のメインバッファに対する anchor_box");
    win.pos(20, 458);
    win.mes("(b) 疑似バッファ枠 (右側枠) 内で resolve した矩形を縮尺投影");
}

} // namespace

// ═══════════════════════════════════════════════════════════════════
// 公開エントリ
// ═══════════════════════════════════════════════════════════════════

void drawDisplayDemo(Screen& win) {
    // Virtual サブウィンドウは Virtual デモ以外では非表示にする
    if (g_displaySubVisible &&
        static_cast<DisplayDemo>(g_demoIndex) != DisplayDemo::Virtual) {
        ensureVirtualSubWindows(false);
        win.select();
    }

    switch (static_cast<DisplayDemo>(g_demoIndex)) {
    case DisplayDemo::HiDPI:
        drawHiDPIDemo(win);
        break;
    case DisplayDemo::Virtual:
        drawVirtualScreenDemo(win);
        break;
    case DisplayDemo::Anchor:
        drawAnchorDemo(win);
        break;
    default:
        break;
    }
}

void processDisplayAction(Screen& win) {
    if (isModifierKeyPressed()) return;

    switch (static_cast<DisplayDemo>(g_demoIndex)) {
    case DisplayDemo::HiDPI:
        // 現状アクションなし（DPI 表示は常時更新）
        break;
    case DisplayDemo::Virtual:
        if (getkey('V')) {
            ensureVirtualSubWindows(!g_displaySubVisible);
            g_actionLog = std::format("仮想画面比較サブウィンドウを{}にしました",
                                      g_displaySubVisible ? "表示" : "非表示");
            await(180);
        }
        if (getkey('R')) {
            if (g_virtOffScreen.valid()) drawVirtualTestPattern(g_virtOffScreen, "screen_mode_virtual = OFF");
            if (g_virtOnScreen.valid())  drawVirtualTestPattern(g_virtOnScreen,  "screen_mode_virtual = ON  (vscale_linear)");
            win.select();
            g_actionLog = "サブウィンドウへテストパターン再描画";
            await(180);
        }
        break;
    case DisplayDemo::Anchor:
        if (getkey('1')) { g_anchorPresetIndex = 0; await(180); }
        else if (getkey('3')) { g_anchorPresetIndex = 1; await(180); }
        else if (getkey('5')) { g_anchorPresetIndex = 2; await(180); }
        break;
    default:
        break;
    }
}

// デモ離脱時の後始末（UserApp.cpp から onDemoChanged で呼ぶ）
void onDisplayDemoLeft() {
    if (g_displaySubVisible) {
        ensureVirtualSubWindows(false);
    }
}
