// HspppLib/src/boot/WinMain.cpp
#include <windows.h>
import hsppp;

// Static Library 内の WinMain を強制的にリンクさせるためのおまじない (MSVC用)
// ユーザー側のリンカ設定で /ENTRY を指定しなくても動くようにする効果が期待できます
#pragma comment(linker, "/include:wWinMain")

// ユーザーが UserApp.cpp で定義する関数(前方宣言)
// extern "C" にするかは設計次第ですが、一旦 C++ リンクで進めます
extern int hspMain();

// アプリケーションのエントリーポイント
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    // 1. HSPPPエンジンの初期化
    // (COM, Direct2D Factory, ウィンドウクラス登録など)
    hsppp::internal::init_system();

    // 2. ユーザーコード (hspMain) の実行
    // ユーザーはここから screen() などを呼び出す
    int ret = hspMain();

    // 3. エンジンの終了処理
    // (リソース開放など)
    hsppp::internal::close_system();

    return ret;
}
