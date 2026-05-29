// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/src/core/hsppp_ginfo.inl
// ginfo, font, sysfont, title, width関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    // ============================================================
    // ginfo - ウィンドウ情報の取得（HSP互換）
    // ============================================================
    int ginfo(int type, const std::source_location& location) {
        return safe_call(location, [&]() -> int {
            // パラメータチェック
            if (type < 0 || type > 27) {
                throw HspError(ERR_OUT_OF_RANGE, "ginfoのtypeは0～27の範囲で指定してください", location);
            }
            using namespace internal;
        
        auto currentSurface = getCurrentSurface();
        auto pWindow = currentSurface ? std::dynamic_pointer_cast<HspWindow>(currentSurface) : nullptr;
        
        switch (type) {
        case 0:  // マウスカーソルX座標（HSP仕様: ウィンドウクライアント領域内の論理座標）
        {
            POINT pt;
            GetCursorPos(&pt);
            if (pWindow && pWindow->getHwnd()) {
                ScreenToClient(pWindow->getHwnd(), &pt);
                int lx = 0, ly = 0;
                pWindow->physToLogical(static_cast<int>(pt.x), static_cast<int>(pt.y), lx, ly);
                return lx;
            }
            return static_cast<int>(pt.x);
        }
        case 1:  // マウスカーソルY座標（HSP仕様: ウィンドウクライアント領域内の論理座標）
        {
            POINT pt;
            GetCursorPos(&pt);
            if (pWindow && pWindow->getHwnd()) {
                ScreenToClient(pWindow->getHwnd(), &pt);
                int lx = 0, ly = 0;
                pWindow->physToLogical(static_cast<int>(pt.x), static_cast<int>(pt.y), lx, ly);
                return ly;
            }
            return static_cast<int>(pt.y);
        }
        case 2:  // アクティブなウィンドウID
        {
            HWND hwndActive = GetForegroundWindow();
            // g_surfacesを検索してウィンドウIDを返す
            for (const auto& pair : g_surfaces) {
                auto pWin = std::dynamic_pointer_cast<HspWindow>(pair.second);
                if (pWin && pWin->getHwnd() == hwndActive) {
                    return static_cast<int>(pair.first);
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
                        return static_cast<int>(pair.first);
                    }
                }
            }
            return 0;
        }
        case 4:  // ウィンドウの左上X座標（論理 px / HSP3 公式準拠）
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                UINT dpi = pWindow->getCurrentDpi();
                if (dpi != 0 && dpi != 96) {
                    return MulDiv(static_cast<int>(rect.left), 96, static_cast<int>(dpi));
                }
                return static_cast<int>(rect.left);
            }
            return 0;
        }
        case 5:  // ウィンドウの左上Y座標（論理 px）
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                UINT dpi = pWindow->getCurrentDpi();
                if (dpi != 0 && dpi != 96) {
                    return MulDiv(static_cast<int>(rect.top), 96, static_cast<int>(dpi));
                }
                return static_cast<int>(rect.top);
            }
            return 0;
        }
        case 6:  // ウィンドウの右下X座標（論理 px）
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                UINT dpi = pWindow->getCurrentDpi();
                if (dpi != 0 && dpi != 96) {
                    return MulDiv(static_cast<int>(rect.right), 96, static_cast<int>(dpi));
                }
                return static_cast<int>(rect.right);
            }
            return 0;
        }
        case 7:  // ウィンドウの右下Y座標（論理 px）
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                UINT dpi = pWindow->getCurrentDpi();
                if (dpi != 0 && dpi != 96) {
                    return MulDiv(static_cast<int>(rect.bottom), 96, static_cast<int>(dpi));
                }
                return static_cast<int>(rect.bottom);
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
        case 10:  // ウィンドウ全体のXサイズ（論理 px）
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                int w = static_cast<int>(rect.right - rect.left);
                UINT dpi = pWindow->getCurrentDpi();
                if (dpi != 0 && dpi != 96) {
                    return MulDiv(w, 96, static_cast<int>(dpi));
                }
                return w;
            }
            return currentSurface ? currentSurface->getWidth() : 0;
        }
        case 11:  // ウィンドウ全体のYサイズ（論理 px）
        {
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetWindowRect(pWindow->getHwnd(), &rect);
                int h = static_cast<int>(rect.bottom - rect.top);
                UINT dpi = pWindow->getCurrentDpi();
                if (dpi != 0 && dpi != 96) {
                    return MulDiv(h, 96, static_cast<int>(dpi));
                }
                return h;
            }
            return currentSurface ? currentSurface->getHeight() : 0;
        }
        case 12:  // クライアント領域Xサイズ（論理 px / 仮想 ON/OFF 統一）
        {
            if (pWindow && pWindow->isVirtualEnabled()) {
                return pWindow->getWidth();
            }
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetClientRect(pWindow->getHwnd(), &rect);
                int w = static_cast<int>(rect.right);
                UINT dpi = pWindow->getCurrentDpi();
                if (dpi != 0 && dpi != 96) {
                    return MulDiv(w, 96, static_cast<int>(dpi));
                }
                return w;
            }
            return currentSurface ? currentSurface->getWidth() : 0;
        }
        case 13:  // クライアント領域Yサイズ（論理 px / 仮想 ON/OFF 統一）
        {
            if (pWindow && pWindow->isVirtualEnabled()) {
                return pWindow->getHeight();
            }
            if (pWindow && pWindow->getHwnd()) {
                RECT rect;
                GetClientRect(pWindow->getHwnd(), &rect);
                int h = static_cast<int>(rect.bottom);
                UINT dpi = pWindow->getCurrentDpi();
                if (dpi != 0 && dpi != 96) {
                    return MulDiv(h, 96, static_cast<int>(dpi));
                }
                return h;
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
        case 20:  // デスクトップ全体のXサイズ（プライマリモニタ論理 px / PM Q-2 HSP3 公式準拠）
        {
            using FnGetDpiSys = UINT(WINAPI*)();
            HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
            auto pGetDpiSys = hUser32
                ? reinterpret_cast<FnGetDpiSys>(GetProcAddress(hUser32, "GetDpiForSystem"))
                : nullptr;
            UINT dpi = pGetDpiSys ? pGetDpiSys() : 96;
            int physW = GetSystemMetrics(SM_CXSCREEN);
            if (dpi != 0 && dpi != 96) {
                return MulDiv(physW, 96, static_cast<int>(dpi));
            }
            return physW;
        }
        case 21:  // デスクトップ全体のYサイズ（プライマリモニタ論理 px）
        {
            using FnGetDpiSys = UINT(WINAPI*)();
            HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
            auto pGetDpiSys = hUser32
                ? reinterpret_cast<FnGetDpiSys>(GetProcAddress(hUser32, "GetDpiForSystem"))
                : nullptr;
            UINT dpi = pGetDpiSys ? pGetDpiSys() : 96;
            int physH = GetSystemMetrics(SM_CYSCREEN);
            if (dpi != 0 && dpi != 96) {
                return MulDiv(physH, 96, static_cast<int>(dpi));
            }
            return physH;
        }
        case 22:  // カレントポジションのX座標
            return currentSurface ? currentSurface->getCurrentX() : 0;
        case 23:  // カレントポジションのY座標
            return currentSurface ? currentSurface->getCurrentY() : 0;
        case 24:  // メッセージ割り込み時のウィンドウID
            return wparam();
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
        case 28:  // 画面リフレッシュレート
            return get_framerate(location);
        default:
            return 0;
        }
        });
    }
    
    // ============================================================
    // get_framerate - 画面リフレッシュレートを取得
    // ============================================================
    // マルチモニター環境では最大のリフレッシュレートを返す
    int get_framerate(const std::source_location& location) {
        return safe_call(location, [&]() -> int {
            int maxRefreshRate = 60;  // デフォルト値
            
            // 全モニターを列挙してリフレッシュレートを取得
            EnumDisplayMonitors(nullptr, nullptr, 
                [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
                    MONITORINFOEXW monitorInfo = {};
                    monitorInfo.cbSize = sizeof(MONITORINFOEXW);
                    
                    if (GetMonitorInfoW(hMonitor, &monitorInfo)) {
                        DEVMODEW devMode = {};
                        devMode.dmSize = sizeof(DEVMODEW);
                        
                        if (EnumDisplaySettingsW(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode)) {
                            int refreshRate = static_cast<int>(devMode.dmDisplayFrequency);
                            int* pMaxRate = reinterpret_cast<int*>(lParam);
                            if (refreshRate > *pMaxRate) {
                                *pMaxRate = refreshRate;
                            }
                        }
                    }
                    return TRUE;  // 列挙を続行
                }, 
                reinterpret_cast<LPARAM>(&maxRefreshRate)
            );
            
            return maxRefreshRate;
        });
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
    
    int ginfo_fps(const std::source_location& location) {
        return get_framerate(location);
    }

    // ============================================================
    // messize - テキストサイズ取得（描画せずにサイズのみ計算）
    // ============================================================
    std::pair<int, int> messize(std::string_view text, const std::source_location& location) {
        return safe_call(location, [&] {
            auto currentSurface = getCurrentSurface();
            if (!currentSurface) {
                return std::pair<int, int>{ 0, 0 };
            }
            int w = 0, h = 0;
            currentSurface->measureText(text, w, h);
            return std::pair<int, int>{ w, h };
        });
    }

    // ============================================================
    // font - フォント設定（HSP互換）
    // ============================================================
    int font(std::string_view fontName, OptInt size, OptInt style, [[maybe_unused]] OptInt decorationWidth, const std::source_location& location) {
        return safe_call(location, [&] {
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
        });
    }

    // ============================================================
    // sysfont - システムフォント選択（HSP互換）
    // ============================================================
    void sysfont(OptInt type, const std::source_location& location) {
        safe_call(location, [&] {
            auto currentSurface = getCurrentSurface();
            if (!currentSurface) return;

            int p1 = type.value_or(0);

            // パラメータ範囲チェック (0, 10-17が有効)
            bool validType = (p1 == 0) || (p1 >= 10 && p1 <= 17);
            if (!validType) {
                throw HspError(ERR_OUT_OF_RANGE, "sysfontのtypeは0または10～17の範囲で指定してください", location);
            }

            currentSurface->sysfont(p1);
        });
    }

    // ============================================================
    // title - タイトルバー設定（HSP互換）
    // ============================================================
    void title(std::string_view str, const std::source_location& location) {
        safe_call(location, [&] {
            using namespace internal;

            auto currentSurface = getCurrentSurface();
            if (!currentSurface) return;

            auto pWindow = std::dynamic_pointer_cast<HspWindow>(currentSurface);
            if (pWindow) {
                pWindow->setTitle(str);
            }
        });
    }

    // ============================================================
    // width - ウィンドウサイズ設定（HSP互換）
    // ============================================================
    void width(OptInt clientW, OptInt clientH, OptInt posX, OptInt posY, OptInt option, const std::source_location& location) {
        safe_call(location, [&] {
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
                // 仮想画面 ON 時は論理→物理 拡縮するため、物理クライアントサイズを論理に縛らない。
                if (!pWindow->isVirtualEnabled()) {
                    int maxW = pWindow->getWidth();
                    int maxH = pWindow->getHeight();
                    if (newW > maxW) newW = maxW;
                    if (newH > maxH) newH = maxH;
                }

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
        });
    }

    // ============================================================
    // groll - スクロール位置設定（HSP互換）
    // ============================================================
    void groll(int scrollX, int scrollY, const std::source_location& location) {
        safe_call(location, [&] {
            using namespace internal;

            auto currentSurface = getCurrentSurface();
            if (!currentSurface) return;

            auto pWindow = std::dynamic_pointer_cast<HspWindow>(currentSurface);
            if (!pWindow) return;

            pWindow->setScroll(scrollX, scrollY);
        });
    }

    // ============================================================
    // vscalemode - 仮想画面の補間モード設定
    // ============================================================
    void vscalemode(int mode, const std::source_location& location) {
        safe_call(location, [&] {
            using namespace internal;

            auto currentSurface = getCurrentSurface();
            if (!currentSurface) return;

            auto pWindow = std::dynamic_pointer_cast<HspWindow>(currentSurface);
            if (!pWindow) return;

            D2D1_BITMAP_INTERPOLATION_MODE d2dMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
            switch (mode) {
                case vscale_nearest: d2dMode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR; break;
                case vscale_linear:  d2dMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR; break;
                case vscale_aniso:
                    // D2D1 IDeviceContext::DrawBitmap は ANISOTROPIC 互換相当として
                    // 高品質補間モード (linear) を採用する。専用の異方性フィルタは未提供。
                    d2dMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
                    break;
                default:
                    throw HspError(ERR_OUT_OF_RANGE, "vscalemodeのmodeはvscale_nearest/linear/anisoのいずれかを指定してください", location);
            }
            pWindow->setVirtualInterpolation(d2dMode);
        });
    }

} // namespace hsppp
