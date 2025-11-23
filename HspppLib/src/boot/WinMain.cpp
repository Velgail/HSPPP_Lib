import hsppp;

#include <windows.h>

// ユーザーが定義する関数（前方宣言）
// どこかの .obj にこれが入っていればリンクが通る
extern int hspMain();

// ライブラリが提供する真のエントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // 1. 初期化 (COM, Direct2D Factoryなど)
    hsppp::internal::init_system();

    // 2. ユーザーコード実行
    int ret = hspMain();

    // 3. 終了処理
    hsppp::internal::close_system();

    return ret;
}