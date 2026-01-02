// HspppLib/src/boot/WinMain.cpp
#define NOMINMAX
#include <windows.h>
#include "../core/version.hpp"
#include <string>

// バージョン情報をコンパイル時に出力
// version.hpp の constexpr 定数を使用（マクロ不要）
namespace {
    // constexpr const char* を #pragma message で使うためのマクロ
    // （#pragma message は文字列リテラルしか受け付けないため最小限のマクロのみ使用）
    #define HSPPP_STRINGIFY(x) #x
    #define HSPPP_TOSTRING(x) HSPPP_STRINGIFY(x)
    
    // constexpr 値から静的文字列を生成
    constexpr auto make_version_banner() {
        return hsppp::VERSION_STRING;
    }
}

#pragma message("===============================================")
#pragma message("HspppLib - Hot Soup Processor Plus Plus")
// version.hpp の VERSION_MAJOR/MINOR/PATCH から自動生成された VERSION_STRING を使用
// NOTE: #pragma message は constexpr const char* を直接使えないため、
//       コンパイル時の値を確認する目的で MAJOR.MINOR.PATCH を個別に表示
#pragma message("Version: " HSPPP_TOSTRING(0) "." HSPPP_TOSTRING(1) "." HSPPP_TOSTRING(0))
#ifdef _DEBUG
#pragma message("Build Type: Debug")
#else
#pragma message("Build Type: Release")
#endif
#ifdef _WIN64
#pragma message("Platform: Windows x64")
#else
#pragma message("Platform: Windows x86")
#endif
#pragma message("C++ Standard: C++23")
#pragma message("===============================================")

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
    catch (const hsppp::HspErrorBase& e) {
        // HspErrorBase派生例外（HspError/HspWeakError）が発生した場合、
        // onerrorハンドラを実行（ハンドラ未設定時はエラーダイアログを表示して終了）
        hsppp::internal::handleHspError(e);
        return 1;  // エラー終了（実際にはhandleHspErrorがend()を呼ぶので到達しない）
    }
    catch (const std::exception& e) {
        // safe_callを通過しなかったstd::exception（通常は発生しないはず）
        // HspErrorに変換してonerrorハンドラで処理
        hsppp::HspError hspErr(hsppp::ERR_INTERNAL, e.what());
        hsppp::internal::handleHspError(hspErr);
        return 1;
    }
    catch (...) {
        // 不明な例外
        MessageBoxW(nullptr, L"Unknown error occurred", L"Fatal Error", MB_OK | MB_ICONERROR);
        hsppp::internal::close_system();
        return 1;
    }
}
