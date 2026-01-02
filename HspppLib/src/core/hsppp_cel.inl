// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// ============================================================
// hsppp_cel.inl
// Celクラスの実装（OOP版celシリーズ）
// ============================================================
#pragma once

namespace hsppp {

// ============================================================
// Cel class コンストラクタ実装
// ============================================================

Cel::Cel(int id) noexcept 
    : m_id(id), m_valid(internal::g_celDataMap.find(id) != internal::g_celDataMap.end()) {}

// ============================================================
// Cel class メソッド実装
// ============================================================

Cel& Cel::divide(int divX, int divY, const std::source_location& location) {
    if (!m_valid) return *this;
    
    safe_call(location, [&] {
        auto it = internal::g_celDataMap.find(m_id);
        if (it == internal::g_celDataMap.end()) {
            return;
        }
        
        if (divX <= 0 || divY <= 0) {
            return;
        }
        
        it->second.divX = divX;
        it->second.divY = divY;
    });
    
    return *this;
}

Cel& Cel::put(int cellIndex, OptInt x, OptInt y, const std::source_location& location) {
    if (!m_valid) return *this;
    
    safe_call(location, [&] {
        auto it = internal::g_celDataMap.find(m_id);
        if (it == internal::g_celDataMap.end()) {
            return;
        }
        
        auto pSurface = getCurrentSurface();
        if (!pSurface) {
            return;
        }
        
        const internal::CelData& cel = it->second;
        
        // セル番号からソース矩形を計算
        if (cellIndex < 0 || cellIndex >= cel.divX * cel.divY) {
            return;
        }
        
        int cellWidth = cel.width / cel.divX;
        int cellHeight = cel.height / cel.divY;
        
        int srcX = (cellIndex % cel.divX) * cellWidth;
        int srcY = (cellIndex / cel.divX) * cellHeight;
        
        D2D1_RECT_F srcRect = D2D1::RectF(
            static_cast<float>(srcX),
            static_cast<float>(srcY),
            static_cast<float>(srcX + cellWidth),
            static_cast<float>(srcY + cellHeight)
        );
        
        // 描画位置（省略時は現在のpos）
        int destX = x.value_or(pSurface->getCurrentX());
        int destY = y.value_or(pSurface->getCurrentY());
        
        D2D1_RECT_F destRect = D2D1::RectF(
            static_cast<float>(destX),
            static_cast<float>(destY),
            static_cast<float>(destX + cellWidth),
            static_cast<float>(destY + cellHeight)
        );
        
        pSurface->celput(cel.pBitmap.Get(), srcRect, destRect);
    });
    
    return *this;
}

int Cel::width(const std::source_location& location) const {
    return safe_call(location, [&]() -> int {
        if (!m_valid) return 0;
        
        auto it = internal::g_celDataMap.find(m_id);
        if (it == internal::g_celDataMap.end()) {
            return 0;
        }
        
        return it->second.width;
    });
}

int Cel::height(const std::source_location& location) const {
    return safe_call(location, [&]() -> int {
        if (!m_valid) return 0;
        
        auto it = internal::g_celDataMap.find(m_id);
        if (it == internal::g_celDataMap.end()) {
            return 0;
        }
        
        return it->second.height;
    });
}

// ============================================================
// loadCel - Celファクトリー関数
// ============================================================
Cel loadCel(std::string_view filename, OptInt celId, const std::source_location& location) {
    return safe_call(location, [&]() -> Cel {
        int id = celId.value_or(-1);
        
        // 自動割り当てモード
        if (id < 0) {
            id = internal::g_nextCelId++;
        }
        
        // 画像読み込み
        int width = 0, height = 0;
        auto bitmap = internal::loadImageFile(filename, width, height);
        if (!bitmap) {
            throw HspError(ERR_FILE_IO, "loadCel: failed to load image", location);
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
        
        return Cel(id, true);
    });
}

} // namespace hsppp
