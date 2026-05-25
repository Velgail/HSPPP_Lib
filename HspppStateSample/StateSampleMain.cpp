// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// SPDX-License-Identifier: BSL-1.0
//
// ═══════════════════════════════════════════════════════════════════════════
// HspppStateSample - StateGraph + StateScope + SaveData 統合サンプル
// ═══════════════════════════════════════════════════════════════════════════
//
// このサンプルは HSPPP の新 API を 1 本のクリックゲームに集約した参考実装です。
//
//   - StateGraph<T>      … HSP の *label / goto に相当する画面遷移
//   - StateScope<T>      … ステート別ローカル変数（旧 g_xxx グローバル置換）
//   - state_vars<L>(...) … StateScope への登録ショートカット（HSPPP 流儀の小文字グローバル）
//   - SaveWriter/Reader  … Serializable<L> 概念ベースの永続化 + bsave/bload
//
// HSPユーザーへの対応表（旧サンプル踏襲）:
//   HSP:   *title       → HSPPP: GameScreen::Title
//   HSP:   goto *game   → HSPPP: sm.jump(GameScreen::Game)
//   HSP:   gosub        → HSPPP: (on_enter / on_exit で代用)
//   HSP:   bsave/bload  → HSPPP: SaveWriter::finalize() + bsave / bload + SaveReader
//
// ─────────────────────────────────────────────────────────────────────────
// このサンプルが旧 3 ファイル併存（TICKET-005 整理対象）を統合した経緯:
//
//   旧構成:
//     - StateSampleMain.cpp       … StateMachine デモ + グローバル変数 g_score 等
//     - NewStateSampleMain.cpp    … Repository / state<T>() / tick() デモ（削除済）
//     - 旧手動テスト群             … HspppTest プロジェクトへ移管（StateVarsRuntimeTest 等）
//
//   旧 NewStateSampleMain.cpp 等は
//   `hsppp::GameServices::register_repository<>()` を呼んでおり、これは過去 BLOCKER として
//   build-config.md §4.1 に記録されていた C2280 を踏む経路だった。HEAD `3878e17` 以降の
//   検証では `register_repository<>()` を直接呼ぶサンプルが除去されたため当該 BLOCKER は
//   解消相当（test-TICKET-009.md / test-TICKET-010.md §8）。
//
//   そこで本サンプルは案③（PM 採用、TICKET-005 / MSG-011→MSG-012）に従い、
//   `register_repository<>()` を呼ばず、StateScope + state_vars + SaveData だけで
//   旧 3 ファイル相当のデモ要素を 1 本に統合した。具体的な対応:
//
//     - 旧グローバル g_score / g_targetX 等   → State::Game の StateScope ローカル
//     - 旧グローバル g_highScore               → State::Title の StateScope ローカル + savedata 永続
//     - 旧 Repository<PlayerProfile>           → StateScope に bind した Serializable 型
//     - 旧 NewStateSample のサブステートマシン  → 本サンプルでは省略
//       （新 API では `attach_child` 系を撤去し、サブ SM は親 on_update 内で明示的に
//        `child.step()` を呼ぶ規約に変更された / design-TICKET-008 §7.3 / §11.1）。
//     - 旧 NewStateSample 系の手動テスト        → HspppTest プロジェクト（TICKET-006 / TICKET-009 範囲）へ移管済
//
// ═══════════════════════════════════════════════════════════════════════════

// Windows 仮想キーコード（HSPPP 由来の小文字命名と衝突しないよう定数で）
constexpr int KEY_ESCAPE  = 27;
constexpr int KEY_SPACE   = 32;
constexpr int KEY_LBUTTON = 1;

import hsppp;

import <cstddef>;
import <cstdint>;
import <cstring>;
import <exception>;
import <span>;
import <string>;
import <string_view>;
import <vector>;

using namespace hsppp;

// ═══════════════════════════════════════════════════════════════════════════
// ステップ1: 画面（ステート）を enum class で定義
// ═══════════════════════════════════════════════════════════════════════════

enum class GameScreen {
    Splash,
    Title,
    HowToPlay,
    Game,
    Pause,
    GameOver,
    Result
};

// ═══════════════════════════════════════════════════════════════════════════
// ステップ2: Serializable<L> 概念を満たす永続データ型
// ═══════════════════════════════════════════════════════════════════════════
//
// design §7.3 の Serializable<L> 概念は以下 3 つの static メンバを要求する:
//   - static void        serialize(const L&, std::vector<std::byte>&)
//   - static std::size_t deserialize(std::span<const std::byte>, L&)
//   - static std::string_view type_tag()
//
// SaveWriter::write<L>(key, value) / SaveReader::read<L>(key) はこの概念に
// 制約されており、StateScope::snapshot/restore も Serializable 型だけを対象に取る。
// 非 Serializable 型は snapshot で黙ってスキップされる（enumerate で serializable=false）。

struct PlayerProfile {
    std::int32_t high_score = 0;

    static void serialize(const PlayerProfile& v, std::vector<std::byte>& out) {
        // 4-byte little-endian int32
        const auto u = static_cast<std::uint32_t>(v.high_score);
        out.push_back(static_cast<std::byte>(u & 0xFFu));
        out.push_back(static_cast<std::byte>((u >> 8) & 0xFFu));
        out.push_back(static_cast<std::byte>((u >> 16) & 0xFFu));
        out.push_back(static_cast<std::byte>((u >> 24) & 0xFFu));
    }

    static std::size_t deserialize(std::span<const std::byte> in, PlayerProfile& dst) {
        if (in.size() < 4) {
            // バイト不足は 0 にフォールバック（壊れたセーブを致命にしない）
            dst.high_score = 0;
            return in.size();
        }
        const auto b0 = static_cast<std::uint32_t>(in[0]);
        const auto b1 = static_cast<std::uint32_t>(in[1]);
        const auto b2 = static_cast<std::uint32_t>(in[2]);
        const auto b3 = static_cast<std::uint32_t>(in[3]);
        const auto u = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
        dst.high_score = static_cast<std::int32_t>(u);
        return 4;
    }

    static std::string_view type_tag() noexcept { return "PlayerProfile/v1"; }
};

// ═══════════════════════════════════════════════════════════════════════════
// ステップ3: ステート別ローカルデータ（非 Serializable - メモリ寿命のみ）
// ═══════════════════════════════════════════════════════════════════════════
// 旧 StateSampleMain.cpp の g_score / g_targetX 等を、State::Game に束ねた
// StateScope ローカルに置き換えたもの。

struct GameSceneData {
    std::int32_t score        = 0;
    std::int32_t target_x     = 320;
    std::int32_t target_y     = 100;
    std::int32_t target_radius = 30;
    bool         active       = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// セーブファイル I/O ヘルパー
// ═══════════════════════════════════════════════════════════════════════════
//
// StateScope::snapshot() は std::vector<std::byte> を返す。bsave/bload は
// std::vector<std::uint8_t> を扱うので、薄い相互変換で繋ぐ。

constexpr const char* kSaveFilename = "hsppp_state_sample.sav";

static std::vector<std::uint8_t> to_uint8(std::span<const std::byte> bytes) {
    std::vector<std::uint8_t> out(bytes.size());
    if (!bytes.empty()) {
        std::memcpy(out.data(), bytes.data(), bytes.size());
    }
    return out;
}

static std::vector<std::byte> to_byte(std::span<const std::uint8_t> bytes) {
    std::vector<std::byte> out(bytes.size());
    if (!bytes.empty()) {
        std::memcpy(out.data(), bytes.data(), bytes.size());
    }
    return out;
}

// ═══════════════════════════════════════════════════════════════════════════
// メイン関数
// ═══════════════════════════════════════════════════════════════════════════

void hspMain() {
    screen(0, 640, 480);
    title("HSPPP StateGraph + StateScope + SaveData サンプル");

    // ────────────────────────────────────────────────
    // StateGraph 構築
    // ────────────────────────────────────────────────
    StateGraph<GameScreen> sm;
    sm.enable_debug_log(true);
    sm.enable_history(5);

    sm.set_state_name(GameScreen::Splash,    "Splash");
    sm.set_state_name(GameScreen::Title,     "Title");
    sm.set_state_name(GameScreen::HowToPlay, "HowToPlay");
    sm.set_state_name(GameScreen::Game,      "Game");
    sm.set_state_name(GameScreen::Pause,     "Pause");
    sm.set_state_name(GameScreen::GameOver,  "GameOver");
    sm.set_state_name(GameScreen::Result,    "Result");

    // ────────────────────────────────────────────────
    // StateScope 構築・永続変数の bind
    // ────────────────────────────────────────────────
    // PlayerProfile (高スコア) は Title ステートに紐付ける。
    // bind は idempotent（既存があれば返却・初期化引数は無視）。
    StateScope<GameScreen> scope(sm);

    auto& profile = state_vars<PlayerProfile>(scope, GameScreen::Title);

    // セーブファイルからロードを試みる（無ければスキップ）。
    // bload はファイル不在で HspError を投げ得るため try で握る。
    try {
        std::vector<std::uint8_t> buf;
        (void)bload(kSaveFilename, buf);
        if (!buf.empty()) {
            const auto bytes = to_byte(std::span<const std::uint8_t>(buf));
            // restore は対象キーが bind 済みであることを前提とする。
            // PlayerProfile は上の state_vars<PlayerProfile> で bind 済み。
            scope.restore(std::span<const std::byte>(bytes));
            logmes(strf("[Load] high_score=%d を hsppp_state_sample.sav から復元", profile.high_score));
        }
    }
    catch (const std::exception& e) {
        // 初回起動・ファイル不在・version mismatch は致命でないので継続
        logmes(strf("[Load] スキップ: %s", e.what()));
    }

    // ────────────────────────────────────────────────
    // Splash 画面: 約 2 秒で Title へ
    // ────────────────────────────────────────────────
    sm.state(GameScreen::Splash)
      .on_enter([&]() {
          sm.set_timer(GameScreen::Title, 2000);
      })
      .on_update([&](StateGraph<GameScreen>&) {
          redraw(0);
          color(50, 50, 80);
          boxf();

          color(255, 255, 255);
          pos(180, 200);
          mes("HSPPP StateGraph Sample");

          color(200, 200, 200);
          pos(250, 240);
          mes("Loading...");

          // タイマー残量はサンプル表示用に state_elapsed_ms() を使う
          const int elapsed = sm.state_elapsed_ms();
          const int progress_px = (elapsed * 300) / 2000;
          color(100, 100, 100);
          boxf(170, 280, 470, 300);
          color(100, 200, 255);
          boxf(170, 280, 170 + (progress_px > 300 ? 300 : progress_px), 300);

          redraw(1);
          await(16);
      });

    // ────────────────────────────────────────────────
    // Title 画面: ボタン UI でメニュー
    // ────────────────────────────────────────────────
    sm.state(GameScreen::Title)
      .on_enter([&]() {
          objsize(200, 40);
          pos(220, 250);
          button("ゲームスタート", [&]() {
              // Game ステートの GameSceneData を初期化してジャンプ
              auto& gd = state_vars<GameSceneData>(scope, GameScreen::Game);
              gd.score = 0;
              gd.target_x = 100 + rnd(440);
              gd.target_y = 100 + rnd(280);
              gd.target_radius = 30;
              gd.active = true;
              sm.jump(GameScreen::Game);
          });

          pos(220, 300);
          button("遊び方", [&]() { sm.jump(GameScreen::HowToPlay); });

          pos(220, 350);
          button("終了", [&]() { sm.quit(); });
      })
      .on_update([&](StateGraph<GameScreen>&) {
          redraw(0);
          color(30, 30, 60);
          boxf();

          color(255, 220, 100);
          font(msgothic, 48);
          pos(180, 100);
          mes("クリックゲーム");

          font(msgothic, 16);
          color(200, 200, 200);
          pos(250, 180);
          // 旧サンプルでは g_highScore グローバルを表示していた箇所。
          // 本サンプルでは StateScope 経由で取り出す（PlayerProfile は Title に bind 済み）。
          const auto& p = scope.get<PlayerProfile>(GameScreen::Title);
          mes(strf("ハイスコア: %d", p.high_score));

          if (getkey(KEY_SPACE)) {
              auto& gd = state_vars<GameSceneData>(scope, GameScreen::Game);
              gd.score = 0;
              gd.active = true;
              sm.jump(GameScreen::Game);
          }
          if (getkey(KEY_ESCAPE)) {
              sm.quit();
          }

          redraw(1);
          await(16);
      })
      .on_exit([&]() {
          clrobj();
          font(msgothic, 16);
      });

    // ────────────────────────────────────────────────
    // 遊び方画面
    // ────────────────────────────────────────────────
    sm.state(GameScreen::HowToPlay)
      .on_enter([&]() {
          objsize(200, 40);
          pos(220, 400);
          button("戻る", [&]() { sm.back(); });
      })
      .on_update([&](StateGraph<GameScreen>&) {
          redraw(0);
          color(30, 50, 30);
          boxf();

          color(255, 255, 255);
          font(msgothic, 24);
          pos(200, 50);
          mes("遊び方");

          font(msgothic, 16);
          color(220, 220, 220);
          pos(100, 120);
          mes("・赤いターゲットをクリックでスコア加算");
          pos(100, 150);
          mes("・30 秒経過でゲームオーバー");
          pos(100, 180);
          mes("・ESCで一時停止 / タイトルに戻る");
          pos(100, 230);
          mes("操作方法:");
          pos(120, 260);
          mes("マウス左クリック: ターゲットを狙う");
          pos(120, 290);
          mes("Escキー: ポーズ");

          if (getkey(KEY_ESCAPE)) {
              sm.back();
          }

          redraw(1);
          await(16);
      })
      .on_exit([&]() {
          clrobj();
          font(msgothic, 16);
      });

    // ────────────────────────────────────────────────
    // ゲーム画面: 旧 g_score / g_targetX を StateScope ローカルに置換
    // ────────────────────────────────────────────────
    sm.state(GameScreen::Game)
      .on_enter([&]() {
          // bind は idempotent。Title から jump 経由ですでに初期化済だが、
          // 直接ジャンプされた場合の安全策として既存値取得（既存があれば args は無視）。
          auto& gd = state_vars<GameSceneData>(scope, GameScreen::Game);
          if (!gd.active) {
              gd.score = 0;
              gd.target_x = 100 + rnd(440);
              gd.target_y = 100 + rnd(280);
              gd.target_radius = 30;
              gd.active = true;
          }
          // 30 秒で GameOver へ自動遷移。
          //   - 初回 Game エントリ（Title→Game）では set_timer を呼ぶ。
          //   - Pause→Game 復帰時は、Game→Pause 遷移時に呼ばれた pause_timer()
          //     によって perform_transition の自動 cancel から保護された
          //     タイマーが残っているため、resume_timer() で再開するだけでよい
          //     （set_timer を呼ぶと accumulated_ms が 0 リセットされてしまう。
          //      review-TICKET-005 §5 R-2 / design-TICKET-010 §5.2）。
          // on_enter 実行時点では perform_transition により previous_state_ は
          // 旧 state に確定済なので、previous_state() == Pause で
          // Pause→Game 復帰を判定可能。
          const auto prev = sm.previous_state();
          if (!prev.has_value() || prev.value() != GameScreen::Pause) {
              sm.set_timer(GameScreen::GameOver, 30000);
          } else {
              // Pause→Game 復帰: pause_timer() で保持されたタイマーを再開
              // (design-TICKET-010 §5.2)
              sm.resume_timer();
          }
      })
      .on_update([&](StateGraph<GameScreen>&) {
          auto& gd = scope.get<GameSceneData>(GameScreen::Game);

          redraw(0);
          color(20, 20, 40);
          boxf();

          const int elapsed_ms = sm.state_elapsed_ms();
          int remaining = (30000 - elapsed_ms) / 1000;
          if (remaining < 0) remaining = 0;

          color(255, 255, 255);
          pos(20, 20);
          mes(strf("スコア: %d", gd.score));
          pos(520, 20);
          mes(strf("残り: %d秒", remaining));

          color(255, 80, 80);
          circle(gd.target_x - gd.target_radius, gd.target_y - gd.target_radius,
                 gd.target_x + gd.target_radius, gd.target_y + gd.target_radius, 1);

          const int mx = mousex();
          const int my = mousey();

          if (getkey(KEY_LBUTTON)) {
              const int dx = mx - gd.target_x;
              const int dy = my - gd.target_y;
              if (dx * dx + dy * dy <= gd.target_radius * gd.target_radius) {
                  gd.score += 10;
                  gd.target_x = 50 + rnd(540);
                  gd.target_y = 80 + rnd(320);
              }
          }

          color(100, 255, 100);
          circle(mx - 5, my - 5, mx + 5, my + 5, 0);

          if (getkey(KEY_ESCAPE)) {
              sm.pause_timer();   // Pause 中はカウントを止める（design §7.1 新設）
              sm.jump(GameScreen::Pause);
          }

          redraw(1);
          await(16);
      })
      .on_exit([&]() {
          auto* gd = scope.try_get<GameSceneData>(GameScreen::Game);
          if (gd) {
              gd->active = false;
          }
      });

    // ────────────────────────────────────────────────
    // Pause 画面
    // ────────────────────────────────────────────────
    sm.state(GameScreen::Pause)
      .on_enter([&]() {
          objsize(200, 40);
          pos(220, 250);
          button("ゲームに戻る", [&]() {
              // Pause→Game 復帰の resume_timer() は、復帰先 Game の on_enter
              // (`prev == Pause` 分岐) 内で行う（design-TICKET-010 §5.2）。
              // ここでは jump のみ。pause_timer() で保持されたタイマーは
              // perform_transition の auto-cancel から保護されるため、
              // jump 後の Game on_enter で resume するだけで継続できる。
              sm.jump(GameScreen::Game);
          });
          pos(220, 300);
          button("タイトルに戻る", [&]() {
              sm.cancel_timer();
              sm.jump(GameScreen::Title);
          });
      })
      .on_update([&](StateGraph<GameScreen>&) {
          redraw(0);
          color(0, 0, 0);
          boxf();

          color(255, 255, 255);
          font(msgothic, 32);
          pos(260, 150);
          mes("PAUSE");

          font(msgothic, 16);
          color(200, 200, 200);
          // ステート別変数は遷移を跨いで保持される（永続）
          if (auto* gd = scope.try_get<GameSceneData>(GameScreen::Game)) {
              pos(220, 200);
              mes(strf("現在のスコア: %d", gd->score));
          }

          redraw(1);
          await(16);
      })
      .on_exit([&]() {
          clrobj();
          font(msgothic, 16);
          // タイマー再開はボタンハンドラ側で行う（review-TICKET-005 §5 R-1）。
          // ここでは UI 後始末のみ。タイトル戻り経路では既に cancel_timer 済。
      });

    // ────────────────────────────────────────────────
    // GameOver 画面: ハイスコア更新と SaveWriter / bsave による永続化
    // ────────────────────────────────────────────────
    sm.state(GameScreen::GameOver)
      .on_enter([&]() {
          auto& p  = scope.get<PlayerProfile>(GameScreen::Title);
          auto* gd = scope.try_get<GameSceneData>(GameScreen::Game);

          if (gd && gd->score > p.high_score) {
              p.high_score = gd->score;
              logmes(strf("[NewRecord] high_score を %d に更新", p.high_score));
          }

          // ────────── セーブ ──────────
          // 旧サンプルでは保存処理なし。新 API では Serializable 型が bind されていれば
          // snapshot() でまとめてバイト列化できる（PlayerProfile のみが対象、GameSceneData は
          // 非 Serializable なので自動スキップ）。
          try {
              const auto bytes = scope.snapshot();
              const auto buf = to_uint8(std::span<const std::byte>(bytes));
              (void)bsave(kSaveFilename, buf);
              logmes(strf("[Save] %d バイトを %s に保存", static_cast<int>(buf.size()), kSaveFilename));
          }
          catch (const std::exception& e) {
              logmes(strf("[Save] 失敗: %s", e.what()));
          }

          sm.set_timer(GameScreen::Result, 3000);
      })
      .on_update([&](StateGraph<GameScreen>&) {
          redraw(0);
          color(60, 20, 20);
          boxf();

          color(255, 100, 100);
          font(msgothic, 48);
          pos(180, 150);
          mes("TIME UP!");

          font(msgothic, 24);
          color(255, 255, 255);
          if (auto* gd = scope.try_get<GameSceneData>(GameScreen::Game)) {
              pos(220, 250);
              mes(strf("スコア: %d", gd->score));

              const auto& p = scope.get<PlayerProfile>(GameScreen::Title);
              if (gd->score >= p.high_score && gd->score > 0) {
                  color(255, 220, 100);
                  pos(200, 300);
                  mes("★ NEW RECORD! ★");
              }
          }

          redraw(1);
          await(16);
      })
      .on_exit([&]() {
          font(msgothic, 16);
      });

    // ────────────────────────────────────────────────
    // Result 画面
    // ────────────────────────────────────────────────
    sm.state(GameScreen::Result)
      .on_enter([&]() {
          objsize(200, 40);
          pos(220, 350);
          button("もう一度", [&]() {
              auto& gd = state_vars<GameSceneData>(scope, GameScreen::Game);
              gd.score = 0;
              gd.active = true;
              sm.jump(GameScreen::Game);
          });
          pos(220, 400);
          button("タイトルへ", [&]() { sm.jump(GameScreen::Title); });
      })
      .on_update([&](StateGraph<GameScreen>&) {
          redraw(0);
          color(30, 30, 50);
          boxf();

          color(255, 255, 255);
          font(msgothic, 32);
          pos(250, 80);
          mes("RESULT");

          font(msgothic, 24);
          int score_now = 0;
          if (auto* gd = scope.try_get<GameSceneData>(GameScreen::Game)) {
              score_now = gd->score;
          }
          pos(200, 160);
          mes(strf("今回のスコア: %d", score_now));

          color(255, 220, 100);
          const auto& p = scope.get<PlayerProfile>(GameScreen::Title);
          pos(200, 210);
          mes(strf("ハイスコア: %d", p.high_score));

          font(msgothic, 20);
          color(200, 200, 255);
          pos(220, 280);
          if (score_now >= 200)      mes("評価: ★★★ すばらしい！");
          else if (score_now >= 100) mes("評価: ★★ いい調子！");
          else if (score_now >= 50)  mes("評価: ★ がんばろう！");
          else                       mes("評価: もっと練習！");

          redraw(1);
          await(16);
      })
      .on_exit([&]() {
          clrobj();
          font(msgothic, 16);
      });

    // ────────────────────────────────────────────────
    // 起動: Splash から開始
    // ────────────────────────────────────────────────
    sm.start(GameScreen::Splash);
    sm.run();

    // 終了時に遷移グラフを出力（開発用）
    sm.export_graph("state_graph.dot");
    end();
}
