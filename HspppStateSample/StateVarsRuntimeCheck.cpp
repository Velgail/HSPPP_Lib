// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppStateSample/StateVarsRuntimeCheck.cpp
// ─────────────────────────────────────────────────────────────────
// hsppp:state_vars / hsppp:savedata の手動目視検証（HspppStateSample 内蔵）
//
// 目的:
//   StateScope / state_vars / SaveWriter / SaveReader の主要な実行時挙動を
//   サンプル起動 1 回によって目視確認可能にする。
//
//   ライブラリ側責務として「コードで動いていることを画面で証明する」ため、
//   HspppStateSample の Title メニューから到達できる専用画面（GameScreen::
//   StateVarsCheck）が本ファイルに定義された run_all() を呼び、観測 1〜8 の
//   各 PASS/FAIL を画面上にカラー表示する。
//
// 観測 ID とその意味:
//   1. StateScope::bind が idempotent（同一キー再 bind で既存スロット返却・引数無視）
//   2. snapshot -> restore のラウンドトリップ（複数 state / 型）
//   3. partial restore（snapshot に無いキーは現在値を維持）
//   4. SaveReader の例外系（bad magic / version / trailing bytes / missing key / type_tag mismatch）
//   5. StateScope::restore の type_tag mismatch -> HspError(ERR_TYPE_MISMATCH)
//   6. release / release_all_for / release_all 後の try_get == nullptr
//   7. 非 Serializable 型を bind しても enumerate() で serializable=false として現れる
//   8. StateScopeReadView の API 面（const アクセサのみ・mutation API なし）
//
// 設計補足:
//   - 本ファイルが行うのは状態機械の構築・bind・snapshot・restore のみで、
//     画面描画は一切行わない。テストごとに独自の StateGraph<TestState> /
//     StateScope<TestState> を構築するため、StateSampleMain.cpp が保持する
//     GameScreen 系の scope/sm とは干渉しない。
//   - 例外を握りつぶさない（CLAUDE.md 規約）。期待される例外は型と error_code を
//     確認し、想定外の例外は throw のまま伝播させる。
//   - assert / ANSI API / #include は使用禁止（CLAUDE.md 規約）。
//   - header unit の明示 import は build-config.md §4.5 に従う。

import hsppp;

import <vector>;
import <span>;
import <cstddef>;
import <string>;
import <string_view>;
import <stdexcept>;

namespace hsppp_state_sample::vars_check {

    // ─────────────────────────────
    // 観測カウンタ
    // ─────────────────────────────
    constexpr int kObservationCount = 8;

    // 観測 i (0..7) の PASS/FAIL 判定: -1=未実行 / 0=FAIL / 1=PASS。
    // 観測内のチェックは複数回呼ばれるが、1 つでも失敗したら 0、全成功なら 1。
    static int s_obs_state[kObservationCount];

    static int s_passed = 0;            // 個別 sv_check の合計成功数
    static int s_failed = 0;            // 個別 sv_check の合計失敗数
    static int s_last_failed_id = 0;    // 直近で失敗した観測 ID

    static void reset_counters() noexcept {
        for (int i = 0; i < kObservationCount; ++i) {
            s_obs_state[i] = -1;
        }
        s_passed = 0;
        s_failed = 0;
        s_last_failed_id = 0;
    }

    static void mark_observation_started(int observation_id) noexcept {
        const int idx = observation_id - 1;
        if (idx >= 0 && idx < kObservationCount && s_obs_state[idx] < 0) {
            s_obs_state[idx] = 1;  // 暫定 PASS。失敗時に 0 に落とす。
        }
    }

    inline void sv_check(bool cond, int observation_id) {
        mark_observation_started(observation_id);
        const int idx = observation_id - 1;
        if (cond) {
            ++s_passed;
        }
        else {
            ++s_failed;
            s_last_failed_id = observation_id;
            if (idx >= 0 && idx < kObservationCount) {
                s_obs_state[idx] = 0;
            }
        }
    }

    // ─────────────────────────────
    // 試験用ステート enum
    // ─────────────────────────────
    enum class TestState : int {
        Title = 0,
        Game  = 1,
        Pause = 2,
    };

    // Serializable 型 #1
    struct PlayerScore {
        int score = 0;
        int hp    = 0;

        static std::string_view type_tag() noexcept { return "TestPlayerScore/v1"; }

        static void serialize(const PlayerScore& v, std::vector<std::byte>& out) {
            auto push_i32 = [&](int x) {
                const unsigned u = static_cast<unsigned>(x);
                out.push_back(static_cast<std::byte>(u & 0xFFu));
                out.push_back(static_cast<std::byte>((u >> 8) & 0xFFu));
                out.push_back(static_cast<std::byte>((u >> 16) & 0xFFu));
                out.push_back(static_cast<std::byte>((u >> 24) & 0xFFu));
            };
            push_i32(v.score);
            push_i32(v.hp);
        }

        static std::size_t deserialize(std::span<const std::byte> in, PlayerScore& dst) {
            auto pop_i32 = [&](std::size_t off) {
                unsigned u = 0;
                u |= static_cast<unsigned>(std::to_integer<unsigned char>(in[off + 0]));
                u |= static_cast<unsigned>(std::to_integer<unsigned char>(in[off + 1])) << 8;
                u |= static_cast<unsigned>(std::to_integer<unsigned char>(in[off + 2])) << 16;
                u |= static_cast<unsigned>(std::to_integer<unsigned char>(in[off + 3])) << 24;
                return static_cast<int>(u);
            };
            dst.score = pop_i32(0);
            dst.hp    = pop_i32(4);
            return 8;
        }
    };

    // Serializable 型 #2
    struct PauseCursor {
        int index = 0;

        static std::string_view type_tag() noexcept { return "TestPauseCursor/v1"; }

        static void serialize(const PauseCursor& v, std::vector<std::byte>& out) {
            const unsigned u = static_cast<unsigned>(v.index);
            out.push_back(static_cast<std::byte>(u & 0xFFu));
            out.push_back(static_cast<std::byte>((u >> 8) & 0xFFu));
            out.push_back(static_cast<std::byte>((u >> 16) & 0xFFu));
            out.push_back(static_cast<std::byte>((u >> 24) & 0xFFu));
        }

        static std::size_t deserialize(std::span<const std::byte> in, PauseCursor& dst) {
            unsigned u = 0;
            u |= static_cast<unsigned>(std::to_integer<unsigned char>(in[0]));
            u |= static_cast<unsigned>(std::to_integer<unsigned char>(in[1])) << 8;
            u |= static_cast<unsigned>(std::to_integer<unsigned char>(in[2])) << 16;
            u |= static_cast<unsigned>(std::to_integer<unsigned char>(in[3])) << 24;
            dst.index = static_cast<int>(u);
            return 4;
        }
    };

    // 同一ペイロード / 異なる type_tag（SaveReader::read mismatch 検証用）
    struct PlayerScoreV2 {
        int score = 0;
        int hp    = 0;

        static std::string_view type_tag() noexcept { return "TestPlayerScore/v2"; }

        static void serialize(const PlayerScoreV2& v, std::vector<std::byte>& out) {
            PlayerScore tmp{ v.score, v.hp };
            PlayerScore::serialize(tmp, out);
        }
        static std::size_t deserialize(std::span<const std::byte> in, PlayerScoreV2& dst) {
            PlayerScore tmp{};
            const auto n = PlayerScore::deserialize(in, tmp);
            dst.score = tmp.score;
            dst.hp    = tmp.hp;
            return n;
        }
    };

    // 非 Serializable 型
    struct OpaqueHandle {
        int  raw      = 0;
        bool attached = false;
    };

    static_assert(hsppp::Serializable<PlayerScore>,    "PlayerScore must satisfy Serializable");
    static_assert(hsppp::Serializable<PauseCursor>,    "PauseCursor must satisfy Serializable");
    static_assert(hsppp::Serializable<PlayerScoreV2>,  "PlayerScoreV2 must satisfy Serializable");
    static_assert(!hsppp::Serializable<OpaqueHandle>,  "OpaqueHandle must NOT satisfy Serializable");

    template <typename TView>
    concept StateVarsReadViewHasBind = requires(TView& view) {
        view.template bind<PlayerScore>(TestState::Title);
    };

    template <typename TView>
    concept StateVarsReadViewHasRelease = requires(TView& view) {
        view.template release<PlayerScore>(TestState::Title);
    };

    template <typename TView>
    concept StateVarsReadViewHasReleaseAllFor = requires(TView& view) {
        view.release_all_for(TestState::Title);
    };

    template <typename TView>
    concept StateVarsReadViewHasReleaseAll = requires(TView& view) {
        view.release_all();
    };

    template <typename TView>
    concept StateVarsReadViewAllowsPayloadMutation = requires(TView& view) {
        view.template get<PlayerScore>(TestState::Title).score = 0;
    };

    // -----------------------------------------------------------
    // 観測 1: bind idempotent
    // -----------------------------------------------------------
    static void obs1_bind_idempotent() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);

        auto& a = scope.bind<PlayerScore>(TestState::Game, 100, 50);
        sv_check(a.score == 100, 1);
        sv_check(a.hp    == 50,  1);

        auto& b = scope.bind<PlayerScore>(TestState::Game, 999, 999);
        sv_check(&a == &b,       1);
        sv_check(b.score == 100, 1);
        sv_check(b.hp    == 50,  1);

        b.score = 12345;
        sv_check(a.score == 12345, 1);
        sv_check(scope.size() == 1u, 1);
    }

    // -----------------------------------------------------------
    // 観測 2: snapshot -> restore roundtrip
    // -----------------------------------------------------------
    static void obs2_snapshot_restore_roundtrip() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);

        scope.bind<PlayerScore>(TestState::Game,  100, 30).score = 7777;
        scope.bind<PlayerScore>(TestState::Title, 0,   0).hp    = 11;
        scope.bind<PauseCursor>(TestState::Pause).index = 3;

        const auto blob = scope.snapshot();
        sv_check(!blob.empty(), 2);

        hsppp::StateGraph<TestState> sm2;
        hsppp::StateScope<TestState> scope2(sm2);
        scope2.bind<PlayerScore>(TestState::Game);
        scope2.bind<PlayerScore>(TestState::Title);
        scope2.bind<PauseCursor>(TestState::Pause);

        scope2.restore(std::span<const std::byte>(blob.data(), blob.size()));

        sv_check(scope2.get<PlayerScore>(TestState::Game).score  == 7777, 2);
        sv_check(scope2.get<PlayerScore>(TestState::Game).hp     == 30,   2);
        sv_check(scope2.get<PlayerScore>(TestState::Title).score == 0,    2);
        sv_check(scope2.get<PlayerScore>(TestState::Title).hp    == 11,   2);
        sv_check(scope2.get<PauseCursor>(TestState::Pause).index == 3,    2);
    }

    // -----------------------------------------------------------
    // 観測 3: partial restore
    // -----------------------------------------------------------
    static void obs3_partial_restore() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);
        scope.bind<PlayerScore>(TestState::Game, 100, 50).score = 42;
        const auto blob = scope.snapshot();

        hsppp::StateGraph<TestState> sm2;
        hsppp::StateScope<TestState> scope2(sm2);
        scope2.bind<PlayerScore>(TestState::Game);
        scope2.bind<PauseCursor>(TestState::Pause).index = 9;
        scope2.restore(std::span<const std::byte>(blob.data(), blob.size()));

        sv_check(scope2.get<PlayerScore>(TestState::Game).score  == 42, 3);
        sv_check(scope2.get<PauseCursor>(TestState::Pause).index == 9,  3);
    }

    // -----------------------------------------------------------
    // 観測 4: SaveReader 例外
    // -----------------------------------------------------------
    static void obs4_savereader_exceptions() {
        // 4-a) bad magic -> HspError(ERR_TYPE_MISMATCH)
        {
            std::vector<std::byte> bad;
            const char mg[4] = { 'X', 'X', 'X', 'X' };
            for (int i = 0; i < 4; ++i)
                bad.push_back(static_cast<std::byte>(static_cast<unsigned char>(mg[i])));
            const unsigned vers[2] = { 1u, 0u };
            for (int j = 0; j < 2; ++j) {
                const unsigned v = vers[j];
                bad.push_back(static_cast<std::byte>(v & 0xFFu));
                bad.push_back(static_cast<std::byte>((v >> 8) & 0xFFu));
                bad.push_back(static_cast<std::byte>((v >> 16) & 0xFFu));
                bad.push_back(static_cast<std::byte>((v >> 24) & 0xFFu));
            }
            bool got = false;
            int  ec  = 0;
            try {
                hsppp::SaveReader r(std::span<const std::byte>(bad.data(), bad.size()));
                (void)r;
            }
            catch (const hsppp::HspError& e) {
                got = true;
                ec  = e.error_code();
            }
            sv_check(got, 4);
            sv_check(ec == hsppp::ERR_TYPE_MISMATCH, 4);
        }

        // 4-b) wrong version -> HspError(ERR_UNSUPPORTED)
        {
            std::vector<std::byte> bad;
            const char mg[4] = { 'H', 'S', 'P', 'P' };
            for (int i = 0; i < 4; ++i)
                bad.push_back(static_cast<std::byte>(static_cast<unsigned char>(mg[i])));
            const unsigned ver = 9999u;
            bad.push_back(static_cast<std::byte>(ver & 0xFFu));
            bad.push_back(static_cast<std::byte>((ver >> 8) & 0xFFu));
            bad.push_back(static_cast<std::byte>((ver >> 16) & 0xFFu));
            bad.push_back(static_cast<std::byte>((ver >> 24) & 0xFFu));
            for (int i = 0; i < 4; ++i) bad.push_back(std::byte{ 0 });

            bool got = false;
            int  ec  = 0;
            try {
                hsppp::SaveReader r(std::span<const std::byte>(bad.data(), bad.size()));
                (void)r;
            }
            catch (const hsppp::HspError& e) {
                got = true;
                ec  = e.error_code();
            }
            sv_check(got, 4);
            sv_check(ec == hsppp::ERR_UNSUPPORTED, 4);
        }

        // 4-c) trailing bytes -> HspError(ERR_OUT_OF_RANGE)
        {
            hsppp::SaveWriter w;
            PlayerScore ps{ 1, 2 };
            w.write<PlayerScore>("k", ps);
            auto blob = w.finalize();
            blob.push_back(std::byte{ 0xAB });

            bool got = false;
            int  ec  = 0;
            try {
                hsppp::SaveReader r(std::span<const std::byte>(blob.data(), blob.size()));
                (void)r;
            }
            catch (const hsppp::HspError& e) {
                got = true;
                ec  = e.error_code();
            }
            sv_check(got, 4);
            sv_check(ec == hsppp::ERR_OUT_OF_RANGE, 4);
        }

        // 4-d) missing key -> std::out_of_range
        {
            hsppp::SaveWriter w;
            PlayerScore ps{ 1, 2 };
            w.write<PlayerScore>("k1", ps);
            const auto blob = w.finalize();
            hsppp::SaveReader r(std::span<const std::byte>(blob.data(), blob.size()));

            bool got = false;
            try {
                (void)r.read<PlayerScore>("missing");
            }
            catch (const std::out_of_range&) {
                got = true;
            }
            sv_check(got, 4);
            sv_check(!r.has("missing"), 4);
        }

        // 4-e) type_tag mismatch -> HspError(ERR_TYPE_MISMATCH)
        {
            hsppp::SaveWriter w;
            PlayerScore ps{ 1, 2 };
            w.write<PlayerScore>("same_key", ps);
            const auto blob = w.finalize();
            hsppp::SaveReader r(std::span<const std::byte>(blob.data(), blob.size()));

            bool got = false;
            int  ec  = 0;
            try {
                (void)r.read<PlayerScoreV2>("same_key");
            }
            catch (const hsppp::HspError& e) {
                got = true;
                ec  = e.error_code();
            }
            sv_check(got, 4);
            sv_check(ec == hsppp::ERR_TYPE_MISMATCH, 4);
        }
    }

    // -----------------------------------------------------------
    // 観測 5: StateScope::restore type_tag mismatch
    //   StateScope の block_key 形式は "sv:{state_index}:{type_tag}"。
    //   想定 key で、type_tag のみ別物のペイロードを注入する。
    // -----------------------------------------------------------
    static void obs5_scope_restore_type_tag_mismatch() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);
        scope.bind<PlayerScore>(TestState::Game);

        hsppp::SaveWriter w;
        std::vector<std::byte> payload;
        PlayerScore tmp{ 1, 2 };
        PlayerScore::serialize(tmp, payload);

        // state_index for TestState::Game is 1; 期待 type_tag は "TestPlayerScore/v1"
        w.write_raw(std::string_view("sv:1:TestPlayerScore/v1"),
                    std::string_view("DIFFERENT_TAG"),
                    std::span<const std::byte>(payload.data(), payload.size()));
        const auto blob = w.finalize();

        bool got = false;
        int  ec  = 0;
        try {
            scope.restore(std::span<const std::byte>(blob.data(), blob.size()));
        }
        catch (const hsppp::HspError& e) {
            got = true;
            ec  = e.error_code();
        }
        sv_check(got, 5);
        sv_check(ec == hsppp::ERR_TYPE_MISMATCH, 5);
    }

    // -----------------------------------------------------------
    // 観測 6: release family
    // -----------------------------------------------------------
    static void obs6_release_family() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);

        scope.bind<PlayerScore>(TestState::Game,  1, 2);
        scope.bind<PauseCursor>(TestState::Game).index = 5;
        scope.bind<PlayerScore>(TestState::Title, 9, 9);
        scope.bind<PauseCursor>(TestState::Pause).index = 7;
        sv_check(scope.size() == 4u, 6);

        scope.release<PlayerScore>(TestState::Game);
        sv_check(scope.try_get<PlayerScore>(TestState::Game) == nullptr, 6);
        sv_check(scope.try_get<PauseCursor>(TestState::Game) != nullptr, 6);
        sv_check(scope.size() == 3u, 6);

        scope.release_all_for(TestState::Game);
        sv_check(scope.try_get<PauseCursor>(TestState::Game) == nullptr, 6);
        sv_check(scope.try_get<PlayerScore>(TestState::Title) != nullptr, 6);
        sv_check(scope.try_get<PauseCursor>(TestState::Pause) != nullptr, 6);
        sv_check(scope.size() == 2u, 6);

        scope.release_all();
        sv_check(scope.size() == 0u, 6);
        sv_check(scope.try_get<PlayerScore>(TestState::Title) == nullptr, 6);
        sv_check(scope.try_get<PauseCursor>(TestState::Pause) == nullptr, 6);
    }

    // -----------------------------------------------------------
    // 観測 7: enumerate に非 Serializable 型も現れる（serializable=false）
    // -----------------------------------------------------------
    static void obs7_non_serializable_enumerate() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);

        scope.bind<PlayerScore>(TestState::Game, 1, 2);
        scope.bind<OpaqueHandle>(TestState::Pause);

        const auto entries = scope.enumerate();
        sv_check(entries.size() == 2u, 7);

        bool found_ser     = false;
        bool found_non_ser = false;
        for (const auto& e : entries) {
            if (e.serializable) {
                found_ser = true;
                sv_check(e.type_tag == std::string("TestPlayerScore/v1"), 7);
            }
            else {
                found_non_ser = true;
                sv_check(e.type_tag.empty(), 7);
            }
        }
        sv_check(found_ser,     7);
        sv_check(found_non_ser, 7);

        const auto blob = scope.snapshot();
        sv_check(!blob.empty(), 7);
    }

    // -----------------------------------------------------------
    // 観測 8: StateScopeReadView の read-only API 面
    // -----------------------------------------------------------
    static void obs8_read_view_read_only_surface() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);

        scope.bind<PlayerScore>(TestState::Title, 12, 34);
        auto view = scope.read_view();

        static_assert(!StateVarsReadViewHasBind<decltype(view)>);
        static_assert(!StateVarsReadViewHasRelease<decltype(view)>);
        static_assert(!StateVarsReadViewHasReleaseAllFor<decltype(view)>);
        static_assert(!StateVarsReadViewHasReleaseAll<decltype(view)>);
        static_assert(!StateVarsReadViewAllowsPayloadMutation<decltype(view)>);

        const auto& score = view.get<PlayerScore>(TestState::Title);
        sv_check(score.score == 12, 8);
        sv_check(score.hp == 34, 8);
        sv_check(view.try_get<PlayerScore>(TestState::Title) == &score, 8);
        sv_check(view.contains<PlayerScore>(TestState::Title), 8);
        sv_check(view.enumerate().size() == 1u, 8);
    }

    // -----------------------------------------------------------
    // 公開 API（StateSampleMain.cpp の StateVarsCheck 画面から呼ばれる）
    // -----------------------------------------------------------
    int observation_count() noexcept { return kObservationCount; }

    const char* observation_name(int observation_id) noexcept {
        switch (observation_id) {
        case 1: return "bind idempotent";
        case 2: return "snapshot/restore roundtrip";
        case 3: return "partial restore";
        case 4: return "SaveReader exceptions";
        case 5: return "Scope restore type_tag mismatch";
        case 6: return "release family";
        case 7: return "non-Serializable enumerate";
        case 8: return "ReadView read-only surface";
        default: return "(unknown)";
        }
    }

    // 観測 i (1..8) の現在状態: -1=未実行 / 0=FAIL / 1=PASS
    int observation_state(int observation_id) noexcept {
        const int idx = observation_id - 1;
        if (idx < 0 || idx >= kObservationCount) return -1;
        return s_obs_state[idx];
    }

    int run_all() {
        reset_counters();

        obs1_bind_idempotent();
        obs2_snapshot_restore_roundtrip();
        obs3_partial_restore();
        obs4_savereader_exceptions();
        obs5_scope_restore_type_tag_mismatch();
        obs6_release_family();
        obs7_non_serializable_enumerate();
        obs8_read_view_read_only_surface();

        return s_failed;
    }

    int passed_count() noexcept { return s_passed; }
    int failed_count() noexcept { return s_failed; }
    int last_failed_id() noexcept { return s_last_failed_id; }

}  // namespace hsppp_state_sample::vars_check
