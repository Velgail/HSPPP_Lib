// HspppLib/src/core/hsppp_copy.inl
// gsel, gmode, gcopy, gzoom関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    namespace internal {
        // ============================================================
        // 内部ヘルパー関数: gcopy_impl()
        // gcopy/Screen::gcopyで共有されるコア実装
        // ============================================================
        void gcopy_impl(std::shared_ptr<hsppp::internal::HspSurface> destSurface, 
                       std::shared_ptr<hsppp::internal::HspSurface> srcSurface,
                       int srcX, int srcY, int sizeX, int sizeY,
                       const std::source_location& location) {
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

            // Direct2D 1.1では同じDeviceから作成されたビットマップを直接描画可能
            destContext->DrawBitmap(
                srcBitmap,
                destRect,
                opacity,
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
                srcRect
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

            // サーフェスのgmode設定を取得
            int gmodeMode = destSurface->getGmodeMode();
            int gmodeBlendRate = destSurface->getGmodeBlendRate();

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

            // 補間モード
            D2D1_BITMAP_INTERPOLATION_MODE interpMode =
                (mode == 1) ? D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
                            : D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;

            destContext->SetPrimitiveBlend(primitiveBlend);

            // Direct2D 1.1では同じDeviceから作成されたビットマップを直接描画可能
            destContext->DrawBitmap(
                srcBitmap,
                destRectArea,
                opacity,
                interpMode,
                srcRect
            );

            // ブレンドモードをリセット
            if (primitiveBlend != D2D1_PRIMITIVE_BLEND_SOURCE_OVER) {
                destContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
            }

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
            if (m < 0 || m > 6) {
                throw HspError(ERR_OUT_OF_RANGE, "gmodeのモードは0～6の範囲で指定してください", location);
            }
            if (sx <= 0 || sy <= 0) {
                throw HspError(ERR_OUT_OF_RANGE, "gmodeのサイズは正の値を指定してください", location);
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
    // コピー先サーフェスのgmode設定を使用
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
    // コピー先サーフェスのgmode設定を使用
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
            int p8 = mode.value_or(0);

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
