// HspppLib/src/core/hsppp_system.inl
// システム情報関数・ディレクトリ関数・メモリ操作関数の実装

// 注: ヘッダーのインポートは hsppp.cpp のグローバルモジュールフラグメントで行われています
// Windows API: SHGetFolderPathW, GetUserNameW等を使用
// 必要なヘッダー: <shlobj.h>, <lmcons.h>

namespace hsppp {
    namespace internal {
        // ============================================================
        // ヘルパー関数: UTF-16からUTF-8への変換
        // ============================================================
        inline std::string WideToUtf8(const wchar_t* wideString) {
            if (!wideString || wideString[0] == L'\0') {
                return "";
            }
            int len = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, nullptr, 0, nullptr, nullptr);
            if (len > 0) {
                std::string result(len - 1, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wideString, -1, result.data(), len, nullptr, nullptr);
                return result;
            }
            return "";
        }
    }  // namespace internal

    // ============================================================
    // sysinfo - システム情報の取得（HSP互換）
    // 返り値は型に応じて自動判定
    // 文字列型（0, 1, 2）と整数型（3, 16-39）で結果が異なる
    // ============================================================
    // type 0:   文字列 OS名とバージョン番号
    // type 1:   文字列 ログイン中のユーザー名
    // type 2:   文字列 ネットワーク上のコンピュータ名
    // type 3:   整数 HSPが使用する言語(0=英語/1=日本語)
    // type 16:  整数 使用しているCPUの種類(コード)
    // type 17:  整数 使用しているCPUの数
    // type 33:  整数 物理メモリサイズの使用量(単位%)
    // type 34:  整数 全体の物理メモリサイズ (MB単位)
    // type 35:  整数 空き物理メモリサイズ (MB単位)
    // type 36:  整数 スワップファイルのトータルサイズ (MB単位)
    // type 37:  整数 スワップファイルの空きサイズ (MB単位)
    // type 38:  整数 仮想メモリを含めた全メモリサイズ (MB単位)
    // type 39:  整数 仮想メモリを含めた空きメモリサイズ (MB単位)

    std::string sysinfo_str(int type, const std::source_location& location) {
        switch (type) {
            case 0: {
                // OS名とバージョン番号
                OSVERSIONINFOEXW osvi = {};
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                
                // GetVersionExは非推奨だが互換性のために使用
                #pragma warning(push)
                #pragma warning(disable: 4996)
                if (GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&osvi))) {
                    return std::format("Windows {}.{}.{}", 
                        osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
                }
                #pragma warning(pop)
                return "Windows";
            }
            case 1: {
                // ユーザー名
                wchar_t username[UNLEN + 1] = {};
                DWORD size = UNLEN + 1;
                if (GetUserNameW(username, &size)) {
                    return internal::WideToUtf8(username);
                }
                return "";
            }
            case 2: {
                // コンピュータ名
                wchar_t compname[MAX_COMPUTERNAME_LENGTH + 1] = {};
                DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
                if (GetComputerNameW(compname, &size)) {
                    return internal::WideToUtf8(compname);
                }
                return "";
            }
            default:
                return "";
        }
    }

    // int64_t版のsysinfo（メモリサイズ対応）
    // メモリ関連の情報取得を最適化（GlobalMemoryStatusExを一度だけ呼び出す）
    int64_t sysinfo_int(int type, const std::source_location& location) {
        // メモリ情報取得の最適化：type 33-39 の場合は一度だけ呼び出す
        if (type >= 33 && type <= 39) {
            MEMORYSTATUSEX ms = {};
            ms.dwLength = sizeof(ms);
            if (GlobalMemoryStatusEx(&ms)) {
                switch (type) {
                    case 33:
                        return static_cast<int64_t>(ms.dwMemoryLoad);
                    case 34:
                        return static_cast<int64_t>(ms.ullTotalPhys / (1024 * 1024));
                    case 35:
                        return static_cast<int64_t>(ms.ullAvailPhys / (1024 * 1024));
                    case 36:
                        return static_cast<int64_t>(ms.ullTotalPageFile / (1024 * 1024));
                    case 37:
                        return static_cast<int64_t>(ms.ullAvailPageFile / (1024 * 1024));
                    case 38:
                        return static_cast<int64_t>(ms.ullTotalVirtual / (1024 * 1024));
                    case 39:
                        return static_cast<int64_t>(ms.ullAvailVirtual / (1024 * 1024));
                }
            }
            return 0;
        }

        // メモリ情報以外の処理
        switch (type) {
            case 3:
                // HSPが使用する言語 (常に日本語=1)
                return 1;
            case 16: {
                // CPUの種類
                SYSTEM_INFO si = {};
                GetSystemInfo(&si);
                return static_cast<int64_t>(si.wProcessorArchitecture);
            }
            case 17: {
                // CPUの数
                SYSTEM_INFO si = {};
                GetSystemInfo(&si);
                return static_cast<int64_t>(si.dwNumberOfProcessors);
            }
            default:
                return 0;
        }
    }

    // ============================================================
    // dirinfo - ディレクトリ情報の取得（HSP互換）
    // ============================================================
    // type 0:   カレント(現在の)ディレクトリ(dir_cur)
    // type 1:   HSPの実行ファイルがあるディレクトリ(dir_exe)
    // type 2:   Windowsディレクトリ(dir_win)
    // type 3:   Windowsのシステムディレクトリ(dir_sys)
    // type 4:   コマンドライン文字列(dir_cmdline)
    // type 5:   HSPTVディレクトリ(dir_tv) → 空文字列を返す
    // type 0x10000以上: CSIDL値として特殊フォルダを取得

    std::string dirinfo(int type, const std::source_location& location) {
        wchar_t pathW[MAX_PATH] = {};
        
        if (type >= 0x10000) {
            // CSIDL値として特殊フォルダを取得
            int csidl = type & 0xFFFF;
            if (SUCCEEDED(SHGetFolderPathW(nullptr, csidl, nullptr, SHGFP_TYPE_CURRENT, pathW))) {
                return internal::WideToUtf8(pathW);
            }
            return "";
        }
        
        switch (type) {
            case 0: {
                // カレントディレクトリ
                DWORD size = GetCurrentDirectoryW(MAX_PATH, pathW);
                if (size > 0 && size < MAX_PATH) {
                    return internal::WideToUtf8(pathW);
                }
                return "";
            }
            case 1: {
                // 実行ファイルがあるディレクトリ
                wchar_t exePathW[MAX_PATH] = {};
                if (GetModuleFileNameW(nullptr, exePathW, MAX_PATH) > 0) {
                    // パスからファイル名を除去
                    wchar_t* lastSlash = wcsrchr(exePathW, L'\\');
                    if (lastSlash) {
                        *lastSlash = L'\0';
                    }
                    return internal::WideToUtf8(exePathW);
                }
                return "";
            }
            case 2: {
                // Windowsディレクトリ
                UINT size = GetWindowsDirectoryW(pathW, MAX_PATH);
                if (size > 0 && size < MAX_PATH) {
                    return internal::WideToUtf8(pathW);
                }
                return "";
            }
            case 3: {
                // Windowsシステムディレクトリ
                UINT size = GetSystemDirectoryW(pathW, MAX_PATH);
                if (size > 0 && size < MAX_PATH) {
                    return internal::WideToUtf8(pathW);
                }
                return "";
            }
            case 4: {
                // コマンドライン文字列
                LPWSTR cmdLine = GetCommandLineW();
                if (cmdLine) {
                    return internal::WideToUtf8(cmdLine);
                }
                return "";
            }
            case 5:
                // HSPTVディレクトリ（非対応、空文字列を返す）
                return "";
            default:
                return "";
        }
    }

    // ============================================================
    // dir_* 関数 - dirinfo のラッパー
    // ============================================================

    std::string dir_cur(const std::source_location& location) {
        return dirinfo(dir_type_cur, location);
    }

    std::string dir_exe(const std::source_location& location) {
        return dirinfo(dir_type_exe, location);
    }

    std::string dir_win(const std::source_location& location) {
        return dirinfo(dir_type_win, location);
    }

    std::string dir_sys(const std::source_location& location) {
        return dirinfo(dir_type_sys, location);
    }

    std::string dir_cmdline(const std::source_location& location) {
        return dirinfo(dir_type_cmdline, location);
    }

    std::string dir_desktop(const std::source_location& location) {
        return dirinfo(0x10000 | CSIDL_DESKTOP, location);
    }

    std::string dir_mydoc(const std::source_location& location) {
        return dirinfo(0x10000 | CSIDL_PERSONAL, location);
    }


    // ============================================================
    // sysval互換（Windowsハンドル系）
    // ============================================================

    int64_t hwnd(const std::source_location&) {
        auto surface = getCurrentSurface();
        if (!surface) return 0;

        auto window = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (!window) return 0;

        return static_cast<int64_t>(reinterpret_cast<intptr_t>(window->getHwnd()));
    }

    int64_t hdc(const std::source_location&) {
        // Direct2D描画（ID2D1DeviceContext）を使用しているため、
        // HSPのsysval hdc(GDI HDC)をそのまま提供するのは難しい。
        // 現状は未サポートとして0を返す。
        return 0;
    }

    int64_t hinstance(const std::source_location&) {
        return static_cast<int64_t>(reinterpret_cast<intptr_t>(internal::WindowManager::getInstance().getHInstance()));
    }

    // ============================================================
    // sendmsg - ウィンドウメッセージ送信
    // ============================================================

    int64_t sendmsg(int64_t hwndValue, int msg, int64_t wparam, int64_t lparam, const std::source_location&) {
        HWND hwndHandle = reinterpret_cast<HWND>(static_cast<intptr_t>(hwndValue));
        LRESULT result = SendMessageW(
            hwndHandle,
            static_cast<UINT>(msg),
            static_cast<WPARAM>(wparam),
            static_cast<LPARAM>(lparam)
        );
        return static_cast<int64_t>(result);
    }

    int64_t sendmsg(int64_t hwndValue, int msg, int64_t wparam, std::string_view lparamText, const std::source_location&) {
        // UTF-8からUTF-16に変換（wideTextのライフタイムはSendMessageW呼び出しまで保持される）
        std::wstring wideText = internal::Utf8ToWide(lparamText);
        HWND hwndHandle = reinterpret_cast<HWND>(static_cast<intptr_t>(hwndValue));
        LRESULT result = SendMessageW(
            hwndHandle,
            static_cast<UINT>(msg),
            static_cast<WPARAM>(wparam),
            reinterpret_cast<LPARAM>(wideText.c_str())
        );
        return static_cast<int64_t>(result);
    }

} // namespace hsppp
