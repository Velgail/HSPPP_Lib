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
extern int g_virtInterpIndex;          // gmode_interp プリセット index (0=nearest,1=linear,2=aniso)

namespace {

Screen g_virtualRasterSrc;
bool   g_virtualRasterSrcReady = false;

// 物理サイズプリセット: (width, height, label)
struct PhysPreset { int w; int h; const char* label; };
constexpr PhysPreset kPhysPresets[] = {
    {1920,1080, "1920x1080 (FHD, scale=2.25, dest=1440x1080)" },
    {2560,1440, "2560x1440 (QHD, scale=3.0,  dest=1920x1440)"  },
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
void ensureVirtualRasterSource() {
    if (g_virtualRasterSrcReady && g_virtualRasterSrc.valid()) return;
    g_virtualRasterSrc = buffer({ .width = 64, .height = 64 });
    if (!g_virtualRasterSrc.valid()) return;

    g_virtualRasterSrc.select();
    g_virtualRasterSrc.redraw(0);
    g_virtualRasterSrc.cls(4);

    // ラスタ転送確認用: 2x2 px チェッカー + 斜線（補間差が出やすい）
    for (int y = 0; y < 64; y += 2) {
        for (int x = 0; x < 64; x += 2) {
            const bool on = ((x / 2) + (y / 2)) % 2 == 0;
            g_virtualRasterSrc.color(on ? 255 : 20, on ? 255 : 20, on ? 255 : 20)
                .boxf(x, y, x + 1, y + 1);
        }
    }
    g_virtualRasterSrc.color(255, 0, 0).line(0, 0, 63, 63);
    g_virtualRasterSrc.color(0, 255, 255).line(63, 0, 0, 63);
    g_virtualRasterSrc.redraw(1);
    g_virtualRasterSrcReady = true;
}

void drawScalingTestPattern(Screen& w) {
    ensureVirtualRasterSource();

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
    w.font("MS Gothic", 10, 0);
    w.color(255, 255, 255).pos(430, 6);
    w.mes("font10 sample");

    // 線幅スケール可視化（gline_width API による真の論理 px 線幅 / R-D 検証）
    w.color(0, 0, 0).boxf(0, 424, 640, 480);
    w.font("MS Gothic", 11, 0);
    w.color(255, 255, 255).pos(8, 428);
    w.mes("線幅スケール (gline_width) 確認: 論理 1/2/4/8 px（DPI / 仮想倍率で物理 px も拡大）");
    w.color(255, 64, 64); gline_width(1.0); w.line(20, 450, 220, 450);
    w.color(64, 255, 64); gline_width(2.0); w.line(20, 458, 220, 458);
    w.color(64, 64, 255); gline_width(4.0); w.line(20, 468, 220, 468);
    w.color(255, 200, 64); gline_width(8.0); w.line(20, 478, 220, 478);
    gline_width(1.0);  // 後続描画のため既定値復帰（HSP3 互換 / R-D 後方互換）
    w.color(255, 255, 255).pos(228, 447); w.mes("1px");
    w.pos(228, 455); w.mes("2px");
    w.pos(228, 465); w.mes("4px");
    w.pos(228, 475); w.mes("8px");

    // ラスタ転送（gcopy / gzoom）可視化 + gmode_interp 切替効果 (R-C 検証)
    if (g_virtualRasterSrc.valid()) {
        gmode_interp(g_virtInterpIndex);   // 現在のラスタ補間モードを current surface に反映
        w.gmode(0, 64, 64);
        w.pos(360, 300);
        w.gcopy(g_virtualRasterSrc.id(), 0, 0, 64, 64);
        w.pos(440, 300);
        // mode = -1 -> サーフェスの gmode_interp 設定を使用 (hsppp_copy.inl §358)
        w.gzoom(160, 160, g_virtualRasterSrc.id(), 0, 0, 64, 64, -1);
        w.color(255, 255, 255).pos(356, 278);
        w.font("MS Gothic", 11, 0);
        static const char* kInterpLabel[] = { "nearest", "linear", "aniso" };
        const int interpIdx = (g_virtInterpIndex >= 0 && g_virtInterpIndex < 3) ? g_virtInterpIndex : 1;
        w.mes(std::format("ラスタ転送 gcopy(等倍) + gzoom(2.5倍): interp = {} (I キーで切替)",
                          kInterpLabel[interpIdx]));
    }

    // フォント品質サンプル（R-B: DWrite ネイティブ HiDPI / 多サイズ並置で目視確認）
    w.color(0, 0, 0).boxf(0, 220, 340, 280);
    w.color(255, 255, 255).pos(8, 222);
    w.font("MS Gothic", 10, 0); w.mes("10px Font HiDPI quality 漢字 ABC");
    w.font("MS Gothic", 14, 0); w.mes("14px Font HiDPI quality 漢字 ABC");
    w.font("MS Gothic", 18, 1); w.mes("18px Bold 漢字 ABC");
    w.font("MS Gothic", 11, 0);  // 後続描画のため既定値復帰

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
    // サイズ変更時は SwapChain が再生成されるため、都度パターンを再描画して反映を保証する。
    drawScalingTestPattern(g_virtScalingScreen);
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
    // TICKET-018 (R-A) 改修以降: ginfo_mesx/mesy は論理 px を返す。
    // 物理 px は (論理 px × DPI / 96) で逆算可能。基準 DPI 96 を仮定 -> WM_DPICHANGED 受信値で補正。
    const int clientLogicalW = ginfo(ginfo_type_mesx);   // 12: クライアント X (論理 px)
    const int clientLogicalH = ginfo(ginfo_type_mesy);   // 13
    const int desktopLogicalW = ginfo(ginfo_type_dispx); // 20: プライマリモニタ論理 px (HSP3 公式準拠)
    const int desktopLogicalH = ginfo(ginfo_type_dispy); // 21
    const int mouseLogicalX = ginfo(ginfo_type_mx);     // 0  (HSP 仕様: 論理 px)
    const int mouseLogicalY = ginfo(ginfo_type_my);     // 1

    // 推定 DPI: WM_DPICHANGED 受信履歴があれば最新値、無ければ 96 を既定。
    const int effectiveDpi = (g_dpiLastReported > 0) ? g_dpiLastReported : 96;
    const int estimatedClientPhysW = clientLogicalW * effectiveDpi / 96;
    const int estimatedClientPhysH = clientLogicalH * effectiveDpi / 96;

    win.color(0, 0, 0).pos(20, 85);
    win.font("MS Gothic", 14, 1);
    win.mes("[HiDPI] DPI awareness / WM_DPICHANGED 目視デモ");

    win.font("MS Gothic", 12, 0);
    win.color(0, 0, 0).pos(20, 110);
    win.mes(std::format("  初期化サイズ (論理 px):     {} x {}", initLogicalW, initLogicalH));
    win.pos(20, 128);
    win.mes(std::format("  現クライアント (論理 px):   {} x {}", clientLogicalW, clientLogicalH));
    win.pos(20, 146);
    win.mes(std::format("  実効 DPI (WM_DPICHANGED 由来 / 既定 96): {}  (100%=96, 150%=144, 200%=192)", effectiveDpi));
    win.pos(20, 164);
    win.mes(std::format("  推定 物理 px (= 論理×DPI/96):  {} x {}", estimatedClientPhysW, estimatedClientPhysH));
    win.pos(20, 182);
    win.mes(std::format("  デスクトップ (論理 px / Primary): {} x {}", desktopLogicalW, desktopLogicalH));
    win.pos(20, 200);
    win.mes(std::format("  マウス座標 (論理 px):       ({}, {})", mouseLogicalX, mouseLogicalY));

    win.color(0, 0, 128).pos(20, 228);
    win.mes(std::format("WM_DPICHANGED 受信回数: {}   最終通知 DPI: {}",
                        g_dpiChangeCount,
                        g_dpiLastReported == 0 ? std::string("（未受信）") : std::to_string(g_dpiLastReported)));

    win.color(64, 64, 64).pos(20, 250);
    win.mes("WM_DPICHANGED ログ（末尾 8 件）:");

    // 黒背景のログ枠
    win.color(32, 32, 32).boxf(20, 268, 620, 380);
    win.color(0, 255, 0).pos(28, 274);
    win.font("MS Gothic", 11, 0);
    if (g_dpiChangeLog.empty()) {
        win.mes("(まだ受信していません。ウィンドウを別 DPI モニタへドラッグすると発火します)");
    } else {
        win.mes(g_dpiChangeLog);
    }

    win.font("MS Gothic", 11, 0);
    win.color(0, 0, 128).pos(20, 395);
    win.mes("操作: マウスを動かすと論理座標が更新 / 別 DPI モニタへドラッグで WM_DPICHANGED 発火");
    win.color(128, 0, 0).pos(20, 412);
    win.mes("注: ginfo_mesx/mesy は TICKET-018 以降論理 px を返します（HSP3 仕様準拠）。");
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
    win.mes("  I           : ラスタ補間 (gmode_interp) nearest / linear / aniso 切替");
    win.pos(40, 302);
    win.mes("  F           : FHD基準 (1920x1080) へ即時切替");
    win.pos(40, 320);
    win.mes("  R           : 既定値へリセット (1920x1080 / linear / dark cyan / interp=linear)");

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
    int livePhysW = 0;
    int livePhysH = 0;
    float liveSx = 1.0f;
    float liveSy = 1.0f;
    float liveS = 1.0f;
    if (g_virtScalingScreen.valid()) {
        livePhysW = g_virtScalingScreen.width();
        livePhysH = g_virtScalingScreen.height();
        liveSx = static_cast<float>(livePhysW) / 640.0f;
        liveSy = static_cast<float>(livePhysH) / 480.0f;
        liveS = (liveSx < liveSy) ? liveSx : liveSy;
    }
    const int dx = rtLogX - logX;
    const int dy = rtLogY - logY;

    win.color(0, 64, 0).pos(20, 340);
    win.mes("論理 <-> 物理 変換 (Virtual サブウィンドウ上のマウス):");
    win.pos(40, 358);
    win.mes(std::format("  logical (Screen::mousex/mousey)            = ({:4}, {:4})", logX, logY));
    win.pos(40, 376);
    win.mes(std::format("  physical = logicalToPhys(logical)          = ({:4}, {:4})", physX, physY));
    win.pos(40, 394);
    win.mes(std::format("  round-trip logical = physToLogical(phys)   = ({:4}, {:4})", rtLogX, rtLogY));
    win.color((dx == 0 && dy == 0) ? 0 : 128, 0, (dx == 0 && dy == 0) ? 128 : 0).pos(40, 412);
    win.mes(std::format("  差分 (round-trip - original) = ({:+d}, {:+d})  -> {}",
                        dx, dy,
                        (dx == 0 && dy == 0) ? "OK (整合)" : "letterbox 領域 or rounding"));
    const float destW = 640.0f * liveS;
    const float destH = 480.0f * liveS;
    const float offX = (static_cast<float>(livePhysW) - destW) * 0.5f;
    const float offY = (static_cast<float>(livePhysH) - destH) * 0.5f;
    win.color(0, 0, 96).pos(20, 430);
    win.mes(std::format("実測サブ窓: {}x{}  scaleX={:.3f} scaleY={:.3f} uniform={:.3f}",
                        livePhysW, livePhysH, liveSx, liveSy, liveS));
    win.pos(20, 446);
    win.mes(std::format("描画先矩形: {:.1f}x{:.1f}  offset=({:.1f},{:.1f})  10px文字 -> {:.2f}px 相当",
                        destW, destH, offX, offY, 10.0f * liveS));

    // マルチウィンドウ + 異 DPI モニタ確認手順 (PM Q-5 (b) / 受け入れ条件 6)
    win.color(96, 0, 96).pos(20, 466);
    win.mes("【マルチウィンドウ異 DPI 確認手順】メイン窓と Virtual サブ窓を別 DPI モニタへ独立ドラッグし、");
    win.pos(20, 482);
    win.mes("HiDPI デモタブの WM_DPICHANGED ログで両ウィンドウの DPI 遷移を観察 (詳細: HspppSample/README.md)。");

    win.color(128, 0, 0).pos(20, 502);
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
        } else if (getkey('I') && !shift) {
            g_virtInterpIndex = (g_virtInterpIndex + 1) % kScaleModePresetCount;
            // gmode_interp はサーフェスメンバ -> 反映は drawScalingTestPattern 再呼出時に行う
            drawScalingTestPattern(g_virtScalingScreen);
            win.select();
            g_actionLog = std::format("gmode_interp -> {}", kScaleModePresets[g_virtInterpIndex].label);
            await(200);
        } else if (getkey('F') && !shift) {
            g_virtPresetIndex = 0;  // 1920x1080
            applyVirtualScalingPresets();
            win.select();
            g_actionLog = "物理サイズ -> 1920x1080 (FHD, scale=2.25)";
            await(200);
        } else if (getkey('R') && !shift) {
            g_virtPresetIndex      = 0;  // 1920x1080
            g_virtScaleModeIndex   = 1;
            g_virtLetterColorIndex = 1;
            g_virtInterpIndex      = 1;  // linear (gmode_interp 既定)
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
