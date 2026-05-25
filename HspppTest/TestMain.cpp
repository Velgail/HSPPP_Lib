// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppTest/TestMain.cpp
// ═══════════════════════════════════════════════════════════════════
// HSPPP テストランナー
// ═══════════════════════════════════════════════════════════════════

import hsppp;
import <string>;
using namespace hsppp;

// テストモジュールからインポート
namespace hsppp_test {
    bool run_compile_tests();
    int run_runtime_tests();
    int get_failed_count();
    int get_passed_count();

    // StateVarsRuntimeTest.cpp（state_vars / savedata ランタイム検証）
    int run_state_vars_tests();
    int get_state_vars_failed_count();
    int get_state_vars_passed_count();
    int get_state_vars_last_failed_id();

    // ApiRuntimeTest.cpp 拡張診断
    int get_first_failed_index();
    const char* get_first_failed_name();
}

// ユーザーのエントリーポイント（テスト実行用）
void hspMain() {
    // テスト結果表示用ウィンドウ
    auto resultWin = screen({.width = 500, .height = 400, .title = "HSPPP Test Results"});
    resultWin.color(240, 240, 240).boxf();
    resultWin.color(0, 0, 0);

    int y = 20;
    auto printLine = [&](const char* text) {
        resultWin.pos(20, y);
        resultWin.mes(text);
        y += 20;
    };

    printLine("═══════════════════════════════════════");
    printLine("     HSPPP API Test Suite");
    printLine("═══════════════════════════════════════");
    y += 10;

    // コンパイルテスト
    printLine("[1] Compile Tests (API signatures)");
    bool compileOk = hsppp_test::run_compile_tests();
    if (compileOk) {
        resultWin.color(0, 128, 0);
        printLine("    ✓ PASSED - All APIs compile correctly");
    } else {
        resultWin.color(255, 0, 0);
        printLine("    ✗ FAILED - Compilation issues detected");
    }
    resultWin.color(0, 0, 0);
    y += 10;

    // ランタイムテスト
    printLine("[2] Runtime Tests (API execution)");
    int runtimePassed = hsppp_test::run_runtime_tests();
    int runtimeFailed = hsppp_test::get_failed_count();

    if (runtimeFailed == 0) {
        resultWin.color(0, 128, 0);
        // snprintf の代わりに単純な文字列
        printLine("    ✓ PASSED - All runtime tests passed");
    } else {
        resultWin.color(255, 0, 0);
        printLine("    ✗ FAILED - Some runtime tests failed");
    }
    resultWin.color(0, 0, 0);
    y += 10;

    // state_vars / savedata ランタイムテスト
    printLine("[3] StateVars/SaveData Runtime Tests");
    int svPassed = hsppp_test::run_state_vars_tests();
    int svFailed = hsppp_test::get_state_vars_failed_count();
    runtimePassed += svPassed;
    runtimeFailed += svFailed;

    if (svFailed == 0) {
        resultWin.color(0, 128, 0);
        printLine("    ✓ PASSED - state_vars/savedata observations 1-7");
    } else {
        resultWin.color(255, 0, 0);
        printLine("    ✗ FAILED - state_vars/savedata observation(s) failed");
    }
    resultWin.color(0, 0, 0);

    y += 10;
    printLine("───────────────────────────────────────");
    
    // サマリー
    resultWin.pos(20, y);
    resultWin.mes("Summary:");
    y += 20;

    resultWin.pos(40, y);
    if (runtimePassed > 0) {
        resultWin.color(0, 128, 0);
        resultWin.mes("Passed tests: OK");
    }
    y += 20;

    resultWin.pos(40, y);
    if (runtimeFailed > 0) {
        resultWin.color(255, 0, 0);
        resultWin.mes("Failed tests: SOME FAILURES");
    } else {
        resultWin.color(0, 128, 0);
        resultWin.mes("Failed tests: 0");
    }
    y += 30;

    resultWin.color(0, 0, 0);
    printLine("───────────────────────────────────────");

    // 最終結果
    if (compileOk && runtimeFailed == 0) {
        resultWin.color(0, 128, 0);
        printLine("  ★ ALL TESTS PASSED ★");
    } else {
        resultWin.color(255, 0, 0);
        printLine("  ✗ SOME TESTS FAILED");
    }

    y += 20;
    resultWin.color(128, 128, 128);
    printLine("Press any key or close window to exit...");

    // ─────────────────────────────────────────────────────────────────
    // ハーネス健全化の一環としてテスト結果を text file へ
    // 永続化する（GUI を観測できない CI / 自動テスト環境向け診断出力）。
    // std::fstream header unit は環境依存で解決不能なため、既存の UTF-16 Win32
    // file API 経由で実装されている bsave を使う。
    // 出力先: 実行時 CWD 直下 "hsppp_test_result.txt"
    // ─────────────────────────────────────────────────────────────────
    {
        std::string result = "HSPPP Test Suite Result\n";
        result += "  compile_block_ok = ";
        result += (compileOk ? "true" : "false");
        result += "\n";
        result += "  runtime_passed   = " + std::to_string(runtimePassed - svPassed) + "\n";
        result += "  runtime_failed   = " + std::to_string(runtimeFailed - svFailed) + "\n";
        result += "  sv_passed        = " + std::to_string(svPassed) + "\n";
        result += "  sv_failed        = " + std::to_string(svFailed) + "\n";
        result += "  sv_last_failed_id= " + std::to_string(hsppp_test::get_state_vars_last_failed_id()) + "\n";
        result += "  rt_first_failed_index = " + std::to_string(hsppp_test::get_first_failed_index()) + "\n";
        result += "  rt_first_failed_name  = ";
        result += hsppp_test::get_first_failed_name();
        result += "\n";
        result += "  total_passed     = " + std::to_string(runtimePassed) + "\n";
        result += "  total_failed     = " + std::to_string(runtimeFailed) + "\n";
        result += "  exit_code        = " + std::to_string((compileOk && runtimeFailed == 0) ? 0 : 1) + "\n";

        bsave("hsppp_test_result.txt", result);
    }

    // 結果を表示して待機
    await(10000);  // 10秒待機

    end( (compileOk && runtimeFailed == 0) ? 0 : 1);
}
