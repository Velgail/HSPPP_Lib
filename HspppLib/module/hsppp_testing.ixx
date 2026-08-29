// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp_testing.ixx
// 白箱テスト専用モジュール。HspppTest からのみ import される想定。
// 本番 `hsppp` モジュールの利用者には不可視。
//
// 設計方針:
//   - 本番 `hsppp` モジュールの export 表面を一切汚染しない（独立モジュール）。
//   - LogicalRenderContext クラス自体は export せず、等価動作の自由関数シムのみを公開。
//   - Windows 依存型 (UINT 等) を表面に出さず、標準型 (unsigned int) で受ける。
//   - インターフェース単位では `#include` 禁止規約を遵守 (import も不要)。

export module hsppp_testing;

export namespace hsppp::testing {

    // LogicalRenderContext::PresentMapping と等価な POD 表現。
    // Windows 依存型を含まない。意図的に内部型と別名にして型同一性を持たせない。
    struct PresentMappingView {
        float scale;        // 論理 → 物理 の uniform scale
        float offsetX;      // 物理 backbuffer 上での描画開始 X (letterbox 中央寄せ込み)
        float offsetY;      //                                 Y
        float destW;        // 物理上の描画幅 (= logW * scale)
        float destH;        // 物理上の描画高 (= logH * scale)
    };

    // LogicalRenderContext::update + computePresentMapping を 1 回呼ぶ純関数シム。
    [[nodiscard]] PresentMappingView compute_present_mapping(
        int logW, int logH,
        int physClientW, int physClientH,
        unsigned int currentDpi,
        bool virtualEnabled);

    // LogicalRenderContext::update + physToLogical を 1 回呼ぶ純関数シム。
    void phys_to_logical(
        int logW, int logH,
        int physClientW, int physClientH,
        unsigned int currentDpi,
        bool virtualEnabled,
        int physX, int physY,
        int& outLogX, int& outLogY);

    // LogicalRenderContext::update + logicalToPhys を 1 回呼ぶ純関数シム。
    void logical_to_phys(
        int logW, int logH,
        int physClientW, int physClientH,
        unsigned int currentDpi,
        bool virtualEnabled,
        int logX, int logY,
        int& outPhysX, int& outPhysY);

} // namespace hsppp::testing
