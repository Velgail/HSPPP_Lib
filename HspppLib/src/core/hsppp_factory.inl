// HspppLib/src/core/hsppp_factory.inl
// screen/buffer/bgscr生成関数の実装
// hsppp.cpp から #include されることを想定

namespace hsppp {

    // ============================================================
    // screen 関数の実装
    // ============================================================

    // 内部実装：ウィンドウ作成の共通処理
    static Screen createWindowInternal(
        int id,
        int width,
        int height,
        int mode,
        int pos_x,
        int pos_y,
        int client_w,
        int client_h,
        std::string_view title
    ) {
        using namespace internal;

        // パラメータ範囲チェック
        if (width <= 0 || height <= 0) {
            throw HspError(ERR_OUT_OF_RANGE, "screenのサイズは正の値を指定してください", std::source_location::current());
        }
        if (width > 16384 || height > 16384) {
            throw HspError(ERR_OUT_OF_RANGE, "screenのサイズが大きすぎます（16384以下）", std::source_location::current());
        }

        // 既存のサーフェスを削除
        if (g_surfaces.find(id) != g_surfaces.end()) {
            g_surfaces.erase(id);
        }

        // ID0はデフォルトでサイズ固定
        if (id == 0) {
            mode |= screen_fixedsize;
        }

        // ウィンドウスタイルの決定
        // WS_CLIPCHILDREN: 子ウィンドウ（GUIコントロール）の領域を親の描画から除外
        DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
        DWORD dwExStyle = 0;

        // サイズ固定でない場合は、サイズ変更可能なスタイルを追加
        if (!(mode & screen_fixedsize)) {
            dwStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;
        }

        // ツールウィンドウ
        if (mode & screen_tool) {
            dwExStyle |= WS_EX_TOOLWINDOW;
        }

        // 深い縁のあるウィンドウ
        if (mode & screen_frame) {
            dwExStyle |= WS_EX_CLIENTEDGE;
        }

        // 非表示ウィンドウ（初期状態では非表示、後でgselで表示可能）
        bool isHidden = (mode & screen_hide) != 0;

        // クライアントサイズの決定（client_w, client_hが0ならwidth, heightと同じ）
        int clientWidth = (client_w > 0) ? client_w : width;
        int clientHeight = (client_h > 0) ? client_h : height;

        // HspWindowインスタンスの作成
        auto window = std::make_shared<HspWindow>(width, height, title);

        // ウィンドウマネージャーの取得
        WindowManager& windowManager = WindowManager::getInstance();

        // ウィンドウの作成
        if (!window->createWindow(
            windowManager.getHInstance(),
            windowManager.getClassName(),
            dwStyle,
            dwExStyle,
            pos_x,
            pos_y,
            clientWidth,
            clientHeight)) {
            MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};  // 無効なScreenを返す
        }

        // Direct2D 1.1リソースの初期化
        if (!window->initialize()) {
            MessageBoxW(nullptr, L"Failed to initialize window", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};  // 無効なScreenを返す
        }

        // Surfaceマップに追加
        g_surfaces[id] = window;

        // カレントサーフェスとして設定（weak_ptrを使用）
        g_currentSurface = window;

        // 非表示フラグが立っていなければウィンドウを表示
        if (!isHidden) {
            ShowWindow(window->getHwnd(), SW_SHOW);
            UpdateWindow(window->getHwnd());
        }

        g_shouldQuit = false;

        // Screen ハンドルを返す（IDと有効フラグのみ）
        return Screen{id, true};
    }

    // OOP版：構造体パラメータ（ID自動採番）
    Screen screen(const ScreenParams& params, const std::source_location& location) {
        return safe_call(location, [&] {
            int id = getNextAutoId();
            return createWindowInternal(
                id,
                params.width,
                params.height,
                params.mode,
                params.pos_x,
                params.pos_y,
                params.client_w,
                params.client_h,
                params.title
            );
        });
    }

    // OOP版：引数なし（ID自動採番、デフォルト設定）
    Screen screen(const std::source_location& location) {
        return safe_call(location, [&] {
            return screen(ScreenParams{}, std::source_location{});
        });
    }

    // HSP互換版：ID明示指定
    Screen screen(
        int id,
        OptInt width,
        OptInt height,
        OptInt mode,
        OptInt pos_x,
        OptInt pos_y,
        OptInt client_w,
        OptInt client_h,
        std::string_view title,
        const std::source_location& location
    ) {
        return safe_call(location, [&] {
            return createWindowInternal(
                id,
                width.value_or(640),
                height.value_or(480),
                mode.value_or(0),
                pos_x.value_or(-1),
                pos_y.value_or(-1),
                client_w.value_or(0),
                client_h.value_or(0),
                title
            );
        });
    }

    // ============================================================
    // buffer - 仮想画面を初期化
    // ============================================================

    // 内部実装：バッファ作成の共通処理
    static Screen createBufferInternal(int id, int width, int height, int mode) {
        using namespace internal;

        // パラメータ範囲チェック
        if (width <= 0 || height <= 0) {
            throw HspError(ERR_OUT_OF_RANGE, "bufferのサイズは正の値を指定してください", std::source_location::current());
        }
        if (width > 16384 || height > 16384) {
            throw HspError(ERR_OUT_OF_RANGE, "bufferのサイズが大きすぎます（16384以下）", std::source_location::current());
        }

        // 既存のサーフェスがある場合の処理
        // HSPでは既存のIDに対してbuffer()を呼ぶと上書きされる（エラーではない）
        auto it = g_surfaces.find(id);
        if (it != g_surfaces.end()) {
            g_surfaces.erase(it);
        }

        // HspBufferインスタンスの作成
        auto buf = std::make_shared<HspBuffer>(width, height);

        // Direct2D 1.1リソースの初期化
        if (!buf->initialize()) {
            MessageBoxW(nullptr, L"Failed to initialize buffer", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};  // 無効なScreenを返す
        }

        // Surfaceマップに追加
        g_surfaces[id] = buf;

        // カレントサーフェスとして設定
        g_currentSurface = buf;

        // Screen ハンドルを返す
        return Screen{id, true};
    }

    // OOP版：構造体パラメータ（ID自動採番）
    Screen buffer(const BufferParams& params, const std::source_location& location) {
        return safe_call(location, [&] {
            int id = getNextAutoId();
            return createBufferInternal(id, params.width, params.height, params.mode);
        });
    }

    // OOP版：引数なし（ID自動採番、デフォルト設定）
    Screen buffer(const std::source_location& location) {
        return safe_call(location, [&] {
            return buffer(BufferParams{}, std::source_location{});
        });
    }

    // HSP互換版：ID明示指定
    Screen buffer(int id, OptInt width, OptInt height, OptInt mode, const std::source_location& location) {
        return safe_call(location, [&] {
            return createBufferInternal(
                id,
                width.value_or(640),
                height.value_or(480),
                mode.value_or(0)
            );
        });
    }

    // ============================================================
    // bgscr - 枠のないウィンドウを初期化
    // ============================================================

    // 内部実装：枠なしウィンドウ作成の共通処理
    static Screen createBgscrInternal(
        int id,
        int width,
        int height,
        int mode,
        int pos_x,
        int pos_y,
        int client_w,
        int client_h
    ) {
        using namespace internal;

        // パラメータ範囲チェック
        if (width <= 0 || height <= 0) {
            throw HspError(ERR_OUT_OF_RANGE, "bgscrのサイズは正の値を指定してください", std::source_location::current());
        }
        if (width > 16384 || height > 16384) {
            throw HspError(ERR_OUT_OF_RANGE, "bgscrのサイズが大きすぎます（16384以下）", std::source_location::current());
        }

        // 既存のサーフェスを削除
        if (g_surfaces.find(id) != g_surfaces.end()) {
            g_surfaces.erase(id);
        }

        // ウィンドウスタイル: 枠なし（WS_POPUP）
        // WS_CLIPCHILDREN: 子ウィンドウ（GUIコントロール）の領域を親の描画から除外
        DWORD dwStyle = WS_POPUP | WS_CLIPCHILDREN;
        DWORD dwExStyle = 0;

        // 非表示ウィンドウ（mode & 2）
        bool isHidden = (mode & 2) != 0;

        // クライアントサイズの決定
        int clientWidth = (client_w > 0) ? client_w : width;
        int clientHeight = (client_h > 0) ? client_h : height;

        // HspWindowインスタンスの作成（タイトルは空）
        auto window = std::make_shared<HspWindow>(width, height, "");

        // ウィンドウマネージャーの取得
        WindowManager& windowManager = WindowManager::getInstance();

        // ウィンドウの作成
        if (!window->createWindow(
            windowManager.getHInstance(),
            windowManager.getClassName(),
            dwStyle,
            dwExStyle,
            pos_x,
            pos_y,
            clientWidth,
            clientHeight)) {
            MessageBoxW(nullptr, L"Failed to create borderless window", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};
        }

        // Direct2D 1.1リソースの初期化
        if (!window->initialize()) {
            MessageBoxW(nullptr, L"Failed to initialize borderless window", L"Error", MB_OK | MB_ICONERROR);
            return Screen{};
        }

        // Surfaceマップに追加
        g_surfaces[id] = window;

        // カレントサーフェスとして設定
        g_currentSurface = window;

        // 非表示フラグが立っていなければウィンドウを表示
        if (!isHidden) {
            ShowWindow(window->getHwnd(), SW_SHOW);
            UpdateWindow(window->getHwnd());
        }

        g_shouldQuit = false;

        return Screen{id, true};
    }

    // OOP版：構造体パラメータ（ID自動採番）
    Screen bgscr(const BgscrParams& params, const std::source_location& location) {
        return safe_call(location, [&] {
            int id = getNextAutoId();
            return createBgscrInternal(
                id,
                params.width,
                params.height,
                params.mode,
                params.pos_x,
                params.pos_y,
                params.client_w,
                params.client_h
            );
        });
    }

    // OOP版：引数なし（ID自動採番、デフォルト設定）
    Screen bgscr(const std::source_location& location) {
        return safe_call(location, [&] {
            return bgscr(BgscrParams{}, std::source_location{});
        });
    }

    // HSP互換版：ID明示指定
    Screen bgscr(int id, OptInt width, OptInt height, OptInt mode,
                 OptInt pos_x, OptInt pos_y, OptInt client_w, OptInt client_h, const std::source_location& location) {
        return safe_call(location, [&] {
            return createBgscrInternal(
                id,
                width.value_or(640),
                height.value_or(480),
                mode.value_or(0),
                pos_x.value_or(-1),
                pos_y.value_or(-1),
                client_w.value_or(0),
                client_h.value_or(0)
            );
        });
    }

} // namespace hsppp
