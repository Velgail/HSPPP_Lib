// HspppLib/module/hsppp_drawing.ixx
// 描画モジュール: mes, boxf, line, circle 等の描画命令

export module hsppp:drawing;

import :types;

import <string_view>;
import <source_location>;
import <utility>;

export namespace hsppp {

    // ============================================================
    // 基本描画関数
    // ============================================================

    /// @brief 描画色を設定
    void color(int r, int g, int b, const std::source_location& location = std::source_location::current());

    /// @brief 描画位置を設定
    void pos(int x, int y, const std::source_location& location = std::source_location::current());

    /// @brief 文字列を描画
    /// @param text 表示するメッセージ
    /// @param sw オプション (1=改行しない, 2=影, 4=縁取り, 8=簡易描画, 16=gmode設定)
    void mes(std::string_view text, OptInt sw = {}, const std::source_location& location = std::source_location::current());

    /// @brief 文字列サイズを取得
    std::pair<int, int> messize(std::string_view text, const std::source_location& location = std::source_location::current());

    /// @brief 矩形を塗りつぶし
    void boxf(int x1, int y1, int x2, int y2, const std::source_location& location = std::source_location::current());

    /// @brief 画面全体を塗りつぶし
    void boxf(const std::source_location& location = std::source_location::current());

    // ============================================================
    // 図形描画
    // ============================================================

    /// @brief 直線を描画
    void line(OptInt x2 = {}, OptInt y2 = {}, OptInt x1 = {}, OptInt y1 = {}, const std::source_location& location = std::source_location::current());

    /// @brief 円を描画
    void circle(OptInt x1 = {}, OptInt y1 = {}, OptInt x2 = {}, OptInt y2 = {}, OptInt fillMode = {}, const std::source_location& location = std::source_location::current());

    /// @brief 1ドットの点を描画
    void pset(OptInt x = {}, OptInt y = {}, const std::source_location& location = std::source_location::current());

    /// @brief 1ドットの色を取得し、選択色として設定
    void pget(OptInt x = {}, OptInt y = {}, const std::source_location& location = std::source_location::current());

    /// @brief 矩形をグラデーションで塗りつぶす
    void gradf(OptInt x = {}, OptInt y = {}, OptInt w = {}, OptInt h = {}, OptInt mode = {}, OptInt color1 = {}, OptInt color2 = {}, const std::source_location& location = std::source_location::current());

    /// @brief 回転する矩形で塗りつぶす
    void grect(OptInt cx = {}, OptInt cy = {}, OptDouble angle = {}, OptInt w = {}, OptInt h = {}, const std::source_location& location = std::source_location::current());

    /// @brief 矩形画像を回転してコピー
    void grotate(OptInt srcId = {}, OptInt srcX = {}, OptInt srcY = {}, OptDouble angle = {}, OptInt dstW = {}, OptInt dstH = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // gsquare - 任意の四角形を描画
    // ============================================================

    /// @brief 任意の四角形を単色塗りつぶし
    void gsquare(int srcId, const Quad& dst, const std::source_location& location = std::source_location::current());

    /// @brief 任意の四角形へ画像をコピー
    void gsquare(int srcId, const Quad& dst, const QuadUV& src, const std::source_location& location = std::source_location::current());

    /// @brief 任意の四角形をグラデーション塗りつぶし
    void gsquare(int srcId, const Quad& dst, const QuadColors& colors, const std::source_location& location = std::source_location::current());

    // ============================================================
    // フォント設定
    // ============================================================

    /// @brief フォントを設定
    int font(std::string_view fontName, OptInt size = {}, OptInt style = {}, OptInt decorationWidth = {}, const std::source_location& location = std::source_location::current());

    /// @brief システム標準のフォントを選択
    void sysfont(OptInt type = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // 色関連関数
    // ============================================================

    /// @brief HSV形式でカラーを設定する
    void hsvcolor(int p1, int p2, int p3, const std::source_location& location = std::source_location::current());

    /// @brief RGB形式でカラーを設定する
    void rgbcolor(int p1, const std::source_location& location = std::source_location::current());

    /// @brief システムカラーを設定する
    void syscolor(int p1, const std::source_location& location = std::source_location::current());

    // ============================================================
    // print - mes別名
    // ============================================================

    /// @brief メッセージを表示（mes命令の別名）
    void print(std::string_view text, OptInt sw = {}, const std::source_location& location = std::source_location::current());

} // namespace hsppp
