// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/src/boot/WinMain.cpp
#define NOMINMAX
#include <windows.h>
#include <string>

import hsppp;

// Static Library 内の WinMain を強制的にリンクさせるためのおまじない (MSVC用)
// ユーザー側のリンカ設定で /ENTRY を指定しなくても動くようにする効果が期待できます
// x86 と x64 でシンボル名が異なるため、プラットフォームごとに指定
#ifdef _M_IX86
    #pragma comment(linker, "/include:_WinMain@16")
#else
    #pragma comment(linker, "/include:WinMain")
#endif

// ユーザーが UserApp.cpp で定義する関数(前方宣言)
// extern "C" にするかは設計次第ですが、一旦 C++ リンクで進めます
extern void hspMain();

// アプリケーションのエントリーポイント
int WINAPI WinMain([[maybe_unused]] _In_ HINSTANCE hInstance, 
                   [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance, 
                   [[maybe_unused]] _In_ LPSTR lpCmdLine, 
                   [[maybe_unused]] _In_ int nCmdShow) {
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
