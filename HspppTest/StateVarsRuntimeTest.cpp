// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppTest/StateVarsRuntimeTest.cpp
// HSPPP hsppp:state_vars / hsppp:savedata runtime verification
//
// Observation IDs:
//   1. StateScope::bind idempotent (re-bind returns existing slot, ignores args)
//   2. snapshot -> restore roundtrip across multiple states and types
//   3. partial restore (keys absent from snapshot keep current value)
//   4. SaveReader exceptions (magic / version / trailing / missing key / type_tag)
//   5. StateScope::restore type_tag mismatch -> HspError(ERR_TYPE_MISMATCH)
//   6. release / release_all_for / release_all then try_get == nullptr
//   7. non-Serializable bind -> enumerate() reports serializable=false
//
// Coding rules (CLAUDE.md): import hsppp only, no #include, no assert,
//                            no ANSI APIs, no exception swallowing.

import hsppp;

import <vector>;
import <span>;
import <cstddef>;
import <string>;
import <string_view>;
import <stdexcept>;

namespace hsppp_test {

    // Counters independent from ApiRuntimeTest's globals.
    static int s_svPassed = 0;
    static int s_svFailed = 0;
    static int s_svLastFailedId = 0;

    inline void sv_check(bool cond, int observation_id) {
        if (cond) {
            ++s_svPassed;
        }
        else {
            ++s_svFailed;
            s_svLastFailedId = observation_id;
        }
    }

    // Test state enum.
    enum class TestState : int {
        Title = 0,
        Game  = 1,
        Pause = 2,
    };

    // Serializable type #1.
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

    // Serializable type #2 (different type, different state).
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

    // Same payload, different type_tag (used for SaveReader::read mismatch).
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

    // Non-Serializable type.
    struct OpaqueHandle {
        int  raw      = 0;
        bool attached = false;
    };

    static_assert(hsppp::Serializable<PlayerScore>,    "PlayerScore must satisfy Serializable");
    static_assert(hsppp::Serializable<PauseCursor>,    "PauseCursor must satisfy Serializable");
    static_assert(hsppp::Serializable<PlayerScoreV2>,  "PlayerScoreV2 must satisfy Serializable");
    static_assert(!hsppp::Serializable<OpaqueHandle>,  "OpaqueHandle must NOT satisfy Serializable");

    // -----------------------------------------------------------
    // Observation 1: bind idempotent
    // -----------------------------------------------------------
    static void sv_test_bind_idempotent() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);

        auto& a = scope.bind<PlayerScore>(TestState::Game, 100, 50);
        sv_check(a.score == 100, 1);
        sv_check(a.hp    == 50,  1);

        // Re-bind with the same key returns the existing slot. Args ignored.
        auto& b = scope.bind<PlayerScore>(TestState::Game, 999, 999);
        sv_check(&a == &b,       1);
        sv_check(b.score == 100, 1);
        sv_check(b.hp    == 50,  1);

        // Mutating through one reference is visible through the other.
        b.score = 12345;
        sv_check(a.score == 12345, 1);
        sv_check(scope.size() == 1u, 1);
    }

    // -----------------------------------------------------------
    // Observation 2: snapshot -> restore roundtrip
    // -----------------------------------------------------------
    static void sv_test_snapshot_restore_roundtrip() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);

        scope.bind<PlayerScore>(TestState::Game,  100, 30).score = 7777;
        scope.bind<PlayerScore>(TestState::Title, 0,   0).hp    = 11;
        scope.bind<PauseCursor>(TestState::Pause).index = 3;

        const auto blob = scope.snapshot();
        sv_check(!blob.empty(), 2);

        // Pre-bind on the target scope is required (design 8.3).
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
    // Observation 3: partial restore
    // -----------------------------------------------------------
    static void sv_test_partial_restore() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);
        scope.bind<PlayerScore>(TestState::Game, 100, 50).score = 42;
        const auto blob = scope.snapshot();

        hsppp::StateGraph<TestState> sm2;
        hsppp::StateScope<TestState> scope2(sm2);
        scope2.bind<PlayerScore>(TestState::Game);            // present in snapshot
        scope2.bind<PauseCursor>(TestState::Pause).index = 9; // NOT in snapshot -> must stay
        scope2.restore(std::span<const std::byte>(blob.data(), blob.size()));

        sv_check(scope2.get<PlayerScore>(TestState::Game).score  == 42, 3);
        sv_check(scope2.get<PauseCursor>(TestState::Pause).index == 9,  3);
    }

    // -----------------------------------------------------------
    // Observation 4: SaveReader exception cases
    // -----------------------------------------------------------
    static void sv_test_savereader_exceptions() {
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
            for (int i = 0; i < 4; ++i) bad.push_back(std::byte{ 0 });  // block_count=0

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

        // 4-c) trailing bytes after blocks -> HspError(ERR_OUT_OF_RANGE)
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
    // Observation 5: StateScope::restore type_tag mismatch
    //   StateScope's block_key format is "sv:{state_index}:{type_tag}".
    //   Inject a payload with the expected key but a different type_tag.
    // -----------------------------------------------------------
    static void sv_test_scope_restore_type_tag_mismatch() {
        hsppp::StateGraph<TestState> sm;
        hsppp::StateScope<TestState> scope(sm);
        scope.bind<PlayerScore>(TestState::Game);

        hsppp::SaveWriter w;
        std::vector<std::byte> payload;
        PlayerScore tmp{ 1, 2 };
        PlayerScore::serialize(tmp, payload);

        // state_index for TestState::Game is 1; expected type_tag "TestPlayerScore/v1".
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
    // Observation 6: release family
    // -----------------------------------------------------------
    static void sv_test_release_family() {
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
    // Observation 7: non-Serializable type appears in enumerate
    // -----------------------------------------------------------
    static void sv_test_non_serializable_enumerate() {
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

        // snapshot must skip non-serializable entries silently.
        const auto blob = scope.snapshot();
        sv_check(!blob.empty(), 7);
    }

    // -----------------------------------------------------------
    // Entry points
    // -----------------------------------------------------------
    int run_state_vars_tests() {
        s_svPassed = 0;
        s_svFailed = 0;
        s_svLastFailedId = 0;

        sv_test_bind_idempotent();                 // 1
        sv_test_snapshot_restore_roundtrip();      // 2
        sv_test_partial_restore();                 // 3
        sv_test_savereader_exceptions();           // 4
        sv_test_scope_restore_type_tag_mismatch(); // 5
        sv_test_release_family();                  // 6
        sv_test_non_serializable_enumerate();      // 7

        return s_svPassed;
    }

    int get_state_vars_failed_count() {
        return s_svFailed;
    }

    int get_state_vars_passed_count() {
        return s_svPassed;
    }

    int get_state_vars_last_failed_id() {
        return s_svLastFailedId;
    }

}  // namespace hsppp_test
