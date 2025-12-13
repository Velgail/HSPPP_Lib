// HspppLib/src/core/hsppp_drawing.inl
// 描画系関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    // ============================================================
    // 描画制御関数
    // ============================================================

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

} // namespace hsppp
