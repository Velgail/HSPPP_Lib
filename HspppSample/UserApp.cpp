// HspppSample/UserApp.cpp
import hsppp;
using namespace hsppp;

// ユーザーのエントリーポイント
int hspMain() {
    // ウィンドウ作成（HSP互換 screen命令）
    //
    // 【遅延初期化】
    // screen命令を書かずにいきなり描画命令を呼んでも動作します。
    // その場合、自動的にデフォルトウィンドウ（640x480, ID0, "HSPPP Window"）が作成されます。
    //
    // 【明示的な作成】
    // ウィンドウサイズやタイトルを指定したい場合は、下記のように screen命令を呼びます。
    // screen p1, p2, p3, p4, p5, p6, p7, p8, title
    //   p1: ウィンドウID (0～)
    //   p2: 画面幅 (デフォルト: 640)
    //   p3: 画面高さ (デフォルト: 480)
    //   p4: 画面モード (デフォルト: 0)
    //       screen_normal (0): フルカラーモード
    //       screen_hide (2): 非表示ウィンドウ
    //       screen_fixedsize (4): サイズ固定
    //       screen_tool (8): ツールウィンドウ
    //       screen_frame (16): 深い縁のあるウィンドウ
    //   p5, p6: ウィンドウ位置X,Y (デフォルト: -1=システム規定)
    //   p7, p8: クライアント領域サイズ (デフォルト: 0=p2,p3と同じ)
    //   title: ウィンドウタイトル
    //
    // 注意: ID0は自動的にサイズ固定、ID1以降はリサイズ可能
    screen(0, 800, 600, screen_normal, -1, -1, 0, 0, "HSPPP Sample Window");

    // 上記の screen() をコメントアウトしても、以下の描画命令だけで動作します：
    // （その場合、640x480のデフォルトウィンドウが自動作成されます）

    // 描画開始
    redraw(0);

    // 背景を白で塗りつぶし
    color(255, 255, 255);
    boxf();

    // 黒い矩形を描画
    color(0, 0, 0);
    boxf(50, 50, 200, 150);

    // 赤いテキストを描画
    color(255, 0, 0);
    pos(100, 200);
    mes("Hello, HSPPP World!");

    // 青いテキストを描画
    color(0, 0, 255);
    pos(100, 250);
    mes("Direct2Dで描画しています");

    // 緑のテキストを描画
    color(0, 128, 0);
    pos(100, 300);
    mes("HSP互換API (C++20 Modules)");

    // 描画終了＆画面反映
    redraw(1);

    // hspMainを抜けると、HSPのstop命令と同じ動作になる
    // ウィンドウは表示され続け、×ボタンで終了できる
    return 0;
}
