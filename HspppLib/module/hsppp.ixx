// HspppLib/module/hsppp.ixx
export module hsppp;

// 必要な標準ライブラリがあればここで import 記述
// import <string>;
// import <format>;

export namespace hsppp {

    // --- Core Functions (HSP compatible) ---

    // ウィンドウ初期化
    // mode: 0=通常, 1=枠なし... (HSP互換)
    export void screen(int id, int width, int height, int mode = 0, const char* title = "HSPPP Window");

    // 描画制御
    // p1: 0=描画予約(Offscreen), 1=画面反映(Present)
    export void redraw(int p1 = 1);

    // 待機＆メッセージ処理
    // 戻り値: 0=終了要求(WM_QUIT等), 1=継続
    // HSPのawaitは時間経過を待つが、ここではメッセージポンプの役割が主
    export int await(int time_ms);

    // --- Drawing Functions ---
    export void color(int r, int g, int b);
    export void pos(int x, int y);
    export void mes(const char* text);
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
