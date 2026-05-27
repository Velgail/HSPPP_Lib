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

// 仮想画面スケーリングデモ用サブウィンドウ（hspMain で生成・論理 640x480 / virtual_resolution=true）
extern Screen g_virtScalingScreen;
extern bool   g_displaySubVisible;   // Virtual サブウィンドウが表示中か

// HiDPI 状態
extern int         g_dpiChangeCount;   // WM_DPICHANGED 受信回数
extern int         g_dpiLastReported;  // 最後に通知された DPI (0=未受信)
extern std::string g_dpiChangeLog;     // WM_DPICHANGED ログ（末尾 8 件）

// アンカー Playground 用 (実装は DemoDrawDisplayAnchor.cpp)
extern void drawAnchorDemo(Screen& mainWin);
extern void processAnchorAction(Screen& mainWin);
extern void onAnchorDemoLeft();
extern void ensureAnchorPlaygroundVisible(bool show);
extern bool g_anchorPlaygroundVisible;

// Virtual スケーリングデモ用
extern int g_virtPresetIndex;          // 物理サイズプリセット index
extern int g_virtScaleModeIndex;       // vscalemode プリセット index (0=nearest,1=linear,2=cubic)
extern int g_virtLetterColorIndex;     // letterbox 色プリセット index

namespace {

// 物理サイズプリセット: (width, height, label)
struct PhysPreset { int w; int h; const char* label; };
constexpr PhysPreset kPhysPresets[] = {
    { 640, 480, "640x480  (x1.0, 4:3)" },
    { 800, 600, "800x600  (x1.25, 4:3)" },
    { 960, 720, "960x720  (x1.5, 4:3)" },
    {1088, 816, "1088x816 (x1.7, 4:3)" },
    {1280, 960, "1280x960 (x2.0, 4:3)" },
    {1280, 720, "1280x720 (16:9)" },
    {1280, 800, "1280x800 (16:10)" },
    { 720, 720, "720x720  (1:1)" },
};
constexpr int kPhysPresetCount = sizeof(kPhysPresets) / sizeof(kPhysPresets[0]);

// vscalemode プリセット
struct ScaleModePreset { int mode; const char* label; };
constexpr ScaleModePreset kScaleModePresets[] = {
    { vscale_nearest, "nearest (vscale_nearest)" },
    { vscale_linear,  "linear  (vscale_linear)"  },
    { vscale_aniso,   "aniso   (vscale_aniso)"   },
};
constexpr int kScaleModePresetCount = sizeof(kScaleModePresets) / sizeof(kScaleModePresets[0]);

// letterbox 色プリセット
struct LetterColorPreset { int r; int g; int b; const char* label; };
constexpr LetterColorPreset kLetterColors[] = {
    {   0,   0,   0, "black (0,0,0)" },
    {   0,  96, 128, "dark cyan (0,96,128)" },
    { 200, 200, 200, "light gray (200,200,200)" },
};
constexpr int kLetterColorCount = sizeof(kLetterColors) / sizeof(kLetterColors[0]);

// 仮想 640x480 論理画面に描くテストパターン
void drawScalingTestPattern(Screen& w) {
    w.select();
    w.redraw(0);
    w.cls(4);

    // 色付きグリッド 16x12 (各 40x40)
    const int gw = 640 / 16;
    const int gh = 480 / 12;
    for (int y = 0; y < 12; ++y) {
        for (int x = 0; x < 16; ++x) {
            int r = (x * 16) & 0xFF;
            int g = (y * 24) & 0xFF;
            int b = ((x + y) * 12) & 0xFF;
            w.color(r, g, b).boxf(x * gw, y * gh, (x + 1) * gw - 1, (y + 1) * gh - 1);
        }
    }

    // 8 px 刻みの細線（補間モード差を視認）
    w.color(255, 255, 255);
    for (int i = 0; i < 640; i += 8) {
        w.line(i, 0, i, 6);
    }
    for (int j = 0; j < 480; j += 8) {
        w.line(0, j, 6, j);
    }

    // 中央十字 + 対角
    w.color(255, 255, 0).line(0, 0, 640, 480);
    w.color(255, 255, 0).line(640, 0, 0, 480);
    w.color(255, 255, 255).line(320, 0, 320, 480);
    w.color(255, 255, 255).line(0, 240, 640, 240);

    // ラベル帯
    w.color(0, 0, 0).boxf(0, 0, 640, 24);
    w.color(255, 255, 0).pos(8, 4);
    w.font("MS Gothic", 14, 1);
    w.mes("Virtual Scaling Demo (logical 640x480)");

    w.redraw(1);
}

// 現在の preset を Virtual サブウィンドウに適用する
void applyVirtualScalingPresets() {
    if (!g_virtScalingScreen.valid()) return;
    const PhysPreset& p = kPhysPresets[g_virtPresetIndex];
    g_virtScalingScreen.select();
    // 物理クライアントサイズを切り替え（virtual_resolution=true なのでバッファに依らずリサイズ可能）
    g_virtScalingScreen.width(p.w, p.h);
    // vscalemode 切替（current-surface 基準）
    vscalemode(kScaleModePresets[g_virtScaleModeIndex].mode);
    // letterbox 色切替
    const LetterColorPreset& lc = kLetterColors[g_virtLetterColorIndex];
    g_virtScalingScreen.letterboxColor(lc.r, lc.g, lc.b);
}

void ensureVirtualSubVisible(bool visible) {
    if (visible == g_displaySubVisible) return;
    if (g_virtScalingScreen.valid()) {
        gsel(g_virtScalingScreen.id(), visible ? 1 : -1);
    }
    g_displaySubVisible = visible;

    if (visible) {
        applyVirtualScalingPresets();
        drawScalingTestPattern(g_virtScalingScreen);
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
    ensureVirtualSubVisible(true);

    win.color(0, 0, 0).pos(20, 85);
    win.font("MS Gothic", 14, 1);
    win.mes("[仮想画面] Virtual Scaling デモ (実行時切替)");

    const PhysPreset&        pp = kPhysPresets[g_virtPresetIndex];
    const ScaleModePreset&   sp = kScaleModePresets[g_virtScaleModeIndex];
    const LetterColorPreset& cp = kLetterColors[g_virtLetterColorIndex];

    win.font("MS Gothic", 12, 0);
    win.color(0, 0, 0).pos(20, 112);
    win.mes("  サブウィンドウ : 論理 640x480 固定 / virtual_resolution=true");
    win.pos(20, 130);
    win.mes(std::format("  物理サイズ : ({:1}/{}) {}",
                        g_virtPresetIndex + 1, kPhysPresetCount, pp.label));
    win.pos(20, 148);
    win.mes(std::format("  vscalemode : ({}/{}) {}",
                        g_virtScaleModeIndex + 1, kScaleModePresetCount, sp.label));
    win.pos(20, 166);
    win.mes(std::format("  letterbox色: ({}/{}) {}",
                        g_virtLetterColorIndex + 1, kLetterColorCount, cp.label));

    win.color(64, 64, 64).pos(20, 194);
    win.mes("操作:");
    win.pos(40, 212);
    win.mes("  S / Shift+S : 物理サイズプリセット 次 / 前");
    win.pos(40, 230);
    win.mes("  M           : vscalemode 切替 (nearest / linear / aniso)");
    win.pos(40, 248);
    win.mes("  C           : letterbox 色切替");
    win.pos(40, 266);
    win.mes("  V           : サブウィンドウ 表示 / 非表示");
    win.pos(40, 284);
    win.mes("  R           : 既定値へリセット (640x480 / linear / dark cyan)");

    // 論理↔物理 round-trip 検証（サブウィンドウのマウス座標）
    int logX = 0, logY = 0;
    int physX = 0, physY = 0;
    int rtLogX = 0, rtLogY = 0;
    if (g_virtScalingScreen.valid()) {
        logX = g_virtScalingScreen.mousex();
        logY = g_virtScalingScreen.mousey();
        g_virtScalingScreen.logicalToPhys(logX, logY, physX, physY);
        g_virtScalingScreen.physToLogical(physX, physY, rtLogX, rtLogY);
    }
    const int dx = rtLogX - logX;
    const int dy = rtLogY - logY;

    win.color(0, 64, 0).pos(20, 314);
    win.mes("論理 <-> 物理 変換 (Virtual サブウィンドウ上のマウス):");
    win.pos(40, 332);
    win.mes(std::format("  logical (Screen::mousex/mousey)            = ({:4}, {:4})", logX, logY));
    win.pos(40, 350);
    win.mes(std::format("  physical = logicalToPhys(logical)          = ({:4}, {:4})", physX, physY));
    win.pos(40, 368);
    win.mes(std::format("  round-trip logical = physToLogical(phys)   = ({:4}, {:4})", rtLogX, rtLogY));
    win.color((dx == 0 && dy == 0) ? 0 : 128, 0, (dx == 0 && dy == 0) ? 128 : 0).pos(40, 386);
    win.mes(std::format("  差分 (round-trip - original) = ({:+d}, {:+d})  -> {}",
                        dx, dy,
                        (dx == 0 && dy == 0) ? "OK (整合)" : "letterbox 領域 or rounding"));

    win.color(128, 0, 0).pos(20, 412);
    win.mes("注: 物理サイズと論理 4:3 アスペクト比が一致しない場合、letterbox 帯が出現します。");
}

// 旧 drawAnchorDemo は DemoDrawDisplayAnchor.cpp の新 drawAnchorDemo (Anchor Playground 委譲版) に置換済。

} // namespace

// ═══════════════════════════════════════════════════════════════════
// 公開エントリ
// ═══════════════════════════════════════════════════════════════════

void drawDisplayDemo(Screen& win) {
    // Virtual サブウィンドウは Virtual デモ以外では非表示にする
    if (g_displaySubVisible &&
        static_cast<DisplayDemo>(g_demoIndex) != DisplayDemo::Virtual) {
        ensureVirtualSubVisible(false);
        win.select();
    }
    // Anchor Playground サブウィンドウも Anchor デモ以外では非表示にする
    if (g_anchorPlaygroundVisible &&
        static_cast<DisplayDemo>(g_demoIndex) != DisplayDemo::Anchor) {
        ensureAnchorPlaygroundVisible(false);
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
    switch (static_cast<DisplayDemo>(g_demoIndex)) {
    case DisplayDemo::HiDPI:
        // 現状アクションなし
        break;
    case DisplayDemo::Virtual: {
        // Ctrl/Alt/Win 押下中は無視（Shift は preset 前送り用に許可）
        if (hsppp::getkey(VK::CONTROL) || hsppp::getkey(VK::MENU) ||
            hsppp::getkey(VK::LWIN)    || hsppp::getkey(VK::RWIN)) return;
        const bool shift = hsppp::getkey(VK::SHIFT) != 0;
        if (getkey('S')) {
            if (shift) {
                g_virtPresetIndex = (g_virtPresetIndex + kPhysPresetCount - 1) % kPhysPresetCount;
            } else {
                g_virtPresetIndex = (g_virtPresetIndex + 1) % kPhysPresetCount;
            }
            applyVirtualScalingPresets();
            win.select();
            g_actionLog = std::format("物理サイズ -> {}", kPhysPresets[g_virtPresetIndex].label);
            await(200);
        } else if (getkey('M') && !shift) {
            g_virtScaleModeIndex = (g_virtScaleModeIndex + 1) % kScaleModePresetCount;
            applyVirtualScalingPresets();
            win.select();
            g_actionLog = std::format("vscalemode -> {}", kScaleModePresets[g_virtScaleModeIndex].label);
            await(200);
        } else if (getkey('C') && !shift) {
            g_virtLetterColorIndex = (g_virtLetterColorIndex + 1) % kLetterColorCount;
            applyVirtualScalingPresets();
            win.select();
            g_actionLog = std::format("letterboxColor -> {}", kLetterColors[g_virtLetterColorIndex].label);
            await(200);
        } else if (getkey('V') && !shift) {
            ensureVirtualSubVisible(!g_displaySubVisible);
            win.select();
            g_actionLog = std::format("Virtual サブウィンドウを{}にしました",
                                      g_displaySubVisible ? "表示" : "非表示");
            await(200);
        } else if (getkey('R') && !shift) {
            g_virtPresetIndex      = 0;
            g_virtScaleModeIndex   = 1;
            g_virtLetterColorIndex = 1;
            applyVirtualScalingPresets();
            drawScalingTestPattern(g_virtScalingScreen);
            win.select();
            g_actionLog = "Virtual プリセットをリセットしました";
            await(200);
        }
        break;
    }
    case DisplayDemo::Anchor:
        processAnchorAction(win);
        break;
    default:
        break;
    }
}

// デモ離脱時の後始末（UserApp.cpp から onDemoChanged で呼ぶ）
void onDisplayDemoLeft() {
    if (g_displaySubVisible) {
        ensureVirtualSubVisible(false);
    }
    if (g_anchorPlaygroundVisible) {
        onAnchorDemoLeft();
    }
}
