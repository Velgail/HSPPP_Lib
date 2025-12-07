// HspppLib/src/core/hsppp_copy.inl
// gsel, gmode, gcopy, gzoom関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    // ============================================================
    // gsel - 描画先指定、ウィンドウ最前面、非表示設定（HSP互換）
    // ============================================================
    void gsel(OptInt id, OptInt mode, const std::source_location& location) {
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
    }

    // ============================================================
    // gmode - 画面コピーモード設定（HSP互換）
    // ============================================================
    void gmode(OptInt mode, OptInt size_x, OptInt size_y, OptInt blend_rate, const std::source_location& location) {
        g_gmodeMode = mode.value_or(0);
        g_gmodeSizeX = size_x.value_or(32);
        g_gmodeSizeY = size_y.value_or(32);
        g_gmodeBlendRate = blend_rate.value_or(0);
    }

    // ============================================================
    // gcopy - 画面コピー（HSP互換）
    // Direct2D 1.1: 共有ビットマップを使用して異なるサーフェス間でコピー
    // ============================================================
    void gcopy(OptInt src_id, OptInt src_x, OptInt src_y, OptInt size_x, OptInt size_y, const std::source_location& location) {
        using namespace internal;

        int p1 = src_id.value_or(0);
        int p2 = src_x.value_or(0);
        int p3 = src_y.value_or(0);
        int p4 = size_x.value_or(g_gmodeSizeX);
        int p5 = size_y.value_or(g_gmodeSizeY);

        try {
            // コピー元サーフェスを取得
            auto srcIt = g_surfaces.find(p1);
            if (srcIt == g_surfaces.end()) {
                throw HspError(ERR_INVALID_HANDLE, "gcopyのコピー元サーフェスが見つかりません", location);
            }
            auto srcSurface = srcIt->second;

            // カレントサーフェス（コピー先）を取得
            auto destSurface = getCurrentSurface();
            if (!destSurface) {
                throw HspError(ERR_INVALID_HANDLE, "gcopyのカレントサーフェスが無効です", location);
            }

            // コピー元のビットマップを取得（Direct2D 1.1の共有ビットマップ）
            auto srcBitmap = srcSurface->getTargetBitmap();
            if (!srcBitmap) {
                throw HspError(ERR_INVALID_HANDLE, "gcopyのコピー元ビットマップが無効です", location);
            }

            // コピー先のDeviceContextを取得
            auto destContext = destSurface->getDeviceContext();
            if (!destContext) {
                throw HspError(ERR_INVALID_HANDLE, "gcopyのコピー先DeviceContextが無効です", location);
            }

            // カレントポジションを取得
            int destX = destSurface->getCurrentX();
            int destY = destSurface->getCurrentY();

            // 描画モードに応じて処理
            bool wasDrawing = g_isDrawing;
            if (g_redrawMode == 1 && !wasDrawing) {
                beginDrawIfNeeded();
            }

            // コピー元の領域
            D2D1_RECT_F srcRect = D2D1::RectF(
                static_cast<FLOAT>(p2),
                static_cast<FLOAT>(p3),
                static_cast<FLOAT>(p2 + p4),
                static_cast<FLOAT>(p3 + p5)
            );

            // コピー先の領域（カレントポジションから）
            D2D1_RECT_F destRect = D2D1::RectF(
                static_cast<FLOAT>(destX),
                static_cast<FLOAT>(destY),
                static_cast<FLOAT>(destX + p4),
                static_cast<FLOAT>(destY + p5)
            );

            // コピーモードに応じた処理
            FLOAT opacity = 1.0f;
            D2D1_PRIMITIVE_BLEND primitiveBlend = D2D1_PRIMITIVE_BLEND_SOURCE_OVER;

            if (g_gmodeMode >= 3 && g_gmodeMode <= 6) {
                // 半透明・加算・減算モードの場合はブレンド率を適用
                opacity = g_gmodeBlendRate / 256.0f;
            }

            if (g_gmodeMode == 5) {
                // 加算ブレンド
                primitiveBlend = D2D1_PRIMITIVE_BLEND_ADD;
            } else if (g_gmodeMode == 6) {
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

            if (g_redrawMode == 1 && !wasDrawing) {
                endDrawAndPresent();
            }
        }
        catch (const std::exception& e) {
            // 例外を書き換え
            throw HspError(ERR_SYSTEM_ERROR, std::format("gcopyでエラーが発生しました: {}", e.what()), location);
        }
    }

    // ============================================================
    // gzoom - 変倍して画面コピー（HSP互換）
    // Direct2D 1.1: 共有ビットマップを使用して異なるサーフェス間でコピー
    // ============================================================
    void gzoom(OptInt dest_w, OptInt dest_h, OptInt src_id, OptInt src_x, OptInt src_y,
               OptInt src_w, OptInt src_h, OptInt mode, const std::source_location& location) {
        using namespace internal;

        int p1 = dest_w.value_or(g_gmodeSizeX);
        int p2 = dest_h.value_or(g_gmodeSizeY);
        int p3 = src_id.value_or(0);
        int p4 = src_x.value_or(0);
        int p5 = src_y.value_or(0);
        int p6 = src_w.value_or(g_gmodeSizeX);
        int p7 = src_h.value_or(g_gmodeSizeY);
        int p8 = mode.value_or(0);

        // コピー元サーフェスを取得
        auto srcIt = g_surfaces.find(p3);
        if (srcIt == g_surfaces.end()) return;
        auto srcSurface = srcIt->second;

        // カレントサーフェス（コピー先）を取得
        auto destSurface = getCurrentSurface();
        if (!destSurface) return;

        // コピー元のビットマップを取得（Direct2D 1.1の共有ビットマップ）
        auto srcBitmap = srcSurface->getTargetBitmap();
        if (!srcBitmap) return;

        // コピー先のDeviceContextを取得
        auto destContext = destSurface->getDeviceContext();
        if (!destContext) return;

        // カレントポジションを取得
        int destX = destSurface->getCurrentX();
        int destY = destSurface->getCurrentY();

        // 描画モードに応じて処理
        bool wasDrawing = g_isDrawing;
        if (g_redrawMode == 1 && !wasDrawing) {
            beginDrawIfNeeded();
        }

        // コピー元の領域
        D2D1_RECT_F srcRect = D2D1::RectF(
            static_cast<FLOAT>(p4),
            static_cast<FLOAT>(p5),
            static_cast<FLOAT>(p4 + p6),
            static_cast<FLOAT>(p5 + p7)
        );

        // コピー先の領域（変倍、カレントポジションから）
        D2D1_RECT_F destRect = D2D1::RectF(
            static_cast<FLOAT>(destX),
            static_cast<FLOAT>(destY),
            static_cast<FLOAT>(destX + p1),
            static_cast<FLOAT>(destY + p2)
        );

        // コピーモードに応じた処理（gcopyと同様にgmodeを尊重）
        FLOAT opacity = 1.0f;
        D2D1_PRIMITIVE_BLEND primitiveBlend = D2D1_PRIMITIVE_BLEND_SOURCE_OVER;

        if (g_gmodeMode >= 3 && g_gmodeMode <= 6) {
            opacity = g_gmodeBlendRate / 256.0f;
        }

        if (g_gmodeMode == 5) {
            // 加算ブレンド
            primitiveBlend = D2D1_PRIMITIVE_BLEND_ADD;
        } else if (g_gmodeMode == 6) {
            // 減算ブレンド（Direct2Dに直接対応がないためMINで近似）
            primitiveBlend = D2D1_PRIMITIVE_BLEND_MIN;
        }

        // 補間モード
        D2D1_BITMAP_INTERPOLATION_MODE interpMode =
            (p8 == 1) ? D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
                      : D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;

        destContext->SetPrimitiveBlend(primitiveBlend);

        // Direct2D 1.1では同じDeviceから作成されたビットマップを直接描画可能
        destContext->DrawBitmap(
            srcBitmap,
            destRect,
            opacity,
            interpMode,
            srcRect
        );

        // ブレンドモードをリセット
        if (primitiveBlend != D2D1_PRIMITIVE_BLEND_SOURCE_OVER) {
            destContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
        }

        if (g_redrawMode == 1 && !wasDrawing) {
            endDrawAndPresent();
        }
    }

} // namespace hsppp
