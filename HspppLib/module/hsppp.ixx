// HspppLib/module/hsppp.ixx
export module hsppp;

// 必要な標準ライブラリをインポート
import <string_view>;

namespace hsppp {

    // --- Screen Mode Flags (HSP Compatible) ---
    export inline constexpr int screen_normal    = 0;    // フルカラーモード
    export inline constexpr int screen_palette   = 1;    // パレットモード（256色）※未実装
    export inline constexpr int screen_hide      = 2;    // 非表示ウィンドウ
    export inline constexpr int screen_fixedsize = 4;    // サイズ固定
    export inline constexpr int screen_tool      = 8;    // ツールウィンドウ
    export inline constexpr int screen_frame     = 16;   // 深い縁のあるウィンドウ

    // --- Core Functions (HSP compatible) ---

    // ウィンドウ初期化（HSP完全互換）
    // screen p1, p2, p3, p4, p5, p6, p7, p8, title
    // p1: ウィンドウID (0～)
    // p2: 画面サイズX (デフォルト: 640)
    // p3: 画面サイズY (デフォルト: 480)
    // p4: 画面モード (デフォルト: 0)
    // p5: ウィンドウ位置X (デフォルト: -1=システム規定)
    // p6: ウィンドウ位置Y (デフォルト: -1=システム規定)
    // p7: クライアントサイズX (デフォルト: 0=p2と同じ)
    // p8: クライアントサイズY (デフォルト: 0=p3と同じ)
    // title: ウィンドウタイトル (HSP拡張)
    export void screen(
        int p1 = 0,
        int p2 = 640,
        int p3 = 480,
        int p4 = 0,
        int p5 = -1,
        int p6 = -1,
        int p7 = 0,
        int p8 = 0,
        std::string_view title = "HSPPP Window"
    );

    // 描画制御
    // p1: 0=描画予約(Offscreen), 1=画面反映(Present)
    export void redraw(int p1 = 1);

    // 待機＆メッセージ処理 (HSP互換)
    // 指定されたミリ秒だけ待機し、その間ウィンドウメッセージを処理する
    export void await(int time_ms);

    // --- Drawing Functions ---
    export void color(int r, int g, int b);
    export void pos(int x, int y);
    export void mes(std::string_view text);  // string_view で安全に
    export void boxf(int x1, int y1, int x2, int y2);
    // 引数なし版 boxf() -> 画面全体
    export void boxf();

    // --- System / Internal ---
    namespace internal {
        // ライブラリの初期化・終了処理(WinMainから呼ばれる)
        export void init_system();
        export void close_system();
    }
}

// グローバル名前空間にユーザー定義関数が存在することを期待する
// (UserApp.cpp で実装される)
extern int hspMain();
