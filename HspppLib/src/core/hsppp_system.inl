// HspppLib/src/core/hsppp_system.inl
// システム情報関数・ディレクトリ関数・メモリ操作関数の実装

// 注: ヘッダーのインポートは hsppp.cpp のグローバルモジュールフラグメントで行われています
// Windows API: SHGetFolderPathW, GetUserNameW等を使用
// 必要なヘッダー: <shlobj.h>, <lmcons.h>

namespace hsppp {

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
                    // UTF-16 → UTF-8変換
                    int len = WideCharToMultiByte(CP_UTF8, 0, username, -1, nullptr, 0, nullptr, nullptr);
                    if (len > 0) {
                        std::string result(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, username, -1, result.data(), len, nullptr, nullptr);
                        return result;
                    }
                }
                return "";
            }
            case 2: {
                // コンピュータ名
                wchar_t compname[MAX_COMPUTERNAME_LENGTH + 1] = {};
                DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
                if (GetComputerNameW(compname, &size)) {
                    // UTF-16 → UTF-8変換
                    int len = WideCharToMultiByte(CP_UTF8, 0, compname, -1, nullptr, 0, nullptr, nullptr);
                    if (len > 0) {
                        std::string result(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, compname, -1, result.data(), len, nullptr, nullptr);
                        return result;
                    }
                }
                return "";
            }
            default:
                return "";
        }
    }

    // int64版のsysinfo（メモリサイズ対応）
    long long sysinfo_int(int type, const std::source_location& location) {
        switch (type) {
            case 3:
                // HSPが使用する言語 (常に日本語=1)
                return 1;
            case 16: {
                // CPUの種類
                SYSTEM_INFO si = {};
                GetSystemInfo(&si);
                return static_cast<long long>(si.wProcessorArchitecture);
            }
            case 17: {
                // CPUの数
                SYSTEM_INFO si = {};
                GetSystemInfo(&si);
                return static_cast<long long>(si.dwNumberOfProcessors);
            }
            case 33: {
                // 物理メモリサイズの使用量(%)
                MEMORYSTATUSEX ms = {};
                ms.dwLength = sizeof(ms);
                if (GlobalMemoryStatusEx(&ms)) {
                    return static_cast<long long>(ms.dwMemoryLoad);
                }
                return 0;
            }
            case 34: {
                // 全体の物理メモリサイズ (MB単位、int64対応)
                MEMORYSTATUSEX ms = {};
                ms.dwLength = sizeof(ms);
                if (GlobalMemoryStatusEx(&ms)) {
                    return static_cast<long long>(ms.ullTotalPhys / (1024 * 1024));
                }
                return 0;
            }
            case 35: {
                // 空き物理メモリサイズ (MB単位、int64対応)
                MEMORYSTATUSEX ms = {};
                ms.dwLength = sizeof(ms);
                if (GlobalMemoryStatusEx(&ms)) {
                    return static_cast<long long>(ms.ullAvailPhys / (1024 * 1024));
                }
                return 0;
            }
            case 36: {
                // スワップファイルのトータルサイズ (MB単位、int64対応)
                MEMORYSTATUSEX ms = {};
                ms.dwLength = sizeof(ms);
                if (GlobalMemoryStatusEx(&ms)) {
                    return static_cast<long long>(ms.ullTotalPageFile / (1024 * 1024));
                }
                return 0;
            }
            case 37: {
                // スワップファイルの空きサイズ (MB単位、int64対応)
                MEMORYSTATUSEX ms = {};
                ms.dwLength = sizeof(ms);
                if (GlobalMemoryStatusEx(&ms)) {
                    return static_cast<long long>(ms.ullAvailPageFile / (1024 * 1024));
                }
                return 0;
            }
            case 38: {
                // 仮想メモリを含めた全メモリサイズ (MB単位、int64対応)
                MEMORYSTATUSEX ms = {};
                ms.dwLength = sizeof(ms);
                if (GlobalMemoryStatusEx(&ms)) {
                    return static_cast<long long>(ms.ullTotalVirtual / (1024 * 1024));
                }
                return 0;
            }
            case 39: {
                // 仮想メモリを含めた空きメモリサイズ (MB単位、int64対応)
                MEMORYSTATUSEX ms = {};
                ms.dwLength = sizeof(ms);
                if (GlobalMemoryStatusEx(&ms)) {
                    return static_cast<long long>(ms.ullAvailVirtual / (1024 * 1024));
                }
                return 0;
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
                int len = WideCharToMultiByte(CP_UTF8, 0, pathW, -1, nullptr, 0, nullptr, nullptr);
                if (len > 0) {
                    std::string result(len - 1, '\0');
                    WideCharToMultiByte(CP_UTF8, 0, pathW, -1, result.data(), len, nullptr, nullptr);
                    return result;
                }
            }
            return "";
        }
        
        switch (type) {
            case 0: {
                // カレントディレクトリ
                DWORD size = GetCurrentDirectoryW(MAX_PATH, pathW);
                if (size > 0 && size < MAX_PATH) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pathW, -1, nullptr, 0, nullptr, nullptr);
                    if (len > 0) {
                        std::string result(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, pathW, -1, result.data(), len, nullptr, nullptr);
                        return result;
                    }
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
                    int len = WideCharToMultiByte(CP_UTF8, 0, exePathW, -1, nullptr, 0, nullptr, nullptr);
                    if (len > 0) {
                        std::string result(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, exePathW, -1, result.data(), len, nullptr, nullptr);
                        return result;
                    }
                }
                return "";
            }
            case 2: {
                // Windowsディレクトリ
                UINT size = GetWindowsDirectoryW(pathW, MAX_PATH);
                if (size > 0 && size < MAX_PATH) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pathW, -1, nullptr, 0, nullptr, nullptr);
                    if (len > 0) {
                        std::string result(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, pathW, -1, result.data(), len, nullptr, nullptr);
                        return result;
                    }
                }
                return "";
            }
            case 3: {
                // Windowsシステムディレクトリ
                UINT size = GetSystemDirectoryW(pathW, MAX_PATH);
                if (size > 0 && size < MAX_PATH) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pathW, -1, nullptr, 0, nullptr, nullptr);
                    if (len > 0) {
                        std::string result(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, pathW, -1, result.data(), len, nullptr, nullptr);
                        return result;
                    }
                }
                return "";
            }
            case 4: {
                // コマンドライン文字列
                LPWSTR cmdLine = GetCommandLineW();
                if (cmdLine) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, cmdLine, -1, nullptr, 0, nullptr, nullptr);
                    if (len > 0) {
                        std::string result(len - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, cmdLine, -1, result.data(), len, nullptr, nullptr);
                        return result;
                    }
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
        return dirinfo(0, location);
    }

    std::string dir_exe(const std::source_location& location) {
        return dirinfo(1, location);
    }

    std::string dir_win(const std::source_location& location) {
        return dirinfo(2, location);
    }

    std::string dir_sys(const std::source_location& location) {
        return dirinfo(3, location);
    }

    std::string dir_cmdline(const std::source_location& location) {
        return dirinfo(4, location);
    }

    std::string dir_desktop(const std::source_location& location) {
        return dirinfo(0x10000 | CSIDL_DESKTOP, location);
    }

    std::string dir_mydoc(const std::source_location& location) {
        return dirinfo(0x10000 | CSIDL_PERSONAL, location);
    }

    // ============================================================
    // peek - バッファから1byte読み出し（HSP互換）
    // ============================================================

    int peek(const std::string& buffer, int index) {
        if (index < 0 || static_cast<size_t>(index) >= buffer.size()) {
            return 0;  // 範囲外は0を返す（HSP互換）
        }
        return static_cast<unsigned char>(buffer[index]);
    }

    int peek(const std::vector<unsigned char>& buffer, int index) {
        if (index < 0 || static_cast<size_t>(index) >= buffer.size()) {
            return 0;
        }
        return buffer[index];
    }

    // ============================================================
    // wpeek - バッファから2byte読み出し（HSP互換）
    // ============================================================

    int wpeek(const std::string& buffer, int index) {
        if (index < 0 || static_cast<size_t>(index + 1) >= buffer.size()) {
            return 0;  // 範囲外は0を返す（HSP互換）
        }
        // リトルエンディアン
        unsigned char lo = static_cast<unsigned char>(buffer[index]);
        unsigned char hi = static_cast<unsigned char>(buffer[index + 1]);
        return lo | (hi << 8);
    }

    int wpeek(const std::vector<unsigned char>& buffer, int index) {
        if (index < 0 || static_cast<size_t>(index + 1) >= buffer.size()) {
            return 0;
        }
        return buffer[index] | (buffer[index + 1] << 8);
    }

    // ============================================================
    // lpeek - バッファから4byte読み出し（HSP互換）
    // ============================================================

    int lpeek(const std::string& buffer, int index) {
        if (index < 0 || static_cast<size_t>(index + 3) >= buffer.size()) {
            return 0;  // 範囲外は0を返す（HSP互換）
        }
        // リトルエンディアン
        unsigned char b0 = static_cast<unsigned char>(buffer[index]);
        unsigned char b1 = static_cast<unsigned char>(buffer[index + 1]);
        unsigned char b2 = static_cast<unsigned char>(buffer[index + 2]);
        unsigned char b3 = static_cast<unsigned char>(buffer[index + 3]);
        return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }

    int lpeek(const std::vector<unsigned char>& buffer, int index) {
        if (index < 0 || static_cast<size_t>(index + 3) >= buffer.size()) {
            return 0;
        }
        return buffer[index] | (buffer[index + 1] << 8) | 
               (buffer[index + 2] << 16) | (buffer[index + 3] << 24);
    }

    // ============================================================
    // poke - バッファに1byte書き込み（HSP互換）
    // ============================================================

    void poke(std::string& buffer, int index, int value, const std::source_location& location) {
        if (index < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "pokeのインデックスが負の値です", location);
        }
        // 必要に応じてバッファを拡張
        if (static_cast<size_t>(index) >= buffer.size()) {
            buffer.resize(index + 1, '\0');
        }
        buffer[index] = static_cast<char>(value & 0xFF);
    }

    void poke(std::vector<unsigned char>& buffer, int index, int value, const std::source_location& location) {
        if (index < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "pokeのインデックスが負の値です", location);
        }
        if (static_cast<size_t>(index) >= buffer.size()) {
            buffer.resize(index + 1, 0);
        }
        buffer[index] = static_cast<unsigned char>(value & 0xFF);
    }

    // 文字列をバッファに書き込むオーバーロード
    void poke(std::string& buffer, int index, const std::string& value, const std::source_location& location) {
        if (index < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "pokeのインデックスが負の値です", location);
        }
        size_t requiredSize = index + value.size() + 1;  // null終端を含む
        if (requiredSize > buffer.size()) {
            buffer.resize(requiredSize, '\0');
        }
        std::copy(value.begin(), value.end(), buffer.begin() + index);
        buffer[index + value.size()] = '\0';
    }

    // ============================================================
    // wpoke - バッファに2byte書き込み（HSP互換）
    // ============================================================

    void wpoke(std::string& buffer, int index, int value, const std::source_location& location) {
        if (index < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "wpokeのインデックスが負の値です", location);
        }
        if (static_cast<size_t>(index + 1) >= buffer.size()) {
            buffer.resize(index + 2, '\0');
        }
        // リトルエンディアン
        buffer[index] = static_cast<char>(value & 0xFF);
        buffer[index + 1] = static_cast<char>((value >> 8) & 0xFF);
    }

    void wpoke(std::vector<unsigned char>& buffer, int index, int value, const std::source_location& location) {
        if (index < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "wpokeのインデックスが負の値です", location);
        }
        if (static_cast<size_t>(index + 1) >= buffer.size()) {
            buffer.resize(index + 2, 0);
        }
        buffer[index] = static_cast<unsigned char>(value & 0xFF);
        buffer[index + 1] = static_cast<unsigned char>((value >> 8) & 0xFF);
    }

    // ============================================================
    // lpoke - バッファに4byte書き込み（HSP互換）
    // ============================================================

    void lpoke(std::string& buffer, int index, int value, const std::source_location& location) {
        if (index < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "lpokeのインデックスが負の値です", location);
        }
        if (static_cast<size_t>(index + 3) >= buffer.size()) {
            buffer.resize(index + 4, '\0');
        }
        // リトルエンディアン
        buffer[index] = static_cast<char>(value & 0xFF);
        buffer[index + 1] = static_cast<char>((value >> 8) & 0xFF);
        buffer[index + 2] = static_cast<char>((value >> 16) & 0xFF);
        buffer[index + 3] = static_cast<char>((value >> 24) & 0xFF);
    }

    void lpoke(std::vector<unsigned char>& buffer, int index, int value, const std::source_location& location) {
        if (index < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "lpokeのインデックスが負の値です", location);
        }
        if (static_cast<size_t>(index + 3) >= buffer.size()) {
            buffer.resize(index + 4, 0);
        }
        buffer[index] = static_cast<unsigned char>(value & 0xFF);
        buffer[index + 1] = static_cast<unsigned char>((value >> 8) & 0xFF);
        buffer[index + 2] = static_cast<unsigned char>((value >> 16) & 0xFF);
        buffer[index + 3] = static_cast<unsigned char>((value >> 24) & 0xFF);
    }

} // namespace hsppp
