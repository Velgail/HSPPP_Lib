// HspppLib/src/core/Media.h
// OOP版メディアクラス
//
// 使用例:
//   Media bgm("resources/music.mp3");
//   bgm.vol(-500).pan(0).loop(true);
//   bgm.play();
//   bgm.stop();

#pragma once

#include <string>
#include <string_view>
#include <memory>

namespace hsppp {

// ============================================================
// Media クラス（OOP版メディア管理）
// ============================================================
class Media {
public:
    // コンストラクタ
    Media();
    explicit Media(std::string_view filename);
    ~Media();

    // コピー禁止、ムーブ許可
    Media(const Media&) = delete;
    Media& operator=(const Media&) = delete;
    Media(Media&& other) noexcept;
    Media& operator=(Media&& other) noexcept;

    // ファイル操作
    bool load(std::string_view filename);
    void unload();

    // 再生制御
    bool play();
    void stop();

    // 設定（メソッドチェーン対応）
    Media& vol(int v);       // -1000（無音）〜 0（最大）
    Media& pan(int p);       // -1000（左）〜 0（中央）〜 1000（右）
    Media& loop(bool l);     // ループ設定
    Media& mode(int m);      // 再生モード (0=通常, 1=ループ, 2=終了まで待機)

    // 取得
    [[nodiscard]] int get_vol() const;
    [[nodiscard]] int get_pan() const;
    [[nodiscard]] bool get_loop() const;
    [[nodiscard]] int get_mode() const;
    [[nodiscard]] int stat() const;       // mmstat相当
    [[nodiscard]] bool playing() const;   // 再生中か
    [[nodiscard]] bool loaded() const;    // ロード済みか

    // ファイル情報
    [[nodiscard]] const std::string& filename() const;

    // 内部バッファID取得（mm系関数との互換用）
    [[nodiscard]] int id() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace hsppp
