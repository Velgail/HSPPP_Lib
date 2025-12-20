// HspppLib/src/boot/WinMain.cpp
#define NOMINMAX
#include <windows.h>
import hsppp;

// Static Library 内の WinMain を強制的にリンクさせるためのおまじない (MSVC用)
// ユーザー側のリンカ設定で /ENTRY を指定しなくても動くようにする効果が期待できます
#pragma comment(linker, "/include:WinMain")

// ユーザーが UserApp.cpp で定義する関数(前方宣言)
// extern "C" にするかは設計次第ですが、一旦 C++ リンクで進めます
extern int hspMain();

// アプリケーションのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    // 1. HSPPPエンジンの初期化
    // (COM, Direct2D Factory, ウィンドウクラス登録など)
    hsppp::internal::init_system();

    // 2. ユーザーコード (hspMain) の実行
    // エラーハンドリング: HspError例外を自動的にキャッチ
    try {
        // ユーザーはここから screen() などを呼び出す
        hspMain();

        // 3. hspMain を抜けた後は、HSPの stop 命令相当の動作
        // WM_QUIT が来るまでメッセージループを回し続ける
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 4. エンジンの終了処理
        // (リソース開放など)
        hsppp::internal::close_system();

        return static_cast<int>(msg.wParam);
    }
    catch (const hsppp::HspError& e) {
        // HspError例外が発生した場合、onerrorハンドラを実行
        // （ハンドラ未設定時はエラーダイアログを表示して終了）
        hsppp::internal::handleHspError(e);
        return 1;  // エラー終了（実際にはhandleHspErrorがend()を呼ぶので到達しない）
    }
    catch (const std::out_of_range& e) {
        // 配列境界外アクセスなどの範囲エラー
        // HspErrorに変換してonerrorハンドラで処理
        hsppp::HspError hspErr(hsppp::ERR_OUT_OF_ARRAY, e.what());
        hsppp::internal::handleHspError(hspErr);
        return 1;
    }
    catch (const std::invalid_argument& e) {
        // 無効な引数エラー
        hsppp::HspError hspErr(hsppp::ERR_OUT_OF_RANGE, e.what());
        hsppp::internal::handleHspError(hspErr);
        return 1;
    }
    catch (const std::runtime_error& e) {
        // ランタイムエラー
        hsppp::HspError hspErr(hsppp::ERR_SYSTEM_ERROR, e.what());
        hsppp::internal::handleHspError(hspErr);
        return 1;
    }
    catch (const std::exception& e) {
        // その他の標準例外もHspErrorに変換してonerrorで処理
        hsppp::HspError hspErr(hsppp::ERR_INTERNAL, e.what());
        hsppp::internal::handleHspError(hspErr);
        return 1;
    }
    catch (...) {
        // 不明な例外
        MessageBoxA(nullptr, "Unknown error occurred", "Fatal Error", MB_OK | MB_ICONERROR);
        hsppp::internal::close_system();
        return 1;
    }
}
