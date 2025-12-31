// HspppLib/module/hsppp_input.ixx
// 入力モジュール: stick, getkey, mouse 等の入力命令

export module hsppp:input;

import :types;

import <source_location>;

export namespace hsppp {

    // ============================================================
    // stick キーコードビットマスク (HSP Compatible)
    // ============================================================

    inline constexpr int stick_left    = 1;       // カーソルキー左(←)
    inline constexpr int stick_up      = 2;       // カーソルキー上(↑)
    inline constexpr int stick_right   = 4;       // カーソルキー右(→)
    inline constexpr int stick_down    = 8;       // カーソルキー下(↓)
    inline constexpr int stick_space   = 16;      // スペースキー
    inline constexpr int stick_enter   = 32;      // Enterキー
    inline constexpr int stick_ctrl    = 64;      // Ctrlキー
    inline constexpr int stick_esc     = 128;     // ESCキー
    inline constexpr int stick_lbutton = 256;     // マウス左ボタン
    inline constexpr int stick_rbutton = 512;     // マウス右ボタン
    inline constexpr int stick_tab     = 1024;    // TABキー
    inline constexpr int stick_z       = 2048;    // [Z]キー
    inline constexpr int stick_x       = 4096;    // [X]キー
    inline constexpr int stick_c       = 8192;    // [C]キー
    inline constexpr int stick_a       = 16384;   // [A]キー
    inline constexpr int stick_w       = 32768;   // [W]キー
    inline constexpr int stick_d       = 65536;   // [D]キー
    inline constexpr int stick_s       = 131072;  // [S]キー

    // ============================================================
    // キー入力関数
    // ============================================================

    /// @brief キー入力情報を取得
    /// @param nonTrigger 非トリガータイプキー指定（押しっぱなしでも検出するキー）
    /// @param checkActive ウィンドウアクティブチェック (0=常に取得, 1=アクティブ時のみ)
    /// @return キー入力状態のビットフラグ
    int stick(OptInt nonTrigger = {}, OptInt checkActive = {}, const std::source_location& location = std::source_location::current());

    /// @brief 指定したキーが押されているかチェック
    /// @param keycode 仮想キーコード (VK_LEFT=37, VK_UP=38, etc.)
    /// @return 押されていれば1、押されていなければ0
    int getkey(int keycode, const std::source_location& location = std::source_location::current());

    // ============================================================
    // マウス関連関数
    // ============================================================

    /// @brief マウスカーソルの座標を設定
    /// @param x X座標 (ディスプレイ座標)
    /// @param y Y座標 (ディスプレイ座標)
    /// @param mode 設定モード (0=負値で非表示, -1=移動+非表示, 1=移動のみ, 2=移動+表示)
    void mouse(OptInt x = {}, OptInt y = {}, OptInt mode = {}, const std::source_location& location = std::source_location::current());

    /// @brief マウスカーソルのX座標を取得
    /// @return 現在のウィンドウ内でのX座標
    int mousex(const std::source_location& location = std::source_location::current());

    /// @brief マウスカーソルのY座標を取得
    /// @return 現在のウィンドウ内でのY座標
    int mousey(const std::source_location& location = std::source_location::current());

    /// @brief マウスホイールの移動量を取得
    /// @return ホイール移動量
    int mousew(const std::source_location& location = std::source_location::current());

} // namespace hsppp
