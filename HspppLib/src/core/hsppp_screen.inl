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

} // namespace hsppp
