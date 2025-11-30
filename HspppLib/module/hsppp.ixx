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
    export inline constexpr int screen_offscreen = 32;   // 描画先として初期化 (HSP3Dish/HGIMG4)
    export inline constexpr int screen_usergcopy = 64;   // 描画用シェーダー (HGIMG4)
    export inline constexpr int screen_fullscreen = 256; // フルスクリーン (bgscr用)


    // ============================================================
    // ScreenParams 構造体（Designated Initializers用）
    // ============================================================
    
    /// @brief screen命令のパラメータ構造体
    /// @details 多くのパラメータを指定する場合に使用
    /// @example screen({.width = 800, .height = 600});
    export struct ScreenParams {
        int width    = 640;     ///< 画面サイズX
        int height   = 480;     ///< 画面サイズY
        int mode     = 0;       ///< 画面モード (screen_* フラグの組み合わせ)
        int pos_x    = -1;      ///< ウィンドウ位置X (-1=システム規定)
        int pos_y    = -1;      ///< ウィンドウ位置Y (-1=システム規定)
        int client_w = 0;       ///< クライアントサイズX (0=widthと同じ)
        int client_h = 0;       ///< クライアントサイズY (0=heightと同じ)
        std::string_view title = "HSPPP Window";  ///< ウィンドウタイトル (HSP拡張)
    };

    /// @brief buffer命令のパラメータ構造体
    export struct BufferParams {
        int width    = 640;     ///< 画面サイズX
        int height   = 480;     ///< 画面サイズY
        int mode     = 0;       ///< 画面モード
    };

    /// @brief bgscr命令のパラメータ構造体
    export struct BgscrParams {
        int width    = 640;     ///< 画面サイズX
        int height   = 480;     ///< 画面サイズY
        int mode     = 0;       ///< 画面モード (0=フルカラー, 2=非表示)
        int pos_x    = -1;      ///< ウィンドウ位置X (-1=システム規定)
        int pos_y    = -1;      ///< ウィンドウ位置Y (-1=システム規定)
        int client_w = 0;       ///< クライアントサイズX (0=widthと同じ)
        int client_h = 0;       ///< クライアントサイズY (0=heightと同じ)
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

        /// @brief 直線を描画
        /// @param x2 ラインの終点X座標
        /// @param y2 ラインの終点Y座標
        /// @param x1 ラインの始点X座標 (省略時=カレントポジション)
        /// @param y1 ラインの始点Y座標 (省略時=カレントポジション)
        /// @return *this（メソッドチェーン用）
        Screen& line(int x2, int y2);
        Screen& line(int x2, int y2, int x1, int y1);

        /// @brief 円を描画
        /// @param x1 矩形の左上X座標
        /// @param y1 矩形の左上Y座標
        /// @param x2 矩形の右下X座標
        /// @param y2 矩形の右下Y座標
        /// @param fillMode 描画モード (0=線, 1=塗りつぶし, デフォルト: 1)
        /// @return *this（メソッドチェーン用）
        Screen& circle(int x1, int y1, int x2, int y2, int fillMode = 1);

        /// @brief 1ドットの点を描画
        /// @param x 画面上のX座標
        /// @param y 画面上のY座標
        /// @return *this（メソッドチェーン用）
        Screen& pset(int x, int y);

        /// @brief カレントポジションに1ドットの点を描画
        /// @return *this（メソッドチェーン用）
        Screen& pset();

        /// @brief 1ドットの色を取得し、選択色として設定
        /// @param x 画面上のX座標
        /// @param y 画面上のY座標
        /// @return *this（メソッドチェーン用）
        Screen& pget(int x, int y);

        /// @brief カレントポジションの色を取得し、選択色として設定
        /// @return *this（メソッドチェーン用）
        Screen& pget();

        /// @brief 幅を取得
        [[nodiscard]] int width() const;

        /// @brief 高さを取得
        [[nodiscard]] int height() const;
    };


    // ============================================================
    // OOP版関数（ID自動採番）
    // ============================================================
    // IDは内部で自動採番され、Screen::id()で取得可能
    // HSP互換のID指定版とは独立して使用可能
    // ============================================================

    /// @brief ウィンドウを初期化（OOP版・構造体）
    /// @param params 初期化パラメータ（IDは自動採番）
    /// @return Screen ハンドル
    /// @example auto win = screen({.width = 800, .height = 600});
    export [[nodiscard]] Screen screen(const ScreenParams& params);

    /// @brief ウィンドウを初期化（OOP版・引数なし）
    /// @return Screen ハンドル（デフォルト設定で作成）
    /// @example auto win = screen();
    export [[nodiscard]] Screen screen();

    /// @brief 仮想画面を初期化（OOP版・構造体）
    /// @param params 初期化パラメータ（IDは自動採番）
    /// @return Screen ハンドル
    export [[nodiscard]] Screen buffer(const BufferParams& params);

    /// @brief 仮想画面を初期化（OOP版・引数なし）
    /// @return Screen ハンドル（デフォルト設定で作成）
    export [[nodiscard]] Screen buffer();

    /// @brief 枠なしウィンドウを初期化（OOP版・構造体）
    /// @param params 初期化パラメータ（IDは自動採番）
    /// @return Screen ハンドル
    export [[nodiscard]] Screen bgscr(const BgscrParams& params);

    /// @brief 枠なしウィンドウを初期化（OOP版・引数なし）
    /// @return Screen ハンドル（デフォルト設定で作成）
    export [[nodiscard]] Screen bgscr();

    // ============================================================
    // HSP互換版関数（ID明示指定）
    // ============================================================
    // 従来のHSP互換APIとして、IDを明示的に指定するバージョン
    // ============================================================

    /// @brief ウィンドウを初期化（HSP互換・省略可能版）
    /// @return Screen ハンドル
    /// @example auto scr = screen(0, 800, 600);
    /// @example screen(0);  // 戻り値を無視してもOK（HSP互換）
    export [[nodiscard]] Screen screen(
        int id,
        OptInt width    = {},
        OptInt height   = {},
        OptInt mode     = {},
        OptInt pos_x    = {},
        OptInt pos_y    = {},
        OptInt client_w = {},
        OptInt client_h = {},
        std::string_view title = "HSPPP Window"
    );

    // ============================================================
    // buffer - 仮想画面を初期化（HSP互換・ID明示指定版）
    // ============================================================
    /// @brief 仮想画面を初期化（メモリ上に画面を作成、表示しない）
    /// @param id ウィンドウID (0～)
    /// @param width 初期化する画面サイズX (デフォルト: 640)
    /// @param height 初期化する画面サイズY (デフォルト: 480)
    /// @param mode 初期化する画面モード (デフォルト: 0=フルカラー)
    /// @return Screen ハンドル
    export [[nodiscard]] Screen buffer(
        int id,
        OptInt width  = {},
        OptInt height = {},
        OptInt mode   = {}
    );

    // ============================================================
    // bgscr - 枠のないウィンドウを初期化（HSP互換・ID明示指定版）
    // ============================================================
    /// @brief 枠のないウィンドウを初期化
    /// @param id ウィンドウID (0～)
    /// @param width 初期化する画面サイズX (デフォルト: 640)
    /// @param height 初期化する画面サイズY (デフォルト: 480)
    /// @param mode 初期化する画面モード (0=フルカラー, 2=非表示)
    /// @param pos_x ウィンドウ位置X (-1=システム規定)
    /// @param pos_y ウィンドウ位置Y (-1=システム規定)
    /// @param client_w ウィンドウのサイズX (省略時=widthと同じ)
    /// @param client_h ウィンドウのサイズY (省略時=heightと同じ)
    /// @return Screen ハンドル
    export [[nodiscard]] Screen bgscr(
        int id,
        OptInt width    = {},
        OptInt height   = {},
        OptInt mode     = {},
        OptInt pos_x    = {},
        OptInt pos_y    = {},
        OptInt client_w = {},
        OptInt client_h = {}
    );

    // ============================================================
    // gsel - 描画先指定、ウィンドウ最前面、非表示設定（HSP互換）
    // ============================================================
    /// @brief 描画先を指定したウィンドウIDに変更
    /// @param id ウィンドウID
    /// @param mode ウィンドウアクティブスイッチ (-1=非表示, 0=影響なし, 1=アクティブ, 2=アクティブ+最前面)
    export void gsel(OptInt id = {}, OptInt mode = {});

    // ============================================================
    // gmode - 画面コピーモード設定（HSP互換）
    // ============================================================
    /// @brief 画面コピーモードを設定
    /// @param mode 画面コピーモード (0～6)
    /// @param size_x コピーする大きさX (デフォルト: 32)
    /// @param size_y コピーする大きさY (デフォルト: 32)
    /// @param blend_rate 半透明合成時のブレンド率 (0～256)
    export void gmode(OptInt mode = {}, OptInt size_x = {}, OptInt size_y = {}, OptInt blend_rate = {});

    // ============================================================
    // gcopy - 画面コピー（HSP互換）
    // ============================================================
    /// @brief 指定したウィンドウIDから現在の描画先にコピー
    /// @param src_id コピー元のウィンドウID
    /// @param src_x コピー元の左上X座標
    /// @param src_y コピー元の左上Y座標
    /// @param size_x コピーする大きさX (省略時=gmodeで設定したサイズ)
    /// @param size_y コピーする大きさY (省略時=gmodeで設定したサイズ)
    export void gcopy(OptInt src_id = {}, OptInt src_x = {}, OptInt src_y = {}, OptInt size_x = {}, OptInt size_y = {});

    // ============================================================
    // gzoom - 変倍して画面コピー（HSP互換）
    // ============================================================
    /// @brief 指定したウィンドウIDから変倍してコピー
    /// @param dest_w 画面にコピーする時の大きさX
    /// @param dest_h 画面にコピーする時の大きさY
    /// @param src_id コピー元のウィンドウID
    /// @param src_x コピー元の左上X座標
    /// @param src_y コピー元の左上Y座標
    /// @param src_w コピーする大きさX
    /// @param src_h コピーする大きさY
    /// @param mode ズームのモード (0=高速, 1=高品質ハーフトーン)
    export void gzoom(OptInt dest_w = {}, OptInt dest_h = {}, OptInt src_id = {}, OptInt src_x = {}, OptInt src_y = {}, OptInt src_w = {}, OptInt src_h = {}, OptInt mode = {});

    // 描画制御
    // p1: 0=描画予約(Offscreen), 1=画面反映(Present)
    export void redraw(int p1 = 1);

    // 待機＆メッセージ処理 (HSP互換)
    // 指定されたミリ秒だけ待機し、その間ウィンドウメッセージを処理する
    export void await(int time_ms);

    // プログラム終了 (HSP互換)
    // p1: 終了コード（省略時は0）
    export [[noreturn]] void end(int exitcode = 0);

    // --- Drawing Functions ---
    export void color(int r, int g, int b);
    export void pos(int x, int y);
    export void mes(std::string_view text);  // string_view で安全に
    export void boxf(int x1, int y1, int x2, int y2);
    // 引数なし版 boxf() -> 画面全体
    export void boxf();

    // ============================================================
    // line - 直線を描画（HSP互換）
    // ============================================================
    /// @brief 直線を描画
    /// @param x2 ラインの終点X座標 (デフォルト: 0)
    /// @param y2 ラインの終点Y座標 (デフォルト: 0)
    /// @param x1 ラインの始点X座標 (省略時=カレントポジション)
    /// @param y1 ラインの始点Y座標 (省略時=カレントポジション)
    /// @note 実行後、(x2, y2)がカレントポジションになる
    export void line(OptInt x2 = {}, OptInt y2 = {}, OptInt x1 = {}, OptInt y1 = {});

    // ============================================================
    // circle - 円を描画（HSP互換）
    // ============================================================
    /// @brief 円を描画
    /// @param x1 矩形の左上X座標 (デフォルト: 0)
    /// @param y1 矩形の左上Y座標 (デフォルト: 0)
    /// @param x2 矩形の右下X座標
    /// @param y2 矩形の右下Y座標
    /// @param fillMode 描画モード (0=線, 1=塗りつぶし, デフォルト: 1)
    export void circle(OptInt x1 = {}, OptInt y1 = {}, OptInt x2 = {}, OptInt y2 = {}, OptInt fillMode = {});

    // ============================================================
    // pset - 1ドットの点を描画（HSP互換）
    // ============================================================
    /// @brief 1ドットの点を描画
    /// @param x 画面上のX座標 (省略時=カレントポジション)
    /// @param y 画面上のY座標 (省略時=カレントポジション)
    export void pset(OptInt x = {}, OptInt y = {});

    // ============================================================
    // pget - 1ドットの色を取得（HSP互換）
    // ============================================================
    /// @brief 1ドットの色を取得し、選択色として設定
    /// @param x 画面上のX座標 (省略時=カレントポジション)
    /// @param y 画面上のY座標 (省略時=カレントポジション)
    /// @note 取得した色はginfo(16/17/18)またはginfo_r/g/bで参照可能
    export void pget(OptInt x = {}, OptInt y = {});

    // ============================================================
    // ginfo - ウィンドウ情報の取得（HSP互換）
    // ============================================================
    /// @brief ウィンドウ関連情報を取得
    /// @param type 取得するタイプ (0～27, 256～261)
    /// @return 指定したタイプの情報値
    /// @note タイプ一覧:
    ///   0-1: マウスカーソル座標(スクリーン), 2: アクティブウィンドウID,
    ///   3: 操作先ウィンドウID, 4-7: ウィンドウ座標, 8-9: 描画基点,
    ///   10-11: ウィンドウサイズ, 12-13: クライアント領域サイズ,
    ///   14-15: メッセージ出力サイズ, 16-18: カラーコード(RGB),
    ///   19: カラーモード, 20-21: デスクトップサイズ, 22-23: カレントポジション,
    ///   24: 割り込みウィンドウID, 25: 未使用ウィンドウID, 26-27: 初期化サイズ
    export int ginfo(int type);

    /// @brief 現在設定されている色コード(R)を取得
    export int ginfo_r();

    /// @brief 現在設定されている色コード(G)を取得
    export int ginfo_g();

    /// @brief 現在設定されている色コード(B)を取得
    export int ginfo_b();

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
