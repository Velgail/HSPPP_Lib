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

// バージョン文字列（"0.1.0" の形式）
constexpr const char* VERSION_STRING = "0.1.0";

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
