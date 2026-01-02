// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp_string.ixx
// 文字列モジュール: strmid, instr, NotePad 等の文字列操作

export module hsppp:string;

import :types;

import <string>;
import <string_view>;
import <vector>;
import <source_location>;
import <format>;
import <algorithm>;

export namespace hsppp {

    // ============================================================
    // 型変換関数
    // ============================================================

    /// @brief 整数値に変換
    [[nodiscard]] int toInt(double p1);
    [[nodiscard]] int toInt(const std::string& p1);

    /// @brief 実数値に変換
    [[nodiscard]] double toDouble(int p1);
    [[nodiscard]] double toDouble(const std::string& p1);

    /// @brief 文字列に変換
    [[nodiscard]] std::string str(double value, const std::source_location& location = std::source_location::current());
    [[nodiscard]] std::string str(int value, const std::source_location& location = std::source_location::current());
    [[nodiscard]] std::string str(int64_t value, const std::source_location& location = std::source_location::current());

    /// @brief 文字列の長さを調べる
    [[nodiscard]] int64_t strlen(const std::string& p1) noexcept;

    // ============================================================
    // 文字列操作関数
    // ============================================================

    /// @brief 文字列の検索をする
    [[nodiscard]] int64_t instr(const std::string& p1, int64_t p2, const std::string& search);
    [[nodiscard]] int64_t instr(const std::string& p1, const std::string& search);

    /// @brief 文字列の一部を取り出す
    [[nodiscard]] std::string strmid(const std::string& p1, int64_t p2, int64_t p3);

    /// @brief 指定した文字だけを取り除く
    [[nodiscard]] std::string strtrim(const std::string& p1, int p2 = 0, int p3 = 32);

    /// @brief 文字列の置換をする
    int64_t strrep(std::string& p1, const std::string& search, const std::string& replace, const std::source_location& location = std::source_location::current());

    /// @brief バッファから文字列読み出し
    int64_t getstr(std::string& dest, const std::string& src, int64_t index, int delimiter = 0, int64_t maxLen = 1024, const std::source_location& location = std::source_location::current());
    int64_t getstr(std::string& dest, const std::vector<uint8_t>& src, int64_t index, int delimiter = 0, int64_t maxLen = 1024, const std::source_location& location = std::source_location::current());

    /// @brief 文字列から分割された要素を取得
    std::vector<std::string> split(const std::string& src, const std::string& delimiter, const std::source_location& location = std::source_location::current());

    /// @brief 書式付き文字列を変換
    [[nodiscard]] std::string strf(const std::string& format);
    [[nodiscard]] std::string strf(const std::string& format, int arg1);
    [[nodiscard]] std::string strf(const std::string& format, double arg1);
    [[nodiscard]] std::string strf(const std::string& format, const std::string& arg1);
    [[nodiscard]] std::string strf(const std::string& format, int arg1, int arg2);
    [[nodiscard]] std::string strf(const std::string& format, int arg1, double arg2);
    [[nodiscard]] std::string strf(const std::string& format, int arg1, const std::string& arg2);
    [[nodiscard]] std::string strf(const std::string& format, double arg1, int arg2);
    [[nodiscard]] std::string strf(const std::string& format, double arg1, double arg2);
    [[nodiscard]] std::string strf(const std::string& format, int arg1, int arg2, int arg3);
    [[nodiscard]] std::string strf(const std::string& format, int arg1, double arg2, const std::string& arg3);

    /// @brief パスの一部を取得
    [[nodiscard]] std::string getpath(const std::string& p1, int p2);

    // ============================================================
    // 文字列変換関数（HSP hsp3utf.as互換）
    // ============================================================

    /// @brief 通常文字列(UTF-8)をunicode(UTF-16)に変換
    [[nodiscard]] std::u16string cnvstow(const std::string& str, const std::source_location& location = std::source_location::current());

    /// @brief unicode(UTF-16)を通常文字列(UTF-8)に変換
    [[nodiscard]] std::string cnvwtos(const std::u16string& wstr, const std::source_location& location = std::source_location::current());

    /// @brief 通常文字列(UTF-8)をANSI(ShiftJIS)文字列に変換
    [[nodiscard]] std::string cnvstoa(const std::string& str, const std::source_location& location = std::source_location::current());

    /// @brief ANSI(ShiftJIS)文字列を通常文字列(UTF-8)に変換
    [[nodiscard]] std::string cnvatos(const std::string& astr, const std::source_location& location = std::source_location::current());

    // ============================================================
    // NotePad クラス - OOP版メモリノートパッド
    // ============================================================

    /// @brief OOP版メモリノートパッド
    class NotePad {
    private:
        std::string m_buffer;

    public:
        /// @brief デフォルトコンストラクタ（空のノートパッド）
        NotePad() = default;

        /// @brief 文字列からの構築
        explicit NotePad(std::string_view text);

        /// @brief std::stringからのムーブ構築
        explicit NotePad(std::string&& text) noexcept;

        // コピー・ムーブはデフォルト
        NotePad(const NotePad&) = default;
        NotePad& operator=(const NotePad&) = default;
        NotePad(NotePad&&) noexcept = default;
        NotePad& operator=(NotePad&&) noexcept = default;

        /// @brief 行数を取得（notemax相当）
        [[nodiscard]] size_t count() const noexcept;

        /// @brief 空かどうか
        [[nodiscard]] bool empty() const noexcept { return m_buffer.empty(); }

        /// @brief 総バイト数を取得（notesize相当）
        [[nodiscard]] size_t size() const noexcept { return m_buffer.size(); }

        /// @brief 指定行の内容を取得（noteget相当）
        [[nodiscard]] std::string get(size_t index) const;

        /// @brief 行を追加（noteadd相当）
        NotePad& add(std::string_view text, int index = -1, int overwrite = 0, const std::source_location& location = std::source_location::current());

        /// @brief 行を削除（notedel相当）
        NotePad& del(size_t index, const std::source_location& location = std::source_location::current());

        /// @brief 全行をクリア
        NotePad& clear() noexcept { m_buffer.clear(); return *this; }

        /// @brief 文字列を検索（notefind相当）
        [[nodiscard]] int find(std::string_view search, int mode = 0, size_t startIndex = 0) const;

        /// @brief ファイルから読み込み（noteload相当）
        NotePad& load(std::string_view filename, size_t maxSize = 0, const std::source_location& location = std::source_location::current());

        /// @brief ファイルへ保存（notesave相当）
        [[nodiscard]] bool save(std::string_view filename, const std::source_location& location = std::source_location::current()) const;

        /// @brief 内部バッファへの参照を取得
        [[nodiscard]] std::string& buffer() noexcept { return m_buffer; }
        [[nodiscard]] const std::string& buffer() const noexcept { return m_buffer; }

        /// @brief 改行区切りの文字列として出力
        [[nodiscard]] const std::string& toString() const noexcept { return m_buffer; }

        /// @brief 明示的な文字列変換
        explicit operator const std::string&() const noexcept { return m_buffer; }
    };

    // ============================================================
    // メモリノートパッド命令セット（HSP互換）
    // ============================================================

    // noteinfo用定数
    inline constexpr int notemax  = 0;
    inline constexpr int notesize = 1;

    // notefind用定数
    inline constexpr int notefind_match = 0;
    inline constexpr int notefind_first = 1;
    inline constexpr int notefind_instr = 2;

    /// @brief 対象バッファ指定
    void notesel(std::string& buffer, const std::source_location& location = std::source_location::current());

    /// @brief 対象バッファの復帰
    void noteunsel(const std::source_location& location = std::source_location::current());

    /// @brief 指定行の追加・変更
    void noteadd(std::string_view text, OptInt index = {}, OptInt overwrite = {}, const std::source_location& location = std::source_location::current());

    /// @brief 行の削除
    void notedel(int index, const std::source_location& location = std::source_location::current());

    /// @brief 指定行を読み込み
    void noteget(std::string& dest, OptInt index = {}, const std::source_location& location = std::source_location::current());

    /// @brief 対象バッファ読み込み
    void noteload(std::string_view filename, OptInt maxSize = {}, const std::source_location& location = std::source_location::current());

    /// @brief 対象バッファ保存
    void notesave(std::string_view filename, const std::source_location& location = std::source_location::current());

    /// @brief メモリノートパッド検索
    int notefind(std::string_view search, OptInt mode = {}, const std::source_location& location = std::source_location::current());

    /// @brief メモリノートパッド情報取得
    int noteinfo(OptInt mode = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // C++標準ライブラリ: std::format 再エクスポート
    // ============================================================

    using std::format;
    using std::format_to;
    using std::format_to_n;
    using std::formatted_size;
    using std::vformat;
    using std::vformat_to;
    using std::make_format_args;
    using std::format_error;

    // ============================================================
    // C++標準ライブラリ: 文字列型
    // ============================================================

    using std::string;
    using std::wstring;
    using std::u8string;
    using std::u16string;
    using std::u32string;

    using std::string_view;
    using std::wstring_view;
    using std::u8string_view;
    using std::u16string_view;
    using std::u32string_view;

    // 文字列変換 (数値 <-> 文字列)
    using std::to_string;
    using std::to_wstring;
    using std::stoi;
    using std::stol;
    using std::stoll;
    using std::stoul;
    using std::stoull;
    using std::stof;
    using std::stod;
    using std::stold;

    using std::getline;

    // ============================================================
    // C++標準ライブラリ: アルゴリズム
    // ============================================================

    using std::transform;
    using std::copy;
    using std::copy_if;
    using std::fill;
    using std::find;
    using std::find_if;
    using std::find_if_not;
    using std::count;
    using std::count_if;
    using std::replace;
    using std::replace_if;
    using std::remove;
    using std::remove_if;
    using std::unique;
    using std::reverse;
    using std::sort;
    using std::stable_sort;
    using std::all_of;
    using std::any_of;
    using std::none_of;
    using std::equal;
    using std::mismatch;
    using std::search;

    // ============================================================
    // デバッグ命令
    // ============================================================

    /// @brief デバッグメッセージ送信
    void logmes(std::string_view message, const std::source_location& location = std::source_location::current());

    /// @brief デバッグメッセージ送信（int版）
    void logmes(int value, const std::source_location& location = std::source_location::current());

    /// @brief デバッグメッセージ送信（double版）
    void logmes(double value, const std::source_location& location = std::source_location::current());

} // namespace hsppp
