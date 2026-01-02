// HspppLib/module/hsppp_media.ixx
// マルチメディアモジュール: mmload, mmplay, mmstop 等のマルチメディア命令と Media クラス

export module hsppp:media;

import :types;

import <string>;
import <string_view>;
import <source_location>;
import <memory>;

export namespace hsppp {

    // ============================================================
    // mm系マルチメディア関数（HSP互換）
    // ============================================================

    /// @brief メディアファイルを読み込み
    /// @param filename 読み込むファイル名
    /// @param bufferId バッファID（省略時は0）
    /// @param mode 再生モード（0=通常, 1=ループ, 2=終了まで待機, 3=CDトラック連続再生）
    /// @return 0=成功、非0=失敗
    int mmload(std::string_view filename, OptInt bufferId = {}, OptInt mode = {},
               const std::source_location& location = std::source_location::current());

    /// @brief メディアを再生
    /// @param bufferId バッファID（省略時は0）
    /// @return 0=成功、非0=失敗
    int mmplay(OptInt bufferId = {}, const std::source_location& location = std::source_location::current());

    /// @brief メディア再生を停止
    /// @param bufferId バッファID（省略時は全停止）
    void mmstop(OptInt bufferId = {}, const std::source_location& location = std::source_location::current());

    /// @brief 音量を設定
    /// @param bufferId バッファID
    /// @param vol 音量（-1000=無音 ～ 0=最大）
    void mmvol(int bufferId, int vol, const std::source_location& location = std::source_location::current());

    /// @brief パンニングを設定
    /// @param bufferId バッファID
    /// @param pan パン（-1000=左 ～ 0=中央 ～ 1000=右）
    void mmpan(int bufferId, int pan, const std::source_location& location = std::source_location::current());

    /// @brief メディアの状態を取得
    /// @param bufferId バッファID
    /// @param mode 取得モード（0=再生状態, 16=拡張状態）
    /// @return 状態値（0=停止, 1=再生中, 2=一時停止）
    int mmstat(int bufferId, OptInt mode = {}, const std::source_location& location = std::source_location::current());

    // ============================================================
    // Media クラス（OOP版メディア管理）
    // ============================================================

    /// @brief メディアファイルを管理するクラス
    /// @details 音声・動画ファイルの読み込み、再生、停止などを行う
    /// 
    /// 使用例:
    /// @code
    /// Media bgm("music.mp3");
    /// bgm.vol(-500).pan(0).loop(true);
    /// bgm.play();
    /// bgm.stop();
    /// @endcode
    class Media {
    public:
        /// @brief デフォルトコンストラクタ
        Media();

        /// @brief ファイル名を指定してコンストラクト
        /// @param filename メディアファイル名
        explicit Media(std::string_view filename, const std::source_location& location = std::source_location::current());

        /// @brief デストラクタ
        ~Media();

        // コピー禁止、ムーブ許可
        Media(const Media&) = delete;
        Media& operator=(const Media&) = delete;
        Media(Media&& other) noexcept;
        Media& operator=(Media&& other) noexcept;

        // ファイル操作

        /// @brief メディアファイルを読み込み
        /// @param filename ファイル名
        /// @return 成功=true、失敗=false
        bool load(std::string_view filename, const std::source_location& location = std::source_location::current());

        /// @brief 読み込んだメディアをアンロード
        void unload();

        // 再生制御

        /// @brief メディアを再生
        /// @return 成功=true、失敗=false
        bool play(const std::source_location& location = std::source_location::current());

        /// @brief メディアを停止
        void stop(const std::source_location& location = std::source_location::current());

        // 設定（メソッドチェーン対応）

        /// @brief 音量を設定
        /// @param v 音量（-1000=無音 ～ 0=最大）
        /// @return 自身の参照（メソッドチェーン用）
        Media& vol(int v, const std::source_location& location = std::source_location::current());

        /// @brief パンニングを設定
        /// @param p パン（-1000=左 ～ 0=中央 ～ 1000=右）
        /// @return 自身の参照（メソッドチェーン用）
        Media& pan(int p, const std::source_location& location = std::source_location::current());

        /// @brief ループ再生を設定
        /// @param l ループする=true、しない=false（次にload()が呼ばれる際に適用されます）
        /// @return 自身の参照（メソッドチェーン用）
        Media& loop(bool l, const std::source_location& location = std::source_location::current());

        /// @brief 再生モードを設定
        /// @param m モード（0=通常, 1=ループ, 2=終了まで待機）（次にload()が呼ばれる際に適用されます）
        /// @return 自身の参照（メソッドチェーン用）
        Media& mode(int m, const std::source_location& location = std::source_location::current());

        /// @brief 動画再生先Screenを指定
        /// @param screenId Screen ID
        /// @return 自身の参照（メソッドチェーン用）
        Media& target(int screenId, const std::source_location& location = std::source_location::current());

        // 取得

        /// @brief 現在の音量を取得
        /// @return 音量（-1000 ～ 0）
        [[nodiscard]] int get_vol() const;

        /// @brief 現在のパンを取得
        /// @return パン（-1000 ～ 1000）
        [[nodiscard]] int get_pan() const;

        /// @brief ループ設定を取得
        /// @return ループする=true
        [[nodiscard]] bool get_loop() const;

        /// @brief 再生モードを取得
        /// @return モード値
        [[nodiscard]] int get_mode() const;

        /// @brief メディアの状態を取得（mmstat相当）
        /// @return 状態値（0=停止, 1=再生中）
        [[nodiscard]] int stat() const;

        /// @brief 再生中かどうか
        /// @return 再生中=true
        [[nodiscard]] bool playing() const;

        /// @brief ロード済みかどうか
        /// @return ロード済み=true
        [[nodiscard]] bool loaded() const;

        /// @brief ファイル名を取得
        /// @return ファイル名
        [[nodiscard]] const std::string& filename() const;

        /// @brief 内部バッファIDを取得（mm系関数との互換用）
        /// @return バッファID
        [[nodiscard]] int id() const;

    private:
        class Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace hsppp
