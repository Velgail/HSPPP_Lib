// HspppLib/src/core/hsppp_screen.inl
// Screenクラスメンバ関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    // ============================================================
    // Screen クラスのメンバ関数実装
    // IDからグローバルマップを経由してSurfaceを取得する
    // ============================================================

    Screen& Screen::color(int r, int g, int b) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->color(r, g, b);
        }
        return *this;
    }

    Screen& Screen::pos(int x, int y) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->pos(x, y);
        }
        return *this;
    }

    Screen& Screen::mes(std::string_view text, OptInt sw) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->mes(text, sw.value_or(0));
        }
        return *this;
    }

    Screen& Screen::boxf(int x1, int y1, int x2, int y2) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->boxf(x1, y1, x2, y2);
        }
        return *this;
    }

    Screen& Screen::boxf() {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->boxf(0, 0, surface->getWidth(), surface->getHeight());
        }
        return *this;
    }

    Screen& Screen::cls(int mode) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->cls(mode);
        }
        return *this;
    }

    Screen& Screen::redraw(int mode) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        bool shouldUpdate = (mode == 1);  // p1=1の場合のみ画面更新
        int newMode = mode % 2;           // 0 or 1

        if (newMode == 0) {
            // バッチモード開始
            if (!surface->isDrawing()) {
                surface->beginDraw();
            }
            surface->setRedrawMode(0);
        }
        else {
            // 即時反映モード
            surface->setRedrawMode(1);
            if (shouldUpdate && surface->isDrawing()) {
                surface->endDrawAndPresent();
            }
        }
        return *this;
    }

    Screen& Screen::select() {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            g_currentSurface = surface;
            g_currentScreenId = m_id;  // GUI命令用にIDを保持
        }
        return *this;
    }

    int Screen::width() const {
        auto surface = getSurfaceById(m_id);
        if (!surface) return 0;

        // HspWindowの場合は現在のクライアントサイズを返す
        auto pWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (pWindow) {
            int w, h;
            pWindow->getCurrentClientSize(w, h);
            return w;
        }

        return surface->getWidth();
    }

    int Screen::height() const {
        auto surface = getSurfaceById(m_id);
        if (!surface) return 0;

        // HspWindowの場合は現在のクライアントサイズを返す
        auto pWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (pWindow) {
            int w, h;
            pWindow->getCurrentClientSize(w, h);
            return h;
        }

        return surface->getHeight();
    }

    Screen& Screen::line(int x2, int y2) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            int startX = surface->getCurrentX();
            int startY = surface->getCurrentY();
            surface->line(x2, y2, startX, startY, false);
        }
        return *this;
    }

    Screen& Screen::line(int x2, int y2, int x1, int y1) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->line(x2, y2, x1, y1, true);
        }
        return *this;
    }

    Screen& Screen::circle(int x1, int y1, int x2, int y2, int fillMode) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->circle(x1, y1, x2, y2, fillMode);
        }
        return *this;
    }

    Screen& Screen::pset(int x, int y) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->pset(x, y);
        }
        return *this;
    }

    Screen& Screen::pset() {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            int px = surface->getCurrentX();
            int py = surface->getCurrentY();
            surface->pset(px, py);
        }
        return *this;
    }

    Screen& Screen::pget(int x, int y) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            int r, g, b;
            surface->pget(x, y, r, g, b);
        }
        return *this;
    }

    Screen& Screen::pget() {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            int px = surface->getCurrentX();
            int py = surface->getCurrentY();
            int r, g, b;
            surface->pget(px, py, r, g, b);
        }
        return *this;
    }

    Screen& Screen::gradf(int x, int y, int w, int h, int mode, int color1, int color2) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->gradf(x, y, w, h, mode, color1, color2);
        }
        return *this;
    }

    Screen& Screen::grect(int cx, int cy, double angle, int w, int h) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->grect(cx, cy, angle, w, h);
        }
        return *this;
    }

    Screen& Screen::font(std::string_view fontName, int size, int style) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->font(fontName, size, style);
        }
        return *this;
    }

    Screen& Screen::sysfont(int type) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->sysfont(type);
        }
        return *this;
    }

    Screen& Screen::title(std::string_view title) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        auto pWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (pWindow) {
            pWindow->setTitle(title);
        }
        return *this;
    }

    Screen& Screen::width(int clientW, int clientH, int posX, int posY, int option) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        auto pWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (!pWindow) return *this;

        // サイズ変更（-1以外の値が指定された場合、またはoption=1の場合）
        if (clientW >= 0 || clientH >= 0) {
            // 現在のサイズを取得
            HWND hwnd = pWindow->getHwnd();
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int newW = (clientW >= 0) ? clientW : (clientRect.right - clientRect.left);
            int newH = (clientH >= 0) ? clientH : (clientRect.bottom - clientRect.top);
            pWindow->setClientSize(newW, newH);
        }

        // 位置変更
        if (option == 0) {
            // option=0: 負の値は現在の位置を維持
            if (posX >= 0 || posY >= 0) {
                HWND hwnd = pWindow->getHwnd();
                RECT windowRect;
                GetWindowRect(hwnd, &windowRect);
                int newX = (posX >= 0) ? posX : windowRect.left;
                int newY = (posY >= 0) ? posY : windowRect.top;
                pWindow->setWindowPos(newX, newY);
            }
        }
        else {
            // option=1: 負の値も含めて設定（マルチモニタ対応）
            HWND hwnd = pWindow->getHwnd();
            RECT windowRect;
            GetWindowRect(hwnd, &windowRect);
            int newX = (posX != -1 || option == 1) ? posX : windowRect.left;
            int newY = (posY != -1 || option == 1) ? posY : windowRect.top;
            if (newX != -1 || newY != -1 || option == 1) {
                // option=1の場合は-1も有効な座標として扱う
                if (option == 1) {
                    pWindow->setWindowPos(posX, posY);
                }
                else if (newX >= 0 || newY >= 0) {
                    pWindow->setWindowPos(
                        (newX >= 0) ? newX : windowRect.left,
                        (newY >= 0) ? newY : windowRect.top
                    );
                }
            }
        }

        return *this;
    }

    Screen& Screen::groll(int scrollX, int scrollY) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        auto pWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (!pWindow) return *this;

        pWindow->setScroll(scrollX, scrollY);
        return *this;
    }

    int Screen::mousex() const {
        auto surface = getSurfaceById(m_id);
        auto pWindow = surface ? std::dynamic_pointer_cast<internal::HspWindow>(surface) : nullptr;

        POINT pt;
        GetCursorPos(&pt);

        if (pWindow && pWindow->getHwnd()) {
            ScreenToClient(pWindow->getHwnd(), &pt);
        }

        return pt.x;
    }

    int Screen::mousey() const {
        auto surface = getSurfaceById(m_id);
        auto pWindow = surface ? std::dynamic_pointer_cast<internal::HspWindow>(surface) : nullptr;

        POINT pt;
        GetCursorPos(&pt);

        if (pWindow && pWindow->getHwnd()) {
            ScreenToClient(pWindow->getHwnd(), &pt);
        }

        return pt.y;
    }

    Screen& Screen::picload(std::string_view filename, int mode) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->picload(filename, mode);
        }
        return *this;
    }

    Screen& Screen::bmpsave(std::string_view filename) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->bmpsave(filename);
        }
        return *this;
    }

    // ============================================================
    // 画面コピー・変形描画（OOP版）
    // ============================================================

    Screen& Screen::gmode(int mode, int sizeX, int sizeY, int blendRate) {
        auto surface = getSurfaceById(m_id);
        if (surface) {
            surface->setGmode(mode, sizeX, sizeY, blendRate);
        }
        return *this;
    }

    Screen& Screen::gcopy(int srcId, int srcX, int srcY, OptInt sizeX, OptInt sizeY) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        // このサーフェスのgmode設定を取得
        int gmodeMode = surface->getGmodeMode();
        int gmodeSizeX = surface->getGmodeSizeX();
        int gmodeSizeY = surface->getGmodeSizeY();
        int gmodeBlendRate = surface->getGmodeBlendRate();

        int copyW = sizeX.value_or(gmodeSizeX);
        int copyH = sizeY.value_or(gmodeSizeY);

        // コピー元サーフェスを取得
        auto srcSurface = getSurfaceById(srcId);
        if (!srcSurface) return *this;

        auto srcBitmap = srcSurface->getTargetBitmap();
        if (!srcBitmap) return *this;

        auto destContext = surface->getDeviceContext();
        if (!destContext) return *this;

        // カレントポジションを取得
        int destX = surface->getCurrentX();
        int destY = surface->getCurrentY();

        // 描画モードに応じて処理
        bool autoManage = (surface->getRedrawMode() == 1 && !surface->isDrawing());
        if (autoManage) {
            surface->beginDraw();
        }
        if (!surface->isDrawing()) return *this;

        // コピー元の領域
        D2D1_RECT_F srcRect = D2D1::RectF(
            static_cast<FLOAT>(srcX),
            static_cast<FLOAT>(srcY),
            static_cast<FLOAT>(srcX + copyW),
            static_cast<FLOAT>(srcY + copyH)
        );

        // コピー先の領域
        D2D1_RECT_F destRect = D2D1::RectF(
            static_cast<FLOAT>(destX),
            static_cast<FLOAT>(destY),
            static_cast<FLOAT>(destX + copyW),
            static_cast<FLOAT>(destY + copyH)
        );

        // コピーモードに応じた処理
        FLOAT opacity = 1.0f;
        D2D1_PRIMITIVE_BLEND primitiveBlend = D2D1_PRIMITIVE_BLEND_SOURCE_OVER;

        if (gmodeMode >= 3 && gmodeMode <= 6) {
            opacity = gmodeBlendRate / 256.0f;
        }

        if (gmodeMode == 5) {
            primitiveBlend = D2D1_PRIMITIVE_BLEND_ADD;
        } else if (gmodeMode == 6) {
            primitiveBlend = D2D1_PRIMITIVE_BLEND_MIN;
        }

        destContext->SetPrimitiveBlend(primitiveBlend);

        destContext->DrawBitmap(
            srcBitmap,
            destRect,
            opacity,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            srcRect
        );

        if (primitiveBlend != D2D1_PRIMITIVE_BLEND_SOURCE_OVER) {
            destContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
        }

        if (autoManage) {
            surface->endDrawAndPresent();
        }

        return *this;
    }

    Screen& Screen::gzoom(int destW, int destH, int srcId, int srcX, int srcY, OptInt srcW, OptInt srcH, int mode) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        // このサーフェスのgmode設定を取得
        int gmodeMode = surface->getGmodeMode();
        int gmodeSizeX = surface->getGmodeSizeX();
        int gmodeSizeY = surface->getGmodeSizeY();
        int gmodeBlendRate = surface->getGmodeBlendRate();

        int copyW = srcW.value_or(gmodeSizeX);
        int copyH = srcH.value_or(gmodeSizeY);

        // コピー元サーフェスを取得
        auto srcSurface = getSurfaceById(srcId);
        if (!srcSurface) return *this;

        auto srcBitmap = srcSurface->getTargetBitmap();
        if (!srcBitmap) return *this;

        auto destContext = surface->getDeviceContext();
        if (!destContext) return *this;

        // カレントポジションを取得
        int destX = surface->getCurrentX();
        int destY = surface->getCurrentY();

        // 描画モードに応じて処理
        bool autoManage = (surface->getRedrawMode() == 1 && !surface->isDrawing());
        if (autoManage) {
            surface->beginDraw();
        }
        if (!surface->isDrawing()) return *this;

        // コピー元の領域
        D2D1_RECT_F srcRect = D2D1::RectF(
            static_cast<FLOAT>(srcX),
            static_cast<FLOAT>(srcY),
            static_cast<FLOAT>(srcX + copyW),
            static_cast<FLOAT>(srcY + copyH)
        );

        // コピー先の領域（変倍）
        D2D1_RECT_F destRect = D2D1::RectF(
            static_cast<FLOAT>(destX),
            static_cast<FLOAT>(destY),
            static_cast<FLOAT>(destX + destW),
            static_cast<FLOAT>(destY + destH)
        );

        // コピーモードに応じた処理
        FLOAT opacity = 1.0f;
        D2D1_PRIMITIVE_BLEND primitiveBlend = D2D1_PRIMITIVE_BLEND_SOURCE_OVER;

        if (gmodeMode >= 3 && gmodeMode <= 6) {
            opacity = gmodeBlendRate / 256.0f;
        }

        if (gmodeMode == 5) {
            primitiveBlend = D2D1_PRIMITIVE_BLEND_ADD;
        } else if (gmodeMode == 6) {
            primitiveBlend = D2D1_PRIMITIVE_BLEND_MIN;
        }

        D2D1_BITMAP_INTERPOLATION_MODE interpMode =
            (mode == 1) ? D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
                        : D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;

        destContext->SetPrimitiveBlend(primitiveBlend);

        destContext->DrawBitmap(
            srcBitmap,
            destRect,
            opacity,
            interpMode,
            srcRect
        );

        if (primitiveBlend != D2D1_PRIMITIVE_BLEND_SOURCE_OVER) {
            destContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
        }

        if (autoManage) {
            surface->endDrawAndPresent();
        }

        return *this;
    }

    Screen& Screen::grotate(int srcId, int srcX, int srcY, double angle, OptInt dstW, OptInt dstH) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        // このサーフェスのgmode設定を取得
        int gmodeSizeX = surface->getGmodeSizeX();
        int gmodeSizeY = surface->getGmodeSizeY();

        int destW = dstW.value_or(gmodeSizeX);
        int destH = dstH.value_or(gmodeSizeY);

        // コピー元サーフェスを取得
        auto srcSurface = getSurfaceById(srcId);
        if (!srcSurface) return *this;

        auto* pSrcBitmap = srcSurface->getTargetBitmap();
        if (!pSrcBitmap) return *this;

        surface->grotate(pSrcBitmap, srcX, srcY, gmodeSizeX, gmodeSizeY, angle, destW, destH);

        return *this;
    }

    Screen& Screen::gsquare(int srcId, const Quad& dst) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        int dstX[4] = { dst.v[0].x, dst.v[1].x, dst.v[2].x, dst.v[3].x };
        int dstY[4] = { dst.v[0].y, dst.v[1].y, dst.v[2].y, dst.v[3].y };

        surface->gsquare(dstX, dstY, nullptr, nullptr, nullptr);

        return *this;
    }

    Screen& Screen::gsquare(int srcId, const Quad& dst, const QuadUV& src) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        int dstX[4] = { dst.v[0].x, dst.v[1].x, dst.v[2].x, dst.v[3].x };
        int dstY[4] = { dst.v[0].y, dst.v[1].y, dst.v[2].y, dst.v[3].y };
        int srcX[4] = { src.v[0].x, src.v[1].x, src.v[2].x, src.v[3].x };
        int srcY[4] = { src.v[0].y, src.v[1].y, src.v[2].y, src.v[3].y };

        if (srcId >= 0) {
            auto srcSurface = getSurfaceById(srcId);
            if (srcSurface) {
                auto* pSrcBitmap = srcSurface->getTargetBitmap();
                surface->gsquare(dstX, dstY, pSrcBitmap, srcX, srcY);
            }
        } else {
            surface->gsquare(dstX, dstY, nullptr, nullptr, nullptr);
        }

        return *this;
    }

    Screen& Screen::gsquare(int srcId, const Quad& dst, const QuadColors& colors) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        int dstX[4] = { dst.v[0].x, dst.v[1].x, dst.v[2].x, dst.v[3].x };
        int dstY[4] = { dst.v[0].y, dst.v[1].y, dst.v[2].y, dst.v[3].y };
        int cols[4] = { colors.colors[0], colors.colors[1], colors.colors[2], colors.colors[3] };

        surface->gsquareGrad(dstX, dstY, cols);

        return *this;
    }

    // ============================================================
    // ウィンドウ表示制御（OOP版）
    // ============================================================

    Screen& Screen::show() {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        auto pWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (pWindow && pWindow->getHwnd()) {
            ShowWindow(pWindow->getHwnd(), SW_SHOW);
            SetForegroundWindow(pWindow->getHwnd());
        }
        return *this;
    }

    Screen& Screen::hide() {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        auto pWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (pWindow && pWindow->getHwnd()) {
            ShowWindow(pWindow->getHwnd(), SW_HIDE);
        }
        return *this;
    }

    Screen& Screen::activate() {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        auto pWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (pWindow && pWindow->getHwnd()) {
            HWND hwnd = pWindow->getHwnd();
            ShowWindow(hwnd, SW_SHOW);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetForegroundWindow(hwnd);
        }
        return *this;
    }

    // ============================================================
    // Cel描画（OOP版・Screen側主体）
    // ============================================================

    Screen& Screen::celput(const Cel& cel, int cellIndex, OptInt x, OptInt y) {
        auto surface = getSurfaceById(m_id);
        if (!surface || !cel.valid()) return *this;

        // カレントを一時的に切り替えてcelput呼び出し
        auto savedSurface = g_currentSurface.lock();
        auto savedId = g_currentScreenId;

        g_currentSurface = surface;
        g_currentScreenId = m_id;

        hsppp::celput(cel.id(), cellIndex, x, y);

        g_currentSurface = savedSurface;
        g_currentScreenId = savedId;

        return *this;
    }

    // ============================================================
    // GUIオブジェクト生成（OOP版・ウィンドウ指定）
    // ============================================================

    int Screen::button(std::string_view name, std::function<int()> callback, bool isGosub) {
        // カレントを一時的に切り替えてbutton呼び出し
        auto savedSurface = g_currentSurface.lock();
        auto savedId = g_currentScreenId;

        auto surface = getSurfaceById(m_id);
        if (surface) {
            g_currentSurface = surface;
            g_currentScreenId = m_id;
        }

        int result = hsppp::button(name, callback, isGosub);

        g_currentSurface = savedSurface;
        g_currentScreenId = savedId;

        return result;
    }

    int Screen::input(std::shared_ptr<std::string> var, int maxLength, int mode) {
        // カレントを一時的に切り替えてinput呼び出し
        auto savedSurface = g_currentSurface.lock();
        auto savedId = g_currentScreenId;

        auto surface = getSurfaceById(m_id);
        if (surface) {
            g_currentSurface = surface;
            g_currentScreenId = m_id;
        }

        int result = hsppp::input(var, maxLength, mode);

        g_currentSurface = savedSurface;
        g_currentScreenId = savedId;

        return result;
    }

    int Screen::mesbox(std::shared_ptr<std::string> var, int maxLength, int mode) {
        // カレントを一時的に切り替えてmesbox呼び出し
        auto savedSurface = g_currentSurface.lock();
        auto savedId = g_currentScreenId;

        auto surface = getSurfaceById(m_id);
        if (surface) {
            g_currentSurface = surface;
            g_currentScreenId = m_id;
        }

        int result = hsppp::mesbox(var, maxLength, mode);

        g_currentSurface = savedSurface;
        g_currentScreenId = savedId;

        return result;
    }

    // ============================================================
    // マウス制御（OOP版）
    // ============================================================

    Screen& Screen::mouse(int x, int y) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        auto pWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (pWindow && pWindow->getHwnd()) {
            // クライアント座標をスクリーン座標に変換
            POINT pt = { x, y };
            ClientToScreen(pWindow->getHwnd(), &pt);
            SetCursorPos(pt.x, pt.y);
        }
        return *this;
    }

} // namespace hsppp
