// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// ============================================================
// hsppp_image.inl
// 画像操作関連の実装 (picload, bmpsave, cel系)
// ============================================================
#pragma once

namespace hsppp {

// getCurrentSurface()はinternal名前空間内で定義されているため、
// internal::を付けずに使用できます（すでにhsppp.cpp内で定義済み）

// ============================================================
// picload - 画像ファイルをロード
// ============================================================
void picload(std::string_view p1, OptInt p2, const std::source_location& location) {
    safe_call(location, [&] {
        int mode = p2.value_or(0);
        
        if (mode < 0 || mode > 2) {
            throw HspError(ERR_OUT_OF_RANGE, "picload: invalid mode (must be 0-2)", location);
        }
        
        auto pSurface = getCurrentSurface();
        if (!pSurface) {
            throw HspError(ERR_FILE_IO, "picload: no active surface", location);
        }
        
        if (!pSurface->picload(p1, mode)) {
            throw HspError(ERR_FILE_IO, "picload: failed to load image", location);
        }
    });
}

// ============================================================
// bmpsave - 画面イメージをBMPファイルに保存
// ============================================================
void bmpsave(std::string_view p1, const std::source_location& location) {
    safe_call(location, [&] {
        auto pSurface = getCurrentSurface();
        if (!pSurface) {
            throw HspError(ERR_FILE_IO, "bmpsave: no active surface", location);
        }
        
        if (!pSurface->bmpsave(p1)) {
            throw HspError(ERR_FILE_IO, "bmpsave: failed to save image", location);
        }
    });
}

// ============================================================
// celload読み込みヘルパー（celloadとloadCelで共有）
// ============================================================
namespace {
    int loadCelDataInternal(std::string_view filename, hsppp::OptInt celId, const std::source_location& location) {
        int id = celId.value_or(-1);
        
        // 自動割り当てモード
        if (id < 0) {
            id = internal::g_nextCelId++;
        }
        
        // 画像読み込み
        int width = 0, height = 0;
        auto bitmap = internal::loadImageFile(filename, width, height);
        if (!bitmap) {
            throw HspError(ERR_FILE_IO, "cel loading: failed to load image", location);
        }
        
        // CelDataを作成
        internal::CelData celData;
        celData.pBitmap = bitmap;
        celData.width = width;
        celData.height = height;
        celData.divX = 1;
        celData.divY = 1;
        celData.centerX = 0;
        celData.centerY = 0;
        celData.filename = std::string(filename);
        
        // マップに登録
        internal::g_celDataMap[id] = std::move(celData);
        
        return id;
    }
}

// ============================================================
// celload - 画像ファイルをバッファにロード（仮想ID）
// ============================================================
int celload(std::string_view p1, OptInt p2, const std::source_location& location) {
    return safe_call(location, [&]() -> int {
        return loadCelDataInternal(p1, p2, location);
    });
}

// ============================================================
// celdiv - 画像素材の分割サイズを設定
// ============================================================
void celdiv(int p1, int p2, int p3, const std::source_location& location) {
    safe_call(location, [&] {
        auto it = internal::g_celDataMap.find(p1);
        if (it == internal::g_celDataMap.end()) {
            throw HspError(ERR_FILE_IO, "celdiv: cel ID not found", location);
        }
        
        if (p2 <= 0 || p3 <= 0) {
            throw HspError(ERR_OUT_OF_RANGE, "celdiv: division must be positive", location);
        }
        
        it->second.divX = p2;
        it->second.divY = p3;
    });
}

// ============================================================
// celput - 画像素材を描画
// ============================================================
void celput(int p1, int p2, OptInt p3, OptInt p4, const std::source_location& location) {
    safe_call(location, [&] {
        ensureDefaultScreen();

        auto surface = getCurrentSurface();
        if (!surface) return;

        // CelIDが有効か確認
        if (internal::g_celDataMap.find(p1) == internal::g_celDataMap.end()) return;

        internal::celput_impl(surface, p1, p2, p3, p4);
    });
}

} // namespace hsppp
