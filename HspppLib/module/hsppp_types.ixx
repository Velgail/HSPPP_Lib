// HspppLib/module/hsppp_types.ixx
// 型定義モジュール: OptInt, OptDouble, Screen, Quad 等の型定義

export module hsppp:types;

import <string_view>;
import <optional>;
import <source_location>;
import <stdexcept>;
import <format>;
import <functional>;
import <span>;
import <memory>;
import <string>;

export namespace hsppp {

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
    inline constexpr detail::OmitTag omit {};


    // ============================================================
    // OptInt - 省略可能なint型
    // ============================================================

    struct OptInt {
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
    // OptInt64 - 省略可能なint64_t型
    // ============================================================

    struct OptInt64 {
    private:
        std::optional<int64_t> value_;

    public:
        /// デフォルト（省略）- `{}` で使用可能
        constexpr OptInt64() noexcept : value_(std::nullopt) {}
        
        /// 省略マーカーからの変換 - `omit` で使用可能
        constexpr OptInt64(detail::OmitTag) noexcept : value_(std::nullopt) {}
        
        /// 値の指定
        constexpr OptInt64(int64_t v) noexcept : value_(v) {}
        constexpr OptInt64(int v) noexcept : value_(static_cast<int64_t>(v)) {}

        [[nodiscard]] constexpr bool is_default() const noexcept { 
            return !value_.has_value(); 
        }

        [[nodiscard]] constexpr int64_t value_or(int64_t def) const noexcept { 
            return value_.value_or(def); 
        }

        [[nodiscard]] constexpr int64_t value() const { 
            return value_.value(); 
        }
    };


    // ============================================================
    // OptDouble - 省略可能なdouble型
    // ============================================================

    struct OptDouble {
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


    // ============================================================
    // Screen Mode Flags (HSP Compatible)
    // ============================================================

    inline constexpr int screen_normal    = 0;    // フルカラーモード
    inline constexpr int screen_palette   = 1;    // パレットモード（256色）※未実装
    inline constexpr int screen_hide      = 2;    // 非表示ウィンドウ
    inline constexpr int screen_fixedsize = 4;    // サイズ固定
    inline constexpr int screen_tool      = 8;    // ツールウィンドウ
    inline constexpr int screen_frame     = 16;   // 深い縁のあるウィンドウ
    inline constexpr int screen_offscreen = 32;   // 描画先として初期化 (HSP3Dish/HGIMG4)
    inline constexpr int screen_usergcopy = 64;   // 描画用シェーダー (HGIMG4)
    inline constexpr int screen_fullscreen = 256; // フルスクリーン (bgscr用)


    // ============================================================
    // ScreenParams 構造体（Designated Initializers用）
    // ============================================================
    
    /// @brief screen命令のパラメータ構造体
    /// @details 多くのパラメータを指定する場合に使用
    /// @example screen({.width = 800, .height = 600});
    struct ScreenParams {
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
    struct BufferParams {
        int width    = 640;     ///< 画面サイズX
        int height   = 480;     ///< 画面サイズY
        int mode     = 0;       ///< 画面モード
    };

    /// @brief bgscr命令のパラメータ構造体
    struct BgscrParams {
        int width    = 640;     ///< 画面サイズX
        int height   = 480;     ///< 画面サイズY
        int mode     = 0;       ///< 画面モード (0=フルカラー, 2=非表示)
        int pos_x    = -1;      ///< ウィンドウ位置X (-1=システム規定)
        int pos_y    = -1;      ///< ウィンドウ位置Y (-1=システム規定)
        int client_w = 0;       ///< クライアントサイズX (0=widthと同じ)
        int client_h = 0;       ///< クライアントサイズY (0=heightと同じ)
        std::string_view title = "HSPPP Window";
    };


    // ============================================================
    // DialogResult - dialog命令の戻り値
    // ============================================================

    /// @brief dialog命令の結果を保持する構造体
    /// @details intへの暗黙の型変換（stat相当）とstd::stringへの暗黙の型変換（refstr相当）をサポート
    struct DialogResult {
        int stat;           ///< ステータス値（ボタンID、成功/失敗など）
        std::string refstr; ///< 文字列結果（ファイルパスなど）

        /// @brief intへの暗黙の型変換 (stat相当)
        operator int() const noexcept { return stat; }

        /// @brief std::stringへの暗黙の型変換 (refstr相当)
        operator std::string() const { return refstr; }

        /// @brief 成功判定 (stat != 0)
        explicit operator bool() const noexcept { return stat != 0; }
    };


    // ============================================================
    // Quad構造体 - gsquare用の4頂点座標（HSP互換）
    // ============================================================

    /// @brief 2次元座標（gsquare頂点用）
    struct Point2i {
        int x = 0;
        int y = 0;
        
        constexpr Point2i() noexcept = default;
        constexpr Point2i(int px, int py) noexcept : x(px), y(py) {}
    };

    /// @brief 4頂点座標（gsquareコピー先用）
    /// @details 頂点順序: 左上, 右上, 右下, 左下（時計回り）
    struct Quad {
        static constexpr size_t vertex_count = 4;
        Point2i v[vertex_count];

        constexpr Quad() noexcept = default;
        
        /// @brief 4つのPoint2iで初期化
        constexpr Quad(Point2i p0, Point2i p1, Point2i p2, Point2i p3) noexcept
            : v{p0, p1, p2, p3} {}

        /// @brief 直接座標で初期化（HSP互換の配列順）
        constexpr Quad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) noexcept
            : v{{x0, y0}, {x1, y1}, {x2, y2}, {x3, y3}} {}

        /// @brief インデックスアクセス（境界チェック付き）
        /// @throws std::out_of_range インデックスが範囲外の場合
        constexpr Point2i& operator[](size_t i) {
            if (i >= vertex_count) throw std::out_of_range("Quad index out of range");
            return v[i];
        }
        constexpr const Point2i& operator[](size_t i) const {
            if (i >= vertex_count) throw std::out_of_range("Quad index out of range");
            return v[i];
        }

        /// @brief 境界チェック付きアクセス（operator[]と同等）
        constexpr Point2i& at(size_t i) {
            if (i >= vertex_count) throw std::out_of_range("Quad index out of range");
            return v[i];
        }
        constexpr const Point2i& at(size_t i) const {
            if (i >= vertex_count) throw std::out_of_range("Quad index out of range");
            return v[i];
        }

        /// @brief 要素数を取得
        [[nodiscard]] static constexpr size_t size() noexcept { return vertex_count; }

        /// @brief 全要素へのspan（安全なイテレータ）
        [[nodiscard]] constexpr std::span<Point2i, vertex_count> data() noexcept { return v; }
        [[nodiscard]] constexpr std::span<const Point2i, vertex_count> data() const noexcept { return v; }

        /// @brief イテレータサポート（範囲ベースfor用）
        constexpr auto begin() noexcept { return data().begin(); }
        constexpr auto end() noexcept { return data().end(); }
        constexpr auto begin() const noexcept { return data().begin(); }
        constexpr auto end() const noexcept { return data().end(); }
    };

    /// @brief 4頂点UV座標（gsquareコピー元用）
    /// @details Quadと同じ構造だが、意味的に区別するために別型
    struct QuadUV {
        static constexpr size_t vertex_count = 4;
        Point2i v[vertex_count];

        constexpr QuadUV() noexcept = default;
        
        constexpr QuadUV(Point2i p0, Point2i p1, Point2i p2, Point2i p3) noexcept
            : v{p0, p1, p2, p3} {}

        constexpr QuadUV(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) noexcept
            : v{{x0, y0}, {x1, y1}, {x2, y2}, {x3, y3}} {}

        /// @brief インデックスアクセス（境界チェック付き）
        constexpr Point2i& operator[](size_t i) {
            if (i >= vertex_count) throw std::out_of_range("QuadUV index out of range");
            return v[i];
        }
        constexpr const Point2i& operator[](size_t i) const {
            if (i >= vertex_count) throw std::out_of_range("QuadUV index out of range");
            return v[i];
        }

        /// @brief 境界チェック付きアクセス（operator[]と同等）
        constexpr Point2i& at(size_t i) {
            if (i >= vertex_count) throw std::out_of_range("QuadUV index out of range");
            return v[i];
        }
        constexpr const Point2i& at(size_t i) const {
            if (i >= vertex_count) throw std::out_of_range("QuadUV index out of range");
            return v[i];
        }

        /// @brief 要素数を取得
        [[nodiscard]] static constexpr size_t size() noexcept { return vertex_count; }

        /// @brief 全要素へのspan（安全なイテレータ）
        [[nodiscard]] constexpr std::span<Point2i, vertex_count> data() noexcept { return v; }
        [[nodiscard]] constexpr std::span<const Point2i, vertex_count> data() const noexcept { return v; }

        /// @brief イテレータサポート（範囲ベースfor用）
        constexpr auto begin() noexcept { return data().begin(); }
        constexpr auto end() noexcept { return data().end(); }
        constexpr auto begin() const noexcept { return data().begin(); }
        constexpr auto end() const noexcept { return data().end(); }
    };

    /// @brief 4頂点カラー（gsquareグラデーション用）
    /// @details RGBカラーコード（0xRRGGBB形式）を4頂点分保持
    struct QuadColors {
        static constexpr size_t color_count = 4;
        int colors[color_count];

        constexpr QuadColors() noexcept : colors{0, 0, 0, 0} {}
        
        constexpr QuadColors(int c0, int c1, int c2, int c3) noexcept
            : colors{c0, c1, c2, c3} {}

        /// @brief インデックスアクセス（境界チェック付き）
        constexpr int& operator[](size_t i) {
            if (i >= color_count) throw std::out_of_range("QuadColors index out of range");
            return colors[i];
        }
        constexpr const int& operator[](size_t i) const {
            if (i >= color_count) throw std::out_of_range("QuadColors index out of range");
            return colors[i];
        }

        /// @brief 境界チェック付きアクセス（operator[]と同等）
        constexpr int& at(size_t i) {
            if (i >= color_count) throw std::out_of_range("QuadColors index out of range");
            return colors[i];
        }
        constexpr const int& at(size_t i) const {
            if (i >= color_count) throw std::out_of_range("QuadColors index out of range");
            return colors[i];
        }

        /// @brief 要素数を取得
        [[nodiscard]] static constexpr size_t size() noexcept { return color_count; }

        /// @brief 全要素へのspan（安全なイテレータ）
        [[nodiscard]] constexpr std::span<int, color_count> data() noexcept { return colors; }
        [[nodiscard]] constexpr std::span<const int, color_count> data() const noexcept { return colors; }

        /// @brief イテレータサポート（範囲ベースfor用）
        constexpr auto begin() noexcept { return data().begin(); }
        constexpr auto end() noexcept { return data().end(); }
        constexpr auto begin() const noexcept { return data().begin(); }
        constexpr auto end() const noexcept { return data().end(); }
    };


    // ============================================================
    // 割り込みハンドラ型定義
    // ============================================================

    // 前方宣言（エラークラスは hsppp_interrupt.ixx で定義）
    class HspErrorBase;
    class HspError;
    class HspWeakError;

    /// @brief 汎用割り込みハンドラ型（onclick, onkey, onexit, oncmd用）
    /// @note ラムダ式、関数ポインタ、関数オブジェクトをサポート
    using InterruptHandler = std::function<int()>;

    /// @brief エラーハンドラ型（onerror用）
    /// @note HspErrorBaseオブジェクトを受け取る（HspError/HspWeakError両方に対応）
    using ErrorHandler = std::function<int(const HspErrorBase&)>;


    // ============================================================
    // Cel クラス - 軽量ハンドル
    // ============================================================

    /// @brief Cel画像素材への軽量ハンドル
    /// @details 画像データと分割情報を持つ。celload()で作成
    class Cel {
    private:
        int m_id;
        bool m_valid;

    public:
        /// @brief 内部用コンストラクタ（ID + valid指定）
        Cel(int id, bool valid) noexcept
            : m_id(id), m_valid(valid) {}

        /// @brief IDからのコンストラクタ（グローバルマップから有効性を判定）
        explicit Cel(int id) noexcept;

        /// @brief デフォルトコンストラクタ（無効なハンドル）
        Cel() noexcept : m_id(-1), m_valid(false) {}

        // コピー・ムーブはデフォルト
        Cel(const Cel&) = default;
        Cel& operator=(const Cel&) = default;
        Cel(Cel&&) noexcept = default;
        Cel& operator=(Cel&&) noexcept = default;

        /// @brief 有効なハンドルかどうか
        [[nodiscard]] bool valid() const noexcept { return m_valid; }
        [[nodiscard]] explicit operator bool() const noexcept { return valid(); }

        /// @brief cel IDを取得
        [[nodiscard]] int id() const noexcept { return m_id; }

        /// @brief 画像素材の分割サイズを設定
        /// @param divX 横方向の分割数
        /// @param divY 縦方向の分割数
        /// @return *this（メソッドチェーン用）
        Cel& divide(int divX, int divY, const std::source_location& location = std::source_location::current());

        /// @brief 画像素材を描画
        /// @param cellIndex 表示するセル番号
        /// @param x X座標（省略時は現在のpos）
        /// @param y Y座標（省略時は現在のpos）
        /// @return *this（メソッドチェーン用）
        Cel& put(int cellIndex, OptInt x = {}, OptInt y = {}, const std::source_location& location = std::source_location::current());

        /// @brief 画像の幅を取得
        [[nodiscard]] int width(const std::source_location& location = std::source_location::current()) const;

        /// @brief 画像の高さを取得
        [[nodiscard]] int height(const std::source_location& location = std::source_location::current()) const;
    };


    // ============================================================
    // Screen クラス - 軽量ハンドル（実体として操作可能）
    // ============================================================

    /// @brief Surfaceへの軽量ハンドル
    /// @details 実体のように `.` でアクセスできる。内部は ID のみ保持
    class Screen {
    private:
        int m_id;
        bool m_valid;

    public:
        /// @brief 内部用コンストラクタ（ID + valid指定）
        Screen(int id, bool valid) noexcept
            : m_id(id), m_valid(valid) {}

        /// @brief IDからのコンストラクタ（グローバルマップから有効性を判定）
        explicit Screen(int id) noexcept;

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
        Screen& color(int r, int g, int b, const std::source_location& location = std::source_location::current());

        /// @brief 描画位置を設定
        Screen& pos(int x, int y, const std::source_location& location = std::source_location::current());

        /// @brief 文字列を描画
        /// @param text メッセージ文字列
        /// @param sw オプション (1=改行しない, 2=影, 4=縁取り, 8=簡易描画, 16=gmode設定)
        Screen& mes(std::string_view text, OptInt sw = {}, const std::source_location& location = std::source_location::current());

        /// @brief 矩形を塗りつぶし
        Screen& boxf(int x1, int y1, int x2, int y2, const std::source_location& location = std::source_location::current());

        /// @brief 画面全体を塗りつぶし
        Screen& boxf(const std::source_location& location = std::source_location::current());

        /// @brief 画面クリア
        /// @param mode クリアする時の色 (0=白, 1=明るい灰色, 2=灰色, 3=暗い灰色, 4=黒)
        Screen& cls(int mode = 0, const std::source_location& location = std::source_location::current());

        /// @brief 描画制御
        Screen& redraw(int mode = 1, const std::source_location& location = std::source_location::current());

        /// @brief このScreenをカレントに設定（HSPのgsel相当）
        Screen& select(const std::source_location& location = std::source_location::current());

        /// @brief 直線を描画
        Screen& line(int x2, int y2, const std::source_location& location = std::source_location::current());
        Screen& line(int x2, int y2, int x1, int y1, const std::source_location& location = std::source_location::current());

        /// @brief 円を描画
        Screen& circle(int x1, int y1, int x2, int y2, int fillMode = 1, const std::source_location& location = std::source_location::current());

        /// @brief 1ドットの点を描画
        Screen& pset(int x, int y, const std::source_location& location = std::source_location::current());

        /// @brief カレントポジションに1ドットの点を描画
        Screen& pset(const std::source_location& location = std::source_location::current());

        /// @brief 1ドットの色を取得し、選択色として設定
        Screen& pget(int x, int y, const std::source_location& location = std::source_location::current());

        /// @brief カレントポジションの色を取得し、選択色として設定
        Screen& pget(const std::source_location& location = std::source_location::current());

        /// @brief 矩形をグラデーションで塗りつぶす（OOP版）
        Screen& gradf(int x, int y, int w, int h, int mode, int color1, int color2, const std::source_location& location = std::source_location::current());

        /// @brief 回転する矩形で塗りつぶす（OOP版）
        Screen& grect(int cx, int cy, double angle, int w, int h, const std::source_location& location = std::source_location::current());

        /// @brief 幅を取得
        [[nodiscard]] int width(const std::source_location& location = std::source_location::current()) const;

        /// @brief 高さを取得
        [[nodiscard]] int height(const std::source_location& location = std::source_location::current()) const;

        /// @brief ウィンドウサイズ・位置を設定（width命令のOOP版）
        Screen& width(int clientW, int clientH = -1, int posX = -1, int posY = -1, int option = 0, const std::source_location& location = std::source_location::current());

        /// @brief スクロール位置を設定（groll命令のOOP版）
        Screen& groll(int scrollX, int scrollY, const std::source_location& location = std::source_location::current());

        /// @brief フォントを設定
        Screen& font(std::string_view fontName, int size = 12, int style = 0, const std::source_location& location = std::source_location::current());

        /// @brief システムフォントを選択
        Screen& sysfont(int type = 0, const std::source_location& location = std::source_location::current());

        /// @brief タイトルバーを設定
        Screen& title(std::string_view title, const std::source_location& location = std::source_location::current());

        /// @brief 画像ファイルをロード
        Screen& picload(std::string_view filename, int mode = 0, const std::source_location& location = std::source_location::current());

        /// @brief 画面イメージをBMPファイルに保存
        Screen& bmpsave(std::string_view filename, const std::source_location& location = std::source_location::current());

        /// @brief このウィンドウ内でのマウスカーソルX座標を取得
        [[nodiscard]] int mousex() const;

        /// @brief このウィンドウ内でのマウスカーソルY座標を取得
        [[nodiscard]] int mousey() const;

        // ============================================================
        // 割り込みハンドラ（OOP版・ウィンドウ別設定）
        // ============================================================

        /// @brief クリック割り込みを設定
        Screen& onclick(InterruptHandler handler, const std::source_location& location = std::source_location::current());

        /// @brief Windowsメッセージ割り込みを設定
        Screen& oncmd(InterruptHandler handler, int messageId, const std::source_location& location = std::source_location::current());

        /// @brief キー割り込みを設定
        Screen& onkey(InterruptHandler handler, const std::source_location& location = std::source_location::current());

        // ============================================================
        // 画面コピー・変形描画（OOP版）
        // ============================================================

        /// @brief 画面コピーモードを設定（OOP版）
        Screen& gmode(int mode, int sizeX = 32, int sizeY = 32, int blendRate = 256, const std::source_location& location = std::source_location::current());

        /// @brief 画面コピー（OOP版）
        Screen& gcopy(int srcId, int srcX, int srcY, OptInt sizeX = {}, OptInt sizeY = {}, const std::source_location& location = std::source_location::current());

        /// @brief 変倍して画面コピー（OOP版）
        Screen& gzoom(int destW, int destH, int srcId, int srcX, int srcY, OptInt srcW = {}, OptInt srcH = {}, int mode = 0, const std::source_location& location = std::source_location::current());

        /// @brief 矩形画像を回転してコピー（OOP版）
        Screen& grotate(int srcId, int srcX, int srcY, double angle, OptInt dstW = {}, OptInt dstH = {}, const std::source_location& location = std::source_location::current());

        /// @brief 任意の四角形を単色塗りつぶし（OOP版）
        Screen& gsquare(int srcId, const Quad& dst, const std::source_location& location = std::source_location::current());

        /// @brief 任意の四角形へ画像をコピー（OOP版）
        Screen& gsquare(int srcId, const Quad& dst, const QuadUV& src, const std::source_location& location = std::source_location::current());

        /// @brief 任意の四角形をグラデーション塗りつぶし（OOP版）
        Screen& gsquare(int srcId, const Quad& dst, const QuadColors& colors, const std::source_location& location = std::source_location::current());

        // ============================================================
        // ウィンドウ表示制御（OOP版）
        // ============================================================

        /// @brief ウィンドウを表示（gsel id, 1 相当）
        Screen& show(const std::source_location& location = std::source_location::current());

        /// @brief ウィンドウを非表示（gsel id, -1 相当）
        Screen& hide(const std::source_location& location = std::source_location::current());

        /// @brief ウィンドウを最前面でアクティブ化（gsel id, 2 相当）
        Screen& activate(const std::source_location& location = std::source_location::current());

        // ============================================================
        // Cel描画（OOP版・Screen側主体）
        // ============================================================

        /// @brief 画像素材を描画（Screen側主体版）
        Screen& celput(const Cel& cel, int cellIndex, OptInt x = {}, OptInt y = {}, const std::source_location& location = std::source_location::current());

        // ============================================================
        // GUIオブジェクト生成（OOP版・ウィンドウ指定）
        // ============================================================

        /// @brief ボタンを生成（OOP版）
        int button(std::string_view name, std::function<int()> callback, bool isGosub = true, const std::source_location& location = std::source_location::current());

        /// @brief 入力ボックスを生成（OOP版・shared_ptr版）
        int input(std::shared_ptr<std::string> var, int maxLength = 1024, int mode = 0, const std::source_location& location = std::source_location::current());

        /// @brief 複数行入力ボックスを生成（OOP版・shared_ptr版）
        int mesbox(std::shared_ptr<std::string> var, int maxLength = 4096, int mode = 0, const std::source_location& location = std::source_location::current());

        /// @brief オブジェクトのサイズと間隔を設定（OOP版）
        Screen& objsize(int sizeX, int sizeY = 24, int spaceY = 0, const std::source_location& location = std::source_location::current());

        /// @brief マウスカーソルの座標を設定（OOP版）
        Screen& mouse(int x, int y, const std::source_location& location = std::source_location::current());

        // ============================================================
        // 追加GUIオブジェクト生成（OOP版）
        // ============================================================

        /// @brief チェックボックスを生成（OOP版・shared_ptr版）
        int chkbox(std::string_view label, std::shared_ptr<int> var, const std::source_location& location = std::source_location::current());

        /// @brief コンボボックスを生成（OOP版・shared_ptr版）
        int combox(std::shared_ptr<int> var, int expandY, std::string_view items, const std::source_location& location = std::source_location::current());

        /// @brief リストボックスを生成（OOP版・shared_ptr版）
        int listbox(std::shared_ptr<int> var, int expandY, std::string_view items, const std::source_location& location = std::source_location::current());

        // ============================================================
        // GUIオブジェクト設定（OOP版）
        // ============================================================

        /// @brief オブジェクトモード設定（OOP版）
        Screen& objmode(int mode, int tabMove = -1, const std::source_location& location = std::source_location::current());

        /// @brief オブジェクトのカラー設定（OOP版）
        Screen& objcolor(int r, int g, int b, const std::source_location& location = std::source_location::current());

        // ============================================================
        // GUIオブジェクト操作（OOP版）
        // ============================================================

        /// @brief オブジェクトの内容を変更（OOP版・文字列）
        Screen& objprm(int objectId, std::string_view value, const std::source_location& location = std::source_location::current());

        /// @brief オブジェクトの内容を変更（OOP版・整数）
        Screen& objprm(int objectId, int value, const std::source_location& location = std::source_location::current());

        /// @brief オブジェクトの有効・無効を設定（OOP版）
        Screen& objenable(int objectId, int enable = 1, const std::source_location& location = std::source_location::current());

        /// @brief オブジェクトに入力フォーカスを設定（OOP版）
        Screen& objsel(int objectId, const std::source_location& location = std::source_location::current());
    };

} // namespace hsppp
