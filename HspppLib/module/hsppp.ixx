// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/module/hsppp.ixx
// メインモジュールインターフェース: 各パーティションを統合してエクスポート
//
// 使用例:
//   import hsppp;
//   hsppp::screen(0, 800, 600);
//   hsppp::color(255, 0, 0);
//   hsppp::mes("Hello, HSPPP!");

export module hsppp;

// 各パーティションを再エクスポート
export import :types;
export import :screen;
export import :drawing;
export import :input;
export import :math;
export import :string;
export import :file;
export import :interrupt;
export import :media;

// バージョン情報のインクルードとエクスポート
#include "../src/core/version.hpp"

export namespace hsppp {

// バージョン情報取得関数
struct version_info {
    int major;
    int minor;
    int patch;
    const char* string;
    const char* build_type;
    const char* platform;
    const char* cxx_version;
};

// バージョン情報を取得
inline version_info get_version() noexcept {
    return {
        VERSION_MAJOR,
        VERSION_MINOR,
        VERSION_PATCH,
        VERSION_STRING,
        BUILD_TYPE,
        PLATFORM,
        CXX_VERSION
    };
}

// バージョン文字列を取得（簡易版）
inline const char* version() noexcept {
    return VERSION_STRING;
}

} // namespace hsppp
