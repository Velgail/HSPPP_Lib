// HspppLib/module/hsppp.ixx
export module hsppp;

// 必要な標準ライブラリをインポート
import <string_view>;
import <optional>;

namespace hsppp {

    // ============================================================
    // 省略可能パラメータの基盤
    // ============================================================
    // HSPでは `screen , 800, 600` のように任意の位置でパラメータを
    // 省略できる。C++では `omit` キーワードでこれを実現する。
    //
    // 使用例:
    //   screen(omit, 800, 600);        // ID=0(省略), 800x600
    //   screen(1, omit, omit, screen_hide);  // サイズ省略
    //   screen({}, 800, 600);          // {} でも省略可能
    // ============================================================

    namespace detail {
        struct OmitTag {
            constexpr OmitTag() noexcept = default;
        };
    }

    /// @brief パラメータの省略を示すプレースホルダー
    /// @details HSPの `,` による省略を `omit` で表現
    /// @note C++26で `_` は特別な意味を持つため、`omit` を使用
    export inline constexpr detail::OmitTag omit {};


    // ============================================================
    // OptInt - 省略可能なint型
    // ============================================================

    export struct OptInt {
    private:
        std::optional<int> value_;

    public:
        /// デフォルト（省略）- `{}` で使用可能
        constexpr OptInt() noexcept : value_(std::nullopt) {}
        
        /// 省略マーカーからの変換 - `omit` で使用可能
        constexpr OptInt(detail::OmitTag) noexcept : value_(std::nullopt) {}
        
        /// 値の指定
        constexpr OptInt(int v) noexcept : value_(v) {}

        [[nodiscard]] constexpr bool is_default() const noexcept { 
            return !value_.has_value(); 
        }

        [[nodiscard]] constexpr int value_or(int def) const noexcept { 
            return value_.value_or(def); 
        }

        [[nodiscard]] constexpr int value() const { 
            return value_.value(); 
        }
    };


    // ============================================================
    // OptDouble - 省略可能なdouble型
    // ============================================================

    export struct OptDouble {
    private:
        std::optional<double> value_;

    public:
        constexpr OptDouble() noexcept : value_(std::nullopt) {}
        constexpr OptDouble(detail::OmitTag) noexcept : value_(std::nullopt) {}
        constexpr OptDouble(double v) noexcept : value_(v) {}
        constexpr OptDouble(int v) noexcept : value_(static_cast<double>(v)) {}

        [[nodiscard]] constexpr bool is_default() const noexcept { 
            return !value_.has_value(); 
        }

        [[nodiscard]] constexpr double value_or(double def) const noexcept { 
            return value_.value_or(def); 
        }

        [[nodiscard]] constexpr double value() const { 
            return value_.value(); 
        }
    };


    // --- Screen Mode Flags (HSP Compatible) ---
    export inline constexpr int screen_normal    = 0;    // フルカラーモード
    export inline constexpr int screen_palette   = 1;    // パレットモード（256色）※未実装
    export inline constexpr int screen_hide      = 2;    // 非表示ウィンドウ
    export inline constexpr int screen_fixedsize = 4;    // サイズ固定
    export inline constexpr int screen_tool      = 8;    // ツールウィンドウ
    export inline constexpr int screen_frame     = 16;   // 深い縁のあるウィンドウ


    // ============================================================
    // ScreenParams 構造体（Designated Initializers用）
    // ============================================================
    
    /// @brief screen命令のパラメータ構造体
    /// @details 多くのパラメータを指定する場合に使用
    /// @example screen({.width = 800, .height = 600});
    export struct ScreenParams {
        int id       = 0;       ///< p1: ウィンドウID (0～)
        int width    = 640;     ///< p2: 画面サイズX
        int height   = 480;     ///< p3: 画面サイズY
        int mode     = 0;       ///< p4: 画面モード (screen_* フラグの組み合わせ)
        int pos_x    = -1;      ///< p5: ウィンドウ位置X (-1=システム規定)
        int pos_y    = -1;      ///< p6: ウィンドウ位置Y (-1=システム規定)
        int client_w = 0;       ///< p7: クライアントサイズX (0=widthと同じ)
        int client_h = 0;       ///< p8: クライアントサイズY (0=heightと同じ)
        std::string_view title = "HSPPP Window";  ///< ウィンドウタイトル (HSP拡張)
    };


    // ============================================================
    // Screen クラス - 軽量ハンドル（実体として操作可能）
    // ============================================================
    //
    // 使用例:
    //   auto mainscr = screen(omit, 800, 600);  // Screen を返す
    //   mainscr.color(255, 255, 255);           // . でアクセス
    //   mainscr.boxf();
    //   mainscr.mes("Hello!");
    //
    // 内部的には ID のみを保持し、グローバルマップから Surface を取得
    // これにより、モジュールインターフェースと実装の型を分離できる
    // ============================================================

    /// @brief Surfaceへの軽量ハンドル
    /// @details 実体のように `.` でアクセスできる。内部は ID のみ保持
    export class Screen {
    private:
        int m_id;
        bool m_valid;

    public:
        /// @brief 内部用コンストラクタ
        Screen(int id, bool valid) noexcept
            : m_id(id), m_valid(valid) {}

        /// @brief デフォルトコンストラクタ（無効なハンドル）
        Screen() noexcept : m_id(-1), m_valid(false) {}

        // コピー・ムーブはデフォルト
        Screen(const Screen&) = default;
        Screen& operator=(const Screen&) = default;
        Screen(Screen&&) noexcept = default;
        Screen& operator=(Screen&&) noexcept = default;

        /// @brief 有効なハンドルかどうか
        [[nodiscard]] bool valid() const noexcept { return m_valid; }
        [[nodiscard]] explicit operator bool() const noexcept { return valid(); }

        /// @brief ウィンドウIDを取得
        [[nodiscard]] int id() const noexcept { return m_id; }

        // ============================================================
        // 描画命令（メンバ関数版）
        // ============================================================

        /// @brief 描画色を設定
        Screen& color(int r, int g, int b);

        /// @brief 描画位置を設定
        Screen& pos(int x, int y);

        /// @brief 文字列を描画
        Screen& mes(std::string_view text);

        /// @brief 矩形を塗りつぶし
        Screen& boxf(int x1, int y1, int x2, int y2);

        /// @brief 画面全体を塗りつぶし
        Screen& boxf();

        /// @brief 描画制御
        Screen& redraw(int mode = 1);

        /// @brief このScreenをカレントに設定（HSPのgsel相当）
        Screen& select();

        /// @brief 幅を取得
        [[nodiscard]] int width() const;

        /// @brief 高さを取得
        [[nodiscard]] int height() const;
    };


    // --- Core Functions (HSP compatible) ---

    /// @brief ウィンドウを初期化（構造体版）
    /// @param params 初期化パラメータ
    /// @return Screen ハンドル
    /// @example auto scr = screen({.width = 800, .height = 600});
    export [[nodiscard]] Screen screen(const ScreenParams& params);

    /// @brief ウィンドウを初期化（HSP互換・省略可能版）
    /// @return Screen ハンドル
    /// @example auto scr = screen(omit, 800, 600);
    /// @example screen();  // 戻り値を無視してもOK（HSP互換）
    export [[nodiscard]] Screen screen(
        OptInt id       = {},
        OptInt width    = {},
        OptInt height   = {},
        OptInt mode     = {},
        OptInt pos_x    = {},
        OptInt pos_y    = {},
        OptInt client_w = {},
        OptInt client_h = {},
        std::string_view title = "HSPPP Window"
    );

    // 描画制御
    // p1: 0=描画予約(Offscreen), 1=画面反映(Present)
    export void redraw(int p1 = 1);

    // 待機＆メッセージ処理 (HSP互換)
    // 指定されたミリ秒だけ待機し、その間ウィンドウメッセージを処理する
    export void await(int time_ms);

    // --- Drawing Functions ---
    export void color(int r, int g, int b);
    export void pos(int x, int y);
    export void mes(std::string_view text);  // string_view で安全に
    export void boxf(int x1, int y1, int x2, int y2);
    // 引数なし版 boxf() -> 画面全体
    export void boxf();

    // --- System / Internal ---
    namespace internal {
        // ライブラリの初期化・終了処理(WinMainから呼ばれる)
        export void init_system();
        export void close_system();
    }
}

// グローバル名前空間にユーザー定義関数が存在することを期待する
// (UserApp.cpp で実装される)
extern int hspMain();
