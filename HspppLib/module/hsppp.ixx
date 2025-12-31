// HspppLib/module/hsppp.ixx
export module hsppp;

// 必要な標準ライブラリをインポート
import <string_view>;
import <optional>;
import <source_location>;
import <stdexcept>;
import <format>;
import <functional>;
import <random>;
import <chrono>;
import <cmath>;
import <string>;
import <algorithm>;
import <numbers>;
import <vector>;
import <span>;

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
    // OptInt64 - 省略可能なint64_t型
    // ============================================================

    export struct OptInt64 {
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

    // --- stick キーコードビットマスク (HSP Compatible) ---
    export inline constexpr int stick_left    = 1;       // カーソルキー左(←)
    export inline constexpr int stick_up      = 2;       // カーソルキー上(↑)
    export inline constexpr int stick_right   = 4;       // カーソルキー右(→)
    export inline constexpr int stick_down    = 8;       // カーソルキー下(↓)
    export inline constexpr int stick_space   = 16;      // スペースキー
    export inline constexpr int stick_enter   = 32;      // Enterキー
    export inline constexpr int stick_ctrl    = 64;      // Ctrlキー
    export inline constexpr int stick_esc     = 128;     // ESCキー
    export inline constexpr int stick_lbutton = 256;     // マウス左ボタン
    export inline constexpr int stick_rbutton = 512;     // マウス右ボタン
    export inline constexpr int stick_tab     = 1024;    // TABキー
    export inline constexpr int stick_z       = 2048;    // [Z]キー
    export inline constexpr int stick_x       = 4096;    // [X]キー
    export inline constexpr int stick_c       = 8192;    // [C]キー
    export inline constexpr int stick_a       = 16384;   // [A]キー
    export inline constexpr int stick_w       = 32768;   // [W]キー
    export inline constexpr int stick_d       = 65536;   // [D]キー
    export inline constexpr int stick_s       = 131072;  // [S]キー

    // --- gmode コピーモード定数 (HSP Compatible) ---
    export inline constexpr int gmode_copy       = 0;    // 通常コピー
    export inline constexpr int gmode_mem        = 1;    // メモリ間コピー（高速）
    export inline constexpr int gmode_and        = 2;    // 透明色付きコピー
    export inline constexpr int gmode_or         = 3;    // 半透明合成コピー
    export inline constexpr int gmode_alpha      = 4;    // 半透明合成コピー
    export inline constexpr int gmode_add        = 5;    // 加算合成コピー
    export inline constexpr int gmode_sub        = 6;    // 減算合成コピー

    // --- ginfo type定数 (HSP Compatible) ---
    // 注意: HSPでは ginfo_* は「値を返すマクロ/システム変数」の見た目を持つため、
    // ここでは「ginfo(int type)のセレクタ」であることが明確な *_type_* 命名にする。
    export inline constexpr int ginfo_type_mx        = 0;     // スクリーン上のマウスカーソルX座標
    export inline constexpr int ginfo_type_my        = 1;     // スクリーン上のマウスカーソルY座標
    export inline constexpr int ginfo_type_act       = 2;     // アクティブなウィンドウID
    export inline constexpr int ginfo_type_sel       = 3;     // 操作先ウィンドウID
    export inline constexpr int ginfo_type_wx1       = 4;     // ウィンドウの左上X座標
    export inline constexpr int ginfo_type_wy1       = 5;     // ウィンドウの左上Y座標
    export inline constexpr int ginfo_type_wx2       = 6;     // ウィンドウの右下X座標
    export inline constexpr int ginfo_type_wy2       = 7;     // ウィンドウの右下Y座標
    export inline constexpr int ginfo_type_vx        = 8;     // 画面の可視範囲の左上X（groll基点）
    export inline constexpr int ginfo_type_vy        = 9;     // 画面の可視範囲の左上Y（groll基点）
    export inline constexpr int ginfo_type_sizex     = 10;    // ウィンドウ全体のXサイズ
    export inline constexpr int ginfo_type_sizey     = 11;    // ウィンドウ全体のYサイズ
    export inline constexpr int ginfo_type_mesx      = 12;    // クライアント領域のXサイズ
    export inline constexpr int ginfo_type_mesy      = 13;    // クライアント領域のYサイズ
    export inline constexpr int ginfo_type_messizex  = 14;    // 最後のmes出力のXサイズ
    export inline constexpr int ginfo_type_messizey  = 15;    // 最後のmes出力のYサイズ
    export inline constexpr int ginfo_type_r         = 16;    // カレントカラーのR成分
    export inline constexpr int ginfo_type_g         = 17;    // カレントカラーのG成分
    export inline constexpr int ginfo_type_b         = 18;    // カレントカラーのB成分
    // 互換: 以前の命名（typeの意味は同じ）
    export inline constexpr int ginfo_type_colr      = ginfo_type_r;
    export inline constexpr int ginfo_type_colg      = ginfo_type_g;
    export inline constexpr int ginfo_type_colb      = ginfo_type_b;
    export inline constexpr int ginfo_type_paluse    = 19;    // パレットモードフラグ
    export inline constexpr int ginfo_type_dispx     = 20;    // デスクトップのXサイズ
    export inline constexpr int ginfo_type_dispy     = 21;    // デスクトップのYサイズ
    export inline constexpr int ginfo_type_cx        = 22;    // カレントポジションX
    export inline constexpr int ginfo_type_cy        = 23;    // カレントポジションY
    export inline constexpr int ginfo_type_intid     = 24;    // メッセージ割り込み時のウィンドウID
    export inline constexpr int ginfo_type_newid     = 25;    // 未使用ウィンドウID
    export inline constexpr int ginfo_type_sx        = 26;    // 画面の初期化Xサイズ
    export inline constexpr int ginfo_type_sy        = 27;    // 画面の初期化Yサイズ

    // --- dirinfo type定数 (HSP Compatible) ---
    // 注意: HSPでは dir_* は値を返すマクロ/システム変数の見た目を持つため、
    // ここでは「dirinfo(int type)のセレクタ」であることが明確な dir_type_* 命名にする。
    export inline constexpr int dir_type_cur     = 0;     // カレントディレクトリ
    export inline constexpr int dir_type_exe     = 1;     // 実行ファイルディレクトリ
    export inline constexpr int dir_type_win     = 2;     // Windowsディレクトリ
    export inline constexpr int dir_type_sys     = 3;     // Windowsシステムディレクトリ
    export inline constexpr int dir_type_cmdline = 4;     // コマンドライン文字列
    export inline constexpr int dir_type_tv      = 5;     // HSPTVディレクトリ（常に空）


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
        std::string_view title = "HSPPP Window";
    };

    // ============================================================
    // DialogResult - dialog命令の戻り値
    // ============================================================

    /// @brief dialog命令の結果を保持する構造体
    /// @details intへの暗黙の型変換（stat相当）とstd::stringへの暗黙の型変換（refstr相当）をサポート
    export struct DialogResult {
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
    // HSPのgsquare命令では、4頂点を配列で指定する:
    //   gsquare srcId, dstX, dstY, srcX, srcY
    // C++では型安全な構造体で表現する。
    // 
    // 使用例:
    //   Quad dst = {{0, 0}, {100, 0}, {100, 100}, {0, 100}};
    //   gsquare(-1, dst);  // 単色塗りつぶし
    //
    //   QuadUV src = {{0, 0}, {32, 0}, {32, 32}, {0, 32}};
    //   gsquare(0, dst, src);  // 画像コピー
    //
    //   QuadColors colors = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
    //   gsquare(gsquare_grad, dst, colors);  // グラデーション
    // ============================================================

    /// @brief 2次元座標（gsquare頂点用）
    export struct Point2i {
        int x = 0;
        int y = 0;
        
        constexpr Point2i() noexcept = default;
        constexpr Point2i(int px, int py) noexcept : x(px), y(py) {}
    };

    /// @brief 4頂点座標（gsquareコピー先用）
    /// @details 頂点順序: 左上, 右上, 右下, 左下（時計回り）
    export struct Quad {
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
    export struct QuadUV {
        static constexpr size_t vertex_count = 4;
        Point2i v[vertex_count];

        constexpr QuadUV() noexcept = default;
        
        constexpr QuadUV(Point2i p0, Point2i p1, Point2i p2, Point2i p3) noexcept
            : v{p0, p1, p2, p3} {}

        constexpr QuadUV(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) noexcept
            : v{{x0, y0}, {x1, y1}, {x2, y2}, {x3, y3}} {}

        /// @brief インデックスアクセス（境界チェック付き）
        /// @throws std::out_of_range インデックスが範囲外の場合
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
    export struct QuadColors {
        static constexpr size_t color_count = 4;
        int colors[color_count];

        constexpr QuadColors() noexcept : colors{0, 0, 0, 0} {}
        
        constexpr QuadColors(int c0, int c1, int c2, int c3) noexcept
            : colors{c0, c1, c2, c3} {}

        /// @brief インデックスアクセス（境界チェック付き）
        /// @throws std::out_of_range インデックスが範囲外の場合
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
    // 割り込みハンドラ型定義（Screen クラスより前に定義が必要）
    // ============================================================

    // 前方宣言（HspErrorクラスはこのファイルの後半で定義）
    export class HspError;

    /// @brief 汎用割り込みハンドラ型（onclick, onkey, onexit, oncmd用）
    /// @note ラムダ式、関数ポインタ、関数オブジェクトをサポート
    export using InterruptHandler = std::function<int()>;

    /// @brief エラーハンドラ型（onerror用）
    /// @note HspErrorオブジェクトを受け取る
    export using ErrorHandler = std::function<int(const HspError&)>;

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

    /// @brief Cel画像素材への軽量ハンドル
    /// @details 画像データと分割情報を持つ。celload()で作成
    export class Cel {
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
        Cel& divide(int divX, int divY);

        /// @brief 画像素材を描画
        /// @param cellIndex 表示するセル番号
        /// @param x X座標（省略時は現在のpos）
        /// @param y Y座標（省略時は現在のpos）
        /// @return *this（メソッドチェーン用）
        Cel& put(int cellIndex, OptInt x = {}, OptInt y = {});

        /// @brief 画像の幅を取得
        [[nodiscard]] int width() const;

        /// @brief 画像の高さを取得
        [[nodiscard]] int height() const;
    };

    /// @brief Surfaceへの軽量ハンドル
    /// @details 実体のように `.` でアクセスできる。内部は ID のみ保持
    export class Screen {
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
        Screen& color(int r, int g, int b);

        /// @brief 描画位置を設定
        Screen& pos(int x, int y);

        /// @brief 文字列を描画
        /// @param text メッセージ文字列
        /// @param sw オプション (1=改行しない, 2=影, 4=縁取り, 8=簡易描画, 16=gmode設定)
        Screen& mes(std::string_view text, OptInt sw = {});

        /// @brief 矩形を塗りつぶし
        Screen& boxf(int x1, int y1, int x2, int y2);

        /// @brief 画面全体を塗りつぶし
        Screen& boxf();

        /// @brief 画面クリア
        /// @param mode クリアする時の色 (0=白, 1=明るい灰色, 2=灰色, 3=暗い灰色, 4=黒)
        Screen& cls(int mode = 0);

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

        /// @brief 矩形をグラデーションで塗りつぶす（OOP版）
        /// @param x 矩形の左上X座標
        /// @param y 矩形の左上Y座標
        /// @param w 矩形のXサイズ
        /// @param h 矩形のYサイズ
        /// @param mode グラデーションのモード (0=横方向, 1=縦方向)
        /// @param color1 塗りつぶし色1 (RGBカラーコード)
        /// @param color2 塗りつぶし色2 (RGBカラーコード)
        /// @return *this（メソッドチェーン用）
        Screen& gradf(int x, int y, int w, int h, int mode, int color1, int color2);

        /// @brief 回転する矩形で塗りつぶす（OOP版）
        /// @param cx 矩形の中心X座標
        /// @param cy 矩形の中心Y座標
        /// @param angle 回転角度（ラジアン）
        /// @param w Xサイズ
        /// @param h Yサイズ
        /// @return *this（メソッドチェーン用）
        Screen& grect(int cx, int cy, double angle, int w, int h);

        /// @brief 幅を取得
        [[nodiscard]] int width() const;

        /// @brief 高さを取得
        [[nodiscard]] int height() const;

        /// @brief ウィンドウサイズ・位置を設定（width命令のOOP版）
        /// @param clientW クライアントサイズX
        /// @param clientH クライアントサイズY (-1=変更なし)
        /// @param posX ウィンドウ位置X (-1=変更なし)
        /// @param posY ウィンドウ位置Y (-1=変更なし)
        /// @param option 座標設定オプション (0=負値で現在維持, 1=負値も設定)
        /// @return *this（メソッドチェーン用）
        Screen& width(int clientW, int clientH = -1, int posX = -1, int posY = -1, int option = 0);

        /// @brief スクロール位置を設定（groll命令のOOP版）
        /// @param scrollX 描画基点X座標
        /// @param scrollY 描画基点Y座標
        /// @return *this（メソッドチェーン用）
        Screen& groll(int scrollX, int scrollY);

        /// @brief フォントを設定
        /// @param fontName フォント名
        /// @param size フォントサイズ (デフォルト: 12)
        /// @param style フォントスタイル (デフォルト: 0)
        /// @return *this（メソッドチェーン用）
        Screen& font(std::string_view fontName, int size = 12, int style = 0);

        /// @brief システムフォントを選択
        /// @param type フォント種類 (0=HSP標準, 10-17=システムフォント)
        /// @return *this（メソッドチェーン用）
        Screen& sysfont(int type = 0);

        /// @brief タイトルバーを設定
        /// @param title タイトル文字列
        /// @return *this（メソッドチェーン用）
        Screen& title(std::string_view title);

        /// @brief 画像ファイルをロード
        /// @param filename ファイル名
        /// @param mode モード (0=初期化してロード, 1=現在の画面に重ねる, 2=黒で初期化してロード)
        /// @return *this（メソッドチェーン用）
        Screen& picload(std::string_view filename, int mode = 0);

        /// @brief 画面イメージをBMPファイルに保存
        /// @param filename ファイル名
        /// @return *this（メソッドチェーン用）
        Screen& bmpsave(std::string_view filename);

        /// @brief このウィンドウ内でのマウスカーソルX座標を取得
        /// @return X座標
        [[nodiscard]] int mousex() const;

        /// @brief このウィンドウ内でのマウスカーソルY座標を取得
        /// @return Y座標
        [[nodiscard]] int mousey() const;

        // ============================================================
        // 割り込みハンドラ（OOP版・ウィンドウ別設定）
        // ============================================================

        /// @brief クリック割り込みを設定
        /// @param handler コールバック関数/ラムダ (nullptr で解除)
        Screen& onclick(InterruptHandler handler);

        /// @brief Windowsメッセージ割り込みを設定
        /// @param handler コールバック関数/ラムダ (nullptr で解除)
        /// @param messageId 監視するメッセージID
        Screen& oncmd(InterruptHandler handler, int messageId);

        /// @brief キー割り込みを設定
        /// @param handler コールバック関数/ラムダ (nullptr で解除)
        Screen& onkey(InterruptHandler handler);

        // ============================================================
        // 画面コピー・変形描画（OOP版）
        // ============================================================

        /// @brief 画面コピーモードを設定（OOP版）
        /// @param mode 画面コピーモード (0～6)
        /// @param sizeX コピーする大きさX (デフォルト: 32)
        /// @param sizeY コピーする大きさY (デフォルト: 32)
        /// @param blendRate 半透明合成時のブレンド率 (0～256)
        /// @return *this（メソッドチェーン用）
        Screen& gmode(int mode, int sizeX = 32, int sizeY = 32, int blendRate = 256);

        /// @brief 画面コピー（OOP版）
        /// @param srcId コピー元のウィンドウID
        /// @param srcX コピー元の左上X座標
        /// @param srcY コピー元の左上Y座標
        /// @param sizeX コピーする大きさX (省略時=gmodeで設定したサイズ)
        /// @param sizeY コピーする大きさY (省略時=gmodeで設定したサイズ)
        /// @return *this（メソッドチェーン用）
        Screen& gcopy(int srcId, int srcX, int srcY, OptInt sizeX = {}, OptInt sizeY = {});

        /// @brief 変倍して画面コピー（OOP版）
        /// @param destW 画面にコピーする時の大きさX
        /// @param destH 画面にコピーする時の大きさY
        /// @param srcId コピー元のウィンドウID
        /// @param srcX コピー元の左上X座標
        /// @param srcY コピー元の左上Y座標
        /// @param srcW コピーする大きさX (省略時=gmodeサイズ)
        /// @param srcH コピーする大きさY (省略時=gmodeサイズ)
        /// @param mode ズームのモード (0=高速, 1=高品質ハーフトーン)
        /// @return *this（メソッドチェーン用）
        Screen& gzoom(int destW, int destH, int srcId, int srcX, int srcY, OptInt srcW = {}, OptInt srcH = {}, int mode = 0);

        /// @brief 矩形画像を回転してコピー（OOP版）
        /// @param srcId コピー元のウィンドウID
        /// @param srcX コピー元の左上X座標
        /// @param srcY コピー元の左上Y座標
        /// @param angle 回転角度（ラジアン）
        /// @param dstW コピー先のXサイズ (省略時=gmodeで設定したサイズ)
        /// @param dstH コピー先のYサイズ (省略時=gmodeで設定したサイズ)
        /// @return *this（メソッドチェーン用）
        Screen& grotate(int srcId, int srcX, int srcY, double angle, OptInt dstW = {}, OptInt dstH = {});

        /// @brief 任意の四角形を単色塗りつぶし（OOP版）
        /// @param srcId ウィンドウID (マイナス値=-1～-256で単色塗りつぶし)
        /// @param dst コピー先座標（4頂点）
        /// @return *this（メソッドチェーン用）
        Screen& gsquare(int srcId, const Quad& dst);

        /// @brief 任意の四角形へ画像をコピー（OOP版）
        /// @param srcId コピー元のウィンドウID (0以上)
        /// @param dst コピー先座標（4頂点）
        /// @param src コピー元座標（4頂点）
        /// @return *this（メソッドチェーン用）
        Screen& gsquare(int srcId, const Quad& dst, const QuadUV& src);

        /// @brief 任意の四角形をグラデーション塗りつぶし（OOP版）
        /// @param srcId gsquare_grad (-257) を指定
        /// @param dst コピー先座標（4頂点）
        /// @param colors 頂点の色（4色）
        /// @return *this（メソッドチェーン用）
        Screen& gsquare(int srcId, const Quad& dst, const QuadColors& colors);

        // ============================================================
        // ウィンドウ表示制御（OOP版）
        // ============================================================

        /// @brief ウィンドウを表示（gsel id, 1 相当）
        /// @return *this（メソッドチェーン用）
        Screen& show();

        /// @brief ウィンドウを非表示（gsel id, -1 相当）
        /// @return *this（メソッドチェーン用）
        Screen& hide();

        /// @brief ウィンドウを最前面でアクティブ化（gsel id, 2 相当）
        /// @return *this（メソッドチェーン用）
        Screen& activate();

        // ============================================================
        // Cel描画（OOP版・Screen側主体）
        // ============================================================

        /// @brief 画像素材を描画（Screen側主体版）
        /// @param cel Celハンドル
        /// @param cellIndex 表示するセル番号
        /// @param x X座標（省略時は現在のpos）
        /// @param y Y座標（省略時は現在のpos）
        /// @return *this（メソッドチェーン用）
        Screen& celput(const Cel& cel, int cellIndex, OptInt x = {}, OptInt y = {});

        // ============================================================
        // GUIオブジェクト生成（OOP版・ウィンドウ指定）
        // ============================================================

        /// @brief ボタンを生成（OOP版）
        /// @param name ボタンのラベル
        /// @param callback クリック時のコールバック
        /// @param isGosub true=gosub風, false=goto風
        /// @return オブジェクトID
        int button(std::string_view name, std::function<int()> callback, bool isGosub = true);

        /// @brief 入力ボックスを生成（OOP版・shared_ptr版）
        /// @param var 文字列変数（shared_ptr）
        /// @param maxLength 最大文字数
        /// @param mode 入力モード (0=文字列, 2=数値)
        /// @return オブジェクトID
        int input(std::shared_ptr<std::string> var, int maxLength = 1024, int mode = 0);

        /// @brief 複数行入力ボックスを生成（OOP版・shared_ptr版）
        /// @param var 文字列変数（shared_ptr）
        /// @param maxLength 最大文字数
        /// @param mode 入力モード
        /// @return オブジェクトID
        int mesbox(std::shared_ptr<std::string> var, int maxLength = 4096, int mode = 0);

        /// @brief オブジェクトのサイズと間隔を設定（OOP版）
        /// @param sizeX オブジェクトの横幅
        /// @param sizeY オブジェクトの高さ
        /// @param spaceY 次のオブジェクトとの縦間隔（デフォルト0）
        /// @return *this（メソッドチェーン用）
        Screen& objsize(int sizeX, int sizeY = 24, int spaceY = 0);

        /// @brief マウスカーソルの座標を設定（OOP版）
        /// @param x X座標（クライアント座標）
        /// @param y Y座標（クライアント座標）
        /// @return *this（メソッドチェーン用）
        Screen& mouse(int x, int y);

        // ============================================================
        // 追加GUIオブジェクト生成（OOP版）
        // ============================================================

        /// @brief チェックボックスを生成（OOP版・shared_ptr版）
        /// @param label チェックボックスの内容表示文字列
        /// @param var チェックボックスの状態を保持する変数（shared_ptr）
        /// @return オブジェクトID
        int chkbox(std::string_view label, std::shared_ptr<int> var);

        /// @brief コンボボックスを生成（OOP版・shared_ptr版）
        /// @param var コンボボックスの状態を保持する変数（shared_ptr）
        /// @param expandY 拡張Yサイズ（リスト表示用）
        /// @param items コンボボックスの内容（\n区切りの文字列）
        /// @return オブジェクトID
        int combox(std::shared_ptr<int> var, int expandY, std::string_view items);

        /// @brief リストボックスを生成（OOP版・shared_ptr版）
        /// @param var リストボックスの状態を保持する変数（shared_ptr）
        /// @param expandY 拡張Yサイズ
        /// @param items リストボックスの内容（\n区切りの文字列）
        /// @return オブジェクトID
        int listbox(std::shared_ptr<int> var, int expandY, std::string_view items);

        // ============================================================
        // GUIオブジェクト設定（OOP版）
        // ============================================================

        /// @brief オブジェクトモード設定（OOP版）
        /// @param mode フォント設定モード (objmode_* 定数)
        /// @param tabMove TABキーフォーカス移動 (0=無効, 1=有効、省略時は変更なし)
        /// @return *this（メソッドチェーン用）
        Screen& objmode(int mode, int tabMove = -1);

        /// @brief オブジェクトのカラー設定（OOP版）
        /// @param r 赤輝度 (0〜255)
        /// @param g 緑輝度 (0〜255)
        /// @param b 青輝度 (0〜255)
        /// @return *this（メソッドチェーン用）
        Screen& objcolor(int r, int g, int b);

        // ============================================================
        // GUIオブジェクト操作（OOP版）
        // ============================================================

        /// @brief オブジェクトの内容を変更（OOP版・文字列）
        /// @param objectId オブジェクトID
        /// @param value 変更するパラメータの内容（文字列）
        /// @return *this（メソッドチェーン用）
        Screen& objprm(int objectId, std::string_view value);

        /// @brief オブジェクトの内容を変更（OOP版・整数）
        /// @param objectId オブジェクトID
        /// @param value 変更するパラメータの内容（数値）
        /// @return *this（メソッドチェーン用）
        Screen& objprm(int objectId, int value);

        /// @brief オブジェクトの有効・無効を設定（OOP版）
        /// @param objectId オブジェクトID
        /// @param enable 0=無効, それ以外=有効（デフォルト1）
        /// @return *this（メソッドチェーン用）
        Screen& objenable(int objectId, int enable = 1);

        /// @brief オブジェクトに入力フォーカスを設定（OOP版）
        /// @param objectId オブジェクトID
        /// @return *this（メソッドチェーン用）
        Screen& objsel(int objectId);
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
        std::string_view title = "HSPPP Window",
        const std::source_location& location = std::source_location::current()
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
        OptInt mode   = {},
        const std::source_location& location = std::source_location::current()
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
        OptInt client_h = {},
        const std::source_location& location = std::source_location::current()
    );

    // ============================================================
    // gsel - 描画先指定、ウィンドウ最前面、非表示設定（HSP互換）
    // ============================================================
    /// @brief 描画先を指定したウィンドウIDに変更
    /// @param id ウィンドウID
    /// @param mode ウィンドウアクティブスイッチ (-1=非表示, 0=影響なし, 1=アクティブ, 2=アクティブ+最前面)
    export void gsel(OptInt id = {}, OptInt mode = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // gmode - 画面コピーモード設定（HSP互換）
    // ============================================================
    /// @brief 画面コピーモードを設定
    /// @param mode 画面コピーモード (0～6)
    /// @param size_x コピーする大きさX (デフォルト: 32)
    /// @param size_y コピーする大きさY (デフォルト: 32)
    /// @param blend_rate 半透明合成時のブレンド率 (0～256)
    export void gmode(OptInt mode = {}, OptInt size_x = {}, OptInt size_y = {}, OptInt blend_rate = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // gcopy - 画面コピー（HSP互換）
    // ============================================================
    /// @brief 指定したウィンドウIDから現在の描画先にコピー
    /// @param src_id コピー元のウィンドウID
    /// @param src_x コピー元の左上X座標
    /// @param src_y コピー元の左上Y座標
    /// @param size_x コピーする大きさX (省略時=gmodeで設定したサイズ)
    /// @param size_y コピーする大きさY (省略時=gmodeで設定したサイズ)
    export void gcopy(OptInt src_id = {}, OptInt src_x = {}, OptInt src_y = {}, OptInt size_x = {}, OptInt size_y = {}, const std::source_location& location = std::source_location::current());

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
    export void gzoom(OptInt dest_w = {}, OptInt dest_h = {}, OptInt src_id = {}, OptInt src_x = {}, OptInt src_y = {}, OptInt src_w = {}, OptInt src_h = {}, OptInt mode = {}, const std::source_location& location = std::source_location::current());

    // 描画制御
    // p1: 0=描画予約(Offscreen), 1=画面反映(Present)
    export void redraw(int p1 = 1, const std::source_location& location = std::source_location::current());

    // 待機＆メッセージ処理 (HSP互換)
    // 指定されたミリ秒だけ待機し、その間ウィンドウメッセージを処理する
    export void await(int time_ms, const std::source_location& location = std::source_location::current());

    // プログラム終了 (HSP互換)
    // p1: 終了コード（省略時は0）
    export [[noreturn]] void end(int exitcode = 0, const std::source_location& location = std::source_location::current());

    // ============================================================
    // cls - 画面クリア（HSP互換）
    // ============================================================
    /// @brief ウィンドウ内の情報をすべてクリア
    /// @param p1 クリアする時の色 (0=白, 1=明るい灰色, 2=灰色, 3=暗い灰色, 4=黒)
    /// @note cls命令で画面をクリアすると、フォントやカラー設定が初期状態に戻る
    export void cls(OptInt p1 = {}, const std::source_location& location = std::source_location::current());

    // --- Image Functions ---
    /// @brief 画像ファイルをロード
    /// @param p1 ファイル名
    /// @param p2 モード (0=初期化してロード, 1=現在の画面に重ねる, 2=黒で初期化してロード)
    export void picload(std::string_view p1, OptInt p2 = {},
                       const std::source_location& location = std::source_location::current());
    
    /// @brief 画面イメージをBMPファイルに保存
    /// @param p1 ファイル名
    export void bmpsave(std::string_view p1,
                       const std::source_location& location = std::source_location::current());
    
    /// @brief 画像ファイルをバッファにロード（仮想ID）
    /// @param p1 ファイル名
    /// @param p2 cel ID（省略時は自動割り当て）
    /// @return 割り当てられたcel ID
    export int celload(std::string_view p1, OptInt p2 = {},
                      const std::source_location& location = std::source_location::current());
    
    /// @brief 画像素材の分割サイズを設定
    /// @param p1 cel ID
    /// @param p2 横方向の分割数
    /// @param p3 縦方向の分割数
    export void celdiv(int p1, int p2, int p3,
                      const std::source_location& location = std::source_location::current());
    
    /// @brief 画像素材を描画
    /// @param p1 cel ID
    /// @param p2 表示するセル番号
    /// @param p3 X座標（省略時は現在のpos）
    /// @param p4 Y座標（省略時は現在のpos）
    export void celput(int p1, int p2, OptInt p3 = {}, OptInt p4 = {},
                      const std::source_location& location = std::source_location::current());

    // --- Drawing Functions ---
    export void color(int r, int g, int b,
                     const std::source_location& location = std::source_location::current());
    export void pos(int x, int y,
                   const std::source_location& location = std::source_location::current());
    export void mes(std::string_view text, OptInt sw = {},
                   const std::source_location& location = std::source_location::current());
    export std::pair<int, int> messize(std::string_view text,
                   const std::source_location& location = std::source_location::current());
    export void boxf(int x1, int y1, int x2, int y2,
                    const std::source_location& location = std::source_location::current());
    // 引数なし版 boxf() -> 画面全体
    export void boxf(const std::source_location& location = std::source_location::current());

    // ============================================================
    // line - 直線を描画（HSP互換）
    // ============================================================
    /// @brief 直線を描画
    /// @param x2 ラインの終点X座標 (デフォルト: 0)
    /// @param y2 ラインの終点Y座標 (デフォルト: 0)
    /// @param x1 ラインの始点X座標 (省略時=カレントポジション)
    /// @param y1 ラインの始点Y座標 (省略時=カレントポジション)
    /// @note 実行後、(x2, y2)がカレントポジションになる
    export void line(OptInt x2 = {}, OptInt y2 = {}, OptInt x1 = {}, OptInt y1 = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // circle - 円を描画（HSP互換）
    // ============================================================
    /// @brief 円を描画
    /// @param x1 矩形の左上X座標 (デフォルト: 0)
    /// @param y1 矩形の左上Y座標 (デフォルト: 0)
    /// @param x2 矩形の右下X座標
    /// @param y2 矩形の右下Y座標
    /// @param fillMode 描画モード (0=線, 1=塗りつぶし, デフォルト: 1)
    export void circle(OptInt x1 = {}, OptInt y1 = {}, OptInt x2 = {}, OptInt y2 = {}, OptInt fillMode = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // pset - 1ドットの点を描画（HSP互換）
    // ============================================================
    /// @brief 1ドットの点を描画
    /// @param x 画面上のX座標 (省略時=カレントポジション)
    /// @param y 画面上のY座標 (省略時=カレントポジション)
    export void pset(OptInt x = {}, OptInt y = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // pget - 1ドットの色を取得（HSP互換）
    // ============================================================
    /// @brief 1ドットの色を取得し、選択色として設定
    /// @param x 画面上のX座標 (省略時=カレントポジション)
    /// @param y 画面上のY座標 (省略時=カレントポジション)
    /// @note 取得した色はginfo(16/17/18)またはginfo_r/g/bで参照可能
    export void pget(OptInt x = {}, OptInt y = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // gradf - 矩形をグラデーションで塗りつぶす（HSP互換）
    // ============================================================
    /// @brief 矩形をグラデーションで塗りつぶす
    /// @param x 矩形の左上X座標 (省略時=0)
    /// @param y 矩形の左上Y座標 (省略時=0)
    /// @param w 矩形のXサイズ (省略時=画面サイズ)
    /// @param h 矩形のYサイズ (省略時=画面サイズ)
    /// @param mode グラデーションのモード (0=横方向, 1=縦方向)
    /// @param color1 塗りつぶし色1 (RGBカラーコード $rrggbb形式)
    /// @param color2 塗りつぶし色2 (RGBカラーコード $rrggbb形式)
    export void gradf(OptInt x = {}, OptInt y = {}, OptInt w = {}, OptInt h = {}, OptInt mode = {}, OptInt color1 = {}, OptInt color2 = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // grect - 回転する矩形で塗りつぶす（HSP互換）
    // ============================================================
    /// @brief 回転する矩形で塗りつぶす
    /// @param cx 矩形の中心X座標
    /// @param cy 矩形の中心Y座標
    /// @param angle 回転角度（ラジアン）
    /// @param w Xサイズ (省略時=gmodeで設定したサイズ)
    /// @param h Yサイズ (省略時=gmodeで設定したサイズ)
    export void grect(OptInt cx = {}, OptInt cy = {}, OptDouble angle = {}, OptInt w = {}, OptInt h = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // grotate - 矩形画像を回転してコピー（HSP互換）
    // ============================================================
    /// @brief 矩形画像を回転してコピー
    /// @param srcId コピー元のウィンドウID
    /// @param srcX コピー元の左上X座標
    /// @param srcY コピー元の左上Y座標
    /// @param angle 回転角度（ラジアン）
    /// @param dstW コピー先のXサイズ (省略時=gmodeで設定したサイズ)
    /// @param dstH コピー先のYサイズ (省略時=gmodeで設定したサイズ)
    export void grotate(OptInt srcId = {}, OptInt srcX = {}, OptInt srcY = {}, OptDouble angle = {}, OptInt dstW = {}, OptInt dstH = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // gsquare - 任意の四角形を描画（HSP互換）
    // ============================================================
    // 使用例:
    //   Quad dst = {{0, 0}, {100, 0}, {100, 100}, {0, 100}};
    //   gsquare(-1, dst);  // 単色塗りつぶし
    //
    //   QuadUV src = {{0, 0}, {32, 0}, {32, 32}, {0, 32}};
    //   gsquare(0, dst, src);  // 画像コピー
    //
    //   QuadColors colors = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00};
    //   gsquare(gsquare_grad, dst, colors);  // グラデーション
    // ============================================================

    /// @brief 任意の四角形を単色塗りつぶし
    /// @param srcId ウィンドウID (マイナス値=-1～-256で単色塗りつぶし)
    /// @param dst コピー先座標（4頂点: 左上, 右上, 右下, 左下）
    export void gsquare(int srcId, const Quad& dst, const std::source_location& location = std::source_location::current());

    /// @brief 任意の四角形へ画像をコピー
    /// @param srcId コピー元のウィンドウID (0以上)
    /// @param dst コピー先座標（4頂点）
    /// @param src コピー元座標（4頂点）
    export void gsquare(int srcId, const Quad& dst, const QuadUV& src, const std::source_location& location = std::source_location::current());

    /// @brief 任意の四角形をグラデーション塗りつぶし
    /// @param srcId gsquare_grad (-257) を指定
    /// @param dst コピー先座標（4頂点）
    /// @param colors 頂点の色（4色、RGBカラーコード 0xRRGGBB形式）
    export void gsquare(int srcId, const Quad& dst, const QuadColors& colors, const std::source_location& location = std::source_location::current());

    // gsquare用定数
    export inline constexpr int gsquare_grad = -257;

    // ============================================================
    // print - メッセージ表示（HSP互換・mes別名）
    // ============================================================
    /// @brief メッセージを表示（mes命令の別名）
    /// @param text 表示するメッセージ
    /// @param sw オプション (1=改行しない, 2=影, 4=縁取り, 8=簡易描画, 16=gmode設定)
    export void print(std::string_view text, OptInt sw = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // gettime - 時間・日付を取得（HSP互換）
    // ============================================================
    /// @brief 時間・日付を取得する
    /// @param type 取得するタイプ (0=年, 1=月, 2=曜日, 3=日, 4=時, 5=分, 6=秒, 7=ミリ秒)
    /// @return 指定したタイプの時間・日付情報
    export int gettime(int type, const std::source_location& location = std::source_location::current());

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
    export int ginfo(int type, const std::source_location& location = std::source_location::current());

    // ginfo_* マクロ/システム変数互換（C++では関数として提供）
    export int ginfo_mx(const std::source_location& location = std::source_location::current());
    export int ginfo_my(const std::source_location& location = std::source_location::current());
    export int ginfo_act(const std::source_location& location = std::source_location::current());
    export int ginfo_sel(const std::source_location& location = std::source_location::current());
    export int ginfo_wx1(const std::source_location& location = std::source_location::current());
    export int ginfo_wy1(const std::source_location& location = std::source_location::current());
    export int ginfo_wx2(const std::source_location& location = std::source_location::current());
    export int ginfo_wy2(const std::source_location& location = std::source_location::current());
    export int ginfo_vx(const std::source_location& location = std::source_location::current());
    export int ginfo_vy(const std::source_location& location = std::source_location::current());
    export int ginfo_sizex(const std::source_location& location = std::source_location::current());
    export int ginfo_sizey(const std::source_location& location = std::source_location::current());
    export int ginfo_mesx(const std::source_location& location = std::source_location::current());
    export int ginfo_mesy(const std::source_location& location = std::source_location::current());
    export int ginfo_messizex(const std::source_location& location = std::source_location::current());
    export int ginfo_messizey(const std::source_location& location = std::source_location::current());
    export int ginfo_paluse(const std::source_location& location = std::source_location::current());
    export int ginfo_dispx(const std::source_location& location = std::source_location::current());
    export int ginfo_dispy(const std::source_location& location = std::source_location::current());
    export int ginfo_cx(const std::source_location& location = std::source_location::current());
    export int ginfo_cy(const std::source_location& location = std::source_location::current());
    export int ginfo_intid(const std::source_location& location = std::source_location::current());
    export int ginfo_newid(const std::source_location& location = std::source_location::current());
    export int ginfo_sx(const std::source_location& location = std::source_location::current());
    export int ginfo_sy(const std::source_location& location = std::source_location::current());

    /// @brief 現在設定されている色コード(R)を取得
    export int ginfo_r(const std::source_location& location = std::source_location::current());

    /// @brief 現在設定されている色コード(G)を取得
    export int ginfo_g(const std::source_location& location = std::source_location::current());

    /// @brief 現在設定されている色コード(B)を取得
    export int ginfo_b(const std::source_location& location = std::source_location::current());

    // ============================================================
    // font - フォント設定（HSP互換）
    // ============================================================
    /// @brief フォントを設定
    /// @param fontName フォント名 ("MS Gothic", "MS 明朝" など)
    /// @param size フォントサイズ (デフォルト: 12)
    /// @param style フォントスタイル (1=太字, 2=イタリック, 4=下線, 8=打消し線, 16=アンチエイリアス)
    /// @param decorationWidth フォント修飾の幅 (デフォルト: 1)
    /// @return 結果 (0=成功, -1=失敗)
    export int font(std::string_view fontName, OptInt size = {}, OptInt style = {}, OptInt decorationWidth = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // sysfont - システムフォント選択（HSP互換）
    // ============================================================
    /// @brief システム標準のフォントを選択
    /// @param type フォント種類指定
    ///   0: HSP標準システムフォント
    ///   10: OEM文字セットの固定幅フォント
    ///   11: Windows文字セットの固定幅システムフォント
    ///   12: Windows文字セットの可変幅システムフォント
    ///   13: 標準システムフォント
    ///   17: デフォルトGUIフォント
    export void sysfont(OptInt type = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // title - タイトルバー設定（HSP互換）
    // ============================================================
    /// @brief ウィンドウのタイトルバーを設定
    /// @param str タイトル文字列
    export void title(std::string_view str, const std::source_location& location = std::source_location::current());

    // ============================================================
    // width - ウィンドウサイズ設定（HSP互換）
    // ============================================================
    /// @brief ウィンドウサイズと位置を設定
    /// @param clientW クライアントサイズX (-1=変更なし)
    /// @param clientH クライアントサイズY (-1=変更なし)
    /// @param posX ウィンドウ位置X (-1=変更なし)
    /// @param posY ウィンドウ位置Y (-1=変更なし)
    /// @param option 座標設定オプション (0=負値で現在維持, 1=負値も設定)
    export void width(OptInt clientW = {}, OptInt clientH = {}, OptInt posX = {}, OptInt posY = {}, OptInt option = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // groll - スクロール位置設定（HSP互換）
    // ============================================================
    /// @brief グラフィック面の描画基点座標を設定
    /// @param scrollX 描画基点X座標
    /// @param scrollY 描画基点Y座標
    /// @note クライアントサイズが画面サイズより小さい場合に、どの部分を表示するかを設定
    export void groll(int scrollX, int scrollY, const std::source_location& location = std::source_location::current());

    // ============================================================
    // stick - キー入力情報取得（HSP互換）
    // ============================================================
    /// @brief キー入力情報を取得
    /// @param nonTrigger 非トリガータイプキー指定（押しっぱなしでも検出するキー）
    /// @param checkActive ウィンドウアクティブチェック (0=常に取得, 1=アクティブ時のみ)
    /// @return キー入力状態のビットフラグ
    /// @note 返り値のビット:
    ///   1=←, 2=↑, 4=→, 8=↓, 16=スペース, 32=Enter, 64=Ctrl, 128=ESC,
    ///   256=左クリック, 512=右クリック, 1024=TAB, 2048=Z, 4096=X, 8192=C,
    ///   16384=A, 32768=W, 65536=D, 131072=S
    export int stick(OptInt nonTrigger = {}, OptInt checkActive = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // getkey - キー入力チェック（HSP互換）
    // ============================================================
    /// @brief 指定したキーが押されているかチェック
    /// @param keycode 仮想キーコード (VK_LEFT=37, VK_UP=38, etc.)
    /// @return 押されていれば1、押されていなければ0
    export int getkey(int keycode, const std::source_location& location = std::source_location::current());

    // ============================================================
    // mouse - マウスカーソル座標設定（HSP互換）
    // ============================================================
    /// @brief マウスカーソルの座標を設定
    /// @param x X座標 (ディスプレイ座標)
    /// @param y Y座標 (ディスプレイ座標)
    /// @param mode 設定モード (0=負値で非表示, -1=移動+非表示, 1=移動のみ, 2=移動+表示)
    export void mouse(OptInt x = {}, OptInt y = {}, OptInt mode = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // mousex - マウスカーソルのX座標（HSP互換）
    // ============================================================
    /// @brief マウスカーソルのX座標を取得
    /// @return 現在のウィンドウ内でのX座標
    export int mousex(const std::source_location& location = std::source_location::current());

    // ============================================================
    // mousey - マウスカーソルのY座標（HSP互換）
    // ============================================================
    /// @brief マウスカーソルのY座標を取得
    /// @return 現在のウィンドウ内でのY座標
    export int mousey(const std::source_location& location = std::source_location::current());

    // ============================================================
    // mousew - マウスカーソルのホイール値（HSP互換）
    // ============================================================
    /// @brief マウスホイールの移動量を取得
    /// @return ホイール移動量
    export int mousew(const std::source_location& location = std::source_location::current());

    // ============================================================
    // wait - 実行を一定時間中断する（HSP互換）
    // ============================================================
    /// @brief 指定時間だけ実行を中断する
    /// @param time 待ち時間 (10ms単位、デフォルト: 100=1秒)
    /// @note awaitよりCPU負荷が軽い
    export void wait(OptInt time = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // stop - プログラム実行を一時停止（HSP互換）
    // ============================================================
    /// @brief プログラムを一時停止し、割り込みを待機
    /// @note 割り込みイベントによりジャンプするまで停止
    export void stop(const std::source_location& location = std::source_location::current());

    // ============================================================
    // マルチメディア制御命令（HSP互換）
    // ============================================================
    //
    // 実装方針:
    //   - SE（WAV/短尺音声）: XAudio2（低遅延、多重再生）
    //   - BGM/動画（MP3, MP4, 長尺）: Media Foundation（ストリーミング）
    //   - ファイル種類・サイズに応じて内部で自動振り分け
    //
    // MCIは非推奨のため、mci命令は提供しない。
    // ============================================================

    /// @brief メディアファイル読み込み
    /// @param filename ファイル名
    /// @param bufferId 割り当てるメディアバッファID (0〜)
    /// @param mode 再生モード (0=通常, 1=ループ, 2=終了まで待機, +16=動画をウィンドウ全体で再生)
    /// @return 成功時0、失敗時非0
    /// @note WAV(2MB以下)はメモリにロード、それ以外はストリーミング準備
    export int mmload(std::string_view filename, OptInt bufferId = {}, OptInt mode = {},
                      const std::source_location& location = std::source_location::current());

    /// @brief メディア再生
    /// @param bufferId 再生するメディアバッファID
    /// @return 成功時0、失敗時非0
    export int mmplay(OptInt bufferId = {},
                      const std::source_location& location = std::source_location::current());

    /// @brief メディア再生の停止
    /// @param bufferId メディアバッファID (-1または省略で全停止)
    export void mmstop(OptInt bufferId = {},
                      const std::source_location& location = std::source_location::current());

    /// @brief 音量の設定
    /// @param bufferId メディアバッファID
    /// @param vol ボリューム値 (-1000=無音 〜 0=最大)
    /// @note リニアな聴感変化 (dmmvolとは異なる)
    export void mmvol(int bufferId, int vol,
                     const std::source_location& location = std::source_location::current());

    /// @brief パンニングの設定
    /// @param bufferId メディアバッファID
    /// @param pan パンニング値 (-1000=左 〜 0=中央 〜 1000=右)
    /// @note WAVファイルでのみ有効
    export void mmpan(int bufferId, int pan,
                     const std::source_location& location = std::source_location::current());

    /// @brief メディアの状態取得
    /// @param bufferId メディアバッファID
    /// @param mode 取得モード (0=フラグ, 1=ボリューム, 2=パン, 16=再生中フラグ)
    /// @return 指定した状態値
    export int mmstat(int bufferId, OptInt mode = {},
                     const std::source_location& location = std::source_location::current());

    // ============================================================
    // Media クラス（OOP版メディア管理）
    // ============================================================
    // 使用例:
    //   Media bgm("resources/music.mp3");
    //   bgm.vol(-500).pan(0).loop(true);
    //   bgm.play();
    //   bgm.stop();

    export class Media {
    public:
        Media();
        explicit Media(std::string_view filename);
        ~Media();

        Media(const Media&) = delete;
        Media& operator=(const Media&) = delete;
        Media(Media&& other) noexcept;
        Media& operator=(Media&& other) noexcept;

        /// @brief メディアファイルをロード
        bool load(std::string_view filename);
        
        /// @brief メディアをアンロード
        void unload();

        /// @brief 再生開始
        bool play();
        
        /// @brief 停止
        void stop();

        /// @brief 音量設定（メソッドチェーン対応）
        /// @param v -1000（無音）〜 0（最大）
        Media& vol(int v);
        
        /// @brief パン設定（メソッドチェーン対応）
        /// @param p -1000（左）〜 0（中央）〜 1000（右）
        Media& pan(int p);
        
        /// @brief ループ設定
        Media& loop(bool l);
        
        /// @brief 再生モード設定 (0=通常, 1=ループ, 2=終了まで待機)
        Media& mode(int m);

        /// @brief 動画再生先Screenを指定
        /// @param screenId 描画先のScreen ID（screen/bgscr で作成したウィンドウ）
        Media& target(int screenId);

        /// @brief 現在の音量取得
        [[nodiscard]] int get_vol() const;
        
        /// @brief 現在のパン取得
        [[nodiscard]] int get_pan() const;
        
        /// @brief ループ設定取得
        [[nodiscard]] bool get_loop() const;
        
        /// @brief 再生モード取得
        [[nodiscard]] int get_mode() const;
        
        /// @brief 状態取得（mmstat相当）
        [[nodiscard]] int stat() const;
        
        /// @brief 再生中か
        [[nodiscard]] bool playing() const;
        
        /// @brief ロード済みか
        [[nodiscard]] bool loaded() const;
        
        /// @brief ファイル名取得
        [[nodiscard]] const std::string& filename() const;
        
        /// @brief 内部バッファID取得（mm系との互換用）
        [[nodiscard]] int id() const;

    private:
        class Impl;
        std::unique_ptr<Impl> m_impl;
    };

    // ============================================================
    // 割り込み情報構造体
    // ============================================================

    /// @brief 割り込み発生時のパラメータ（システム変数相当）
    /// @note onclick, onkey, onexit, oncmd で使用
    export struct InterruptParams {
        int iparam;     ///< 割り込み要因パラメータ
        int wparam;     ///< Windows wParam
        int lparam;     ///< Windows lParam
    };

    /// @brief 現在の割り込みパラメータを取得
    export const InterruptParams& getInterruptParams();

    /// @brief システム変数 iparam を取得
    export int iparam(const std::source_location& location = std::source_location::current());

    /// @brief システム変数 wparam を取得
    export int wparam(const std::source_location& location = std::source_location::current());

    /// @brief システム変数 lparam を取得
    export int lparam(const std::source_location& location = std::source_location::current());

    // ============================================================
    // sysval互換（Windowsハンドル系）
    // ============================================================

    /// @brief 現在選択されているウィンドウのハンドル（sysval hwnd）
    export int64_t hwnd(const std::source_location& location = std::source_location::current());

    /// @brief 現在のデバイスコンテキスト（sysval hdc）
    /// @note Direct2D描画のためGDIのHDCは基本的に提供できない。現状は0を返す。
    export int64_t hdc(const std::source_location& location = std::source_location::current());

    /// @brief 現在のインスタンスハンドル（sysval hinstance）
    export int64_t hinstance(const std::source_location& location = std::source_location::current());

    // ============================================================
    // sendmsg - ウィンドウメッセージの送信（HSP互換）
    // ============================================================

    /// @brief 指定したウィンドウにメッセージを送信
    /// @param hwndValue 送信先ウィンドウハンドル
    /// @param msg メッセージID
    /// @param wparam wParam
    /// @param lparam lParam
    /// @return SendMessageの戻り値（LRESULT）
    export int64_t sendmsg(int64_t hwndValue, int msg, int64_t wparam = 0, int64_t lparam = 0, const std::source_location& location = std::source_location::current());

    /// @brief lParamにUTF-8文字列ポインタを渡す簡易版（内部でUTF-16へ変換しSendMessageWを呼び出す）
    export int64_t sendmsg(int64_t hwndValue, int msg, int64_t wparam, std::string_view lparamText, const std::source_location& location = std::source_location::current());

    // ============================================================
    // 文字列操作（HSP拡張）
    // ============================================================

    // ============================================================
    // エラーコード定義（HSP互換）
    // ============================================================

    export inline constexpr int ERR_NONE              = 0;   // エラーなし
    export inline constexpr int ERR_SYNTAX            = 1;   // 文法エラー（コンパイル時）
    export inline constexpr int ERR_ILLEGAL_FUNCTION  = 2;   // 不正な呼び出し
    export inline constexpr int ERR_LABEL_REQUIRED    = 3;   // ラベル指定が必要
    export inline constexpr int ERR_OUT_OF_MEMORY     = 4;   // メモリ不足
    export inline constexpr int ERR_TYPE_MISMATCH     = 5;   // 型が違う
    export inline constexpr int ERR_OUT_OF_ARRAY      = 6;   // 配列の要素が無効
    export inline constexpr int ERR_OUT_OF_RANGE      = 7;   // パラメータの値が異常
    export inline constexpr int ERR_DIVIDE_BY_ZERO    = 8;   // 0で除算した
    export inline constexpr int ERR_BUFFER_OVERFLOW   = 9;   // バッファオーバーフロー
    export inline constexpr int ERR_UNSUPPORTED       = 10;  // サポートされていない
    export inline constexpr int ERR_EXPRESSION        = 11;  // 式エラー（計算式）
    export inline constexpr int ERR_FILE_IO           = 12;  // ファイルI/Oエラー
    export inline constexpr int ERR_WINDOW_INIT       = 13;  // ウィンドウ初期化エラー
    export inline constexpr int ERR_INVALID_HANDLE    = 14;  // 無効なハンドル
    export inline constexpr int ERR_EXTERNAL_EXECUTE  = 15;  // 外部実行エラー
    export inline constexpr int ERR_SYSTEM_ERROR      = 16;  // システムエラー
    export inline constexpr int ERR_INTERNAL          = 17;  // 内部エラー

    // ============================================================
    // HspError - エラー例外クラス
    // ============================================================

    /// @brief HSPエラー例外
    /// @details C++例外システムと統合されたHSPエラーハンドリング
    export class HspError : public std::runtime_error {
    private:
        int m_errorCode;
        int m_lineNumber;
        std::string m_fileName;
        std::string m_functionName;
        std::string m_message;  // 元のエラーメッセージ（ユーザー向け）

    public:
        /// @brief エラー例外を構築
        /// @param errorCode エラーコード (ERR_*)
        /// @param message エラーメッセージ
        /// @param location 発生場所（std::source_locationから自動取得）
        HspError(int errorCode,
                std::string_view message,
                const std::source_location& location = std::source_location::current())
            : std::runtime_error(std::format("[HSP Error {}] {}", errorCode, message))
            , m_errorCode(errorCode)
            , m_lineNumber(static_cast<int>(location.line()))
            , m_fileName(location.file_name())
            , m_functionName(location.function_name())
            , m_message(message)
        {}

        /// @brief エラーコードを取得
        [[nodiscard]] int error_code() const noexcept { return m_errorCode; }

        /// @brief 行番号を取得
        [[nodiscard]] int line_number() const noexcept { return m_lineNumber; }

        /// @brief ファイル名を取得
        [[nodiscard]] const std::string& file_name() const noexcept { return m_fileName; }

        /// @brief 関数名を取得
        [[nodiscard]] const std::string& function_name() const noexcept { return m_functionName; }

        /// @brief 元のエラーメッセージを取得（ユーザー向け）
        [[nodiscard]] const std::string& message() const noexcept { return m_message; }
    };

    // ============================================================
    // onerror - エラー発生時にジャンプ（HSP互換）
    // ============================================================
    /// @brief エラー発生時の割り込みを設定
    /// @param handler エラーハンドラ関数/ラムダ
    /// @note ハンドラはHspErrorオブジェクトを受け取る
    /// @note ハンドラ終了後、自動的にend()が呼ばれる（HSP仕様）
    export void onerror(ErrorHandler handler, const std::source_location& location = std::source_location::current());

    /// @brief onerror割り込みの一時停止/再開
    /// @param enable 0=停止, 1=再開
    export void onerror(int enable, const std::source_location& location = std::source_location::current());

    // ============================================================
    // onclick - クリック割り込み実行指定（HSP互換）
    // ============================================================
    /// @brief マウスクリック時の割り込みを設定
    /// @param handler コールバック関数/ラムダ
    export void onclick(InterruptHandler handler, const std::source_location& location = std::source_location::current());

    /// @brief onclick割り込みの一時停止/再開
    /// @param enable 0=停止, 1=再開
    export void onclick(int enable, const std::source_location& location = std::source_location::current());

    // ============================================================
    // oncmd - Windowsメッセージ割り込み実行指定（HSP互換）
    // ============================================================
    /// @brief Windowsメッセージ受信時の割り込みを設定
    /// @param handler コールバック関数/ラムダ (nullptr で解除)
    /// @param messageId 監視するメッセージID
    export void oncmd(InterruptHandler handler, int messageId, const std::source_location& location = std::source_location::current());

    /// @brief 指定メッセージIDの割り込みの一時停止/再開
    /// @param enable 0=停止, 1=再開
    /// @param messageId メッセージID
    export void oncmd(int enable, int messageId, const std::source_location& location = std::source_location::current());

    /// @brief oncmd割り込み全体の一時停止/再開
    /// @param enable 0=停止, 1=再開
    export void oncmd(int enable, const std::source_location& location = std::source_location::current());

    // ============================================================
    // onexit - 終了時にジャンプ（HSP互換）
    // ============================================================
    /// @brief 終了ボタン押下時の割り込みを設定
    /// @param handler コールバック関数/ラムダ (nullptr で解除)
    /// @note 設定されると end() を呼ぶまで終了しなくなる
    export void onexit(InterruptHandler handler, const std::source_location& location = std::source_location::current());

    /// @brief onexit割り込みの一時停止/再開
    /// @param enable 0=停止, 1=再開
    export void onexit(int enable, const std::source_location& location = std::source_location::current());

    // ============================================================
    // onkey - キー割り込み実行指定（HSP互換）
    // ============================================================
    /// @brief キー入力時の割り込みを設定
    /// @param handler コールバック関数/ラムダ (nullptr で解除)
    export void onkey(InterruptHandler handler, const std::source_location& location = std::source_location::current());

    /// @brief onkey割り込みの一時停止/再開
    /// @param enable 0=停止, 1=再開
    export void onkey(int enable, const std::source_location& location = std::source_location::current());

    // ============================================================
    // ============================================================
    // <cmath> 再エクスポート
    // ============================================================
    // 方針: <cmath> の関数群は hsppp 名前空間に全再エクスポート。
    //       これにより、ユーザーが std::sin と hsppp::sin を併用しても
    //       名前衝突が発生しない（同一実体を指す）。
    //
    // 使用例:
    //   import hsppp;
    //   hsppp::sin(hsppp::deg2rad(45.0));  // 45度のサイン
    //   hsppp::sqrt(2.0);                  // √2
    //   hsppp::pow(2.0, 10.0);             // 2^10

    // --- 基本数学関数 ---
    export using std::abs;      // 絶対値 (int, float, double, long double)
    export using std::fabs;     // 浮動小数点絶対値
    export using std::fmod;     // 浮動小数点剰余
    export using std::remainder;// IEEE剰余
    export using std::fmax;     // 最大値
    export using std::fmin;     // 最小値
    export using std::fdim;     // 正の差
    export using std::fma;      // 融合積和

    // --- 指数・対数 ---
    export using std::exp;      // e^x
    export using std::exp2;     // 2^x
    export using std::expm1;    // e^x - 1
    export using std::log;      // 自然対数
    export using std::log10;    // 常用対数
    export using std::log2;     // 2を底とする対数
    export using std::log1p;    // log(1 + x)

    // --- 累乗・平方根 ---
    export using std::pow;      // x^y
    export using std::sqrt;     // 平方根
    export using std::cbrt;     // 立方根
    export using std::hypot;    // √(x² + y²)

    // --- 三角関数（ラジアン） ---
    export using std::sin;      // サイン
    export using std::cos;      // コサイン
    export using std::tan;      // タンジェント
    export using std::asin;     // アークサイン
    export using std::acos;     // アークコサイン
    export using std::atan;     // アークタンジェント
    export using std::atan2;    // アークタンジェント(y, x)

    // --- 双曲線関数 ---
    export using std::sinh;     // ハイパボリックサイン
    export using std::cosh;     // ハイパボリックコサイン
    export using std::tanh;     // ハイパボリックタンジェント
    export using std::asinh;    // 逆双曲線サイン
    export using std::acosh;    // 逆双曲線コサイン
    export using std::atanh;    // 逆双曲線タンジェント

    // --- 切り捨て・切り上げ・丸め ---
    export using std::ceil;     // 切り上げ
    export using std::floor;    // 切り捨て
    export using std::trunc;    // 0方向への切り捨て
    export using std::round;    // 四捨五入
    export using std::nearbyint;// 現在の丸めモードで整数化
    export using std::rint;     // 現在の丸めモードで整数化（例外通知あり）

    // --- 分解・合成 ---
    export using std::frexp;    // 仮数と指数に分解
    export using std::ldexp;    // 仮数×2^指数
    export using std::modf;     // 整数部と小数部に分解
    export using std::scalbn;   // x × FLT_RADIX^n
    export using std::ilogb;    // 指数部を整数で取得
    export using std::logb;     // 指数部を浮動小数点で取得

    // --- 符号・分類 ---
    export using std::copysign; // 符号コピー
    export using std::signbit;  // 符号ビット判定
    export using std::isnan;    // NaN判定
    export using std::isinf;    // 無限大判定
    export using std::isfinite; // 有限判定
    export using std::isnormal; // 正規化数判定
    export using std::fpclassify;// 浮動小数点分類

    // --- 特殊関数 (C++17) ---
    export using std::erf;      // 誤差関数
    export using std::erfc;     // 相補誤差関数
    export using std::tgamma;   // ガンマ関数
    export using std::lgamma;   // ガンマ関数の対数

    // ============================================================
    // HSPPP拡張: 度数法対応
    // ============================================================

    /// @brief 度数法（度）をラジアンに変換
    /// @param degrees 角度（度単位）
    /// @return ラジアン値
    export [[nodiscard]] inline constexpr double deg2rad(double degrees) noexcept {
        return degrees * std::numbers::pi_v<double> / 180.0;
    }

    /// @brief ラジアンを度数法（度）に変換
    /// @param radians 角度（ラジアン単位）
    /// @return 度数法での角度値
    export [[nodiscard]] inline constexpr double rad2deg(double radians) noexcept {
        return radians * 180.0 / std::numbers::pi_v<double>;
    }

    /// @brief 乱数を発生
    /// @param p1 乱数の範囲（1〜32768）
    /// @return 0から(p1-1)までの乱数
    export [[nodiscard]] int rnd(int p1);

    /// @brief 乱数発生の初期化
    /// @param p1 乱数シード（省略時は時刻ベース）
    export void randomize(OptInt p1 = {}, const std::source_location& location = std::source_location::current());

    /// @brief 一定範囲内の整数を返す
    /// @param p1 対象となる値
    /// @param p2 最小値（省略可）
    /// @param p3 最大値（省略可）
    /// @return p2〜p3の範囲内に収まる整数
    export [[nodiscard]] int limit(int p1, OptInt p2 = {}, OptInt p3 = {});

    /// @brief 一定範囲内の実数を返す
    /// @param p1 対象となる値
    /// @param p2 最小値（省略可）
    /// @param p3 最大値（省略可）
    /// @return p2〜p3の範囲内に収まる実数
    export [[nodiscard]] double limitf(double p1, OptDouble p2 = {}, OptDouble p3 = {});

    // ============================================================
    // イージング関数（HSP互換）
    // ============================================================

    // イージング計算式のタイプ定数
    export inline constexpr int ease_linear         = 0;    ///< リニア（直線補間）
    export inline constexpr int ease_quad_in        = 1;    ///< 加速（Quadratic）
    export inline constexpr int ease_quad_out       = 2;    ///< 減速（Quadratic）
    export inline constexpr int ease_quad_inout     = 3;    ///< 加速→減速（Quadratic）
    export inline constexpr int ease_cubic_in       = 4;    ///< 加速（Cubic）
    export inline constexpr int ease_cubic_out      = 5;    ///< 減速（Cubic）
    export inline constexpr int ease_cubic_inout    = 6;    ///< 加速→減速（Cubic）
    export inline constexpr int ease_quartic_in     = 7;    ///< 加速（Quartic）
    export inline constexpr int ease_quartic_out    = 8;    ///< 減速（Quartic）
    export inline constexpr int ease_quartic_inout  = 9;    ///< 加速→減速（Quartic）
    export inline constexpr int ease_bounce_in      = 10;   ///< バウンス効果（入）
    export inline constexpr int ease_bounce_out     = 11;   ///< バウンス効果（出）
    export inline constexpr int ease_bounce_inout   = 12;   ///< バウンス効果（入出）
    export inline constexpr int ease_shake_in       = 13;   ///< シェイク効果（入）
    export inline constexpr int ease_shake_out      = 14;   ///< シェイク効果（出）
    export inline constexpr int ease_shake_inout    = 15;   ///< シェイク効果（入出）
    export inline constexpr int ease_loop           = 0x100; ///< 補間のループ（他タイプと加算可能）

    /// @brief イージング関数の計算式を設定
    /// @param p1 出力される最小値（実数値）
    /// @param p2 出力される最大値（実数値）
    /// @param p3 計算式のタイプ値（ease_*定数、省略時は前回値維持）
    export void setease(double p1, double p2, OptInt p3 = {}, const std::source_location& location = std::source_location::current());

    /// @brief イージング値を整数で取得
    /// @param p1 時間経過値（0〜最大値）
    /// @param p2 最大値（省略時は4096）
    /// @return イージング計算結果（整数）
    export [[nodiscard]] int getease(int p1, OptInt p2 = {}, const std::source_location& location = std::source_location::current());

    /// @brief イージング値を実数で取得
    /// @param p1 時間経過値（0.0〜最大値）
    /// @param p2 最大値（省略時は1.0）
    /// @return イージング計算結果（実数）
    export [[nodiscard]] double geteasef(double p1, OptDouble p2 = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // ソート関数（HSP互換）
    // ============================================================

    /// @brief 配列変数を数値でソート
    /// @param arr ソート対象の整数配列（参照で直接変更）
    /// @param order 並び順（0=小さい順、1=大きい順）
    export void sortval(std::vector<int>& arr, OptInt order = {}, const std::source_location& location = std::source_location::current());

    /// @brief 配列変数を数値でソート（double版）
    /// @param arr ソート対象の実数配列（参照で直接変更）
    /// @param order 並び順（0=小さい順、1=大きい順）
    export void sortval(std::vector<double>& arr, OptInt order = {}, const std::source_location& location = std::source_location::current());

    /// @brief 配列変数を文字列でソート
    /// @param arr ソート対象の文字列配列（参照で直接変更）
    /// @param order 並び順（0=小さい順、1=大きい順）
    export void sortstr(std::vector<std::string>& arr, OptInt order = {}, const std::source_location& location = std::source_location::current());

    /// @brief メモリノート文字列をソート
    /// @param note ソート対象のメモリノート形式文字列（参照で直接変更）
    /// @param order 並び順（0=小さい順、1=大きい順）
    export void sortnote(std::string& note, OptInt order = {}, const std::source_location& location = std::source_location::current());

    /// @brief ソート元のインデックスを取得
    /// @param index 取得するインデックス番号
    /// @return ソート前のインデックス値
    export [[nodiscard]] int sortget(int index, const std::source_location& location = std::source_location::current());

    // ============================================================
    // デバッグ命令（HSP互換）
    // ============================================================

    /// @brief デバッグメッセージ送信
    /// @param message ログに記録するメッセージ
    /// @note Visual Studio Output に出力
    export void logmes(std::string_view message, const std::source_location& location = std::source_location::current());

    /// @brief デバッグメッセージ送信（int版）
    /// @param value ログに記録する数値
    export void logmes(int value, const std::source_location& location = std::source_location::current());

    /// @brief デバッグメッセージ送信（double版）
    /// @param value ログに記録する数値
    export void logmes(double value, const std::source_location& location = std::source_location::current());

    // ============================================================
    // 型変換関数（HSP互換）
    // ============================================================

    /// @brief 整数値に変換
    /// @param p1 変換元の値
    /// @return 整数値
    export [[nodiscard]] int toInt(double p1);
    export [[nodiscard]] int toInt(const std::string& p1);

    /// @brief 実数値に変換
    /// @param p1 変換元の値
    /// @return 実数値
    export [[nodiscard]] double toDouble(int p1);
    export [[nodiscard]] double toDouble(const std::string& p1);

    /// @brief 文字列に変換
    /// @param p1 変換元の値
    /// @return 文字列
    export [[nodiscard]] std::string str(double value, const std::source_location& location = std::source_location::current());
    export [[nodiscard]] std::string str(int value, const std::source_location& location = std::source_location::current());
    export [[nodiscard]] std::string str(int64_t value, const std::source_location& location = std::source_location::current());

    /// @brief 文字列の長さを調べる
    /// @param p1 文字列
    /// @return 文字列の長さ（バイト数）
    export [[nodiscard]] int64_t strlen(const std::string& p1);

    // ============================================================
    // 文字列操作関数（HSP互換）
    // ============================================================

    /// @brief 文字列の検索をする
    /// @param p1 検索される文字列が格納されている文字列型変数名
    /// @param p2 検索を始めるインデックス (デフォルト: 0)
    /// @param search 検索する文字列
    /// @return 見つかった場合はインデックス、見つからなかった場合は-1
    /// @note p2がマイナス値の場合は常に-1が返される
    export [[nodiscard]] int64_t instr(const std::string& p1, int64_t p2, const std::string& search);
    export [[nodiscard]] int64_t instr(const std::string& p1, const std::string& search);

    /// @brief 文字列の一部を取り出す
    /// @param p1 取り出すもとの文字列が格納されている変数
    /// @param p2 取り出し始めのインデックス (-1で右から取り出し)
    /// @param p3 取り出す文字数
    /// @return 取り出した文字列
    /// @note p2=-1の場合は右からp3文字を取り出す
    export [[nodiscard]] std::string strmid(const std::string& p1, int64_t p2, int64_t p3);

    /// @brief 指定した文字だけを取り除く
    /// @param p1 元の文字列が代入された変数
    /// @param p2 除去する位置の指定 (0=両端, 1=左端, 2=右端, 3=全て)
    /// @param p3 文字コード (デフォルト: 32=半角スペース)
    /// @return 除去後の文字列
    export [[nodiscard]] std::string strtrim(const std::string& p1, int p2 = 0, int p3 = 32);

    /// @brief 文字列の置換をする
    /// @param p1 検索される文字列が格納されている文字列型変数（参照で変更される）
    /// @param search 検索する文字列
    /// @param replace 置換する文字列
    /// @return 置換した回数
    /// @note HSPでは変数を直接書き換えるが、C++では戻り値で置換回数を返す
    export int64_t strrep(std::string& p1, const std::string& search, const std::string& replace, const std::source_location& location = std::source_location::current());

    /// @brief バッファから文字列読み出し
    /// @param dest 内容を読み出す先の変数
    /// @param src バッファを割り当てた変数
    /// @param index バッファのインデックス（バイト単位）
    /// @param delimiter 区切りキャラクタのASCIIコード（デフォルト: 0=改行まで）
    /// @param maxLen 読み出しを行う最大文字数（デフォルト: 1024）
    /// @return 読み出されたバイト数（次のインデックスまでの移動量）
    export int64_t getstr(std::string& dest, const std::string& src, int64_t index, int delimiter = 0, int64_t maxLen = 1024, const std::source_location& location = std::source_location::current());
    export int64_t getstr(std::string& dest, const std::vector<uint8_t>& src, int64_t index, int delimiter = 0, int64_t maxLen = 1024, const std::source_location& location = std::source_location::current());

    /// @brief 文字列から分割された要素を取得（HSP互換版）
    /// @param src 元の文字列が代入された変数
    /// @param delimiter 区切り用文字列
    /// @return 分割された文字列のベクター
    /// @note HSPと異なり、C++ではvectorで返す（要素数制限なし）
    export std::vector<std::string> split(const std::string& src, const std::string& delimiter, const std::source_location& location = std::source_location::current());

    /// @brief 書式付き文字列を変換（HSP互換）
    /// @param format 書式指定文字列（printf形式）
    /// @return 変換された文字列
    /// @note HSPのstrf互換。%d, %x, %f, %s等をサポート
    export [[nodiscard]] std::string strf(const std::string& format);
    export [[nodiscard]] std::string strf(const std::string& format, int arg1);
    export [[nodiscard]] std::string strf(const std::string& format, double arg1);
    export [[nodiscard]] std::string strf(const std::string& format, const std::string& arg1);
    export [[nodiscard]] std::string strf(const std::string& format, int arg1, int arg2);
    export [[nodiscard]] std::string strf(const std::string& format, int arg1, double arg2);
    export [[nodiscard]] std::string strf(const std::string& format, int arg1, const std::string& arg2);
    export [[nodiscard]] std::string strf(const std::string& format, double arg1, int arg2);
    export [[nodiscard]] std::string strf(const std::string& format, double arg1, double arg2);
    export [[nodiscard]] std::string strf(const std::string& format, int arg1, int arg2, int arg3);
    export [[nodiscard]] std::string strf(const std::string& format, int arg1, double arg2, const std::string& arg3);

    /// @brief パスの一部を取得
    /// @param p1 取り出す元の文字列（ファイルパス）
    /// @param p2 情報のタイプ指定
    ///   0: 文字列のコピー（操作なし）
    ///   1: 拡張子を除くファイル名
    ///   2: 拡張子のみ（.???）
    ///   8: ディレクトリ情報を取り除く
    ///   16: 文字列を小文字に変換
    ///   32: ディレクトリ情報のみ
    /// @return 変換された文字列
    /// @note タイプ値は合計して複数指定可能
    export [[nodiscard]] std::string getpath(const std::string& p1, int p2);

    // ============================================================
    // 文字列変換関数（HSP hsp3utf.as互換）
    // ============================================================
    // HSPPPは内部的にUTF-8を使用。外部DLLやCOMとの連携時に使用。
    // ============================================================

    /// @brief 通常文字列(UTF-8)をunicode(UTF-16)に変換
    /// @param str 変換元の文字列(UTF-8)
    /// @return UTF-16文字列（std::u16stringで返す）
    export [[nodiscard]] std::u16string cnvstow(const std::string& str, const std::source_location& location = std::source_location::current());

    /// @brief unicode(UTF-16)を通常文字列(UTF-8)に変換
    /// @param wstr 変換元のunicode文字列(UTF-16)
    /// @return UTF-8文字列
    export [[nodiscard]] std::string cnvwtos(const std::u16string& wstr, const std::source_location& location = std::source_location::current());

    /// @brief 通常文字列(UTF-8)をANSI(ShiftJIS)文字列に変換
    /// @param str 変換元の文字列(UTF-8)
    /// @return ANSI文字列（std::stringにShiftJISバイト列を格納）
    export [[nodiscard]] std::string cnvstoa(const std::string& str, const std::source_location& location = std::source_location::current());

    /// @brief ANSI(ShiftJIS)文字列を通常文字列(UTF-8)に変換
    /// @param astr 変換元のANSI文字列
    /// @return UTF-8文字列
    export [[nodiscard]] std::string cnvatos(const std::string& astr, const std::source_location& location = std::source_location::current());

    // ============================================================
    // NotePad クラス - OOP版メモリノートパッド
    // ============================================================
    // HSPのnote系命令のOOP版ラッパー。
    // グローバル状態に依存せず、インスタンス単位でバッファを管理。
    //
    // 重要: HSPのnote系命令と同じ挙動を再現。
    //       noteadd "test\ntest" は2行として追加される。
    //       内部的には std::string で改行区切りテキスト全体を保持。
    //
    // 使用例:
    //   hsppp::NotePad note;
    //   note.load("test.txt");
    //   note.add("New Line");
    //   note.save("test_updated.txt");
    //
    //   for (size_t i = 0; i < note.count(); ++i) {
    //       std::cout << note.get(i) << std::endl;
    //   }
    // ============================================================

    /// @brief OOP版メモリノートパッド
    /// @details 改行区切りのテキストを行単位で操作するクラス。
    ///          内部的には std::string で改行区切りテキスト全体を保持。
    ///          HSPのnote系命令と同じ挙動を再現。
    export class NotePad {
    private:
        std::string m_buffer;

    public:
        /// @brief デフォルトコンストラクタ（空のノートパッド）
        NotePad() = default;

        /// @brief 文字列からの構築
        /// @param text 改行区切りのテキスト
        explicit NotePad(std::string_view text);

        /// @brief std::stringからのムーブ構築
        explicit NotePad(std::string&& text) noexcept;

        // コピー・ムーブはデフォルト
        NotePad(const NotePad&) = default;
        NotePad& operator=(const NotePad&) = default;
        NotePad(NotePad&&) noexcept = default;
        NotePad& operator=(NotePad&&) noexcept = default;

        // ============================================================
        // 行の取得・設定
        // ============================================================

        /// @brief 行数を取得（notemax相当）
        /// @note 改行文字の数+1を返す（HSP互換）
        [[nodiscard]] size_t count() const noexcept;

        /// @brief 空かどうか
        [[nodiscard]] bool empty() const noexcept { return m_buffer.empty(); }

        /// @brief 総バイト数を取得（notesize相当）
        [[nodiscard]] size_t size() const noexcept { return m_buffer.size(); }

        /// @brief 指定行の内容を取得（noteget相当）
        /// @param index 行インデックス（0始まり）
        /// @return 指定行の文字列（範囲外の場合は空文字列）
        [[nodiscard]] std::string get(size_t index) const;

        // ============================================================
        // 行の追加・削除・変更
        // ============================================================

        /// @brief 行を追加（noteadd相当）
        /// @param text 追加する文字列（改行を含む場合は複数行として追加）
        /// @param index 挿入位置（省略または-1で末尾）
        /// @param overwrite 上書きモード（0=挿入, 1=上書き）
        /// @return *this（メソッドチェーン用）
        NotePad& add(std::string_view text, int index = -1, int overwrite = 0);

        /// @brief 行を削除（notedel相当）
        /// @param index 削除する行のインデックス
        /// @return *this（メソッドチェーン用）
        NotePad& del(size_t index);

        /// @brief 全行をクリア
        /// @return *this（メソッドチェーン用）
        NotePad& clear() noexcept { m_buffer.clear(); return *this; }

        // ============================================================
        // 検索
        // ============================================================

        /// @brief 文字列を検索（notefind相当）
        /// @param search 検索文字列
        /// @param mode 検索モード (0=完全一致, 1=先頭一致, 2=部分一致)
        /// @param startIndex 検索開始インデックス
        /// @return 見つかった行のインデックス（見つからない場合は-1）
        [[nodiscard]] int find(std::string_view search, int mode = 0, size_t startIndex = 0) const;

        // ============================================================
        // ファイル入出力
        // ============================================================

        /// @brief ファイルから読み込み（noteload相当）
        /// @param filename ファイル名
        /// @param maxSize 最大サイズ（省略時は制限なし）
        /// @return *this（メソッドチェーン用）
        NotePad& load(std::string_view filename, size_t maxSize = 0);

        /// @brief ファイルへ保存（notesave相当）
        /// @param filename ファイル名
        /// @return 保存に成功した場合true
        [[nodiscard]] bool save(std::string_view filename) const;

        // ============================================================
        // 変換・アクセス
        // ============================================================

        /// @brief 内部バッファへの参照を取得
        [[nodiscard]] std::string& buffer() noexcept { return m_buffer; }
        [[nodiscard]] const std::string& buffer() const noexcept { return m_buffer; }

        /// @brief 改行区切りの文字列として出力（toString互換）
        [[nodiscard]] const std::string& toString() const noexcept { return m_buffer; }

        /// @brief 明示的な文字列変換（意図しないコピーを防ぐためexplicit）
        explicit operator const std::string&() const noexcept { return m_buffer; }
    };

    // ============================================================
    // メモリノートパッド命令セット（HSP互換）
    // ============================================================

    // noteinfo用定数（HSPのマクロ互換）
    export inline constexpr int notemax  = 0;
    export inline constexpr int notesize = 1;

    // notefind用定数（HSPのマクロ互換）
    export inline constexpr int notefind_match = 0;
    export inline constexpr int notefind_first = 1;
    export inline constexpr int notefind_instr = 2;

    /// @brief 対象バッファ指定
    export void notesel(std::string& buffer, const std::source_location& location = std::source_location::current());

    /// @brief 対象バッファの復帰
    export void noteunsel(const std::source_location& location = std::source_location::current());

    /// @brief 指定行の追加・変更
    /// @param text 追加・変更をする文字列
    /// @param index 追加するインデックス（省略/-1で末尾）
    /// @param overwrite 上書きモード（0=追加, 1=上書き）
    export void noteadd(std::string_view text, OptInt index = {}, OptInt overwrite = {}, const std::source_location& location = std::source_location::current());

    /// @brief 行の削除
    export void notedel(int index, const std::source_location& location = std::source_location::current());

    /// @brief 指定行を読み込み
    export void noteget(std::string& dest, OptInt index = {}, const std::source_location& location = std::source_location::current());

    /// @brief 対象バッファ読み込み
    export void noteload(std::string_view filename, OptInt maxSize = {}, const std::source_location& location = std::source_location::current());

    /// @brief 対象バッファ保存
    export void notesave(std::string_view filename, const std::source_location& location = std::source_location::current());

    /// @brief メモリノートパッド検索
    export int notefind(std::string_view search, OptInt mode = {}, const std::source_location& location = std::source_location::current());

    /// @brief メモリノートパッド情報取得
    export int noteinfo(OptInt mode = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // 色関連関数（HSP互換）
    // ============================================================

    /// @brief HSV形式でカラーを設定する
    /// @param p1 HSV形式 H値（0〜191）
    /// @param p2 HSV形式 S値（0〜255）
    /// @param p3 HSV形式 V値（0〜255）
    export void hsvcolor(int p1, int p2, int p3, const std::source_location& location = std::source_location::current());

    /// @brief RGB形式でカラーを設定する
    /// @param p1 RGB形式 カラーコード値（$rrggbb形式）
    export void rgbcolor(int p1, const std::source_location& location = std::source_location::current());

    /// @brief システムカラーを設定する
    /// @param p1 システムカラーインデックス（0〜30）
    export void syscolor(int p1, const std::source_location& location = std::source_location::current());

    // ============================================================
    // システム情報関数（HSP互換）
    // ============================================================

    /// @brief システム情報を文字列で取得する
    /// @param type 取得するタイプ (0=OS名, 1=ユーザー名, 2=コンピュータ名)
    /// @return システム情報文字列
    export [[nodiscard]] std::string sysinfo_str(int type, const std::source_location& location = std::source_location::current());

    /// @brief システム情報を整数で取得する
    /// @param type 取得するタイプ
    ///   3: HSPが使用する言語 (0=英語/1=日本語)
    ///   16: CPUの種類（アーキテクチャコード）
    ///   17: CPUの数
    ///   33: 物理メモリ使用量(%)
    ///   34: 全体の物理メモリサイズ(MB)
    ///   35: 空き物理メモリサイズ(MB)
    ///   36: スワップファイルのトータルサイズ(MB)
    ///   37: スワップファイルの空きサイズ(MB)
    ///   38: 仮想メモリを含めた全メモリサイズ(MB)
    ///   39: 仮想メモリを含めた空きメモリサイズ(MB)
    /// @return システム情報値（int64_tで64bitメモリに対応）
    export [[nodiscard]] int64_t sysinfo_int(int type, const std::source_location& location = std::source_location::current());

    // ============================================================
    // ディレクトリ情報関数（HSP互換）
    // ============================================================

    /// @brief ディレクトリ情報を取得する
    /// @param type 取得するタイプ
    ///   0: カレントディレクトリ(dir_cur)
    ///   1: 実行ファイルがあるディレクトリ(dir_exe)
    ///   2: Windowsディレクトリ(dir_win)
    ///   3: Windowsシステムディレクトリ(dir_sys)
    ///   4: コマンドライン文字列(dir_cmdline)
    ///   5: HSPTVディレクトリ(dir_tv) ※常に空文字列
    ///   0x10000以上: CSIDL値として特殊フォルダを取得
    /// @return ディレクトリパス
    export [[nodiscard]] std::string dirinfo(int type, const std::source_location& location = std::source_location::current());

    /// @brief カレントディレクトリを取得
    export [[nodiscard]] std::string dir_cur(const std::source_location& location = std::source_location::current());

    /// @brief 実行ファイルがあるディレクトリを取得
    export [[nodiscard]] std::string dir_exe(const std::source_location& location = std::source_location::current());

    /// @brief Windowsディレクトリを取得
    export [[nodiscard]] std::string dir_win(const std::source_location& location = std::source_location::current());

    /// @brief Windowsシステムディレクトリを取得
    export [[nodiscard]] std::string dir_sys(const std::source_location& location = std::source_location::current());

    /// @brief コマンドライン文字列を取得
    export [[nodiscard]] std::string dir_cmdline(const std::source_location& location = std::source_location::current());

    /// @brief デスクトップディレクトリを取得
    export [[nodiscard]] std::string dir_desktop(const std::source_location& location = std::source_location::current());

    /// @brief マイドキュメントディレクトリを取得
    export [[nodiscard]] std::string dir_mydoc(const std::source_location& location = std::source_location::current());

    // ============================================================
    // ファイル操作命令（HSP互換）
    // ============================================================

    // --- exec実行モードフラグ ---
    export inline constexpr int exec_normal     = 0;    ///< ノーマル実行
    export inline constexpr int exec_minimized  = 2;    ///< 最小化モードで実行
    export inline constexpr int exec_shellexec  = 16;   ///< 関連付けされたアプリケーションを実行
    export inline constexpr int exec_print      = 32;   ///< ファイルを印刷する

    /// @brief Windowsのファイルを実行する
    /// @param filename 対象となるファイル名
    /// @param mode ファイル実行モード（exec_*の組み合わせ）
    /// @param command コンテキストメニューの操作名（省略可）
    /// @return 実行結果（0=成功, それ以外=エラーコード）
    export int exec(const std::string& filename, OptInt mode = {}, const std::string& command = "",
                    const std::source_location& location = std::source_location::current());

    /// @brief ディレクトリ移動
    /// @param dirname 移動先ディレクトリ名
    /// @note 失敗した場合はエラー12が発生
    export void chdir(const std::string& dirname, const std::source_location& location = std::source_location::current());

    /// @brief ディレクトリ作成
    /// @param dirname 作成するディレクトリ名
    /// @note 1階層先までしか作成できない。失敗した場合はエラー12が発生
    export void mkdir(const std::string& dirname, const std::source_location& location = std::source_location::current());

    /// @brief ファイル削除
    /// @param filename 削除するファイル名
    /// @note 失敗した場合はエラー12が発生
    export void deletefile(const std::string& filename, const std::source_location& location = std::source_location::current());

    /// @brief ファイルのコピー
    /// @param src コピー元ファイル名
    /// @param dest コピー先ファイル名
    export void bcopy(const std::string& src, const std::string& dest, const std::source_location& location = std::source_location::current());

    /// @brief ファイルのサイズ取得
    /// @param filename サイズを調べるファイルの名前
    /// @return ファイルサイズ（ファイルが存在しない場合は-1）
    export [[nodiscard]] int64_t exist(const std::string& filename, const std::source_location& location = std::source_location::current());

    /// @brief ディレクトリ一覧を取得
    /// @param filemask 一覧のためのファイルマスク（例: "*.*", "*.txt"）
    /// @param mode ディレクトリ取得モード
    ///   0: すべてのファイル
    ///   1: ディレクトリを除くすべてのファイル
    ///   2: 隠し属性・システム属性を除くすべてのファイル
    ///   3: ディレクトリ・隠し属性・システム属性以外のすべてのファイル
    ///   5: ディレクトリのみ
    ///   6: 隠し属性・システム属性ファイルのみ
    ///   7: ディレクトリと隠し属性・システム属性ファイルのみ
    /// @return ファイル名のリスト
    export [[nodiscard]] std::vector<std::string> dirlist(const std::string& filemask, OptInt mode = {},
                                            const std::source_location& location = std::source_location::current());

    /// @brief バッファにファイルをロード
    /// @param filename ロードするファイル名
    /// @param buffer 内容を読み込む先の変数
    /// @param size ロードされるサイズ（-1で自動）
    /// @param offset ファイルのオフセット
    /// @return 読み込んだバイト数
    export int64_t bload(const std::string& filename, std::string& buffer, OptInt64 size = {}, OptInt64 offset = {},
                    const std::source_location& location = std::source_location::current());
    export int64_t bload(const std::string& filename, std::vector<uint8_t>& buffer, OptInt64 size = {}, OptInt64 offset = {},
                    const std::source_location& location = std::source_location::current());

    /// @brief バッファをファイルにセーブ
    /// @param filename セーブするファイル名
    /// @param buffer セーブする内容
    /// @param size セーブするサイズ（-1で自動）
    /// @param offset ファイルのオフセット
    /// @return 書き込んだバイト数
    export int64_t bsave(const std::string& filename, const std::string& buffer, OptInt64 size = {}, OptInt64 offset = {},
                     const std::source_location& location = std::source_location::current());
    export int64_t bsave(const std::string& filename, const std::vector<uint8_t>& buffer, OptInt64 size = {}, OptInt64 offset = {},
                     const std::source_location& location = std::source_location::current());

    // ============================================================
    // ダイアログ命令（HSP互換）
    // ============================================================

    // --- dialogタイプ定数 ---
    export inline constexpr int dialog_info     = 0;    ///< 標準メッセージボックス + [OK]
    export inline constexpr int dialog_warning  = 1;    ///< 警告メッセージボックス + [OK]
    export inline constexpr int dialog_yesno    = 2;    ///< 標準メッセージボックス + [はい][いいえ]
    export inline constexpr int dialog_yesno_w  = 3;    ///< 警告メッセージボックス + [はい][いいえ]
    export inline constexpr int dialog_open     = 16;   ///< ファイルOPEN(開く)ダイアログ
    export inline constexpr int dialog_save     = 17;   ///< ファイルSAVE(保存)ダイアログ
    export inline constexpr int dialog_color    = 32;   ///< カラー選択ダイアログ(固定色)
    export inline constexpr int dialog_colorex  = 33;   ///< カラー選択ダイアログ(RGBを自由に選択)

    /// @brief ダイアログを開く
    /// @param message メッセージ文字列（ファイルダイアログ時は拡張子）
    /// @param type ダイアログタイプ（dialog_*定数）
    /// @param option オプション文字列（タイトルや拡張子説明）
    /// @return ダイアログの結果（int/stringに変換可能）
    /// @note メッセージボックス: ボタンIDを返す (1=OK, 6=はい, 7=いいえ)
    /// @note ファイルダイアログ: 選択されたファイルパスを返す（キャンセル時は空文字列）
    /// @note カラーダイアログ: 成功時は1、キャンセル時は0を返す。選択された色はginfo_r/g/bで取得可能
    export DialogResult dialog(const std::string& message, OptInt type = {}, const std::string& option = "",
                     const std::source_location& location = std::source_location::current());

    // ============================================================
    // GUIオブジェクト制御命令（HSP互換）
    // ============================================================

    // --- objmode定数 ---
    export inline constexpr int objmode_normal    = 0;    ///< HSP標準フォントを使用
    export inline constexpr int objmode_guifont   = 1;    ///< デフォルトGUIフォントを使用
    export inline constexpr int objmode_usefont   = 2;    ///< font命令で選択されているフォントを使用
    export inline constexpr int objmode_usecolor  = 4;    ///< color命令/objcolor命令の色を使用

    /// @brief オブジェクトサイズ設定
    /// @param sizeX オブジェクトのX方向のサイズ（ドット単位、デフォルト: 64）
    /// @param sizeY オブジェクトのY方向のサイズ（ドット単位、デフォルト: 24）
    /// @param spaceY Y方向の最低確保行サイズ（デフォルト: 0）
    export void objsize(OptInt sizeX = {}, OptInt sizeY = {}, OptInt spaceY = {},
                       const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトモード設定
    /// @param mode フォント設定モード (objmode_* 定数)
    /// @param tabMove TABキーフォーカス移動 (0=無効, 1=有効、省略時は変更なし)
    export void objmode(OptInt mode = {}, OptInt tabMove = {},
                       const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトのカラー設定
    /// @param r 赤輝度 (0〜255)
    /// @param g 緑輝度 (0〜255)
    /// @param b 青輝度 (0〜255)
    export void objcolor(OptInt r = {}, OptInt g = {}, OptInt b = {},
                        const std::source_location& location = std::source_location::current());

    /// @brief ボタン表示
    /// @param name ボタンの名前
    /// @param callback 押した時に呼ばれるコールバック関数
    /// @param isGosub gosub形式かどうか (true=gosub, false=goto)
    /// @return オブジェクトID (stat相当)
    export int button(std::string_view name, std::function<int()> callback, bool isGosub = false,
                     const std::source_location& location = std::source_location::current());

    // ============================================================
    // GUIコントロール: shared_ptr版のみ提供（ライフタイム安全性）
    // ============================================================
    // 
    // 【設計上の注意】input/mesbox/chkbox/combox/listbox には参照版がありません。
    // 
    // HSPでは変数はすべてグローバルスコープなので、GUIオブジェクトが
    // 変数を参照し続けても問題ありませんでした。しかしC++では、
    // ローカル変数を参照で渡すと、関数を抜けた後にGUIがその変数を
    // 参照し続け、ダングリングポインタ（未定義動作）となります。
    //
    // 安全のため、これらの関数は std::shared_ptr 版のみ提供します。
    // 
    // 使用例:
    //   auto strVar = std::make_shared<std::string>("initial");
    //   input(strVar, 200, 24, 256);
    //
    //   auto intVar = std::make_shared<int>(42);
    //   input(intVar, 100, 24);
    //
    //   auto mesVar = std::make_shared<std::string>("Line1\nLine2");
    //   mesbox(mesVar, 300, 200, 1);  // 編集可能
    //
    //   auto checkState = std::make_shared<int>(0);
    //   chkbox("Enable", checkState);
    //
    //   auto comboIndex = std::make_shared<int>(0);
    //   combox(comboIndex, 100, "Option A\nOption B\nOption C");
    // ============================================================

    /// @brief 入力ボックス表示（文字列変数用）
    /// @param var 入力のための文字列変数（shared_ptr）
    /// @param sizeX 入力ボックスのXサイズ（省略時はobjsizeに従う）
    /// @param sizeY 入力ボックスのYサイズ（省略時はobjsizeに従う）
    /// @param maxLen 入力できる最大文字数（省略時は256）
    /// @return オブジェクトID (stat相当)
    /// @note 参照版は提供していません（ライフタイム安全性のため）。
    ///       std::make_shared<std::string>(初期値) で変数を作成してください。
    export int input(std::shared_ptr<std::string> var, OptInt sizeX = {}, OptInt sizeY = {}, OptInt maxLen = {},
                    const std::source_location& location = std::source_location::current());

    /// @brief 入力ボックス表示（整数変数用）
    /// @param var 入力のための整数変数（shared_ptr）
    /// @param sizeX 入力ボックスのXサイズ（省略時はobjsizeに従う）
    /// @param sizeY 入力ボックスのYサイズ（省略時はobjsizeに従う）
    /// @param maxLen 入力できる最大文字数（省略時は32）
    /// @return オブジェクトID (stat相当)
    /// @note 参照版は提供していません（ライフタイム安全性のため）。
    ///       std::make_shared<int>(初期値) で変数を作成してください。
    export int input(std::shared_ptr<int> var, OptInt sizeX = {}, OptInt sizeY = {}, OptInt maxLen = {},
                    const std::source_location& location = std::source_location::current());

    /// @brief メッセージボックス表示
    /// @param var 表示メッセージを保持する文字列変数（shared_ptr）
    /// @param sizeX メッセージボックスのXサイズ（省略時はobjsizeに従う）
    /// @param sizeY メッセージボックスのYサイズ（省略時はobjsizeに従う）
    /// @param style スタイル (0=読取専用, 1=編集可能, +4=横スクロールバー, +8=自動ラップ無効)
    /// @param maxLen 入力できる最大文字数（省略時は32767）
    /// @return オブジェクトID (stat相当)
    /// @note 参照版は提供していません（ライフタイム安全性のため）。
    ///       std::make_shared<std::string>(初期値) で変数を作成してください。
    export int mesbox(std::shared_ptr<std::string> var, OptInt sizeX = {}, OptInt sizeY = {}, OptInt style = {}, OptInt maxLen = {},
                     const std::source_location& location = std::source_location::current());

    /// @brief チェックボックス表示
    /// @param label チェックボックスの内容表示文字列
    /// @param var チェックボックスの状態を保持する変数（shared_ptr: 0=OFF, 1=ON）
    /// @return オブジェクトID (stat相当)
    /// @note 参照版は提供していません（ライフタイム安全性のため）。
    ///       std::make_shared<int>(初期値) で変数を作成してください。
    export int chkbox(std::string_view label, std::shared_ptr<int> var,
                     const std::source_location& location = std::source_location::current());

    /// @brief コンボボックス表示
    /// @param var コンボボックスの状態を保持する変数（shared_ptr: 選択インデックス）
    /// @param expandY 拡張Yサイズ（リスト表示用、100〜150程度推奨）
    /// @param items コンボボックスの内容（\n区切りの文字列）
    /// @return オブジェクトID (stat相当)
    /// @note 参照版は提供していません（ライフタイム安全性のため）。
    ///       std::make_shared<int>(初期値) で変数を作成してください。
    export int combox(std::shared_ptr<int> var, OptInt expandY, std::string_view items,
                     const std::source_location& location = std::source_location::current());

    /// @brief リストボックス表示
    /// @param var リストボックスの状態を保持する変数（shared_ptr: 選択インデックス）
    /// @param expandY 拡張Yサイズ
    /// @param items リストボックスの内容（\n区切りの文字列）
    /// @return オブジェクトID (stat相当)
    /// @note 参照版は提供していません（ライフタイム安全性のため）。
    ///       std::make_shared<int>(初期値) で変数を作成してください。
    export int listbox(std::shared_ptr<int> var, OptInt expandY, std::string_view items,
                      const std::source_location& location = std::source_location::current());

    // ============================================================
    // オブジェクト操作
    // ============================================================

    /// @brief オブジェクトをクリア
    /// @param startId 消去するオブジェクトID(開始)（省略時は0）
    /// @param endId 消去するオブジェクトID(終了)（省略/-1で最終IDまで）
    export void clrobj(OptInt startId = {}, OptInt endId = {},
                      const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトの内容を変更
    /// @param objectId オブジェクトID
    /// @param value 変更するパラメータの内容（文字列）
    export void objprm(int objectId, std::string_view value,
                      const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトの内容を変更（整数版）
    /// @param objectId オブジェクトID
    /// @param value 変更するパラメータの内容（数値）
    export void objprm(int objectId, int value,
                      const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトに入力フォーカスを設定
    /// @param objectId オブジェクトID（-1で現在のフォーカスIDを取得）
    /// @return フォーカスが当たっているオブジェクトID（objectId=-1時）
    export int objsel(OptInt objectId = {},
                     const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトの有効・無効を設定
    /// @param objectId オブジェクトID
    /// @param enable 0=無効, それ以外=有効
    export void objenable(int objectId, OptInt enable = {},
                         const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトのフォーカス移動モードを設定
    /// @param objectId オブジェクトID
    /// @param mode フォーカス移動モード (1=移動可能, 2=移動不可, 3=スキップ, +4=全選択)
    export void objskip(int objectId, OptInt mode = {},
                       const std::source_location& location = std::source_location::current());

    // ============================================================
    // メモリ管理関数（HSP互換）




    // ============================================================
    // 数学定数（HSP hspmath.as互換）
    // ============================================================
    export inline constexpr double M_PI = std::numbers::pi_v<double>;           // 円周率
    export inline constexpr double M_PI_2 = std::numbers::pi_v<double> / 2.0;   // 円周率/2
    export inline constexpr double M_PI_4 = std::numbers::pi_v<double> / 4.0;   // 円周率/4
    export inline constexpr double M_1_PI = 1.0 / std::numbers::pi_v<double>;   // 1/円周率
    export inline constexpr double M_2_PI = 2.0 / std::numbers::pi_v<double>;   // 2/円周率
    export inline constexpr double M_E = std::numbers::e_v<double>;             // ネイピア数
    export inline constexpr double M_LOG2E = std::numbers::log2e_v<double>;     // 2を底とするeの対数
    export inline constexpr double M_LOG10E = std::numbers::log10e_v<double>;   // 10を底とするeの対数
    export inline constexpr double M_LN2 = std::numbers::ln2_v<double>;         // eを底とする2の対数
    export inline constexpr double M_LN10 = std::numbers::ln10_v<double>;       // eを底とする10の対数
    export inline constexpr double M_SQRT2 = std::numbers::sqrt2_v<double>;     // 2の平方根
    export inline constexpr double M_SQRT1_2 = 1.0 / std::numbers::sqrt2_v<double>; // 1/√2
    export inline constexpr double M_SQRT3 = std::numbers::sqrt3_v<double>;     // 3の平方根
    export inline constexpr double M_SQRTPI = 1.0 / std::numbers::inv_sqrtpi_v<double>; // 円周率の平方根

    // ============================================================
    // C++標準ライブラリ: 文字列関連 (std::formatなど)
    // ============================================================
    // C++20 <format> - 型安全なフォーマット（printfの現代的代替）
    //
    // 使用例:
    //   import hsppp;
    //   auto s1 = hsppp::format("Hello, {}!", "World");
    //   auto s2 = hsppp::format("{:05d}", 42);          // "00042"
    //   auto s3 = hsppp::format("{:.2f}", 3.14159);     // "3.14"
    //   auto s4 = hsppp::format("{0} + {0} = {1}", 2, 4); // "2 + 2 = 4"

    export using std::format;           // 書式付き文字列生成
    export using std::format_to;        // イテレータへフォーマット出力
    export using std::format_to_n;      // 最大n文字までフォーマット出力
    export using std::formatted_size;   // 必要なバッファサイズを計算
    export using std::vformat;          // 動的引数フォーマット
    export using std::vformat_to;       // 動的引数でイテレータへ出力
    export using std::make_format_args; // フォーマット引数作成

    // フォーマットエラー
    export using std::format_error;     // フォーマットエラー例外

    // ============================================================
    // C++標準ライブラリ: 文字列型
    // ============================================================
    // <string> - 文字列クラス
    //
    // 使用例:
    //   import hsppp;
    //   hsppp::string s = "Hello";       // std::string
    //   auto len = s.size();             // 5
    //   s += " World";                   // "Hello World"

    export using std::string;           // 文字列クラス
    export using std::wstring;          // ワイド文字列クラス
    export using std::u8string;         // UTF-8文字列 (C++20)
    export using std::u16string;        // UTF-16文字列
    export using std::u32string;        // UTF-32文字列

    // <string_view> - 文字列ビュー（非所有、軽量参照）
    //
    // 使用例:
    //   import hsppp;
    //   void process(hsppp::string_view sv) { /* コピーなしで参照 */ }
    //   process("literal");              // 文字列リテラルから変換
    //   process(myString);               // std::stringから変換

    export using std::string_view;      // 文字列ビュー
    export using std::wstring_view;     // ワイド文字列ビュー
    export using std::u8string_view;    // UTF-8文字列ビュー (C++20)
    export using std::u16string_view;   // UTF-16文字列ビュー
    export using std::u32string_view;   // UTF-32文字列ビュー

    // 文字列変換 (数値 <-> 文字列)
    export using std::to_string;        // 数値→文字列
    export using std::to_wstring;       // 数値→ワイド文字列
    export using std::stoi;             // 文字列→int
    export using std::stol;             // 文字列→long
    export using std::stoll;            // 文字列→long long
    export using std::stoul;            // 文字列→unsigned long
    export using std::stoull;           // 文字列→uint64_t
    export using std::stof;             // 文字列→float
    export using std::stod;             // 文字列→double
    export using std::stold;            // 文字列→long double

    // getline (ストリームから1行読み込み)
    export using std::getline;

    // ============================================================
    // C++標準ライブラリ: アルゴリズム (文字列操作に便利)
    // ============================================================
    // 
    // 使用例:
    //   import hsppp;
    //   hsppp::string s = "Hello";
    //   hsppp::transform(s.begin(), s.end(), s.begin(), ::toupper);
    //   // s = "HELLO"

    export using std::transform;        // 変換
    export using std::copy;             // コピー
    export using std::copy_if;          // 条件付きコピー
    export using std::fill;             // 埋める
    export using std::find;             // 検索
    export using std::find_if;          // 条件検索
    export using std::find_if_not;      // 否定条件検索
    export using std::count;            // カウント
    export using std::count_if;         // 条件カウント
    export using std::replace;          // 置換
    export using std::replace_if;       // 条件置換
    export using std::remove;           // 削除（イテレータ移動）
    export using std::remove_if;        // 条件削除
    export using std::unique;           // 重複削除
    export using std::reverse;          // 反転
    export using std::sort;             // ソート
    export using std::stable_sort;      // 安定ソート
    export using std::all_of;           // 全て満たすか
    export using std::any_of;           // いずれか満たすか
    export using std::none_of;          // 全て満たさないか
    export using std::equal;            // 等価判定
    export using std::mismatch;         // 不一致検出
    export using std::search;           // 部分列検索

    // ============================================================
    // C++標準ライブラリ: ユーティリティ
    // ============================================================

    // <optional> - 省略可能な値
    export using std::optional;
    export using std::nullopt;
    export using std::make_optional;

    // <vector> - 動的配列
    export using std::vector;

    // <functional> - 関数オブジェクト
    export using std::function;
    export using std::invoke;
    export using std::bind_front;       // 前方引数束縛 (C++20)

    // ============================================================
    // Cel Factory Function (OOP版)
    // ============================================================
    /// @brief 画像ファイルをロードしてCelオブジェクトを作成
    /// @param filename ファイル名
    /// @param celId cel ID（省略時は自動割り当て）
    /// @return Celオブジェクト
    export Cel loadCel(std::string_view filename, OptInt celId = {},
                      const std::source_location& location = std::source_location::current());

    // --- System / Internal ---
    namespace internal {
        // ライブラリの初期化・終了処理(WinMainから呼ばれる)
        export void init_system(const std::source_location& location = std::source_location::current());
        export void close_system(const std::source_location& location = std::source_location::current());

        // HspError例外を処理（エラーハンドラを呼び出す）
        export void handleHspError(const HspError& error, const std::source_location& location = std::source_location::current());
    }
}

// グローバル名前空間にユーザー定義関数が存在することを期待する
// (UserApp.cpp で実装される)
extern int hspMain();
