// HspppSample/UserApp.cpp
import hsppp;
using namespace hsppp;

// ユーザーのエントリーポイント
int hspMain() {
    // ═══════════════════════════════════════════════════════════
    // HSPPPの2つのスタイル
    // ═══════════════════════════════════════════════════════════
    //
    // 【スタイル1: HSP互換（グローバル関数）】
    //   screen(omit, 800, 600);
    //   color(255, 255, 255);
    //   boxf();
    //
    // 【スタイル2: オブジェクト指向（. でアクセス）】
    //   auto mainscr = screen(omit, 800, 600);
    //   mainscr.color(255, 255, 255);
    //   mainscr.boxf();
    //
    // どちらも混在可能！
    // ═══════════════════════════════════════════════════════════

    // オブジェクト指向スタイル: auto で Screen を受け取る
    auto mainscr = screen(omit, 800, 600, {}, {}, {}, {}, {}, "HSPPP - Object Style Demo");

    // メソッドチェーンも可能！
    mainscr.redraw(0)
           .color(255, 255, 255)
           .boxf()
           .color(0, 0, 0)
           .boxf(50, 50, 200, 150);

    // 個別に呼び出すことも可能
    mainscr.color(255, 0, 0);
    mainscr.pos(100, 200);
    mainscr.mes("Hello, HSPPP World!");

    mainscr.color(0, 0, 255);
    mainscr.pos(100, 250);
    mainscr.mes("auto mainscr = screen(...); でオブジェクト取得");

    mainscr.color(0, 128, 0);
    mainscr.pos(100, 300);
    mainscr.mes("mainscr.color().boxf().mes() とチェーン可能");

    mainscr.color(128, 0, 128);
    mainscr.pos(100, 350);
    mainscr.mes("従来のグローバル関数も引き続き使用可能");

    // 画面に反映
    mainscr.redraw(1);

    // ───────────────────────────────────────────────────────────
    // グローバル関数との混在例
    // ───────────────────────────────────────────────────────────
    // mainscr.select();  // このScreenをカレントに設定（gsel相当）
    // color(255, 255, 0);  // グローバル関数でも操作可能
    // pos(100, 400);
    // mes("グローバル関数でも操作可能");

    return 0;
}
