// HspppLib/module/hsppp_math.ixx
// 数学モジュール: <cmath> 再エクスポート, deg2rad, rnd 等

export module hsppp:math;

import :types;

import <cmath>;
import <numbers>;
import <source_location>;
import <vector>;
import <string>;

export namespace hsppp {

    // ============================================================
    // <cmath> 再エクスポート
    // ============================================================

    // --- 基本数学関数 ---
    using std::abs;
    using std::fabs;
    using std::fmod;
    using std::remainder;
    using std::fmax;
    using std::fmin;
    using std::fdim;
    using std::fma;

    // --- 指数・対数 ---
    using std::exp;
    using std::exp2;
    using std::expm1;
    using std::log;
    using std::log10;
    using std::log2;
    using std::log1p;

    // --- 累乗・平方根 ---
    using std::pow;
    using std::sqrt;
    using std::cbrt;
    using std::hypot;

    // --- 三角関数（ラジアン） ---
    using std::sin;
    using std::cos;
    using std::tan;
    using std::asin;
    using std::acos;
    using std::atan;
    using std::atan2;

    // --- 双曲線関数 ---
    using std::sinh;
    using std::cosh;
    using std::tanh;
    using std::asinh;
    using std::acosh;
    using std::atanh;

    // --- 切り捨て・切り上げ・丸め ---
    using std::ceil;
    using std::floor;
    using std::trunc;
    using std::round;
    using std::nearbyint;
    using std::rint;

    // --- 分解・合成 ---
    using std::frexp;
    using std::ldexp;
    using std::modf;
    using std::scalbn;
    using std::ilogb;
    using std::logb;

    // --- 符号・分類 ---
    using std::copysign;
    using std::signbit;
    using std::isnan;
    using std::isinf;
    using std::isfinite;
    using std::isnormal;
    using std::fpclassify;

    // --- 特殊関数 (C++17) ---
    using std::erf;
    using std::erfc;
    using std::tgamma;
    using std::lgamma;

    // ============================================================
    // 角度変換
    // ============================================================

    /// @brief 度数法（度）をラジアンに変換
    [[nodiscard]] inline constexpr double deg2rad(double degrees) noexcept {
        return degrees * std::numbers::pi_v<double> / 180.0;
    }

    /// @brief ラジアンを度数法（度）に変換
    [[nodiscard]] inline constexpr double rad2deg(double radians) noexcept {
        return radians * 180.0 / std::numbers::pi_v<double>;
    }

    // ============================================================
    // 乱数
    // ============================================================

    /// @brief 乱数を発生
    /// @param p1 乱数の範囲（1〜32768）
    /// @return 0から(p1-1)までの乱数
    [[nodiscard]] int rnd(int p1);

    /// @brief 乱数発生の初期化
    /// @param p1 乱数シード（省略時は時刻ベース）
    void randomize(OptInt p1 = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // 範囲制限
    // ============================================================

    /// @brief 一定範囲内の整数を返す
    [[nodiscard]] int limit(int p1, OptInt p2 = {}, OptInt p3 = {});

    /// @brief 一定範囲内の実数を返す
    [[nodiscard]] double limitf(double p1, OptDouble p2 = {}, OptDouble p3 = {});

    // ============================================================
    // イージング関数
    // ============================================================

    // イージング計算式のタイプ定数
    inline constexpr int ease_linear         = 0;
    inline constexpr int ease_quad_in        = 1;
    inline constexpr int ease_quad_out       = 2;
    inline constexpr int ease_quad_inout     = 3;
    inline constexpr int ease_cubic_in       = 4;
    inline constexpr int ease_cubic_out      = 5;
    inline constexpr int ease_cubic_inout    = 6;
    inline constexpr int ease_quartic_in     = 7;
    inline constexpr int ease_quartic_out    = 8;
    inline constexpr int ease_quartic_inout  = 9;
    inline constexpr int ease_bounce_in      = 10;
    inline constexpr int ease_bounce_out     = 11;
    inline constexpr int ease_bounce_inout   = 12;
    inline constexpr int ease_shake_in       = 13;
    inline constexpr int ease_shake_out      = 14;
    inline constexpr int ease_shake_inout    = 15;
    inline constexpr int ease_loop           = 0x100;

    /// @brief イージング関数の計算式を設定
    void setease(double p1, double p2, OptInt p3 = {}, const std::source_location& location = std::source_location::current());

    /// @brief イージング値を整数で取得
    [[nodiscard]] int getease(int p1, OptInt p2 = {}, const std::source_location& location = std::source_location::current());

    /// @brief イージング値を実数で取得
    [[nodiscard]] double geteasef(double p1, OptDouble p2 = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // ソート関数
    // ============================================================

    /// @brief 配列変数を数値でソート（int版）
    void sortval(std::vector<int>& arr, OptInt order = {}, const std::source_location& location = std::source_location::current());

    /// @brief 配列変数を数値でソート（double版）
    void sortval(std::vector<double>& arr, OptInt order = {}, const std::source_location& location = std::source_location::current());

    /// @brief 配列変数を文字列でソート
    void sortstr(std::vector<std::string>& arr, OptInt order = {}, const std::source_location& location = std::source_location::current());

    /// @brief メモリノート文字列をソート
    void sortnote(std::string& note, OptInt order = {}, const std::source_location& location = std::source_location::current());

    /// @brief ソート元のインデックスを取得
    [[nodiscard]] int sortget(int index, const std::source_location& location = std::source_location::current());

    // ============================================================
    // 数学定数（HSP hspmath.as互換）
    // ============================================================

    inline constexpr double M_PI = std::numbers::pi_v<double>;
    inline constexpr double M_PI_2 = std::numbers::pi_v<double> / 2.0;
    inline constexpr double M_PI_4 = std::numbers::pi_v<double> / 4.0;
    inline constexpr double M_1_PI = 1.0 / std::numbers::pi_v<double>;
    inline constexpr double M_2_PI = 2.0 / std::numbers::pi_v<double>;
    inline constexpr double M_E = std::numbers::e_v<double>;
    inline constexpr double M_LOG2E = std::numbers::log2e_v<double>;
    inline constexpr double M_LOG10E = std::numbers::log10e_v<double>;
    inline constexpr double M_LN2 = std::numbers::ln2_v<double>;
    inline constexpr double M_LN10 = std::numbers::ln10_v<double>;
    inline constexpr double M_SQRT2 = std::numbers::sqrt2_v<double>;
    inline constexpr double M_SQRT1_2 = 1.0 / std::numbers::sqrt2_v<double>;
    inline constexpr double M_SQRT3 = std::numbers::sqrt3_v<double>;
    inline constexpr double M_SQRTPI = 1.0 / std::numbers::inv_sqrtpi_v<double>;

} // namespace hsppp
