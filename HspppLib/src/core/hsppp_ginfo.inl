// HspppLib/src/core/hsppp_ginfo.inl
// ginfo, font, sysfont, title, width関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    // ============================================================
    // ginfo - ウィンドウ情報の取得（HSP互換）
    // ============================================================
    int ginfo(int type, const std::source_location& location) {
        // パラメータチェック
        if (type < 0 || type > 27) {
            throw HspError(ERR_OUT_OF_RANGE, "ginfoのtypeは0～27の範囲で指定してください", location);
        }
        using namespace internal;
        
        auto currentSurface = getCurrentSurface();
        auto pWindow = currentSurface ? std::dynamic_pointer_cast<HspWindow>(currentSurface) : nullptr;
        
        switch (type) {
        case 0:  // スクリーン上のマウスカーソルX座標
        {
            POINT pt;
            GetCursorPos(&pt);
            return pt.x;
        }
        case 1:  // スクリーン上のマウスカーソルY座標
        {
            POINT pt;
            GetCursorPos(&pt);
            return pt.y;
        }
        case 2:  // アクティブなウィンドウID
        {
            HWND hwndActive = GetForegroundWindow();
            // g_surfacesを検索してウィンドウIDを返す
            for (const auto& pair : g_surfaces) {
                auto pWin = std::dynamic_pointer_cast<HspWindow>(pair.second);
                if (pWin && pWin->getHwnd() == hwndActive) {
                    return pair.first;
                }
            }
            return -1;  // HSP以外のウィンドウがアクティブ
        }
        case 3:  // 操作先ウィンドウID
        {
            auto current = g_currentSurface.lock();
            if (current) {
                for (const auto& pair : g_surfaces) {
                    if (pair.second == current) {
                        return pair.first;
                    }
                }
            }
            return 0;
        }
        case 4:  // ウィンドウの左上X座標
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.left;
            }
            return 0;
        }
        case 5:  // ウィンドウの左上Y座標
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.top;
            }
            return 0;
        }
        case 6:  // ウィンドウの右下X座標
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.right;
            }
            return 0;
        }
        case 7:  // ウィンドウの右下Y座標
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.bottom;
            }
            return 0;
        }
        case 8:  // ウィンドウの描画基点X座標（grollで設定）
        {
            if (pWindow) {
                return pWindow->getScrollX();
            }
            return 0;
        }
        case 9:  // ウィンドウの描画基点Y座標（grollで設定）
        {
            if (pWindow) {
                return pWindow->getScrollY();
            }
            return 0;
        }
        case 10:  // ウィンドウ全体のXサイズ
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.right - rect.left;
            }
            return currentSurface ? currentSurface->getWidth() : 0;
        }
        case 11:  // ウィンドウ全体のYサイズ
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                return rect.bottom - rect.top;
            }
            return currentSurface ? currentSurface->getHeight() : 0;
        }
        case 12:  // クライアント領域Xサイズ
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetClientRect(pWindow->getHwnd(), &rect);
                return rect.right;
            }
            return currentSurface ? currentSurface->getWidth() : 0;
        }
        case 13:  // クライアント領域Yサイズ
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetClientRect(pWindow->getHwnd(), &rect);
                return rect.bottom;
            }
            return currentSurface ? currentSurface->getHeight() : 0;
        }
        case 14:  // 最後のmes出力Xサイズ
            return currentSurface ? currentSurface->getLastMesSizeX() : 0;
        case 15:  // 最後のmes出力Yサイズ
            return currentSurface ? currentSurface->getLastMesSizeY() : 0;
        case 16:  // 現在設定されているカラーコード(R)
        {
            if (currentSurface) {
                auto color = currentSurface->getCurrentColor();
                return static_cast<int>(color.r * 255.0f);
            }
            return 0;
        }
        case 17:  // 現在設定されているカラーコード(G)
        {
            if (currentSurface) {
                auto color = currentSurface->getCurrentColor();
                return static_cast<int>(color.g * 255.0f);
            }
            return 0;
        }
        case 18:  // 現在設定されているカラーコード(B)
        {
            if (currentSurface) {
                auto color = currentSurface->getCurrentColor();
                return static_cast<int>(color.b * 255.0f);
            }
            return 0;
        }
        case 19:  // デスクトップのカラーモード（常にフルカラー）
            return 0;
        case 20:  // デスクトップ全体のXサイズ
            return GetSystemMetrics(SM_CXSCREEN);
        case 21:  // デスクトップ全体のYサイズ
            return GetSystemMetrics(SM_CYSCREEN);
        case 22:  // カレントポジションのX座標
            return currentSurface ? currentSurface->getCurrentX() : 0;
        case 23:  // カレントポジションのY座標
            return currentSurface ? currentSurface->getCurrentY() : 0;
        case 24:  // メッセージ割り込み時のウィンドウID（未実装）
            return 0;
        case 25:  // 未使用ウィンドウID
        {
            for (int i = 0; ; ++i) {
                if (g_surfaces.find(i) == g_surfaces.end()) {
                    return i;
                }
            }
        }
        case 26:  // 画面の初期化Xサイズ
            return currentSurface ? currentSurface->getWidth() : 0;
        case 27:  // 画面の初期化Yサイズ
            return currentSurface ? currentSurface->getHeight() : 0;
        default:
            return 0;
        }
    }

    // ginfo_* マクロ/システム変数互換（C++では関数として提供）
    int ginfo_mx(const std::source_location& location) { return ginfo(ginfo_type_mx, location); }
    int ginfo_my(const std::source_location& location) { return ginfo(ginfo_type_my, location); }
    int ginfo_act(const std::source_location& location) { return ginfo(ginfo_type_act, location); }
    int ginfo_sel(const std::source_location& location) { return ginfo(ginfo_type_sel, location); }
    int ginfo_wx1(const std::source_location& location) { return ginfo(ginfo_type_wx1, location); }
    int ginfo_wy1(const std::source_location& location) { return ginfo(ginfo_type_wy1, location); }
    int ginfo_wx2(const std::source_location& location) { return ginfo(ginfo_type_wx2, location); }
    int ginfo_wy2(const std::source_location& location) { return ginfo(ginfo_type_wy2, location); }
    int ginfo_vx(const std::source_location& location) { return ginfo(ginfo_type_vx, location); }
    int ginfo_vy(const std::source_location& location) { return ginfo(ginfo_type_vy, location); }
    int ginfo_sizex(const std::source_location& location) { return ginfo(ginfo_type_sizex, location); }
    int ginfo_sizey(const std::source_location& location) { return ginfo(ginfo_type_sizey, location); }
    int ginfo_mesx(const std::source_location& location) { return ginfo(ginfo_type_mesx, location); }
    int ginfo_mesy(const std::source_location& location) { return ginfo(ginfo_type_mesy, location); }
    int ginfo_messizex(const std::source_location& location) { return ginfo(ginfo_type_messizex, location); }
    int ginfo_messizey(const std::source_location& location) { return ginfo(ginfo_type_messizey, location); }
    int ginfo_paluse(const std::source_location& location) { return ginfo(ginfo_type_paluse, location); }
    int ginfo_dispx(const std::source_location& location) { return ginfo(ginfo_type_dispx, location); }
    int ginfo_dispy(const std::source_location& location) { return ginfo(ginfo_type_dispy, location); }
    int ginfo_cx(const std::source_location& location) { return ginfo(ginfo_type_cx, location); }
    int ginfo_cy(const std::source_location& location) { return ginfo(ginfo_type_cy, location); }
    int ginfo_intid(const std::source_location& location) { return ginfo(ginfo_type_intid, location); }
    int ginfo_newid(const std::source_location& location) { return ginfo(ginfo_type_newid, location); }
    int ginfo_sx(const std::source_location& location) { return ginfo(ginfo_type_sx, location); }
    int ginfo_sy(const std::source_location& location) { return ginfo(ginfo_type_sy, location); }

    // ginfo_r, ginfo_g, ginfo_b マクロの代わりとなる関数
    int ginfo_r(const std::source_location& location) {
        return ginfo(ginfo_type_r, location);
    }

    int ginfo_g(const std::source_location& location) {
        return ginfo(ginfo_type_g, location);
    }

    int ginfo_b(const std::source_location& location) {
        return ginfo(ginfo_type_b, location);
    }

    // ============================================================
    // messize - テキストサイズ取得（描画せずにサイズのみ計算）
    // ============================================================
    std::pair<int, int> messize(std::string_view text, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) {
            return { 0, 0 };
        }
        int w = 0, h = 0;
        currentSurface->measureText(text, w, h);
        return { w, h };
    }

    // ============================================================
    // font - フォント設定（HSP互換）
    // ============================================================
    int font(std::string_view fontName, OptInt size, OptInt style, OptInt decorationWidth, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return -1;

        int p1 = size.value_or(12);
        int p2 = style.value_or(0);
        // p3 (decorationWidth) は現在未使用（mes命令のオプションで使用予定）

        // パラメータ範囲チェック
        if (p1 <= 0) {
            throw HspError(ERR_OUT_OF_RANGE, "fontのサイズは正の値を指定してください", location);
        }
        if (p1 > 10000) {
            throw HspError(ERR_OUT_OF_RANGE, "fontのサイズが大きすぎます（10000以下）", location);
        }
        if (p2 < 0 || p2 > 31) {
            throw HspError(ERR_OUT_OF_RANGE, "fontのスタイルは0～31の範囲で指定してください", location);
        }

        bool success = currentSurface->font(fontName, p1, p2);
        return success ? 0 : -1;
    }

    // ============================================================
    // sysfont - システムフォント選択（HSP互換）
    // ============================================================
    void sysfont(OptInt type, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int p1 = type.value_or(0);

        // パラメータ範囲チェック (0, 10-17が有効)
        bool validType = (p1 == 0) || (p1 >= 10 && p1 <= 17);
        if (!validType) {
            throw HspError(ERR_OUT_OF_RANGE, "sysfontのtypeは0または10～17の範囲で指定してください", location);
        }

        currentSurface->sysfont(p1);
    }

    // ============================================================
    // title - タイトルバー設定（HSP互換）
    // ============================================================
    void title(std::string_view str, const std::source_location& location) {
        using namespace internal;

        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        auto pWindow = std::dynamic_pointer_cast<HspWindow>(currentSurface);
        if (pWindow) {
            pWindow->setTitle(str);
        }
    }

    // ============================================================
    // width - ウィンドウサイズ設定（HSP互換）
    // ============================================================
    void width(OptInt clientW, OptInt clientH, OptInt posX, OptInt posY, OptInt option, const std::source_location& location) {
        using namespace internal;

        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        auto pWindow = std::dynamic_pointer_cast<HspWindow>(currentSurface);
        if (!pWindow) return;

        int p1 = clientW.value_or(-1);
        int p2 = clientH.value_or(-1);
        int p3 = posX.value_or(-1);
        int p4 = posY.value_or(-1);
        int p5 = option.value_or(0);

        HWND hwnd = pWindow->getHwnd();
        if (!hwnd) return;

        // サイズ変更
        if (p1 >= 0 || p2 >= 0) {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int newW = (p1 >= 0) ? p1 : (clientRect.right - clientRect.left);
            int newH = (p2 >= 0) ? p2 : (clientRect.bottom - clientRect.top);
            
            // screen/buffer/bgscrの初期化サイズを超えないようにクランプ
            int maxW = pWindow->getWidth();
            int maxH = pWindow->getHeight();
            if (newW > maxW) newW = maxW;
            if (newH > maxH) newH = maxH;
            
            pWindow->setClientSize(newW, newH);
        }

        // 位置変更
        if (p5 == 0) {
            // option=0: 負の値は現在の位置を維持
            if (p3 >= 0 || p4 >= 0) {
                RECT windowRect;
                GetWindowRect(hwnd, &windowRect);
                int newX = (p3 >= 0) ? p3 : windowRect.left;
                int newY = (p4 >= 0) ? p4 : windowRect.top;
                pWindow->setWindowPos(newX, newY);
            }
        }
        else {
            // option=1: 負の値も含めて設定（マルチモニタ対応）
            pWindow->setWindowPos(p3, p4);
        }
    }

    // ============================================================
    // groll - スクロール位置設定（HSP互換）
    // ============================================================
    void groll(int scrollX, int scrollY, const std::source_location& location) {
        using namespace internal;

        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        auto pWindow = std::dynamic_pointer_cast<HspWindow>(currentSurface);
        if (!pWindow) return;

        pWindow->setScroll(scrollX, scrollY);
    }

} // namespace hsppp
