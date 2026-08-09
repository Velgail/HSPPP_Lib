// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/src/core/hsppp_copy.inl
// gsel, gmode, gcopy, gzoom関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    namespace internal {
        // gmode 2〜7をHSPの整数RGB演算で合成する。
        // GPUターゲットを読み戻すため低速だが、演算結果を近似せず互換性を優先する経路。
        bool gcopy_software_impl(std::shared_ptr<HspSurface> destSurface,
                                 std::shared_ptr<HspSurface> srcSurface,
                                 int srcX, int srcY, int sizeX, int sizeY,
                                 int mode, int blendRate) {
            if (!destSurface || !srcSurface) return false;
            if (sizeX <= 0 || sizeY <= 0) return true;
            auto* destBitmap = destSurface->getTargetBitmap();
            auto* srcBitmap = srcSurface->getTargetBitmap();
            auto* destContext = destSurface->getDeviceContext();
            if (!destBitmap || !srcBitmap || !destContext) return false;

            const bool destWasDrawing = destSurface->isDrawing();
            const bool srcWasDrawing = srcSurface != destSurface && srcSurface->isDrawing();
            if (srcWasDrawing) srcSurface->endDraw();
            if (destWasDrawing) destSurface->endDraw();

            auto restoreDrawing = [&] {
                if (srcWasDrawing) srcSurface->beginDraw();
                if (destWasDrawing) destSurface->beginDraw();
            };

            // HSPのBlt実装と同じく、コピー元・コピー先の両方で領域をクリップする。
            // mode 7のマスクは、クリップ前の指定幅だけ右側にある画像を参照する。
            const int requestedSrcX = srcX;
            const int requestedSrcY = srcY;
            const int requestedDestX = destSurface->getCurrentX();
            const int requestedDestY = destSurface->getCurrentY();
            const int requestedWidth = sizeX;
            const int requestedHeight = sizeY;

            const int clipLeft = (std::max)({0, -requestedSrcX, -requestedDestX});
            const int clipTop = (std::max)({0, -requestedSrcY, -requestedDestY});
            int clippedWidth = requestedWidth - clipLeft;
            int clippedHeight = requestedHeight - clipTop;
            const int clippedSrcX = requestedSrcX + clipLeft;
            const int clippedSrcY = requestedSrcY + clipTop;
            const int clippedDestX = requestedDestX + clipLeft;
            const int clippedDestY = requestedDestY + clipTop;
            const int clippedMaskX = requestedSrcX + requestedWidth + clipLeft;

            clippedWidth = (std::min)(clippedWidth, srcSurface->getWidth() - clippedSrcX);
            clippedWidth = (std::min)(clippedWidth, destSurface->getWidth() - clippedDestX);
            if (mode == 7) {
                clippedWidth = (std::min)(clippedWidth, srcSurface->getWidth() - clippedMaskX);
            }
            clippedHeight = (std::min)(clippedHeight, srcSurface->getHeight() - clippedSrcY);
            clippedHeight = (std::min)(clippedHeight, destSurface->getHeight() - clippedDestY);
            if (clippedWidth <= 0 || clippedHeight <= 0) {
                restoreDrawing();
                return true;
            }

            const D2D1_SIZE_U srcBitmapSize = srcBitmap->GetPixelSize();
            const D2D1_SIZE_U destBitmapSize = destBitmap->GetPixelSize();
            const double srcScaleX = static_cast<double>(srcBitmapSize.width) /
                                     static_cast<double>((std::max)(1, srcSurface->getWidth()));
            const double srcScaleY = static_cast<double>(srcBitmapSize.height) /
                                     static_cast<double>((std::max)(1, srcSurface->getHeight()));
            const double destScaleX = static_cast<double>(destBitmapSize.width) /
                                      static_cast<double>((std::max)(1, destSurface->getWidth()));
            const double destScaleY = static_cast<double>(destBitmapSize.height) /
                                      static_cast<double>((std::max)(1, destSurface->getHeight()));

            int srcScrollX = 0;
            int srcScrollY = 0;
            int destScrollX = 0;
            int destScrollY = 0;
            if (auto srcWindow = std::dynamic_pointer_cast<HspWindow>(srcSurface)) {
                srcScrollX = srcWindow->getScrollX();
                srcScrollY = srcWindow->getScrollY();
            }
            if (auto destWindow = std::dynamic_pointer_cast<HspWindow>(destSurface)) {
                destScrollX = destWindow->getScrollX();
                destScrollY = destWindow->getScrollY();
            }

            const int srcPxX = static_cast<int>(std::floor((clippedSrcX - srcScrollX) * srcScaleX));
            const int srcPxY = static_cast<int>(std::floor((clippedSrcY - srcScrollY) * srcScaleY));
            const int srcPxW = (std::max)(1, static_cast<int>(std::round(clippedWidth * srcScaleX)));
            const int srcPxH = (std::max)(1, static_cast<int>(std::round(clippedHeight * srcScaleY)));
            const int maskPxX = static_cast<int>(std::floor((clippedMaskX - srcScrollX) * srcScaleX));
            const int destPxX = static_cast<int>(std::floor((clippedDestX - destScrollX) * destScaleX));
            const int destPxY = static_cast<int>(std::floor((clippedDestY - destScrollY) * destScaleY));
            const int destPxW = (std::max)(1, static_cast<int>(std::round(clippedWidth * destScaleX)));
            const int destPxH = (std::max)(1, static_cast<int>(std::round(clippedHeight * destScaleY)));

            if (srcPxX < 0 || srcPxY < 0 || destPxX < 0 || destPxY < 0 ||
                srcPxX + srcPxW > static_cast<int>(srcBitmapSize.width) ||
                (mode == 7 && (maskPxX < 0 || maskPxX + srcPxW > static_cast<int>(srcBitmapSize.width))) ||
                srcPxY + srcPxH > static_cast<int>(srcBitmapSize.height) ||
                destPxX + destPxW > static_cast<int>(destBitmapSize.width) ||
                destPxY + destPxH > static_cast<int>(destBitmapSize.height)) {
                restoreDrawing();
                return false;
            }

            auto readRegion = [](ID2D1Bitmap1* bitmap, int x, int y, int width, int height,
                                 ComPtr<ID2D1Bitmap1>& cpuBitmap, D2D1_MAPPED_RECT& mapped) -> bool {
                auto context = D2DDeviceManager::getInstance().createDeviceContext();
                if (!context) return false;
                const auto pixelFormat = bitmap->GetPixelFormat();
                const auto props = D2D1::BitmapProperties1(
                    D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                    pixelFormat);
                if (FAILED(context->CreateBitmap(D2D1::SizeU(width, height), nullptr, 0,
                                                 props, cpuBitmap.GetAddressOf()))) return false;
                const D2D1_RECT_U rect{
                    static_cast<UINT32>(x), static_cast<UINT32>(y),
                    static_cast<UINT32>(x + width), static_cast<UINT32>(y + height)};
                if (FAILED(cpuBitmap->CopyFromBitmap(nullptr, bitmap, &rect))) return false;
                return SUCCEEDED(cpuBitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped));
            };

            ComPtr<ID2D1Bitmap1> srcCpu;
            ComPtr<ID2D1Bitmap1> maskCpu;
            ComPtr<ID2D1Bitmap1> destCpu;
            D2D1_MAPPED_RECT srcMapped{};
            D2D1_MAPPED_RECT maskMapped{};
            D2D1_MAPPED_RECT destMapped{};
            if (!readRegion(srcBitmap, srcPxX, srcPxY, srcPxW, srcPxH,
                            srcCpu, srcMapped) ||
                (mode == 7 && !readRegion(srcBitmap, maskPxX, srcPxY, srcPxW, srcPxH,
                                          maskCpu, maskMapped)) ||
                !readRegion(destBitmap, destPxX, destPxY, destPxW, destPxH,
                            destCpu, destMapped)) {
                if (srcCpu && srcMapped.bits) srcCpu->Unmap();
                if (maskCpu && maskMapped.bits) maskCpu->Unmap();
                if (destCpu && destMapped.bits) destCpu->Unmap();
                restoreDrawing();
                return false;
            }

            std::vector<std::uint8_t> result(static_cast<size_t>(destPxW) * destPxH * 4);
            const auto currentColor = destSurface->getCurrentColor();
            const int keyB = static_cast<int>(std::round(currentColor.b * 255.0f));
            const int keyG = static_cast<int>(std::round(currentColor.g * 255.0f));
            const int keyR = static_cast<int>(std::round(currentColor.r * 255.0f));

            for (int y = 0; y < destPxH; ++y) {
                const int sy = (std::min)(srcPxH - 1, y * srcPxH / destPxH);
                const auto* srcRow = srcMapped.bits + static_cast<size_t>(sy) * srcMapped.pitch;
                const auto* maskRow = mode == 7
                    ? maskMapped.bits + static_cast<size_t>(sy) * maskMapped.pitch
                    : nullptr;
                const auto* destRow = destMapped.bits + static_cast<size_t>(y) * destMapped.pitch;
                auto* outRow = result.data() + static_cast<size_t>(y) * destPxW * 4;
                for (int x = 0; x < destPxW; ++x) {
                    const int sx = (std::min)(srcPxW - 1, x * srcPxW / destPxW);
                    const auto* source = srcRow + static_cast<size_t>(sx) * 4;
                    const auto* destination = destRow + static_cast<size_t>(x) * 4;
                    auto* output = outRow + static_cast<size_t>(x) * 4;

                    const bool transparent = mode == 2
                        ? source[0] == 0 && source[1] == 0 && source[2] == 0
                        : mode == 4 && source[0] == keyB && source[1] == keyG && source[2] == keyR;
                    if (transparent) {
                        output[0] = destination[0];
                        output[1] = destination[1];
                        output[2] = destination[2];
                    } else if (mode == 2) {
                        output[0] = source[0];
                        output[1] = source[1];
                        output[2] = source[2];
                    } else if (mode == 3 || mode == 4) {
                        for (int channel = 0; channel < 3; ++channel) {
                            output[channel] = static_cast<std::uint8_t>(
                                ((source[channel] * blendRate) >> 8) +
                                ((destination[channel] * (256 - blendRate)) >> 8));
                        }
                    } else if (mode == 5) {
                        for (int channel = 0; channel < 3; ++channel) {
                            output[channel] = static_cast<std::uint8_t>((std::min)(
                                255, destination[channel] + source[channel] * blendRate / 256));
                        }
                    } else if (mode == 6) {
                        for (int channel = 0; channel < 3; ++channel) {
                            output[channel] = static_cast<std::uint8_t>((std::max)(
                                0, destination[channel] - source[channel] * blendRate / 256));
                        }
                    } else {  // mode 7: 右隣の画像をRGB別アルファ成分として使う
                        const auto* mask = maskRow + static_cast<size_t>(sx) * 4;
                        for (int channel = 0; channel < 3; ++channel) {
                            if (mask[channel] == 0) {
                                output[channel] = destination[channel];
                            } else if (mask[channel] == 255) {
                                output[channel] = source[channel];
                            } else {
                                output[channel] = static_cast<std::uint8_t>(
                                    ((destination[channel] * (255 - mask[channel])) >> 8) +
                                    ((source[channel] * mask[channel]) >> 8));
                            }
                        }
                    }
                    output[3] = 255;
                }
            }
            srcCpu->Unmap();
            if (maskCpu) maskCpu->Unmap();
            destCpu->Unmap();

            ComPtr<ID2D1Bitmap1> outputBitmap;
            const auto outputProps = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_NONE,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
            if (FAILED(destContext->CreateBitmap(
                    D2D1::SizeU(destPxW, destPxH), result.data(), destPxW * 4,
                    outputProps, outputBitmap.GetAddressOf()))) {
                restoreDrawing();
                return false;
            }

            if (srcWasDrawing) srcSurface->beginDraw();
            destSurface->beginDraw();
            const auto oldBlend = destContext->GetPrimitiveBlend();
            destContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
            const D2D1_RECT_F destinationRect = D2D1::RectF(
                static_cast<float>(clippedDestX), static_cast<float>(clippedDestY),
                static_cast<float>(clippedDestX + clippedWidth),
                static_cast<float>(clippedDestY + clippedHeight));
            destContext->DrawBitmap(outputBitmap.Get(), destinationRect, 1.0f,
                                    D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
            destContext->SetPrimitiveBlend(oldBlend);
            if (!destWasDrawing) destSurface->endDrawAndPresent();
            return true;
        }

        // ============================================================
        // 内部ヘルパー関数: gcopy_impl()
        // gcopy/Screen::gcopyで共有されるコア実装
        // ============================================================
        void gcopy_impl(std::shared_ptr<hsppp::internal::HspSurface> destSurface, 
                       std::shared_ptr<hsppp::internal::HspSurface> srcSurface,
                       int srcX, int srcY, int sizeX, int sizeY,
                       const std::source_location& location) {
            if (sizeX <= 0 || sizeY <= 0) return;
            if (!srcSurface) {
                throw HspError(ERR_INVALID_HANDLE, "gcopyのコピー元サーフェスが見つかりません", location);
            }
            
            auto srcBitmap = srcSurface->getTargetBitmap();
            if (!srcBitmap) {
                throw HspError(ERR_INVALID_HANDLE, "gcopyのコピー元ビットマップが無効です", location);
            }

            auto destContext = destSurface->getDeviceContext();
            if (!destContext) {
                throw HspError(ERR_INVALID_HANDLE, "gcopyのコピー先DeviceContextが無効です", location);
            }

            // サーフェスのgmode設定を取得
            int gmodeMode = destSurface->getGmodeMode();
            int gmodeBlendRate = destSurface->getGmodeBlendRate();

            if (gmodeMode >= 2 && gmodeMode <= 7) {
                if (gcopy_software_impl(destSurface, srcSurface, srcX, srcY, sizeX, sizeY,
                                        gmodeMode, gmodeBlendRate)) {
                    return;
                }
                throw HspError(ERR_SYSTEM_ERROR, "gcopyのgmode合成に失敗しました", location);
            }

            // カレントポジションを取得
            int destX = destSurface->getCurrentX();
            int destY = destSurface->getCurrentY();

            // 描画モードに応じて処理
            bool autoManage = (destSurface->getRedrawMode() == 1 && !destSurface->isDrawing());
            if (autoManage) {
                destSurface->beginDraw();
            }
            if (!destSurface->isDrawing()) return;

            // コピー元の領域
            D2D1_RECT_F srcRect = D2D1::RectF(
                static_cast<FLOAT>(srcX),
                static_cast<FLOAT>(srcY),
                static_cast<FLOAT>(srcX + sizeX),
                static_cast<FLOAT>(srcY + sizeY)
            );

            // コピー先の領域（カレントポジションから）
            D2D1_RECT_F destRect = D2D1::RectF(
                static_cast<FLOAT>(destX),
                static_cast<FLOAT>(destY),
                static_cast<FLOAT>(destX + sizeX),
                static_cast<FLOAT>(destY + sizeY)
            );

            // コピーモードに応じた処理（サーフェスのgmode設定を使用）
            FLOAT opacity = 1.0f;
            D2D1_PRIMITIVE_BLEND primitiveBlend = D2D1_PRIMITIVE_BLEND_SOURCE_OVER;

            if (gmodeMode >= 3 && gmodeMode <= 6) {
                opacity = gmodeBlendRate / 256.0f;
            }

            if (gmodeMode == 5) {
                // 加算ブレンド
                primitiveBlend = D2D1_PRIMITIVE_BLEND_ADD;
            } else if (gmodeMode == 6) {
                // 減算ブレンド（Direct2Dに直接対応がないためMINで近似）
                primitiveBlend = D2D1_PRIMITIVE_BLEND_MIN;
            }

            destContext->SetPrimitiveBlend(primitiveBlend);

            // 補間モードはコピー先サーフェスの gmode_interp 設定を使用
            D2D1_INTERPOLATION_MODE interpMode = destSurface->getGmodeInterp();

            // Direct2D 1.1では同じDeviceから作成されたビットマップを直接描画可能
            destContext->DrawBitmap(
                srcBitmap,
                &destRect,
                opacity,
                interpMode,
                &srcRect,
                nullptr
            );

            // ブレンドモードをリセット
            if (primitiveBlend != D2D1_PRIMITIVE_BLEND_SOURCE_OVER) {
                destContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
            }

            if (autoManage) {
                destSurface->endDrawAndPresent();
            }
        }

        // ============================================================
        // 内部ヘルパー関数: gzoom_impl()
        // gzoom/Screen::gzoomで共有されるコア実装
        // ============================================================
        void gzoom_impl(std::shared_ptr<hsppp::internal::HspSurface> destSurface,
                       int destW, int destH,
                       std::shared_ptr<hsppp::internal::HspSurface> srcSurface,
                       int srcX, int srcY, int srcW, int srcH, int mode,
                       const std::source_location& location) {
            if (!srcSurface) {
                throw HspError(ERR_INVALID_HANDLE, "gzoomのコピー元サーフェスが見つかりません", location);
            }

            auto srcBitmap = srcSurface->getTargetBitmap();
            if (!srcBitmap) {
                throw HspError(ERR_INVALID_HANDLE, "gzoomのコピー元ビットマップが無効です", location);
            }

            auto destContext = destSurface->getDeviceContext();
            if (!destContext) {
                throw HspError(ERR_INVALID_HANDLE, "gzoomのコピー先DeviceContextが無効です", location);
            }

            // カレントポジションを取得
            int destX = destSurface->getCurrentX();
            int destY = destSurface->getCurrentY();

            // 描画モードに応じて処理
            bool autoManage = (destSurface->getRedrawMode() == 1 && !destSurface->isDrawing());
            if (autoManage) {
                destSurface->beginDraw();
            }
            if (!destSurface->isDrawing()) return;

            // コピー元の領域
            D2D1_RECT_F srcRect = D2D1::RectF(
                static_cast<FLOAT>(srcX),
                static_cast<FLOAT>(srcY),
                static_cast<FLOAT>(srcX + srcW),
                static_cast<FLOAT>(srcY + srcH)
            );

            // コピー先の領域（変倍、カレントポジションから）
            D2D1_RECT_F destRectArea = D2D1::RectF(
                static_cast<FLOAT>(destX),
                static_cast<FLOAT>(destY),
                static_cast<FLOAT>(destX + destW),
                static_cast<FLOAT>(destY + destH)
            );

            // 補間モード
            // mode < 0  : サーフェスの gmode_interp 設定を使用（gmode_interp 命令と整合）
            // mode == 0 : NEAREST_NEIGHBOR（gzoom 引数による per-call 明示指定 / HSP3 互換）
            // mode == 1 : LINEAR （gzoom 引数による per-call 明示指定 / HSP3 互換）
            // mode == 2 : ANISOTROPIC （gzoom 引数による per-call 明示指定）
            D2D1_INTERPOLATION_MODE interpMode;
            switch (mode) {
            case 0:  interpMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR; break;
            case 1:  interpMode = D2D1_INTERPOLATION_MODE_LINEAR;            break;
            case 2:  interpMode = D2D1_INTERPOLATION_MODE_ANISOTROPIC;       break;
            default: interpMode = destSurface->getGmodeInterp();             break;
            }

            // Direct2D 1.1では同じDeviceから作成されたビットマップを直接描画可能
            destContext->DrawBitmap(
                srcBitmap,
                &destRectArea,
                1.0f,
                interpMode,
                &srcRect,
                nullptr
            );

            if (autoManage) {
                destSurface->endDrawAndPresent();
            }
        }
    } // namespace internal

    // ============================================================
    // gsel - 描画先指定、ウィンドウ最前面、非表示設定（HSP互換）
    // ============================================================
    void gsel(OptInt id, OptInt mode, const std::source_location& location) {
        safe_call(location, [&] {
            using namespace internal;

            int p1 = id.value_or(0);
            int p2 = mode.value_or(0);

            // 指定されたIDのサーフェスを取得
            auto it = g_surfaces.find(p1);
            if (it == g_surfaces.end()) {
                return;  // 存在しないIDは無視
            }

            auto surface = it->second;

            // カレントサーフェスとして設定
            g_currentSurface = surface;
            g_currentScreenId = p1;  // GUI命令用にIDを保持

            // HspWindowの場合はウィンドウ操作
            auto pWindow = std::dynamic_pointer_cast<HspWindow>(surface);
            if (pWindow) {
                HWND hwnd = pWindow->getHwnd();
                switch (p2) {
                case -1:
                    // 非表示にする
                    ShowWindow(hwnd, SW_HIDE);
                    break;
                case 0:
                    // 特に影響なし（描画先のみ変更）
                    break;
                case 1:
                    // アクティブにする
                    ShowWindow(hwnd, SW_SHOW);
                    SetForegroundWindow(hwnd);
                    break;
                case 2:
                    // アクティブ＋最前面
                    ShowWindow(hwnd, SW_SHOW);
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    SetForegroundWindow(hwnd);
                    break;
                }
            }
        });
    }

    // ============================================================
    // gmode - 画面コピーモード設定（HSP互換）
    // カレントサーフェスのgmode設定を変更する
    // ============================================================
    void gmode(OptInt mode, OptInt size_x, OptInt size_y, OptInt blend_rate, const std::source_location& location) {
        safe_call(location, [&] {
            int m = mode.value_or(0);
            int sx = size_x.value_or(32);
            int sy = size_y.value_or(32);
            int br = blend_rate.value_or(0);

            // パラメータ範囲チェック
            if (m < 0 || m > 7) {
                throw HspError(ERR_OUT_OF_RANGE, "gmodeのモードは0～7の範囲で指定してください", location);
            }
            if (sx < 0 || sy < 0) {
                throw HspError(ERR_OUT_OF_RANGE, "gmodeのサイズは0以上を指定してください", location);
            }
            if (br < 0 || br > 256) {
                throw HspError(ERR_OUT_OF_RANGE, "gmodeのブレンド率は0～256の範囲で指定してください", location);
            }

            // カレントサーフェスのgmode設定を変更
            auto currentSurface = getCurrentSurface();
            if (currentSurface) {
                currentSurface->setGmode(m, sx, sy, br);
            }
        });
    }

    // ============================================================
    // gcopy - 画面コピー（HSP互換）
    // Direct2D 1.1: 共有ビットマップを使用して異なるサーフェス間でコピー
    // gmodeで指定された合成モードを適用する
    // ============================================================
    void gcopy(OptInt src_id, OptInt src_x, OptInt src_y, OptInt size_x, OptInt size_y, const std::source_location& location) {
        safe_call(location, [&] {
            using namespace internal;

            // カレントサーフェス（コピー先）を取得
            auto destSurface = getCurrentSurface();
            if (!destSurface) {
                throw HspError(ERR_INVALID_HANDLE, "gcopyのカレントサーフェスが無効です", location);
            }

            // サーフェスのgmode設定を取得
            int gmodeSizeX = destSurface->getGmodeSizeX();
            int gmodeSizeY = destSurface->getGmodeSizeY();

            int p1 = src_id.value_or(0);
            int p2 = src_x.value_or(0);
            int p3 = src_y.value_or(0);
            int p4 = size_x.value_or(gmodeSizeX);
            int p5 = size_y.value_or(gmodeSizeY);

            // コピー元サーフェスを取得
            auto srcIt = g_surfaces.find(p1);
            if (srcIt == g_surfaces.end()) {
                throw HspError(ERR_INVALID_HANDLE, "gcopyのコピー元サーフェスが見つかりません", location);
            }
            auto srcSurface = srcIt->second;

            // 共通実装ヘルパーを呼ぶ
            gcopy_impl(destSurface, srcSurface, p2, p3, p4, p5, location);
        });
    }

    // ============================================================
        // gzoom - 変倍して画面コピー（HSP互換）
        // Direct2D 1.1: 共有ビットマップを使用して異なるサーフェス間でコピー
        // gmodeの合成モードは使わない。省略された転送サイズだけgmode設定を参照する。
    // ============================================================
    void gzoom(OptInt dest_w, OptInt dest_h, OptInt src_id, OptInt src_x, OptInt src_y,
               OptInt src_w, OptInt src_h, OptInt mode, const std::source_location& location) {
        safe_call(location, [&] {
            using namespace internal;

            // カレントサーフェス（コピー先）を取得
            auto destSurface = getCurrentSurface();
            if (!destSurface) {
                throw HspError(ERR_INVALID_HANDLE, "gzoomのカレントサーフェスが無効です", location);
            }

            // サーフェスのgmode設定を取得
            int gmodeSizeX = destSurface->getGmodeSizeX();
            int gmodeSizeY = destSurface->getGmodeSizeY();

            int p1 = dest_w.value_or(gmodeSizeX);
            int p2 = dest_h.value_or(gmodeSizeY);
            int p3 = src_id.value_or(0);
            int p4 = src_x.value_or(0);
            int p5 = src_y.value_or(0);
            int p6 = src_w.value_or(gmodeSizeX);
            int p7 = src_h.value_or(gmodeSizeY);
            int p8 = mode.value_or(0);  // HSP既定: 補間なし（COLORONCOLOR相当）
            if (p8 < -1 || p8 > 2) {
                throw HspError(ERR_OUT_OF_RANGE,
                    "gzoomのモードは0(nearest)/1(linear)、またはHSP++拡張の-1/2を指定してください",
                    location);
            }

            // コピー元サーフェスを取得
            auto srcIt = g_surfaces.find(p3);
            if (srcIt == g_surfaces.end()) {
                throw HspError(ERR_INVALID_HANDLE, "gzoomのコピー元サーフェスが見つかりません", location);
            }
            auto srcSurface = srcIt->second;

            // 共通実装ヘルパーを呼ぶ
            gzoom_impl(destSurface, p1, p2, srcSurface, p4, p5, p6, p7, p8, location);
        });
    }

} // namespace hsppp
