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
    // テンプレートヘルパー関数: peek/poke 系の共通化
    // ============================================================

    namespace {
        // peek 系のテンプレート実装
        template<typename BufferType>
        int peek_impl(const BufferType& buffer, int64_t index) {
            if (index < 0 || static_cast<size_t>(index) >= buffer.size()) {
                return 0;
            }
            return static_cast<uint8_t>(buffer[static_cast<size_t>(index)]);
        }

        template<typename BufferType>
        int wpeek_impl(const BufferType& buffer, int64_t index) {
            if (index < 0 || static_cast<size_t>(index + 1) >= buffer.size()) {
                return 0;
            }
            auto lo = static_cast<uint8_t>(buffer[static_cast<size_t>(index)]);
            auto hi = static_cast<uint8_t>(buffer[static_cast<size_t>(index + 1)]);
            return lo | (hi << 8);
        }

        template<typename BufferType>
        int lpeek_impl(const BufferType& buffer, int64_t index) {
            if (index < 0 || static_cast<size_t>(index + 3) >= buffer.size()) {
                return 0;
            }
            auto b0 = static_cast<uint8_t>(buffer[static_cast<size_t>(index)]);
            auto b1 = static_cast<uint8_t>(buffer[static_cast<size_t>(index + 1)]);
            auto b2 = static_cast<uint8_t>(buffer[static_cast<size_t>(index + 2)]);
            auto b3 = static_cast<uint8_t>(buffer[static_cast<size_t>(index + 3)]);
            return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
        }

        // poke 系のテンプレート実装
        template<typename BufferType>
        void poke_impl(BufferType& buffer, int64_t index, int value, const std::source_location& location) {
            if (index < 0) {
                throw HspError(ERR_OUT_OF_RANGE, "pokeのインデックスが負の値です", location);
            }
            if (static_cast<size_t>(index) >= buffer.size()) {
                buffer.resize(static_cast<size_t>(index + 1));
            }
            using CharType = typename BufferType::value_type;
            buffer[static_cast<size_t>(index)] = static_cast<CharType>(value & 0xFF);
        }

        template<typename BufferType>
        void wpoke_impl(BufferType& buffer, int64_t index, int value, const std::source_location& location) {
            if (index < 0) {
                throw HspError(ERR_OUT_OF_RANGE, "wpokeのインデックスが負の値です", location);
            }
            if (static_cast<size_t>(index + 1) >= buffer.size()) {
                buffer.resize(static_cast<size_t>(index + 2));
            }
            using CharType = typename BufferType::value_type;
            buffer[static_cast<size_t>(index)] = static_cast<CharType>(value & 0xFF);
            buffer[static_cast<size_t>(index + 1)] = static_cast<CharType>((value >> 8) & 0xFF);
        }

        template<typename BufferType>
        void lpoke_impl(BufferType& buffer, int64_t index, int value, const std::source_location& location) {
            if (index < 0) {
                throw HspError(ERR_OUT_OF_RANGE, "lpokeのインデックスが負の値です", location);
            }
            if (static_cast<size_t>(index + 3) >= buffer.size()) {
                buffer.resize(static_cast<size_t>(index + 4));
            }
            using CharType = typename BufferType::value_type;
            buffer[static_cast<size_t>(index)] = static_cast<CharType>(value & 0xFF);
            buffer[static_cast<size_t>(index + 1)] = static_cast<CharType>((value >> 8) & 0xFF);
            buffer[static_cast<size_t>(index + 2)] = static_cast<CharType>((value >> 16) & 0xFF);
            buffer[static_cast<size_t>(index + 3)] = static_cast<CharType>((value >> 24) & 0xFF);
        }
    }  // anonymous namespace

    // ============================================================
    // peek - バッファから1byte読み出し（HSP互換）
    // ============================================================

    int peek(const std::string& buffer, int64_t index) {
        return peek_impl(buffer, index);
    }

    int peek(const std::vector<uint8_t>& buffer, int64_t index) {
        return peek_impl(buffer, index);
    }

    // ============================================================
    // wpeek - バッファから2byte読み出し（HSP互換）
    // ============================================================

    int wpeek(const std::string& buffer, int64_t index) {
        return wpeek_impl(buffer, index);
    }

    int wpeek(const std::vector<uint8_t>& buffer, int64_t index) {
        return wpeek_impl(buffer, index);
    }

    // ============================================================
    // lpeek - バッファから4byte読み出し（HSP互換）
    // ============================================================

    int lpeek(const std::string& buffer, int64_t index) {
        return lpeek_impl(buffer, index);
    }

    int lpeek(const std::vector<uint8_t>& buffer, int64_t index) {
        return lpeek_impl(buffer, index);
    }

    // ============================================================
    // poke - バッファに1byte書き込み（HSP互換）
    // ============================================================

    void poke(std::string& buffer, int64_t index, int value, const std::source_location& location) {
        poke_impl(buffer, index, value, location);
    }

    void poke(std::vector<uint8_t>& buffer, int64_t index, int value, const std::source_location& location) {
        poke_impl(buffer, index, value, location);
    }

    // 文字列をバッファに書き込むオーバーロード
    void poke(std::string& buffer, int64_t index, const std::string& value, const std::source_location& location) {
        if (index < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "pokeのインデックスが負の値です", location);
        }
        size_t requiredSize = static_cast<size_t>(index) + value.size() + 1;  // null終端を含む
        if (requiredSize > buffer.size()) {
            buffer.resize(requiredSize, '\0');
        }
        std::copy(value.begin(), value.end(), buffer.begin() + static_cast<size_t>(index));
        buffer[static_cast<size_t>(index) + value.size()] = '\0';
    }

    // ============================================================
    // wpoke - バッファに2byte書き込み（HSP互換）
    // ============================================================

    void wpoke(std::string& buffer, int64_t index, int value, const std::source_location& location) {
        wpoke_impl(buffer, index, value, location);
    }

    void wpoke(std::vector<uint8_t>& buffer, int64_t index, int value, const std::source_location& location) {
        wpoke_impl(buffer, index, value, location);
    }

    // ============================================================
    // lpoke - バッファに4byte書き込み（HSP互換）
    // ============================================================

    void lpoke(std::string& buffer, int64_t index, int value, const std::source_location& location) {
        lpoke_impl(buffer, index, value, location);
    }

    void lpoke(std::vector<uint8_t>& buffer, int64_t index, int value, const std::source_location& location) {
        lpoke_impl(buffer, index, value, location);
    }

    // ============================================================
    // memcpy - メモリブロックのコピー（HSP互換）
    // ============================================================

    void memcpy(std::string& dest, const std::string& src, int64_t size, int64_t destOffset, int64_t srcOffset, const std::source_location& location) {
        if (size <= 0) {
            return;
        }
        if (destOffset < 0 || srcOffset < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "memcpyのオフセットが負の値です", location);
        }
        if (static_cast<size_t>(srcOffset + size) > src.size()) {
            throw HspError(ERR_BUFFER_OVERFLOW, "memcpyのコピー元がバッファ範囲を超えています", location);
        }
        // コピー先のサイズを必要に応じて拡張
        size_t requiredSize = static_cast<size_t>(destOffset + size);
        if (requiredSize > dest.size()) {
            dest.resize(requiredSize, '\0');
        }
        // std::memcpyを使用（高速）
        std::memcpy(dest.data() + static_cast<size_t>(destOffset), src.data() + static_cast<size_t>(srcOffset), static_cast<size_t>(size));
    }

    void memcpy(std::vector<uint8_t>& dest, const std::vector<uint8_t>& src, int64_t size, int64_t destOffset, int64_t srcOffset, const std::source_location& location) {
        if (size <= 0) {
            return;
        }
        if (destOffset < 0 || srcOffset < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "memcpyのオフセットが負の値です", location);
        }
        if (static_cast<size_t>(srcOffset + size) > src.size()) {
            throw HspError(ERR_BUFFER_OVERFLOW, "memcpyのコピー元がバッファ範囲を超えています", location);
        }
        size_t requiredSize = static_cast<size_t>(destOffset + size);
        if (requiredSize > dest.size()) {
            dest.resize(requiredSize, 0);
        }
        std::memcpy(dest.data() + static_cast<size_t>(destOffset), src.data() + static_cast<size_t>(srcOffset), static_cast<size_t>(size));
    }

    void memcpy(std::vector<uint8_t>& dest, const std::string& src, int64_t size, int64_t destOffset, int64_t srcOffset, const std::source_location& location) {
        if (size <= 0) {
            return;
        }
        if (destOffset < 0 || srcOffset < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "memcpyのオフセットが負の値です", location);
        }
        if (static_cast<size_t>(srcOffset + size) > src.size()) {
            throw HspError(ERR_BUFFER_OVERFLOW, "memcpyのコピー元がバッファ範囲を超えています", location);
        }
        size_t requiredSize = static_cast<size_t>(destOffset + size);
        if (requiredSize > dest.size()) {
            dest.resize(requiredSize, 0);
        }
        std::memcpy(dest.data() + static_cast<size_t>(destOffset), src.data() + static_cast<size_t>(srcOffset), static_cast<size_t>(size));
    }

    void memcpy(std::string& dest, const std::vector<uint8_t>& src, int64_t size, int64_t destOffset, int64_t srcOffset, const std::source_location& location) {
        if (size <= 0) {
            return;
        }
        if (destOffset < 0 || srcOffset < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "memcpyのオフセットが負の値です", location);
        }
        if (static_cast<size_t>(srcOffset + size) > src.size()) {
            throw HspError(ERR_BUFFER_OVERFLOW, "memcpyのコピー元がバッファ範囲を超えています", location);
        }
        size_t requiredSize = static_cast<size_t>(destOffset + size);
        if (requiredSize > dest.size()) {
            dest.resize(requiredSize, '\0');
        }
        std::memcpy(dest.data() + static_cast<size_t>(destOffset), src.data() + static_cast<size_t>(srcOffset), static_cast<size_t>(size));
    }

    // ============================================================
    // memset - メモリブロックのクリア（HSP互換）
    // ============================================================

    namespace {
        template<typename BufferType>
        void memset_impl(BufferType& dest, int value, int64_t size, int64_t offset, const std::source_location& location) {
            if (offset < 0) {
                throw HspError(ERR_OUT_OF_RANGE, "memsetのオフセットが負の値です", location);
            }
            
            // size=0 の場合は全体をクリア
            size_t actualSize = (size <= 0) ? dest.size() - static_cast<size_t>(offset) : static_cast<size_t>(size);
            
            if (static_cast<size_t>(offset) + actualSize > dest.size()) {
                throw HspError(ERR_BUFFER_OVERFLOW, "memsetがバッファ範囲を超えています", location);
            }
            
            std::memset(dest.data() + static_cast<size_t>(offset), value & 0xFF, actualSize);
        }
    }

    void memset(std::string& dest, int value, int64_t size, int64_t offset, const std::source_location& location) {
        memset_impl(dest, value, size, offset, location);
    }

    void memset(std::vector<uint8_t>& dest, int value, int64_t size, int64_t offset, const std::source_location& location) {
        memset_impl(dest, value, size, offset, location);
    }

    // ============================================================
    // memexpand - メモリブロックの再確保（HSP互換）
    // ============================================================

    namespace {
        template<typename BufferType, typename FillValue>
        void memexpand_impl(BufferType& dest, int64_t newSize, FillValue fillValue) {
            // 最小サイズは64（HSP互換）
            int64_t actualSize = (newSize < 64) ? 64 : newSize;
            
            // 既存のサイズより小さい場合は何もしない（HSP互換）
            if (static_cast<size_t>(actualSize) <= dest.size()) {
                return;
            }
            
            // 以前の内容を保持しつつサイズを拡張
            dest.resize(static_cast<size_t>(actualSize), fillValue);
        }
    }

    void memexpand(std::string& dest, int64_t newSize, const std::source_location& location) {
        memexpand_impl(dest, newSize, '\0');
    }

    void memexpand(std::vector<uint8_t>& dest, int64_t newSize, const std::source_location& location) {
        memexpand_impl(dest, newSize, static_cast<uint8_t>(0));
    }

} // namespace hsppp
