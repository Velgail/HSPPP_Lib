// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/src/core/version.hpp
// バージョン管理定義
//
// セマンティックバージョニング: MAJOR.MINOR.PATCH
// - MAJOR: 後方互換性のない API 変更
// - MINOR: 後方互換性のある機能追加
// - PATCH: 後方互換性のあるバグ修正

#pragma once

namespace hsppp {

// バージョン番号の定義
constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 1;
constexpr int VERSION_PATCH = 0;

// constexpr 整数→文字列変換ヘルパー
namespace detail {
    // コンパイル時に整数の桁数を計算
    consteval int count_digits(int n) {
        return n < 10 ? 1 : 1 + count_digits(n / 10);
    }

    // 固定長文字列バッファ
    template<int N>
    struct FixedString {
        char data[N + 1] = {};
        constexpr FixedString() = default;
        constexpr const char* c_str() const { return data; }
    };

    // 整数を文字列に変換
    template<int N>
    consteval FixedString<N> int_to_string(int value) {
        FixedString<N> result;
        int temp = value;
        for (int i = N - 1; i >= 0; --i) {
            result.data[i] = '0' + (temp % 10);
            temp /= 10;
        }
        return result;
    }

    // 複数の文字列を結合
    template<int N1, int N2, int N3, int N4, int N5>
    consteval FixedString<N1 + N2 + N3 + N4 + N5> concat5(
        const FixedString<N1>& s1,
        const FixedString<N2>& s2,
        const FixedString<N3>& s3,
        const FixedString<N4>& s4,
        const FixedString<N5>& s5
    ) {
        FixedString<N1 + N2 + N3 + N4 + N5> result;
        int pos = 0;
        for (int i = 0; i < N1; ++i) result.data[pos++] = s1.data[i];
        for (int i = 0; i < N2; ++i) result.data[pos++] = s2.data[i];
        for (int i = 0; i < N3; ++i) result.data[pos++] = s3.data[i];
        for (int i = 0; i < N4; ++i) result.data[pos++] = s4.data[i];
        for (int i = 0; i < N5; ++i) result.data[pos++] = s5.data[i];
        return result;
    }

    // 1文字の文字列
    consteval FixedString<1> make_char(char c) {
        FixedString<1> result;
        result.data[0] = c;
        return result;
    }
}

// バージョン文字列（自動生成: "MAJOR.MINOR.PATCH" の形式）
inline constexpr auto VERSION_STRING_BUF = detail::concat5(
    detail::int_to_string<detail::count_digits(VERSION_MAJOR)>(VERSION_MAJOR),
    detail::make_char('.'),
    detail::int_to_string<detail::count_digits(VERSION_MINOR)>(VERSION_MINOR),
    detail::make_char('.'),
    detail::int_to_string<detail::count_digits(VERSION_PATCH)>(VERSION_PATCH)
);
inline constexpr const char* VERSION_STRING = VERSION_STRING_BUF.c_str();

// バージョン番号の数値（比較用: 0xMMNNPP）
constexpr int VERSION_NUMBER = 
    (VERSION_MAJOR << 16) | (VERSION_MINOR << 8) | VERSION_PATCH;

// ビルド情報
#ifdef _DEBUG
constexpr const char* BUILD_TYPE = "Debug";
#else
constexpr const char* BUILD_TYPE = "Release";
#endif

// プラットフォーム情報
#if defined(_WIN64)
constexpr const char* PLATFORM = "Windows x64";
#elif defined(_WIN32)
constexpr const char* PLATFORM = "Windows x86";
#else
constexpr const char* PLATFORM = "Unknown";
#endif

// C++ バージョン
#if __cplusplus >= 202302L
constexpr const char* CXX_VERSION = "C++23";
#elif __cplusplus >= 202002L
constexpr const char* CXX_VERSION = "C++20";
#else
constexpr const char* CXX_VERSION = "C++??";
#endif

} // namespace hsppp
