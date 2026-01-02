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
using namespace hsppp;

// テストモジュールからインポート
namespace hsppp_test {
    bool run_compile_tests();
    int run_runtime_tests();
    int get_failed_count();
    int get_passed_count();
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

    // 結果を表示して待機
    await(10000);  // 10秒待機

    end( (compileOk && runtimeFailed == 0) ? 0 : 1);
    return 0;
}
