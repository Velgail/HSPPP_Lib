// HspppLib/src/core/hsppp_drawing.inl
// 描画系関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    // ============================================================
    // 描画制御関数
    // ============================================================

    // cls - 画面クリア（HSP互換）
    void cls(OptInt p1, const std::source_location& location) {
        int mode = p1.value_or(0);

        // パラメータチェック
        if (mode < 0 || mode > 4) {
            throw HspError(ERR_OUT_OF_RANGE, "clsのパラメータは0～4の範囲で指定してください", location);
        }

        // カレントサーフェス取得（自動的にデフォルトウィンドウ作成）
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        currentSurface->cls(mode);
    }

    // 描画制御（HSP互換）
    void redraw(int p1, const std::source_location& location) {
        // パラメータチェック
        if (p1 < 0 || p1 > 3) {
            throw HspError(ERR_OUT_OF_RANGE, "redrawのパラメータは0～3の範囲で指定してください", location);
        }
        // カレントサーフェス取得（自動的にデフォルトウィンドウ作成）
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        // p1の値に応じて描画モードを設定
        // 0: モード0に設定（仮想画面のみ）
        // 1: モード1に設定＋画面更新
        // 2: モード0に設定のみ（画面更新なし）
        // 3: モード1に設定のみ（画面更新なし）

        bool shouldUpdate = (p1 == 1);  // p1=1の場合のみ画面更新
        int newMode = p1 % 2;           // 0 or 1

        if (newMode == 0) {
            // モード0に切り替え: 仮想画面のみに描画（バッチモード）
            // 既に描画中でなければBeginDrawを呼ぶ
            if (!currentSurface->isDrawing()) {
                currentSurface->beginDraw();
            }
            currentSurface->setRedrawMode(0);
        }
        else {
            // モード1に切り替え: 即座に反映
            currentSurface->setRedrawMode(1);

            // shouldUpdateがtrueなら即座に画面更新
            if (shouldUpdate && currentSurface->isDrawing()) {
                currentSurface->endDrawAndPresent();
            }
        }
    }

    // 待機＆メッセージ処理 (HSP互換)
    void await(int time_ms, const std::source_location& location) {
        // パラメータチェック
        if (time_ms < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "awaitの待ち時間は0以上の値を指定してください", location);
        }
        
        MSG msg;
        DWORD currentTime = GetTickCount();

        // 初回呼び出し、または時刻が巻き戻った場合は現在時刻を基準にする
        if (g_lastAwaitTime == 0 || currentTime < g_lastAwaitTime) {
            g_lastAwaitTime = currentTime;
        }

        // 前回からの経過時間を計算
        DWORD elapsed = currentTime - g_lastAwaitTime;

        // 指定時間に満たない場合は待機
        if (elapsed < (DWORD)time_ms) {
            DWORD waitTime = time_ms - elapsed;
            DWORD endTime = currentTime + waitTime;

            // 待機中もメッセージを処理
            while (GetTickCount() < endTime) {
                // ペンディング中の割り込みを処理
                if (processPendingInterrupt()) {
                    // 割り込みハンドラが呼ばれた
                }

                if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) {
                        g_shouldQuit = true;
                        return;
                    }
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else {
                    Sleep(1);
                }
            }
        }
        else {
            // すでに指定時間を超過している場合もメッセージ処理だけ行う
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                // ペンディング中の割り込みを処理
                if (processPendingInterrupt()) {
                    // 割り込みハンドラが呼ばれた
                }

                if (msg.message == WM_QUIT) {
                    g_shouldQuit = true;
                    return;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        // 次回のawaitのために現在時刻を記録
        g_lastAwaitTime = GetTickCount();
    }

    // プログラム終了 (HSP互換)
    [[noreturn]] void end(int exitcode, const std::source_location& location) {
        // 描画中のサーフェスがあれば終了処理
        auto currentSurface = getCurrentSurface();
        if (currentSurface && currentSurface->isDrawing()) {
            currentSurface->endDrawAndPresent();
        }

        // リソースのクリーンアップ
        internal::close_system();

        // プロセス終了
        ExitProcess(static_cast<UINT>(exitcode));
    }

    // ============================================================
    // 基本描画関数
    // ============================================================

    // 描画色設定
    void color(int r, int g, int b, const std::source_location& location) {
        // パラメータ範囲チェック（例外を使用したエラー処理のデモ）
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            throw HspError(ERR_OUT_OF_RANGE, "color値は0~255の範囲で指定してください", location);
        }

        auto currentSurface = getCurrentSurface();
        if (currentSurface) {
            currentSurface->color(r, g, b);
        }
    }

    // 描画位置設定
    void pos(int x, int y, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (currentSurface) {
            currentSurface->pos(x, y);
        }
    }

    // 文字列描画
    void mes(std::string_view text, OptInt sw, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (currentSurface) {
            currentSurface->mes(text, sw.value_or(0));
        }
    }

    // 矩形塗りつぶし（座標指定版）
    void boxf(int x1, int y1, int x2, int y2, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (currentSurface) {
            currentSurface->boxf(x1, y1, x2, y2);
        }
    }

    // 矩形塗りつぶし（全画面版）
    void boxf(const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (currentSurface) {
            currentSurface->boxf(0, 0, currentSurface->getWidth(), currentSurface->getHeight());
        }
    }

    // ============================================================
    // line - 直線を描画（HSP互換）
    // ============================================================
    void line(OptInt x2, OptInt y2, OptInt x1, OptInt y1, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int endX = x2.value_or(0);
        int endY = y2.value_or(0);

        // x1, y1が省略されたかどうかを判定
        bool useStartPos = !x1.is_default() && !y1.is_default();
        int startX = x1.value_or(currentSurface->getCurrentX());
        int startY = y1.value_or(currentSurface->getCurrentY());

        currentSurface->line(endX, endY, startX, startY, useStartPos);
    }

    // ============================================================
    // circle - 円を描画（HSP互換）
    // ============================================================
    void circle(OptInt x1, OptInt y1, OptInt x2, OptInt y2, OptInt fillMode, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int p1 = x1.value_or(0);
        int p2 = y1.value_or(0);
        int p3 = x2.value_or(currentSurface->getWidth());
        int p4 = y2.value_or(currentSurface->getHeight());
        int p5 = fillMode.value_or(1);

        currentSurface->circle(p1, p2, p3, p4, p5);
    }

    // ============================================================
    // pset - 1ドットの点を描画（HSP互換）
    // ============================================================
    void pset(OptInt x, OptInt y, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int px = x.is_default() ? currentSurface->getCurrentX() : x.value();
        int py = y.is_default() ? currentSurface->getCurrentY() : y.value();

        currentSurface->pset(px, py);
    }

    // ============================================================
    // pget - 1ドットの色を取得（HSP互換）
    // ============================================================
    void pget(OptInt x, OptInt y, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int px = x.is_default() ? currentSurface->getCurrentX() : x.value();
        int py = y.is_default() ? currentSurface->getCurrentY() : y.value();

        int r, g, b;
        currentSurface->pget(px, py, r, g, b);
    }

    // ============================================================
    // gradf - 矩形をグラデーションで塗りつぶす（HSP互換）
    // ============================================================
    void gradf(OptInt x, OptInt y, OptInt w, OptInt h, OptInt mode, OptInt color1, OptInt color2, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int px = x.value_or(0);
        int py = y.value_or(0);
        int pw = w.value_or(currentSurface->getWidth());
        int ph = h.value_or(currentSurface->getHeight());
        int pmode = mode.value_or(0);
        
        // 色が省略された場合は現在の描画色を使用
        D2D1_COLOR_F curColor = currentSurface->getCurrentColor();
        int curColorCode = (static_cast<int>(curColor.r * 255) << 16) |
                          (static_cast<int>(curColor.g * 255) << 8) |
                          static_cast<int>(curColor.b * 255);
        int c1 = color1.value_or(curColorCode);
        int c2 = color2.value_or(curColorCode);

        currentSurface->gradf(px, py, pw, ph, pmode, c1, c2);
    }

    // ============================================================
    // grect - 回転する矩形で塗りつぶす（HSP互換）
    // カレントサーフェスのgmode設定を使用
    // ============================================================
    void grect(OptInt cx, OptInt cy, OptDouble angle, OptInt w, OptInt h, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        int pcx = cx.value_or(0);
        int pcy = cy.value_or(0);
        double pangle = angle.value_or(0.0);
        int pw = w.value_or(currentSurface->getGmodeSizeX());
        int ph = h.value_or(currentSurface->getGmodeSizeY());

        currentSurface->grect(pcx, pcy, pangle, pw, ph);
    }

    // ============================================================
    // grotate - 矩形画像を回転してコピー（HSP互換）
    // カレントサーフェスのgmode設定を使用
    // ============================================================
    void grotate(OptInt srcId, OptInt srcX, OptInt srcY, OptDouble angle, OptInt dstW, OptInt dstH, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        // サーフェスのgmode設定を取得
        int gmodeSizeX = currentSurface->getGmodeSizeX();
        int gmodeSizeY = currentSurface->getGmodeSizeY();

        int psrcId = srcId.value_or(0);
        int psrcX = srcX.value_or(0);
        int psrcY = srcY.value_or(0);
        double pangle = angle.value_or(0.0);
        int pdstW = dstW.value_or(gmodeSizeX);
        int pdstH = dstH.value_or(gmodeSizeY);

        // ソースサーフェスを取得
        auto srcSurface = getSurfaceById(psrcId);
        if (!srcSurface) return;

        auto* pSrcBitmap = srcSurface->getTargetBitmap();
        if (!pSrcBitmap) return;

        // ソースサイズはgmodeで設定されたサイズ
        currentSurface->grotate(pSrcBitmap, psrcX, psrcY, gmodeSizeX, gmodeSizeY, pangle, pdstW, pdstH);
    }

    // ============================================================
    // gsquare - 任意の四角形を描画（HSP互換）
    // ============================================================
    
    // 単色塗りつぶし
    void gsquare(int srcId, const Quad& dst, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        // Quadから配列を抽出
        int dstX[4] = { dst.v[0].x, dst.v[1].x, dst.v[2].x, dst.v[3].x };
        int dstY[4] = { dst.v[0].y, dst.v[1].y, dst.v[2].y, dst.v[3].y };

        if (srcId < 0) {
            // 塗りつぶしモード (-1 ～ -256)
            currentSurface->gsquare(dstX, dstY, nullptr, nullptr, nullptr);
        } else {
            // srcIdが0以上の場合、全体コピーとして扱う（srcは全体領域）
            // 本来は画像コピーモードなので、QuadUV版を使うべき
            // ここでは塗りつぶしとして処理
            currentSurface->gsquare(dstX, dstY, nullptr, nullptr, nullptr);
        }
    }

    // 画像コピー
    void gsquare(int srcId, const Quad& dst, const QuadUV& src, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        // Quad/QuadUVから配列を抽出
        int dstX[4] = { dst.v[0].x, dst.v[1].x, dst.v[2].x, dst.v[3].x };
        int dstY[4] = { dst.v[0].y, dst.v[1].y, dst.v[2].y, dst.v[3].y };
        int srcX[4] = { src.v[0].x, src.v[1].x, src.v[2].x, src.v[3].x };
        int srcY[4] = { src.v[0].y, src.v[1].y, src.v[2].y, src.v[3].y };

        if (srcId < 0) {
            // 負の値は塗りつぶし
            currentSurface->gsquare(dstX, dstY, nullptr, nullptr, nullptr);
        } else {
            // 画像コピーモード
            auto srcSurface = getSurfaceById(srcId);
            if (!srcSurface) return;
            auto* pSrcBitmap = srcSurface->getTargetBitmap();
            currentSurface->gsquare(dstX, dstY, pSrcBitmap, srcX, srcY);
        }
    }

    // グラデーション
    void gsquare(int srcId, const Quad& dst, const QuadColors& colors, const std::source_location& location) {
        auto currentSurface = getCurrentSurface();
        if (!currentSurface) return;

        // Quad/QuadColorsから配列を抽出
        int dstX[4] = { dst.v[0].x, dst.v[1].x, dst.v[2].x, dst.v[3].x };
        int dstY[4] = { dst.v[0].y, dst.v[1].y, dst.v[2].y, dst.v[3].y };
        int cols[4] = { colors.colors[0], colors.colors[1], colors.colors[2], colors.colors[3] };

        if (srcId == gsquare_grad) {
            // グラデーションモード
            currentSurface->gsquareGrad(dstX, dstY, cols);
        } else {
            // グラデーション以外は単色塗りつぶし
            currentSurface->gsquare(dstX, dstY, nullptr, nullptr, nullptr);
        }
    }

    // ============================================================
    // print - メッセージ表示（HSP互換・mes別名）
    // ============================================================
    void print(std::string_view text, OptInt sw, const std::source_location& location) {
        // mes命令と同等
        mes(text, sw, location);
    }

    // ============================================================
    // gettime - 時間・日付を取得（HSP互換）
    // ============================================================
    int gettime(int type, const std::source_location& location) {
        SYSTEMTIME st;
        GetLocalTime(&st);

        switch (type) {
        case 0: return st.wYear;         // 年
        case 1: return st.wMonth;        // 月
        case 2: return st.wDayOfWeek;    // 曜日 (0=日曜)
        case 3: return st.wDay;          // 日
        case 4: return st.wHour;         // 時
        case 5: return st.wMinute;       // 分
        case 6: return st.wSecond;       // 秒
        case 7: return st.wMilliseconds; // ミリ秒
        default:
            throw HspError(ERR_OUT_OF_RANGE, "gettimeのタイプは0～7の範囲で指定してください", location);
        }
    }

} // namespace hsppp
