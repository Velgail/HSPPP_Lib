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

namespace internal {
    bool picloadToSurface(int surfaceId, std::string_view filename, int mode) {
        auto original = getSurfaceById(surfaceId);
        if (!original) return false;

        if (mode == 1) {
            return original->picload(filename, mode);
        }

        // mode 0/2 は、HSPと同じく画像寸法で画面を初期化してから読み込む。
        int imageWidth = 0;
        int imageHeight = 0;
        auto bitmap = loadImageFile(filename, imageWidth, imageHeight);
        if (!bitmap || imageWidth <= 0 || imageHeight <= 0) return false;

        if (auto window = std::dynamic_pointer_cast<HspWindow>(original)) {
            const HWND hwnd = window->getHwnd();
            const LONG_PTR style = hwnd ? GetWindowLongPtrW(hwnd, GWL_STYLE) : 0;
            const LONG_PTR exStyle = hwnd ? GetWindowLongPtrW(hwnd, GWL_EXSTYLE) : 0;
            RECT rect{};
            if (hwnd) GetWindowRect(hwnd, &rect);

            int screenMode = 0;
            if (hwnd && !IsWindowVisible(hwnd)) screenMode |= screen_hide;
            if ((style & WS_THICKFRAME) == 0) screenMode |= screen_fixedsize;
            if ((exStyle & WS_EX_TOOLWINDOW) != 0) screenMode |= screen_tool;
            if ((exStyle & WS_EX_CLIENTEDGE) != 0) screenMode |= screen_frame;
            if (window->isVirtualEnabled()) screenMode |= screen_mode_virtual;

            if ((style & WS_POPUP) != 0) {
                const int bgscrMode = screenMode & (screen_hide | screen_mode_virtual);
                (void)bgscr(surfaceId, imageWidth, imageHeight, bgscrMode,
                            rect.left, rect.top, imageWidth, imageHeight);
            } else {
                std::wstring titleW;
                if (hwnd) {
                    const int titleLength = GetWindowTextLengthW(hwnd);
                    titleW.resize(static_cast<size_t>(titleLength) + 1);
                    GetWindowTextW(hwnd, titleW.data(), titleLength + 1);
                    titleW.resize(static_cast<size_t>(titleLength));
                }
                const std::string title = WideToUtf8(titleW);
                (void)screen(surfaceId, imageWidth, imageHeight, screenMode,
                             rect.left, rect.top, imageWidth, imageHeight, title);
            }
        } else {
            (void)buffer(surfaceId, imageWidth, imageHeight, 0);
        }

        auto destination = getSurfaceById(surfaceId);
        if (!destination) return false;
        destination->cls(mode == 2 ? 4 : 0);
        const D2D1_RECT_F imageRect = D2D1::RectF(
            0.0f, 0.0f,
            static_cast<float>(imageWidth),
            static_cast<float>(imageHeight));
        destination->celput(bitmap.Get(), imageRect, imageRect);
        return true;
    }
}

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
        
        if (!internal::picloadToSurface(g_currentScreenId, p1, mode)) {
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

namespace {
    struct LoadedCellImage {
        int id;
        std::weak_ptr<internal::HspSurface> surface;
    };

    std::map<std::string, LoadedCellImage> g_loadedCellImages;

    int findUnusedSurfaceId() {
        for (int id = 0; ; ++id) {
            if (g_surfaces.find(id) == g_surfaces.end()) {
                return id;
            }
        }
    }
}

// ============================================================
// celload - 画像ファイルを仮想画面へロード
// ============================================================
int celload(std::string_view p1, OptInt p2, OptInt p3, const std::source_location& location) {
    return safe_call(location, [&]() -> int {
        const int requestedId = p2.value_or(-2);
        const int mode = p3.value_or(0);
        if (mode < 0 || mode > 1) {
            throw HspError(ERR_OUT_OF_RANGE, "celload: 初期化モードは0または1を指定してください", location);
        }

        const std::string filename(p1);
        if (requestedId == -2) {
            auto loaded = g_loadedCellImages.find(filename);
            if (loaded != g_loadedCellImages.end()) {
                auto surface = loaded->second.surface.lock();
                auto registered = getSurfaceById(loaded->second.id);
                if (surface && registered == surface) {
                    g_currentSurface = surface;
                    g_currentScreenId = loaded->second.id;
                    return loaded->second.id;
                }
                g_loadedCellImages.erase(loaded);
            }
        }

        int width = 0;
        int height = 0;
        auto bitmap = internal::loadImageFile(p1, width, height);
        if (!bitmap || width <= 0 || height <= 0) {
            throw HspError(ERR_FILE_IO, "celload: 画像ファイルを読み込めません", location);
        }

        const int id = requestedId >= 0 ? requestedId : findUnusedSurfaceId();
        (void)buffer(id, width, height, mode, location);

        auto surface = getSurfaceById(id);
        if (!surface) {
            throw HspError(ERR_INVALID_HANDLE, "celload: 仮想画面を初期化できません", location);
        }

        const D2D1_RECT_F imageRect = D2D1::RectF(
            0.0f,
            0.0f,
            static_cast<float>(width),
            static_cast<float>(height)
        );
        surface->celput(bitmap.Get(), imageRect, imageRect);
        surface->resetCelDivision();
        g_loadedCellImages[filename] = LoadedCellImage{id, surface};
        return id;
    });
}

// ============================================================
// celdiv - 画像素材のセル寸法と中心座標を設定
// ============================================================
void celdiv(int p1, OptInt p2, OptInt p3, OptInt p4, OptInt p5, const std::source_location& location) {
    safe_call(location, [&] {
        auto surface = getSurfaceById(p1);
        if (!surface) {
            throw HspError(ERR_INVALID_HANDLE, "celdiv: 指定されたウィンドウIDがありません", location);
        }

        const int cellWidth = p2.value_or(0);
        const int cellHeight = p3.value_or(0);
        surface->setCelDivision(
            cellWidth > 0 ? cellWidth : surface->getWidth(),
            cellHeight > 0 ? cellHeight : surface->getHeight(),
            p4.value_or(0),
            p5.value_or(0)
        );
    });
}

// ============================================================
// celput - 画像素材を現在のカレントポジションへ描画
// ============================================================
void celput(int p1, OptInt p2, OptDouble p3, OptDouble p4, OptDouble p5, const std::source_location& location) {
    safe_call(location, [&] {
        ensureDefaultScreen();

        auto destination = getCurrentSurface();
        auto source = getSurfaceById(p1);
        if (!destination || !source || !source->getTargetBitmap()) {
            throw HspError(ERR_INVALID_HANDLE, "celput: 指定された画像素材がありません", location);
        }

        const int cellWidth = source->getCelWidth();
        const int cellHeight = source->getCelHeight();
        const int columns = cellWidth > 0 ? source->getWidth() / cellWidth : 0;
        const int rows = cellHeight > 0 ? source->getHeight() / cellHeight : 0;
        const int cellNumber = p2.value_or(0);
        if (columns <= 0 || rows <= 0 || cellNumber < 0 || cellNumber >= columns * rows) {
            throw HspError(ERR_OUT_OF_RANGE, "celput: セル番号が範囲外です", location);
        }

        const int sourceX = (cellNumber % columns) * cellWidth;
        const int sourceY = (cellNumber / columns) * cellHeight;
        const D2D1_RECT_F sourceRect = D2D1::RectF(
            static_cast<float>(sourceX),
            static_cast<float>(sourceY),
            static_cast<float>(sourceX + cellWidth),
            static_cast<float>(sourceY + cellHeight)
        );

        const double zoomX = p3.value_or(1.0);
        const double zoomY = p4.value_or(1.0);
        const double angle = p5.value_or(0.0);
        if (zoomX == 1.0 && zoomY == 1.0 && angle == 0.0) {
            // HSP Win32版も無変形celputはBmscr::Copyへ委譲する。
            // これによりgmode 2〜7の整数演算・透明色・クリップをgcopyと共有する。
            const int originalX = destination->getCurrentX();
            const int originalY = destination->getCurrentY();
            destination->pos(
                originalX - source->getCelCenterX(),
                originalY - source->getCelCenterY());
            try {
                internal::gcopy_impl(
                    destination, source, sourceX, sourceY, cellWidth, cellHeight, location);
            } catch (...) {
                destination->pos(originalX, originalY);
                throw;
            }
            destination->pos(originalX + cellWidth, originalY);
            return;
        }

        destination->celputHsp(
            source->getTargetBitmap(),
            sourceRect,
            static_cast<float>(cellWidth),
            static_cast<float>(cellHeight),
            static_cast<float>(source->getCelCenterX()),
            static_cast<float>(source->getCelCenterY()),
            zoomX,
            zoomY,
            angle
        );
    });
}

} // namespace hsppp
