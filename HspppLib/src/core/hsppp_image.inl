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
}

// ============================================================
// bmpsave - 画面イメージをBMPファイルに保存
// ============================================================
void bmpsave(std::string_view p1, const std::source_location& location) {
    auto pSurface = getCurrentSurface();
    if (!pSurface) {
        throw HspError(ERR_FILE_IO, "bmpsave: no active surface", location);
    }
    
    if (!pSurface->bmpsave(p1)) {
        throw HspError(ERR_FILE_IO, "bmpsave: failed to save image", location);
    }
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
    return loadCelDataInternal(p1, p2, location);
}

// ============================================================
// celdiv - 画像素材の分割サイズを設定
// ============================================================
void celdiv(int p1, int p2, int p3, const std::source_location& location) {
    auto it = internal::g_celDataMap.find(p1);
    if (it == internal::g_celDataMap.end()) {
        throw HspError(ERR_FILE_IO, "celdiv: cel ID not found", location);
    }
    
    if (p2 <= 0 || p3 <= 0) {
        throw HspError(ERR_OUT_OF_RANGE, "celdiv: division must be positive", location);
    }
    
    it->second.divX = p2;
    it->second.divY = p3;
}

// ============================================================
// celput - 画像素材を描画
// ============================================================
void celput(int p1, int p2, OptInt p3, OptInt p4, const std::source_location& location) {
    auto it = internal::g_celDataMap.find(p1);
    if (it == internal::g_celDataMap.end()) {
        throw HspError(ERR_FILE_IO, "celput: cel ID not found", location);
    }
    
    auto pSurface = getCurrentSurface();
    if (!pSurface) {
        throw HspError(ERR_FILE_IO, "celput: no active surface", location);
    }
    
    const internal::CelData& cel = it->second;
    
    // セル番号からソース矩形を計算
    int cellIndex = p2;
    if (cellIndex < 0 || cellIndex >= cel.divX * cel.divY) {
        throw HspError(ERR_OUT_OF_RANGE, "celput: cell index out of range", location);
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
    int destX = p3.value_or(pSurface->getCurrentX());
    int destY = p4.value_or(pSurface->getCurrentY());
    
    D2D1_RECT_F destRect = D2D1::RectF(
        static_cast<float>(destX),
        static_cast<float>(destY),
        static_cast<float>(destX + cellWidth),
        static_cast<float>(destY + cellHeight)
    );
    
    pSurface->celput(cel.pBitmap.Get(), srcRect, destRect);
}

} // namespace hsppp
