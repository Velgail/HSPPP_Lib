// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp_screen.ixx
// 画面制御モジュール: screen, buffer, gsel, redraw 等の画面制御命令

export module hsppp:screen;

import :types;

import <string_view>;
import <source_location>;

export namespace hsppp {

    // ============================================================
    // OOP版関数（ID自動採番）
    // ============================================================

    /// @brief ウィンドウを初期化（OOP版・構造体）
    /// @param params 初期化パラメータ（IDは自動採番）
    /// @return Screen ハンドル
    [[nodiscard]] Screen screen(const ScreenParams& params, const std::source_location& location = std::source_location::current());

    /// @brief ウィンドウを初期化（OOP版・引数なし）
    /// @return Screen ハンドル（デフォルト設定で作成）
    [[nodiscard]] Screen screen(const std::source_location& location = std::source_location::current());

    /// @brief 仮想画面を初期化（OOP版・構造体）
    /// @param params 初期化パラメータ（IDは自動採番）
    /// @return Screen ハンドル
    [[nodiscard]] Screen buffer(const BufferParams& params, const std::source_location& location = std::source_location::current());

    /// @brief 仮想画面を初期化（OOP版・引数なし）
    /// @return Screen ハンドル（デフォルト設定で作成）
    [[nodiscard]] Screen buffer(const std::source_location& location = std::source_location::current());

    /// @brief 枠なしウィンドウを初期化（OOP版・構造体）
    /// @param params 初期化パラメータ（IDは自動採番）
    /// @return Screen ハンドル
    [[nodiscard]] Screen bgscr(const BgscrParams& params, const std::source_location& location = std::source_location::current());

    /// @brief 枠なしウィンドウを初期化（OOP版・引数なし）
    /// @return Screen ハンドル（デフォルト設定で作成）
    [[nodiscard]] Screen bgscr(const std::source_location& location = std::source_location::current());

    // ============================================================
    // HSP互換版関数（ID明示指定）
    // ============================================================

    /// @brief ウィンドウを初期化（HSP互換・省略可能版）
    Screen screen(
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

    /// @brief 仮想画面を初期化（HSP互換・ID明示指定版）
    Screen buffer(
        int id,
        OptInt width  = {},
        OptInt height = {},
        OptInt mode   = {},
        const std::source_location& location = std::source_location::current()
    );

    /// @brief 枠のないウィンドウを初期化（HSP互換・ID明示指定版）
    Screen bgscr(
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
    // 画面制御
    // ============================================================

    /// @brief 描画先を指定したウィンドウIDに変更
    /// @param id ウィンドウID
    /// @param mode ウィンドウアクティブスイッチ (-1=非表示, 0=影響なし, 1=アクティブ, 2=アクティブ+最前面)
    void gsel(OptInt id = {}, OptInt mode = {}, const std::source_location& location = std::source_location::current());

    /// @brief 画面コピーモードを設定
    /// @param mode 画面コピーモード (0～6)
    /// @param size_x コピーする大きさX (デフォルト: 32)
    /// @param size_y コピーする大きさY (デフォルト: 32)
    /// @param blend_rate 半透明合成時のブレンド率 (0～256)
    void gmode(OptInt mode = {}, OptInt size_x = {}, OptInt size_y = {}, OptInt blend_rate = {}, const std::source_location& location = std::source_location::current());

    /// @brief 指定したウィンドウIDから現在の描画先にコピー
    void gcopy(OptInt src_id = {}, OptInt src_x = {}, OptInt src_y = {}, OptInt size_x = {}, OptInt size_y = {}, const std::source_location& location = std::source_location::current());

    /// @brief 指定したウィンドウIDから変倍してコピー
    void gzoom(OptInt dest_w = {}, OptInt dest_h = {}, OptInt src_id = {}, OptInt src_x = {}, OptInt src_y = {}, OptInt src_w = {}, OptInt src_h = {}, OptInt mode = {}, const std::source_location& location = std::source_location::current());

    /// @brief 描画制御
    /// @param p1 0=描画予約(Offscreen), 1=画面反映(Present)
    void redraw(int p1 = 1, const std::source_location& location = std::source_location::current());

    /// @brief 待機＆メッセージ処理 (HSP互換・高精度版)
    /// @details QueryPerformanceCounterを使用した高精度タイマー実装
    void await(int time_ms, const std::source_location& location = std::source_location::current());
    
    /// @brief VSync同期待機
    /// @return 前回のvwait呼び出しからの経過時間（ミリ秒）
    /// @details 画面のVSync信号に同期して待機します。
    ///          戻り値でフレームレートの乱れを検出できます。
    double vwait(const std::source_location& location = std::source_location::current());

    /// @brief プログラム終了 (HSP互換)
    [[noreturn]] void end(int exitcode = 0, const std::source_location& location = std::source_location::current());

    /// @brief ウィンドウ内の情報をすべてクリア
    /// @param p1 クリアする時の色 (0=白, 1=明るい灰色, 2=灰色, 3=暗い灰色, 4=黒)
    void cls(OptInt p1 = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // 画像ファイル操作
    // ============================================================

    /// @brief 画像ファイルをロード
    void picload(std::string_view p1, OptInt p2 = {}, const std::source_location& location = std::source_location::current());
    
    /// @brief 画面イメージをBMPファイルに保存
    void bmpsave(std::string_view p1, const std::source_location& location = std::source_location::current());
    
    /// @brief 画像ファイルをバッファにロード（仮想ID）
    /// @return 割り当てられたcel ID
    int celload(std::string_view p1, OptInt p2 = {}, const std::source_location& location = std::source_location::current());
    
    /// @brief 画像素材の分割サイズを設定
    void celdiv(int p1, int p2, int p3, const std::source_location& location = std::source_location::current());
    
    /// @brief 画像素材を描画
    void celput(int p1, int p2, OptInt p3 = {}, OptInt p4 = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // Cel Factory Function (OOP版)
    // ============================================================

    /// @brief 画像ファイルをロードしてCelオブジェクトを作成
    Cel loadCel(std::string_view filename, OptInt celId = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // ウィンドウ設定
    // ============================================================

    /// @brief ウィンドウのタイトルバーを設定
    void title(std::string_view str, const std::source_location& location = std::source_location::current());

    /// @brief ウィンドウサイズと位置を設定
    void width(OptInt clientW = {}, OptInt clientH = {}, OptInt posX = {}, OptInt posY = {}, OptInt option = {}, const std::source_location& location = std::source_location::current());

    /// @brief グラフィック面の描画基点座標を設定
    void groll(int scrollX, int scrollY, const std::source_location& location = std::source_location::current());

    /// @brief 指定時間だけ実行を中断する
    /// @param time 待ち時間 (10ms単位、デフォルト: 100=1秒)
    void wait(OptInt time = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // ginfo定数
    // ============================================================

    inline constexpr int ginfo_type_mx        = 0;
    inline constexpr int ginfo_type_my        = 1;
    inline constexpr int ginfo_type_act       = 2;
    inline constexpr int ginfo_type_sel       = 3;
    inline constexpr int ginfo_type_wx1       = 4;
    inline constexpr int ginfo_type_wy1       = 5;
    inline constexpr int ginfo_type_wx2       = 6;
    inline constexpr int ginfo_type_wy2       = 7;
    inline constexpr int ginfo_type_vx        = 8;
    inline constexpr int ginfo_type_vy        = 9;
    inline constexpr int ginfo_type_sizex     = 10;
    inline constexpr int ginfo_type_sizey     = 11;
    inline constexpr int ginfo_type_mesx      = 12;
    inline constexpr int ginfo_type_mesy      = 13;
    inline constexpr int ginfo_type_messizex  = 14;
    inline constexpr int ginfo_type_messizey  = 15;
    inline constexpr int ginfo_type_r         = 16;
    inline constexpr int ginfo_type_g         = 17;
    inline constexpr int ginfo_type_b         = 18;
    inline constexpr int ginfo_type_colr      = ginfo_type_r;
    inline constexpr int ginfo_type_colg      = ginfo_type_g;
    inline constexpr int ginfo_type_colb      = ginfo_type_b;
    inline constexpr int ginfo_type_paluse    = 19;
    inline constexpr int ginfo_type_dispx     = 20;
    inline constexpr int ginfo_type_dispy     = 21;
    inline constexpr int ginfo_type_cx        = 22;
    inline constexpr int ginfo_type_cy        = 23;
    inline constexpr int ginfo_type_intid     = 24;
    inline constexpr int ginfo_type_newid     = 25;
    inline constexpr int ginfo_type_sx        = 26;
    inline constexpr int ginfo_type_sy        = 27;
    inline constexpr int ginfo_type_fps       = 28;  // 画面リフレッシュレート（マルチモニター時は最大値）

    // ============================================================
    // ginfo関数
    // ============================================================

    /// @brief ウィンドウ関連情報を取得
    int ginfo(int type, const std::source_location& location = std::source_location::current());
    
    /// @brief 画面のリフレッシュレート（Hz）を取得
    /// @details マルチモニター環境では接続されているモニターの中で最大のリフレッシュレートを返します
    int get_framerate(const std::source_location& location = std::source_location::current());

    // ginfo_* マクロ/システム変数互換
    int ginfo_mx(const std::source_location& location = std::source_location::current());
    int ginfo_my(const std::source_location& location = std::source_location::current());
    int ginfo_act(const std::source_location& location = std::source_location::current());
    int ginfo_sel(const std::source_location& location = std::source_location::current());
    int ginfo_wx1(const std::source_location& location = std::source_location::current());
    int ginfo_wy1(const std::source_location& location = std::source_location::current());
    int ginfo_wx2(const std::source_location& location = std::source_location::current());
    int ginfo_wy2(const std::source_location& location = std::source_location::current());
    int ginfo_vx(const std::source_location& location = std::source_location::current());
    int ginfo_vy(const std::source_location& location = std::source_location::current());
    int ginfo_sizex(const std::source_location& location = std::source_location::current());
    int ginfo_sizey(const std::source_location& location = std::source_location::current());
    int ginfo_mesx(const std::source_location& location = std::source_location::current());
    int ginfo_mesy(const std::source_location& location = std::source_location::current());
    int ginfo_messizex(const std::source_location& location = std::source_location::current());
    int ginfo_messizey(const std::source_location& location = std::source_location::current());
    int ginfo_paluse(const std::source_location& location = std::source_location::current());
    int ginfo_dispx(const std::source_location& location = std::source_location::current());
    int ginfo_dispy(const std::source_location& location = std::source_location::current());
    int ginfo_cx(const std::source_location& location = std::source_location::current());
    int ginfo_cy(const std::source_location& location = std::source_location::current());
    int ginfo_intid(const std::source_location& location = std::source_location::current());
    int ginfo_newid(const std::source_location& location = std::source_location::current());
    int ginfo_sx(const std::source_location& location = std::source_location::current());
    int ginfo_sy(const std::source_location& location = std::source_location::current());
    int ginfo_r(const std::source_location& location = std::source_location::current());
    int ginfo_g(const std::source_location& location = std::source_location::current());
    int ginfo_b(const std::source_location& location = std::source_location::current());
    int ginfo_fps(const std::source_location& location = std::source_location::current());

    /// @brief 時間・日付を取得する
    /// @param type 取得するタイプ (0=年, 1=月, 2=曜日, 3=日, 4=時, 5=分, 6=秒, 7=ミリ秒)
    int gettime(int type, const std::source_location& location = std::source_location::current());

    // ============================================================
    // gmode定数
    // ============================================================

    inline constexpr int gmode_copy       = 0;
    inline constexpr int gmode_mem        = 1;
    inline constexpr int gmode_and        = 2;
    inline constexpr int gmode_or         = 3;
    inline constexpr int gmode_alpha      = 4;
    inline constexpr int gmode_add        = 5;
    inline constexpr int gmode_sub        = 6;

    // gsquare用定数
    inline constexpr int gsquare_grad = -257;

} // namespace hsppp
