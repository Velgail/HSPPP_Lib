// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppSample/DemoDrawDisplayAnchor.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - アンカー Playground
//
//   表示系デモ (Display::Anchor) の実体。
//
//   訴求戦略:
//     - 専用サブウィンドウ "Anchor Playground" を物理リサイズ可能な状態で開き、
//       「アンカー有り (anchor_box / AnchorRect)」と「アンカー無し (ハードコード座標)」
//       を 1 サブウィンドウ内に 2 列並置する。
//     - 物理リサイズ枠ドラッグ + アスペクト比プリセット切替 (4:3 / 16:9 / 16:10 / 21:9 等)
//       で、アンカー有り側のレイアウトが破綻せず、アンカー無し側 (Fixed) が破綻していく様
//       を同フレーム同画面で目視比較できるようにする。
//     - リファレンスグリッドモードでは 9 アンカー全方位 (AnchorH × AnchorV) を同時表示し、
//       辺基準距離 (20px 等) がリサイズ中もそのまま維持されることをガイド矢印と実測値で示す。
//
//   重要:
//     - 仮想画面 (screen_mode_virtual=128) は使用しない。
//       仮想画面 ON だとバッファサイズが論理固定になり、物理リサイズが letterbox/scale
//       で吸収されてアンカーの追従が見えなくなるため。
//     - Fixed 側は「アンカー無しコードがリサイズで崩れる」ことを意図的に露出するための
//       アンチパターン例示。本サンプル外で真似されないようコード内コメントで明記する。
// ═══════════════════════════════════════════════════════════════════

#include "DemoState.h"
import hsppp;
import <format>;
import <string>;
using namespace hsppp;

// ═══════════════════════════════════════════════════════════════════
// グローバル状態（実体は UserApp.cpp）
// ═══════════════════════════════════════════════════════════════════

extern Screen g_anchorPlaygroundScreen;
extern bool   g_anchorPlaygroundVisible;
extern int    g_anchorAspectIndex;
extern bool   g_anchorShowFixed;
extern bool   g_anchorShowGrid;
extern bool   g_anchorShowGuides;

namespace {

// ───────────────────────────────────────────────────────────────────
// アスペクト比プリセット
// ───────────────────────────────────────────────────────────────────

struct AspectPreset { int w; int h; const char* label; };
constexpr AspectPreset kAnchorAspectPresets[] = {
    {  640, 480, "640x480  (4:3)"  },
    {  800, 600, "800x600  (4:3)"  },
    {  960, 540, "960x540  (16:9, 既定)" },
    {  960, 600, "960x600  (16:10)" },
    { 1120, 480, "1120x480 (21:9)" },
    { 1280, 720, "1280x720 (16:9)" },
};
constexpr int kAnchorAspectCount = sizeof(kAnchorAspectPresets) / sizeof(kAnchorAspectPresets[0]);
constexpr int kAnchorAspectDefaultIndex = 2;

// 既定値（R リセット用）
constexpr bool kAnchorShowFixedDefault  = true;
constexpr bool kAnchorShowGridDefault   = false;
constexpr bool kAnchorShowGuidesDefault = true;

// レイアウト定数
constexpr int kHeaderH = 28;
constexpr int kHotkeyBarH = 60;

// Fixed 側「設計時前提サイズ」(意図的な破綻露出のためのハードコード座標基準)
constexpr int kFixedDesignW = 480;
constexpr int kFixedDesignH = 452;  // 540 - 28(header) - 60(hotkey bar)

// ───────────────────────────────────────────────────────────────────
// ヘルパー
// ───────────────────────────────────────────────────────────────────

inline void drawArrow(Screen& w, int x1, int y1, int x2, int y2) {
    w.line(x2, y2, x1, y1);
    // 簡易矢じり (水平方向のみ厳密に追わず両端 3px の T 字)
    w.line(x2 - 3, y2 - 3, x2, y2);
    w.line(x2 - 3, y2 + 3, x2, y2);
    w.line(x2 + 3, y2 - 3, x2, y2);
    w.line(x2 + 3, y2 + 3, x2, y2);
}

inline RectI resolveInSub(AnchorH h, AnchorV v, int ox, int oy, int rw, int rh,
                          int subOx, int subOy, int subW, int subH) {
    AnchorRect rect{ h, v, ox, oy, rw, rh };
    RectI r = rect.resolve(subW, subH);
    return RectI{ r.x1 + subOx, r.y1 + subOy, r.x2 + subOx, r.y2 + subOy };
}

// ───────────────────────────────────────────────────────────────────
// Anchored 側 HUD 描画 (sub 領域 [subOx, subOy] origin / subW × subH)
//   サブ領域サイズに対する AnchorRect::resolve でリサイズに追従する。
// ───────────────────────────────────────────────────────────────────
void drawHudAnchored(Screen& w, int subOx, int subOy, int subW, int subH) {
    // 帯ラベル「ANCHORED (anchor_box)」
    w.color(20, 90, 20).boxf(subOx, subOy, subOx + subW, subOy + 18);
    w.color(255, 255, 255).pos(subOx + 8, subOy + 2);
    w.font("MS Gothic", 12, 1);
    w.mes("ANCHORED  (anchor_box / AnchorRect)");

    // 描画領域 (帯下〜下端)
    const int areaOy = subOy + 18;
    const int areaH  = subH  - 18;
    if (areaH <= 0 || subW <= 0) return;

    // LT: Score
    {
        RectI r = resolveInSub(ah_left, av_top, 20, 20, 140, 32, subOx, areaOy, subW, areaH);
        w.color(40, 80, 200).boxf(r.x1, r.y1, r.x2, r.y2);
        w.color(255, 255, 255).pos(r.x1 + 8, r.y1 + 8);
        w.font("MS Gothic", 13, 1);
        w.mes("Score: 01234");
    }
    // RT: Time
    {
        RectI r = resolveInSub(ah_right, av_top, -20, 20, 140, 32, subOx, areaOy, subW, areaH);
        w.color(40, 80, 200).boxf(r.x1, r.y1, r.x2, r.y2);
        w.color(255, 255, 255).pos(r.x1 + 8, r.y1 + 8);
        w.font("MS Gothic", 13, 1);
        w.mes("Time:  12:34");
    }
    // LB: Settings ボタン
    {
        RectI r = resolveInSub(ah_left, av_bottom, 20, -20, 120, 32, subOx, areaOy, subW, areaH);
        w.color(180, 100, 40).boxf(r.x1, r.y1, r.x2, r.y2);
        w.color(255, 255, 255).pos(r.x1 + 12, r.y1 + 8);
        w.font("MS Gothic", 13, 1);
        w.mes("[Settings]");
        if (g_anchorShowGuides) {
            // 左端 → ボタン左端 のガイド矢印 (実距離 20px)
            w.color(255, 200, 0);
            drawArrow(w, subOx, r.y2 + 4, r.x1, r.y2 + 4);
            w.color(255, 200, 0).pos(subOx + 2, r.y2 + 6);
            w.font("MS Gothic", 11, 0);
            w.mes("20px");
        }
    }
    // RB: Exit ボタン
    {
        RectI r = resolveInSub(ah_right, av_bottom, -20, -20, 120, 32, subOx, areaOy, subW, areaH);
        w.color(180, 60, 60).boxf(r.x1, r.y1, r.x2, r.y2);
        w.color(255, 255, 255).pos(r.x1 + 28, r.y1 + 8);
        w.font("MS Gothic", 13, 1);
        w.mes("[Exit]");
        if (g_anchorShowGuides) {
            // ボタン右端 → 右端 のガイド矢印
            w.color(255, 200, 0);
            drawArrow(w, r.x2, r.y2 + 4, subOx + subW, r.y2 + 4);
            w.color(255, 200, 0).pos(r.x2 + 4, r.y2 + 6);
            w.font("MS Gothic", 11, 0);
            w.mes("20px");
        }
    }
    // CB: Message
    {
        RectI r = resolveInSub(ah_center, av_bottom, 0, -60, 240, 28, subOx, areaOy, subW, areaH);
        w.color(80, 80, 80).boxf(r.x1, r.y1, r.x2, r.y2);
        w.color(255, 255, 0).pos(r.x1 + 20, r.y1 + 6);
        w.font("MS Gothic", 14, 1);
        w.mes("** GAME STARTED **");
    }
    // CM: クロスヘアラベル (中心基準)
    {
        RectI r = resolveInSub(ah_center, av_middle, 0, 0, 120, 28, subOx, areaOy, subW, areaH);
        w.color(120, 120, 120).boxf(r.x1, r.y1, r.x2, r.y2);
        w.color(255, 255, 255).pos(r.x1 + 30, r.y1 + 6);
        w.font("MS Gothic", 12, 0);
        w.mes("+CENTER");
        if (g_anchorShowGuides) {
            // 中心十字
            w.color(255, 255, 255);
            const int cx = subOx + subW / 2;
            const int cy = areaOy + areaH / 2;
            w.line(cx - 30, cy, cx + 30, cy);
            w.line(cx, cy - 20, cx, cy + 20);
        }
    }
}

// ───────────────────────────────────────────────────────────────────
// Fixed 側 HUD 描画 (アンチパターン: ハードコード座標)
//   ⚠ 本サンプル外で真似しないこと。リサイズで破綻するための意図的な悪例。
// ───────────────────────────────────────────────────────────────────
void drawHudFixed(Screen& w, int subOx, int subOy, int subW, int subH) {
    // 帯ラベル「FIXED (broken on resize)」
    w.color(120, 30, 30).boxf(subOx, subOy, subOx + subW, subOy + 18);
    w.color(255, 255, 255).pos(subOx + 8, subOy + 2);
    w.font("MS Gothic", 12, 1);
    w.mes("FIXED  (hardcoded xy, broken on resize)");

    const int areaOx = subOx;
    const int areaOy = subOy + 18;
    (void)subW; (void)subH;  // 意図的にサブ領域サイズを無視

    // LT: Score (固定座標)
    w.color(40, 80, 200).boxf(areaOx + 20, areaOy + 20, areaOx + 160, areaOy + 52);
    w.color(255, 255, 255).pos(areaOx + 28, areaOy + 28);
    w.font("MS Gothic", 13, 1);
    w.mes("Score: 01234");

    // RT: Time (固定座標 — kFixedDesignW=480 設計時の右上)
    {
        const int x1 = areaOx + kFixedDesignW - 140 - 20;  // 320
        const int x2 = x1 + 140;
        w.color(40, 80, 200).boxf(x1, areaOy + 20, x2, areaOy + 52);
        w.color(255, 255, 255).pos(x1 + 8, areaOy + 28);
        w.font("MS Gothic", 13, 1);
        w.mes("Time:  12:34");
    }

    // LB: Settings (固定座標 — kFixedDesignH=452 設計時の下端)
    {
        const int y1 = areaOy + kFixedDesignH - 32 - 20;
        const int y2 = y1 + 32;
        w.color(180, 100, 40).boxf(areaOx + 20, y1, areaOx + 140, y2);
        w.color(255, 255, 255).pos(areaOx + 32, y1 + 8);
        w.font("MS Gothic", 13, 1);
        w.mes("[Settings]");
    }

    // RB: Exit (固定座標)
    {
        const int x1 = areaOx + kFixedDesignW - 120 - 20;
        const int x2 = x1 + 120;
        const int y1 = areaOy + kFixedDesignH - 32 - 20;
        const int y2 = y1 + 32;
        w.color(180, 60, 60).boxf(x1, y1, x2, y2);
        w.color(255, 255, 255).pos(x1 + 28, y1 + 8);
        w.font("MS Gothic", 13, 1);
        w.mes("[Exit]");
    }

    // CB: Message (固定座標 — kFixedDesignW/2 が中央前提)
    {
        const int x1 = areaOx + kFixedDesignW / 2 - 120;
        const int x2 = x1 + 240;
        const int y1 = areaOy + kFixedDesignH - 28 - 60;
        const int y2 = y1 + 28;
        w.color(80, 80, 80).boxf(x1, y1, x2, y2);
        w.color(255, 255, 0).pos(x1 + 20, y1 + 6);
        w.font("MS Gothic", 14, 1);
        w.mes("** GAME STARTED **");
    }

    // CM: クロスヘアラベル (固定中央前提)
    {
        const int x1 = areaOx + kFixedDesignW / 2 - 60;
        const int x2 = x1 + 120;
        const int y1 = areaOy + kFixedDesignH / 2 - 14;
        const int y2 = y1 + 28;
        w.color(120, 120, 120).boxf(x1, y1, x2, y2);
        w.color(255, 255, 255).pos(x1 + 30, y1 + 6);
        w.font("MS Gothic", 12, 0);
        w.mes("+CENTER");
    }
}

// ───────────────────────────────────────────────────────────────────
// セパレータ + ヘッダー帯
// ───────────────────────────────────────────────────────────────────
void drawSeparatorAndHeader(Screen& w, int physW, int physH) {
    const auto& ap = kAnchorAspectPresets[g_anchorAspectIndex];

    // ヘッダー帯 (高さ kHeaderH)
    w.color(30, 30, 60).boxf(0, 0, physW, kHeaderH);
    w.font("MS Gothic", 13, 1);
    w.color(255, 255, 255).pos(8, 4);
    w.mes(std::format("Anchor Playground  |  phys = {} x {}   preset: {}",
                      physW, physH, ap.label));

    // 中央セパレータ縦線 (2px) — Anchored / Fixed 並置時のみ
    if (g_anchorShowFixed && !g_anchorShowGrid) {
        const int sepX = physW / 2;
        const int sepBottom = physH - kHotkeyBarH;
        w.color(140, 140, 140).boxf(sepX - 1, kHeaderH, sepX + 1, sepBottom);
    }
}

// ───────────────────────────────────────────────────────────────────
// 9 アンカー リファレンスグリッド (G キー ON)
// ───────────────────────────────────────────────────────────────────
void drawReferenceGrid(Screen& w, int physW, int physH) {
    // 帯ラベル
    w.color(30, 60, 30).boxf(0, kHeaderH, physW, kHeaderH + 18);
    w.color(255, 255, 255).pos(8, kHeaderH + 2);
    w.font("MS Gothic", 12, 1);
    w.mes("Reference Grid  |  9 anchors (H=Left/Center/Right × V=Top/Middle/Bottom), 20px margin from edges");

    const int subOx = 0;
    const int subOy = kHeaderH + 18;
    const int subW  = physW;
    const int subH  = physH - kHotkeyBarH - (kHeaderH + 18);
    if (subW <= 0 || subH <= 0) return;

    constexpr int boxW = 96;
    constexpr int boxH = 44;

    struct Cell { AnchorH h; AnchorV v; const char* hl; const char* vl; };
    constexpr Cell kCells[] = {
        { ah_left,   av_top,    "L", "T" },
        { ah_center, av_top,    "C", "T" },
        { ah_right,  av_top,    "R", "T" },
        { ah_left,   av_middle, "L", "M" },
        { ah_center, av_middle, "C", "M" },
        { ah_right,  av_middle, "R", "M" },
        { ah_left,   av_bottom, "L", "B" },
        { ah_center, av_bottom, "C", "B" },
        { ah_right,  av_bottom, "R", "B" },
    };

    for (const auto& c : kCells) {
        // ox/oy: 端アンカーは ±20px, 中央アンカーは 0
        int ox = 0, oy = 0;
        if (c.h == ah_left)   ox =  20;
        if (c.h == ah_right)  ox = -20;
        if (c.v == av_top)    oy =  20;
        if (c.v == av_bottom) oy = -20;

        RectI r = resolveInSub(c.h, c.v, ox, oy, boxW, boxH, subOx, subOy, subW, subH);

        // 塗り
        int rr = 60, gg = 60, bb = 60;
        if (c.h == ah_left)   rr = 200;
        if (c.h == ah_right)  bb = 220;
        if (c.h == ah_center) gg = 160;
        w.color(rr, gg, bb).boxf(r.x1, r.y1, r.x2, r.y2);

        // ラベル: "H=Right V=Bottom" + offset
        w.color(255, 255, 255).pos(r.x1 + 6, r.y1 + 4);
        w.font("MS Gothic", 11, 1);
        w.mes(std::format("H={} V={}", c.hl, c.vl));
        w.color(255, 255, 0).pos(r.x1 + 6, r.y1 + 20);
        w.font("MS Gothic", 11, 0);
        w.mes(std::format("ofs=({:+d},{:+d})", ox, oy));

        if (g_anchorShowGuides) {
            // 基準辺 → 対応画面端 のガイド矢印 + 実測 px
            w.color(255, 200, 0);
            if (c.h == ah_left) {
                int yMid = (r.y1 + r.y2) / 2;
                drawArrow(w, subOx, yMid, r.x1, yMid);
                w.pos(subOx + 2, yMid - 12);
                w.font("MS Gothic", 10, 0);
                w.mes("20px");
            } else if (c.h == ah_right) {
                int yMid = (r.y1 + r.y2) / 2;
                drawArrow(w, r.x2, yMid, subOx + subW, yMid);
                w.pos(r.x2 + 4, yMid - 12);
                w.font("MS Gothic", 10, 0);
                w.mes("20px");
            }
            if (c.v == av_top) {
                int xMid = (r.x1 + r.x2) / 2;
                drawArrow(w, xMid, subOy, xMid, r.y1);
                w.pos(xMid + 4, subOy + 2);
                w.font("MS Gothic", 10, 0);
                w.mes("20px");
            } else if (c.v == av_bottom) {
                int xMid = (r.x1 + r.x2) / 2;
                drawArrow(w, xMid, r.y2, xMid, subOy + subH);
                w.pos(xMid + 4, r.y2 + 4);
                w.font("MS Gothic", 10, 0);
                w.mes("20px");
            }
        }
    }
}

// ───────────────────────────────────────────────────────────────────
// 下部ホットキーバー
// ───────────────────────────────────────────────────────────────────
void drawHotkeyBar(Screen& w, int physW, int physH) {
    const int barTop = physH - kHotkeyBarH;
    w.color(20, 20, 20).boxf(0, barTop, physW, physH);

    w.font("MS Gothic", 11, 1);
    w.color(255, 255, 255).pos(8, barTop + 4);
    w.mes(std::format("[A]/[Shift+A] aspect ({}/{})   [G] grid: {}   [C] fixed col: {}   [H] guides: {}",
                      g_anchorAspectIndex + 1, kAnchorAspectCount,
                      g_anchorShowGrid   ? "ON" : "OFF",
                      g_anchorShowFixed  ? "ON" : "OFF",
                      g_anchorShowGuides ? "ON" : "OFF"));
    w.font("MS Gothic", 11, 0);
    w.color(200, 200, 200).pos(8, barTop + 22);
    w.mes("[R] reset   [V] toggle sub-window   |  リサイズ枠をドラッグして物理サイズを連続変更可能");
    w.color(180, 180, 80).pos(8, barTop + 40);
    w.mes("左 = ANCHORED (リサイズに追従)    右 = FIXED (アンカー無し / リサイズで破綻する例)");
}

} // namespace

// ═══════════════════════════════════════════════════════════════════
// 公開エントリ
// ═══════════════════════════════════════════════════════════════════

void ensureAnchorPlaygroundVisible(bool show) {
    if (show == g_anchorPlaygroundVisible) return;
    if (g_anchorPlaygroundScreen.valid()) {
        gsel(g_anchorPlaygroundScreen.id(), show ? 1 : -1);
    }
    g_anchorPlaygroundVisible = show;
}

void applyAnchorAspectPreset() {
    if (!g_anchorPlaygroundScreen.valid()) return;
    const AspectPreset& ap = kAnchorAspectPresets[g_anchorAspectIndex];
    // virtual_resolution=false なので width(w,h) はバッファサイズ = 物理クライアントサイズを直接変更
    g_anchorPlaygroundScreen.width(ap.w, ap.h);
}

void drawAnchorPlayground() {
    if (!g_anchorPlaygroundScreen.valid()) return;
    Screen& w = g_anchorPlaygroundScreen;

    // バッチ描画 (フラッシュ抑止) — cls 前に redraw(0) で描画開始
    w.redraw(0);
    w.cls(4);  // 黒背景

    const int physW = w.width();
    const int physH = w.height();
    if (physW <= 0 || physH <= 0) { w.redraw(1); return; }

    if (g_anchorShowGrid) {
        drawSeparatorAndHeader(w, physW, physH);
        drawReferenceGrid(w, physW, physH);
    } else {
        drawSeparatorAndHeader(w, physW, physH);
        const int areaTop = kHeaderH;
        const int areaBottom = physH - kHotkeyBarH;
        const int areaH = areaBottom - areaTop;
        if (areaH > 0) {
            if (g_anchorShowFixed) {
                const int halfW = physW / 2;
                drawHudAnchored(w, 0,     areaTop, halfW,         areaH);
                drawHudFixed   (w, halfW, areaTop, physW - halfW, areaH);
            } else {
                // Anchored 側を全幅に拡張
                drawHudAnchored(w, 0, areaTop, physW, areaH);
            }
        }
    }
    drawHotkeyBar(w, physW, physH);

    w.redraw(1);
}

// メインウィンドウ側に表示するステータス／誘導
void drawAnchorDemo(Screen& mainWin) {
    // Anchor Playground を可視化 (退場時に onAnchorDemoLeft で非表示化)
    if (!g_anchorPlaygroundVisible) {
        ensureAnchorPlaygroundVisible(true);
        applyAnchorAspectPreset();
    }

    // Playground 描画
    drawAnchorPlayground();

    // メインウィンドウ側はガイド表示のみ
    mainWin.color(0, 0, 0).pos(20, 85);
    mainWin.font("MS Gothic", 14, 1);
    mainWin.mes("[Anchor] アンカーレイアウト Playground");

    const AspectPreset& ap = kAnchorAspectPresets[g_anchorAspectIndex];
    mainWin.font("MS Gothic", 12, 0);
    mainWin.color(0, 0, 0).pos(20, 112);
    mainWin.mes("サブウィンドウ \"Anchor Playground\" を見てください。");
    mainWin.pos(20, 130);
    mainWin.mes("リサイズ枠をドラッグすると左 (Anchored) は追従、右 (Fixed) は破綻します。");

    mainWin.pos(20, 158);
    mainWin.mes(std::format("  アスペクト比 : ({}/{}) {}",
                            g_anchorAspectIndex + 1, kAnchorAspectCount, ap.label));
    mainWin.pos(20, 176);
    mainWin.mes(std::format("  比較対照 (Fixed 列) : {}", g_anchorShowFixed  ? "ON" : "OFF"));
    mainWin.pos(20, 194);
    mainWin.mes(std::format("  9 アンカー リファレンスグリッド : {}", g_anchorShowGrid   ? "ON" : "OFF"));
    mainWin.pos(20, 212);
    mainWin.mes(std::format("  ガイド矢印 + 実測値 : {}", g_anchorShowGuides ? "ON" : "OFF"));

    mainWin.color(64, 64, 64).pos(20, 240);
    mainWin.mes("操作 (Anchor Playground 表示中):");
    mainWin.pos(40, 258);
    mainWin.mes("  A / Shift+A : アスペクト比プリセット 次 / 前");
    mainWin.pos(40, 276);
    mainWin.mes("  G           : 9 アンカー リファレンスグリッド ON/OFF");
    mainWin.pos(40, 294);
    mainWin.mes("  C           : 比較対照 (Fixed 列) ON/OFF");
    mainWin.pos(40, 312);
    mainWin.mes("  H           : ガイド矢印 + 実測 px 表示 ON/OFF");
    mainWin.pos(40, 330);
    mainWin.mes("  R           : 既定値へリセット (16:9 / Fixed=ON / Grid=OFF / Guides=ON)");
    mainWin.pos(40, 348);
    mainWin.mes("  V           : Anchor Playground サブウィンドウ 表示 / 非表示");
    mainWin.pos(40, 366);
    mainWin.mes("  サブウィンドウのリサイズ枠ドラッグ : 物理クライアントサイズを連続変更");

    mainWin.color(128, 0, 0).pos(20, 400);
    mainWin.mes("注: Fixed 側はアンカー無しの \"破綻するアンチパターン\" 例示です。");
    mainWin.pos(20, 418);
    mainWin.mes("    本サンプル外で同じ書き方を真似ないでください。");
}

void processAnchorAction(Screen& mainWin) {
    // Ctrl/Alt/Win 押下中は無視 (Shift は A の逆送り用に許可)
    if (hsppp::getkey(VK::CONTROL) || hsppp::getkey(VK::MENU) ||
        hsppp::getkey(VK::LWIN)    || hsppp::getkey(VK::RWIN)) return;
    const bool shift = hsppp::getkey(VK::SHIFT) != 0;

    if (getkey('A')) {
        if (shift) {
            g_anchorAspectIndex = (g_anchorAspectIndex + kAnchorAspectCount - 1) % kAnchorAspectCount;
        } else {
            g_anchorAspectIndex = (g_anchorAspectIndex + 1) % kAnchorAspectCount;
        }
        applyAnchorAspectPreset();
        mainWin.select();
        g_actionLog = std::format("Anchor aspect -> {}", kAnchorAspectPresets[g_anchorAspectIndex].label);
        await(200);
    } else if (getkey('G') && !shift) {
        g_anchorShowGrid = !g_anchorShowGrid;
        mainWin.select();
        g_actionLog = std::format("Anchor grid -> {}", g_anchorShowGrid ? "ON" : "OFF");
        await(200);
    } else if (getkey('C') && !shift) {
        g_anchorShowFixed = !g_anchorShowFixed;
        mainWin.select();
        g_actionLog = std::format("Anchor fixed col -> {}", g_anchorShowFixed ? "ON" : "OFF");
        await(200);
    } else if (getkey('H') && !shift) {
        g_anchorShowGuides = !g_anchorShowGuides;
        mainWin.select();
        g_actionLog = std::format("Anchor guides -> {}", g_anchorShowGuides ? "ON" : "OFF");
        await(200);
    } else if (getkey('R') && !shift) {
        g_anchorAspectIndex = kAnchorAspectDefaultIndex;
        g_anchorShowFixed   = kAnchorShowFixedDefault;
        g_anchorShowGrid    = kAnchorShowGridDefault;
        g_anchorShowGuides  = kAnchorShowGuidesDefault;
        applyAnchorAspectPreset();
        mainWin.select();
        g_actionLog = "Anchor Playground プリセットをリセットしました";
        await(200);
    } else if (getkey('V') && !shift) {
        ensureAnchorPlaygroundVisible(!g_anchorPlaygroundVisible);
        if (g_anchorPlaygroundVisible) {
            applyAnchorAspectPreset();
        }
        mainWin.select();
        g_actionLog = std::format("Anchor Playground を{}にしました",
                                  g_anchorPlaygroundVisible ? "表示" : "非表示");
        await(200);
    }
}

void onAnchorDemoLeft() {
    if (g_anchorPlaygroundVisible) {
        ensureAnchorPlaygroundVisible(false);
    }
}
