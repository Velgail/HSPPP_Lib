// HspppLib/src/core/hsppp_screen.inl
// Screenクラスメンバ関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    // ============================================================
    // Screen クラスのコンストラクタ実装
    // ============================================================

    Screen::Screen(int id) noexcept
        : m_id(id), m_valid(getSurfaceById(id) != nullptr) {}

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
        int gmodeSizeX = surface->getGmodeSizeX();
        int gmodeSizeY = surface->getGmodeSizeY();

        int copyW = sizeX.value_or(gmodeSizeX);
        int copyH = sizeY.value_or(gmodeSizeY);

        // コピー元サーフェスを取得
        auto srcSurface = getSurfaceById(srcId);
        if (!srcSurface) return *this;

        try {
            // 共通実装ヘルパーを呼ぶ
            hsppp::internal::gcopy_impl(surface, srcSurface, srcX, srcY, copyW, copyH, std::source_location::current());
        }
        catch (const std::exception& e) {
            // エラーをサイレントに抑える（OOP版はエラーハンドリングを呼び出し側に任せる）
        }

        return *this;
    }

    Screen& Screen::gzoom(int destW, int destH, int srcId, int srcX, int srcY, OptInt srcW, OptInt srcH, int mode) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        // このサーフェスのgmode設定を取得
        int gmodeSizeX = surface->getGmodeSizeX();
        int gmodeSizeY = surface->getGmodeSizeY();

        int copyW = srcW.value_or(gmodeSizeX);
        int copyH = srcH.value_or(gmodeSizeY);

        // コピー元サーフェスを取得
        auto srcSurface = getSurfaceById(srcId);
        if (!srcSurface) return *this;

        try {
            // 共通実装ヘルパーを呼ぶ
            hsppp::internal::gzoom_impl(surface, destW, destH, srcSurface, srcX, srcY, copyW, copyH, mode, std::source_location::current());
        }
        catch (const std::exception& e) {
            // エラーをサイレントに抑える（OOP版はエラーハンドリングを呼び出し側に任せる）
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
    // Cel描画 - 内部ヘルパー関数
    // ============================================================

    namespace internal {
        
        void celput_impl(std::shared_ptr<HspSurface> surface, int celId, int cellIndex, OptInt x, OptInt y) {
            // グローバルマップからCelDataを取得
            auto it = g_celDataMap.find(celId);
            if (it == g_celDataMap.end()) return;

            const auto& celData = it->second;
            
            // セル番号の範囲チェック
            int cellCount = celData.divX * celData.divY;
            if (cellIndex < 0 || cellIndex >= cellCount) {
                return;  // 無効なセル番号は無視
            }

            // セルのサイズを計算
            int cellWidth = celData.width / celData.divX;
            int cellHeight = celData.height / celData.divY;

            // セルのソース座標を計算
            int srcX = (cellIndex % celData.divX) * cellWidth;
            int srcY = (cellIndex / celData.divX) * cellHeight;

            D2D1_RECT_F srcRect = D2D1::RectF(
                static_cast<float>(srcX),
                static_cast<float>(srcY),
                static_cast<float>(srcX + cellWidth),
                static_cast<float>(srcY + cellHeight)
            );

            // 描画位置（省略時は現在のpos）
            int destX = x.value_or(surface->getCurrentX());
            int destY = y.value_or(surface->getCurrentY());

            D2D1_RECT_F destRect = D2D1::RectF(
                static_cast<float>(destX),
                static_cast<float>(destY),
                static_cast<float>(destX + cellWidth),
                static_cast<float>(destY + cellHeight)
            );

            // サーフェスのcelput実装を呼ぶ
            surface->celput(celData.pBitmap.Get(), srcRect, destRect);
        }

    } // namespace internal

    // ============================================================
    // Cel描画（OOP版・ヘルパーを呼ぶだけ）
    // ============================================================

    Screen& Screen::celput(const Cel& cel, int cellIndex, OptInt x, OptInt y) {
        if (!cel.valid()) return *this;

        auto surface = getSurfaceById(m_id);
        if (!surface) return *this;

        internal::celput_impl(surface, cel.id(), cellIndex, x, y);
        return *this;
    }

    // ============================================================
    // GUIオブジェクト生成 - 内部ヘルパー関数
    // グローバル版とOOP版の両方から呼ばれる共通実装
    // ============================================================

    namespace internal {
        
        int button_impl(std::shared_ptr<HspSurface> surface, int windowId,
                       std::string_view name, std::function<int()> callback, bool isGosub) {
            auto& objMgr = ObjectManager::getInstance();

            auto pHspWindow = std::dynamic_pointer_cast<HspWindow>(surface);
            if (!pHspWindow) {
                return -1;  // バッファには作成不可
            }

            // オブジェクトサイズを取得
            int objW, objH, objSpace;
            objMgr.getObjSize(objW, objH, objSpace);

            // 現在のカレントポジションを取得
            int posX = surface->getCurrentX();
            int posY = surface->getCurrentY();

            // Win32ボタンを作成
            HWND hwndParent = pHspWindow->getHwnd();
            std::wstring wname = Utf8ToWide(name);

            HWND hwndButton = CreateWindowExW(
                WS_EX_NOPARENTNOTIFY,
                L"BUTTON",
                wname.c_str(),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | BS_PUSHBUTTON,
                posX, posY, objW, objH,
                hwndParent,
                (HMENU)(INT_PTR)(objMgr.getNextId()),
                GetModuleHandle(nullptr),
                nullptr
            );

            if (hwndButton) {
                SetWindowPos(hwndButton, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }

            if (!hwndButton) {
                return -1;
            }

            // フォント設定
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessageW(hwndButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            // ObjectInfoを登録
            ObjectInfo info;
            info.type = ObjectType::Button;
            info.hwnd = hwndButton;
            info.windowId = windowId;
            info.x = posX;
            info.y = posY;
            info.width = objW;
            info.height = objH;
            info.callback = std::move(callback);
            info.isGosub = isGosub;
            info.enabled = true;
            info.focusSkipMode = 1;

            int objectId = objMgr.registerObject(info);

            // カレントポジションを次の行に移動
            int nextY = posY + std::max(objH, objSpace);
            surface->pos(posX, nextY);

            return objectId;
        }

        int input_impl(std::shared_ptr<HspSurface> surface, int windowId,
                      std::shared_ptr<std::string> var, int maxLength, int mode) {
            auto& objMgr = ObjectManager::getInstance();

            auto pHspWindow = std::dynamic_pointer_cast<HspWindow>(surface);
            if (!pHspWindow) {
                return -1;
            }

            int objW, objH, objSpace;
            objMgr.getObjSize(objW, objH, objSpace);

            int w = (maxLength > 0) ? maxLength : objW;
            int h = objH;
            int maxChars = 256;

            int posX = surface->getCurrentX();
            int posY = surface->getCurrentY();

            HWND hwndParent = pHspWindow->getHwnd();
            std::wstring wtext = Utf8ToWide(*var);

            HWND hwndEdit = CreateWindowExW(
                WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
                L"EDIT",
                wtext.c_str(),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | ES_AUTOHSCROLL,
                posX, posY, w, h,
                hwndParent,
                (HMENU)(INT_PTR)(objMgr.getNextId()),
                GetModuleHandle(nullptr),
                nullptr
            );

            if (!hwndEdit) {
                return -1;
            }

            SetWindowPos(hwndEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            SendMessageW(hwndEdit, EM_SETLIMITTEXT, (WPARAM)maxChars, 0);

            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessageW(hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

            ObjectInfo info;
            info.type = ObjectType::Input;
            info.hwnd = hwndEdit;
            info.windowId = windowId;
            info.x = posX;
            info.y = posY;
            info.width = w;
            info.height = h;
            info.ownedStrVar = var;
            info.maxLength = maxChars;
            info.enabled = true;
            info.focusSkipMode = 1;

            int objectId = objMgr.registerObject(info);

            int nextY = posY + std::max(h, objSpace);
            surface->pos(posX, nextY);

            return objectId;
        }

        int mesbox_impl(std::shared_ptr<HspSurface> surface, int windowId,
                       std::shared_ptr<std::string> var, int maxLength, int mode) {
            auto& objMgr = ObjectManager::getInstance();

            auto pHspWindow = std::dynamic_pointer_cast<HspWindow>(surface);
            if (!pHspWindow) {
                return -1;
            }

            int objW, objH, objSpace;
            objMgr.getObjSize(objW, objH, objSpace);

            int w = (maxLength > 0) ? maxLength : objW;
            int h = objH * 3;
            int styleVal = mode;
            int maxChars = 32767;

            int posX = surface->getCurrentX();
            int posY = surface->getCurrentY();

            DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_CLIPSIBLINGS | ES_MULTILINE;

            if ((styleVal & 1) == 0) {
                dwStyle |= ES_READONLY;
            }
            if (styleVal & 4) {
                dwStyle |= WS_HSCROLL;
            }
            if ((styleVal & 8) == 0) {
                dwStyle |= ES_AUTOVSCROLL;
            }

            HWND hwndParent = pHspWindow->getHwnd();

            // 改行文字を\nから\r\nに変換
            std::string convertedText = *var;
            size_t pos = 0;
            while ((pos = convertedText.find('\n', pos)) != std::string::npos) {
                if (pos == 0 || convertedText[pos - 1] != '\r') {
                    convertedText.insert(pos, "\r");
                    pos += 2;
                } else {
                    pos += 1;
                }
            }
            std::wstring wtext = Utf8ToWide(convertedText);

            HWND hwndEdit = CreateWindowExW(
                WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
                L"EDIT",
                wtext.c_str(),
                dwStyle,
                posX, posY, w, h,
                hwndParent,
                (HMENU)(INT_PTR)(objMgr.getNextId()),
                GetModuleHandle(nullptr),
                nullptr
            );

            if (!hwndEdit) {
                return -1;
            }

            SetWindowPos(hwndEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            SendMessageW(hwndEdit, EM_SETLIMITTEXT, (WPARAM)maxChars, 0);

            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessageW(hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

            ObjectInfo info;
            info.type = ObjectType::Mesbox;
            info.hwnd = hwndEdit;
            info.windowId = windowId;
            info.x = posX;
            info.y = posY;
            info.width = w;
            info.height = h;
            info.ownedStrVar = var;
            info.maxLength = maxChars;
            info.enabled = true;
            info.focusSkipMode = 1;

            int objectId = objMgr.registerObject(info);

            int nextY = posY + std::max(h, objSpace);
            surface->pos(posX, nextY);

            return objectId;
        }

    } // namespace internal

    // ============================================================
    // GUIオブジェクト生成（OOP版・ヘルパーを呼ぶだけ）
    // ============================================================

    int Screen::button(std::string_view name, std::function<int()> callback, bool isGosub) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return -1;
        return internal::button_impl(surface, m_id, name, std::move(callback), isGosub);
    }

    int Screen::input(std::shared_ptr<std::string> var, int maxLength, int mode) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return -1;
        return internal::input_impl(surface, m_id, var, maxLength, mode);
    }

    int Screen::mesbox(std::shared_ptr<std::string> var, int maxLength, int mode) {
        auto surface = getSurfaceById(m_id);
        if (!surface) return -1;
        return internal::mesbox_impl(surface, m_id, var, maxLength, mode);
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
