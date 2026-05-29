// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

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

    /// @brief AnchorRect を塗りつぶし（OOP/HSP 共用）
    /// @details 現在の描画先サーフェスのバッファサイズに対して rect.resolve() を行い、
    ///          解決後の座標で boxf(x1,y1,x2,y2) を実行する。
    void boxf(const AnchorRect& rect, const std::source_location& location = std::source_location::current());

    /// @brief アンカー基準で描画位置を設定（HSP 互換命令）
    /// @param anchorH ah_left / ah_center / ah_right
    /// @param anchorV av_top  / av_middle / av_bottom
    /// @param offsetX 基準点からの X オフセット (px、論理座標)
    /// @param offsetY 基準点からの Y オフセット (px、論理座標)
    /// @details 仮想画面 ON 時は論理 px、OFF 時は物理クライアント px が基準となる。
    ///          例: anchor_pos(ah_right, av_bottom, -10, -10) で右下から内側 10px。
    void anchor_pos(int anchorH, int anchorV, int offsetX, int offsetY,
                    const std::source_location& location = std::source_location::current());

    /// @brief アンカー基準の矩形を塗りつぶす（HSP 互換命令）
    /// @param anchorH ah_left / ah_center / ah_right （矩形側の基準辺）
    /// @param anchorV av_top  / av_middle / av_bottom
    /// @param offsetX 基準点からの追加 X オフセット
    /// @param offsetY 基準点からの追加 Y オフセット
    /// @param w 矩形幅 (px)
    /// @param h 矩形高さ (px)
    void anchor_box(int anchorH, int anchorV, int offsetX, int offsetY, int w, int h,
                    const std::source_location& location = std::source_location::current());

    // ============================================================
    // 図形描画
    // ============================================================

    /// @brief 直線を描画
    void line(OptInt x2 = {}, OptInt y2 = {}, OptInt x1 = {}, OptInt y1 = {}, const std::source_location& location = std::source_location::current());

    /// @brief 円を描画
    void circle(OptInt x1 = {}, OptInt y1 = {}, OptInt x2 = {}, OptInt y2 = {}, OptInt fillMode = {}, const std::source_location& location = std::source_location::current());

    /// @brief 1ドットの点を描画
    void pset(OptInt x = {}, OptInt y = {}, const std::source_location& location = std::source_location::current());

    /// @brief 線描画の太さを設定（論理 px / デフォルト 1.0 / HSP3 互換）
    /// @param w 線幅（論理 px）。w <= 0 は 1.0 にクランプ。
    /// @details 対象命令: line / circle (輪郭) / pset。
    ///          仮想画面 ON または DPI ≠ 96 環境では SetTransform(Scale(s)) により
    ///          物理 px へ伝搬される（論理 px 指定）。
    void gline_width(OptDouble w = {}, const std::source_location& location = std::source_location::current());

    /// @brief ラスタ転送系の補間モードを設定
    /// @param mode 0=nearest / 1=linear (既定) / 2=anisotropic
    /// @details 対象命令: picload / celput / gcopy / gzoom (gzoom は引数未指定時のみ)。
    ///          既定 1=linear は従来挙動と互換。
    void gmode_interp(OptInt mode = {}, const std::source_location& location = std::source_location::current());

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

    // フォント名定数（HSP互換: font msgothic, 16 → font(msgothic, 16)）
    inline constexpr std::string_view msgothic      = "MS Gothic";       ///< MSゴシック
    inline constexpr std::string_view msmincho      = "MS Mincho";       ///< MS明朝
    inline constexpr std::string_view mspgothic     = "MS PGothic";      ///< MSPゴシック
    inline constexpr std::string_view mspmincho     = "MS PMincho";      ///< MSP明朝
    inline constexpr std::string_view meiryo        = "Meiryo";          ///< メイリオ
    inline constexpr std::string_view meiryoui      = "Meiryo UI";       ///< メイリオUI
    inline constexpr std::string_view yugothic      = "Yu Gothic";       ///< 遊ゴシック
    inline constexpr std::string_view yugothicui    = "Yu Gothic UI";    ///< 遊ゴシックUI
    inline constexpr std::string_view yumincho      = "Yu Mincho";       ///< 遊明朝
    inline constexpr std::string_view bizudgothic   = "BIZ UDGothic";    ///< BIZ UDゴシック
    inline constexpr std::string_view bizudmincho   = "BIZ UDMincho";    ///< BIZ UD明朝
    inline constexpr std::string_view arial         = "Arial";           ///< Arial
    inline constexpr std::string_view timesnewroman = "Times New Roman"; ///< Times New Roman
    inline constexpr std::string_view couriernew    = "Courier New";     ///< Courier New
    inline constexpr std::string_view verdana       = "Verdana";         ///< Verdana
    inline constexpr std::string_view tahoma        = "Tahoma";          ///< Tahoma
    inline constexpr std::string_view consolas      = "Consolas";        ///< Consolas

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
