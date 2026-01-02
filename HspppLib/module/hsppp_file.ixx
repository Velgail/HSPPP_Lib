// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp_file.ixx
// ファイル操作モジュール: exist, bload, bsave, dirlist 等

export module hsppp:file;

import :types;

import <string>;
import <string_view>;
import <vector>;
import <source_location>;
import <functional>;
import <memory>;

export namespace hsppp {

    // ============================================================
    // exec実行モードフラグ
    // ============================================================

    inline constexpr int exec_normal     = 0;    ///< ノーマル実行
    inline constexpr int exec_minimized  = 2;    ///< 最小化モードで実行
    inline constexpr int exec_shellexec  = 16;   ///< 関連付けされたアプリケーションを実行
    inline constexpr int exec_print      = 32;   ///< ファイルを印刷する

    // ============================================================
    // ファイル操作命令
    // ============================================================

    /// @brief Windowsのファイルを実行する
    int exec(const std::string& filename, OptInt mode = {}, const std::string& command = "",
             const std::source_location& location = std::source_location::current());

    /// @brief ディレクトリ移動
    void chdir(const std::string& dirname, const std::source_location& location = std::source_location::current());

    /// @brief ディレクトリ作成
    void mkdir(const std::string& dirname, const std::source_location& location = std::source_location::current());

    /// @brief ファイル削除
    void deletefile(const std::string& filename, const std::source_location& location = std::source_location::current());

    /// @brief ファイルのコピー
    void bcopy(const std::string& src, const std::string& dest, const std::source_location& location = std::source_location::current());

    /// @brief ファイルのサイズ取得
    [[nodiscard]] int64_t exist(const std::string& filename, const std::source_location& location = std::source_location::current());

    /// @brief ディレクトリ一覧を取得
    [[nodiscard]] std::vector<std::string> dirlist(const std::string& filemask, OptInt mode = {},
                                      const std::source_location& location = std::source_location::current());

    /// @brief バッファにファイルをロード（string版）
    int64_t bload(const std::string& filename, std::string& buffer, OptInt64 size = {}, OptInt64 offset = {},
                  const std::source_location& location = std::source_location::current());

    /// @brief バッファにファイルをロード（vector版）
    int64_t bload(const std::string& filename, std::vector<uint8_t>& buffer, OptInt64 size = {}, OptInt64 offset = {},
                  const std::source_location& location = std::source_location::current());

    /// @brief バッファをファイルにセーブ（string版）
    int64_t bsave(const std::string& filename, const std::string& buffer, OptInt64 size = {}, OptInt64 offset = {},
                  const std::source_location& location = std::source_location::current());

    /// @brief バッファをファイルにセーブ（vector版）
    int64_t bsave(const std::string& filename, const std::vector<uint8_t>& buffer, OptInt64 size = {}, OptInt64 offset = {},
                  const std::source_location& location = std::source_location::current());

    // ============================================================
    // ダイアログ命令
    // ============================================================

    // dialogタイプ定数
    inline constexpr int dialog_info     = 0;
    inline constexpr int dialog_warning  = 1;
    inline constexpr int dialog_yesno    = 2;
    inline constexpr int dialog_yesno_w  = 3;
    inline constexpr int dialog_open     = 16;
    inline constexpr int dialog_save     = 17;
    inline constexpr int dialog_color    = 32;
    inline constexpr int dialog_colorex  = 33;

    /// @brief ダイアログを開く
    DialogResult dialog(const std::string& message, OptInt type = {}, const std::string& option = "",
                        const std::source_location& location = std::source_location::current());

    // ============================================================
    // ディレクトリ情報関数
    // ============================================================

    // dirinfo type定数
    inline constexpr int dir_type_cur     = 0;
    inline constexpr int dir_type_exe     = 1;
    inline constexpr int dir_type_win     = 2;
    inline constexpr int dir_type_sys     = 3;
    inline constexpr int dir_type_cmdline = 4;
    inline constexpr int dir_type_tv      = 5;

    /// @brief ディレクトリ情報を取得する
    [[nodiscard]] std::string dirinfo(int type, const std::source_location& location = std::source_location::current());

    /// @brief カレントディレクトリを取得
    [[nodiscard]] std::string dir_cur(const std::source_location& location = std::source_location::current());

    /// @brief 実行ファイルがあるディレクトリを取得
    [[nodiscard]] std::string dir_exe(const std::source_location& location = std::source_location::current());

    /// @brief Windowsディレクトリを取得
    [[nodiscard]] std::string dir_win(const std::source_location& location = std::source_location::current());

    /// @brief Windowsシステムディレクトリを取得
    [[nodiscard]] std::string dir_sys(const std::source_location& location = std::source_location::current());

    /// @brief コマンドライン文字列を取得
    [[nodiscard]] std::string dir_cmdline(const std::source_location& location = std::source_location::current());

    /// @brief デスクトップディレクトリを取得
    [[nodiscard]] std::string dir_desktop(const std::source_location& location = std::source_location::current());

    /// @brief マイドキュメントディレクトリを取得
    [[nodiscard]] std::string dir_mydoc(const std::source_location& location = std::source_location::current());

    // ============================================================
    // システム情報関数
    // ============================================================

    /// @brief システム情報を文字列で取得する
    [[nodiscard]] std::string sysinfo_str(int type, const std::source_location& location = std::source_location::current());

    /// @brief システム情報を整数で取得する
    [[nodiscard]] int64_t sysinfo_int(int type, const std::source_location& location = std::source_location::current());

    // ============================================================
    // GUIオブジェクト制御命令
    // ============================================================

    // objmode定数
    inline constexpr int objmode_normal    = 0;
    inline constexpr int objmode_guifont   = 1;
    inline constexpr int objmode_usefont   = 2;
    inline constexpr int objmode_usecolor  = 4;

    /// @brief オブジェクトサイズ設定
    void objsize(OptInt sizeX = {}, OptInt sizeY = {}, OptInt spaceY = {},
                 const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトモード設定
    void objmode(OptInt mode = {}, OptInt tabMove = {},
                 const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトのカラー設定
    void objcolor(OptInt r = {}, OptInt g = {}, OptInt b = {},
                  const std::source_location& location = std::source_location::current());

    /// @brief ボタン表示
    int button(std::string_view name, std::function<void()> callback,
               const std::source_location& location = std::source_location::current());

    /// @brief 入力ボックス表示（文字列変数用）
    int input(std::shared_ptr<std::string> var, OptInt sizeX = {}, OptInt sizeY = {}, OptInt maxLen = {},
              const std::source_location& location = std::source_location::current());

    /// @brief メッセージボックス表示
    int mesbox(std::shared_ptr<std::string> var, OptInt sizeX = {}, OptInt sizeY = {}, OptInt style = {}, OptInt maxLen = {},
               const std::source_location& location = std::source_location::current());

    /// @brief チェックボックス表示
    int chkbox(std::string_view label, std::shared_ptr<int> var,
               const std::source_location& location = std::source_location::current());

    /// @brief コンボボックス表示
    int combox(std::shared_ptr<int> var, OptInt expandY, std::string_view items,
               const std::source_location& location = std::source_location::current());

    /// @brief リストボックス表示
    int listbox(std::shared_ptr<int> var, OptInt expandY, std::string_view items,
                const std::source_location& location = std::source_location::current());

    // ============================================================
    // オブジェクト操作
    // ============================================================

    /// @brief オブジェクトをクリア
    void clrobj(OptInt startId = {}, OptInt endId = {},
                const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトの内容を変更（文字列版）
    void objprm(int objectId, std::string_view value,
                const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトの内容を変更（整数版）
    void objprm(int objectId, int value,
                const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトに入力フォーカスを設定
    int objsel(OptInt objectId = {},
               const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトの有効・無効を設定
    void objenable(int objectId, OptInt enable = {},
                   const std::source_location& location = std::source_location::current());

    /// @brief オブジェクトのフォーカス移動モードを設定
    void objskip(int objectId, OptInt mode = {},
                 const std::source_location& location = std::source_location::current());

    // ============================================================
    // C++標準ライブラリ: ユーティリティ
    // ============================================================

    using std::optional;
    using std::nullopt;
    using std::make_optional;

    using std::vector;

    using std::function;
    using std::invoke;
    using std::bind_front;

} // namespace hsppp
