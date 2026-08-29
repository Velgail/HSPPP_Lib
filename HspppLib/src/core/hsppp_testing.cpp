// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/src/core/hsppp_testing.cpp
// 白箱テスト用シムの実装単位。
// グローバルモジュールフラグメントで Internal.h を取り込み、
// LogicalRenderContext を 1 回 instance 化して呼び出す薄いラッパ群を提供する。

module;

#define NOMINMAX
#include <windows.h>
#include "Internal.h"

module hsppp_testing;

namespace hsppp::testing {

PresentMappingView compute_present_mapping(
    int logW, int logH,
    int physClientW, int physClientH,
    unsigned int currentDpi,
    bool virtualEnabled)
{
    hsppp::internal::LogicalRenderContext ctx;
    ctx.update(logW, logH, physClientW, physClientH,
               static_cast<UINT>(currentDpi), virtualEnabled);
    const auto m = ctx.computePresentMapping();
    return PresentMappingView{ m.scale, m.offsetX, m.offsetY, m.destW, m.destH };
}

void phys_to_logical(
    int logW, int logH,
    int physClientW, int physClientH,
    unsigned int currentDpi,
    bool virtualEnabled,
    int physX, int physY,
    int& outLogX, int& outLogY)
{
    hsppp::internal::LogicalRenderContext ctx;
    ctx.update(logW, logH, physClientW, physClientH,
               static_cast<UINT>(currentDpi), virtualEnabled);
    ctx.physToLogical(physX, physY, outLogX, outLogY);
}

void logical_to_phys(
    int logW, int logH,
    int physClientW, int physClientH,
    unsigned int currentDpi,
    bool virtualEnabled,
    int logX, int logY,
    int& outPhysX, int& outPhysY)
{
    hsppp::internal::LogicalRenderContext ctx;
    ctx.update(logW, logH, physClientW, physClientH,
               static_cast<UINT>(currentDpi), virtualEnabled);
    ctx.logicalToPhys(logX, logY, outPhysX, outPhysY);
}

} // namespace hsppp::testing
