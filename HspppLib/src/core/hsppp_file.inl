// HspppLib/src/core/hsppp_file.inl
// ファイル操作命令・ダイアログ命令の実装

// 注: ヘッダーのインポートは hsppp.cpp のグローバルモジュールフラグメントで行われています
// Windows API: ShellExecuteExW, CreateDirectoryW, DeleteFileW, CopyFileW, FindFirstFileW等を使用
// 必要なヘッダー: <shellapi.h>, <commdlg.h>

namespace hsppp {

    // ============================================================
    // ヘルパー関数
    // ============================================================
    namespace internal {
        // UTF-8からUTF-16への変換
        inline std::wstring Utf8ToWide(const std::string& utf8String) {
            if (utf8String.empty()) {
                return L"";
            }
            int len = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, nullptr, 0);
            if (len > 0) {
                std::wstring result(len - 1, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, result.data(), len);
                return result;
            }
            return L"";
        }

        // UTF-16からUTF-8への変換（WideToUtf8は hsppp_system.inl で定義済み）
    }

    // ============================================================
    // exec - Windowsのファイルを実行する（HSP互換）
    // ============================================================
    // mode 0:  ノーマル実行
    // mode 2:  最小化モードで実行
    // mode 16: 関連付けされたアプリケーションを実行
    // mode 32: ファイルを印刷する

    int exec(const std::string& filename, OptInt mode, const std::string& command,
              const std::source_location& location) {
        int execMode = mode.value_or(0);
        std::wstring filenameW = internal::Utf8ToWide(filename);
        // commandW は sei.lpVerb で使用するため、if ブロック外で宣言して寿命を保証
        std::wstring commandW;

        SHELLEXECUTEINFOW sei = {};
        sei.cbSize = sizeof(sei);
        // SEE_MASK_FLAG_NO_UI: エラーダイアログを抑制（HSP互換動作）
        // 失敗時は GetLastError() の戻り値で呼び出し側が判断する
        sei.fMask = SEE_MASK_FLAG_NO_UI;
        sei.hwnd = nullptr;
        sei.nShow = (execMode & exec_minimized) ? SW_SHOWMINIMIZED : SW_SHOWNORMAL;

        if (!command.empty()) {
            // コマンド（操作名）が指定された場合
            commandW = internal::Utf8ToWide(command);
            sei.lpVerb = commandW.c_str();
            sei.lpFile = filenameW.c_str();
            sei.lpParameters = nullptr;
        }
        else if (execMode & exec_print) {
            // 印刷モード
            sei.lpVerb = L"print";
            sei.lpFile = filenameW.c_str();
        }
        else if (execMode & exec_shellexec) {
            // 関連付けアプリで開く
            sei.lpVerb = L"open";
            sei.lpFile = filenameW.c_str();
        }
        else {
            // ノーマル実行（プログラムを直接実行）
            // ファイル名とパラメータを分離する
            // ShellExecuteExWは賢く解釈してくれるため、手動での分割は不要かつ危険
            sei.lpVerb = nullptr;  // デフォルト動作
            sei.lpFile = filenameW.c_str();
            sei.lpParameters = nullptr;
        }

        if (ShellExecuteExW(&sei)) {
            return 0;  // 成功
        }
        else {
            return static_cast<int>(GetLastError());  // エラーコード
        }
    }

    // ============================================================
    // chdir - ディレクトリ移動（HSP互換）
    // ============================================================

    // 注意: chdir() はプロセス全体のカレントディレクトリを変更します。
    // 相対パスを使用する他のコードに影響を与える可能性があります。
    void chdir(const std::string& dirname, const std::source_location& location) {
        std::wstring dirnameW = internal::Utf8ToWide(dirname);
        if (!SetCurrentDirectoryW(dirnameW.c_str())) {
            DWORD err = GetLastError();
            std::string msg = "ディレクトリの変更に失敗しました (Windows error: " + std::to_string(err) + ")";
            throw HspError(12, msg, location);
        }
    }

    // ============================================================
    // mkdir - ディレクトリ作成（HSP互換）
    // ============================================================

    void mkdir(const std::string& dirname, const std::source_location& location) {
        std::wstring dirnameW = internal::Utf8ToWide(dirname);
        if (!CreateDirectoryW(dirnameW.c_str(), nullptr)) {
            DWORD err = GetLastError();
            if (err != ERROR_ALREADY_EXISTS) {
                throw HspError(12, "ファイルが見つからないか無効な名前です", location);
            }
        }
    }

    // ============================================================
    // deletefile - ファイル削除（HSP互換）
    // HSPの delete は C++ の予約語なので deletefile に変更
    // ============================================================

    void deletefile(const std::string& filename, const std::source_location& location) {
        std::wstring filenameW = internal::Utf8ToWide(filename);
        if (!DeleteFileW(filenameW.c_str())) {
            DWORD err = GetLastError();
            std::string msg = "ファイルの削除に失敗しました (Windows error: " + std::to_string(err) + ")";
            throw HspError(12, msg, location);
        }
    }

    // ============================================================
    // bcopy - ファイルのコピー（HSP互換）
    // ============================================================

    // 注意: bcopy() は既存ファイルを警告なしに上書きします（HSP互換動作）。
    void bcopy(const std::string& src, const std::string& dest, const std::source_location& location) {
        std::wstring srcW = internal::Utf8ToWide(src);
        std::wstring destW = internal::Utf8ToWide(dest);
        if (!CopyFileW(srcW.c_str(), destW.c_str(), FALSE)) {
            DWORD err = GetLastError();
            std::string msg = "ファイルのコピーに失敗しました (Windows error: " + std::to_string(err) + ")";
            throw HspError(12, msg, location);
        }
    }

    // ============================================================
    // exist - ファイルのサイズ取得（HSP互換）
    // ============================================================

    int64_t exist(const std::string& filename, const std::source_location& location) {
        std::wstring filenameW = internal::Utf8ToWide(filename);
        WIN32_FILE_ATTRIBUTE_DATA fileInfo;
        
        if (GetFileAttributesExW(filenameW.c_str(), GetFileExInfoStandard, &fileInfo)) {
            // ファイルサイズを取得
            LARGE_INTEGER fileSize;
            fileSize.LowPart = fileInfo.nFileSizeLow;
            fileSize.HighPart = fileInfo.nFileSizeHigh;
            return static_cast<int64_t>(fileSize.QuadPart);
        }
        else {
            // ファイルが存在しない
            return -1;
        }
    }

    // ============================================================
    // dirlist - ディレクトリ一覧を取得（HSP互換）
    // ============================================================
    // mode 0: すべてのファイル
    // mode 1: ディレクトリを除くすべてのファイル
    // mode 2: 隠し属性・システム属性を除くすべてのファイル
    // mode 3: ディレクトリ・隠し属性・システム属性以外のすべてのファイル
    // mode 5: ディレクトリのみ
    // mode 6: 隠し属性・システム属性ファイルのみ
    // mode 7: ディレクトリと隠し属性・システム属性ファイルのみ

    std::vector<std::string> dirlist(const std::string& filemask, OptInt mode, const std::source_location& location) {
        int dirMode = mode.value_or(0);
        std::wstring maskW = internal::Utf8ToWide(filemask);
        
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(maskW.c_str(), &findData);
        
        if (hFind == INVALID_HANDLE_VALUE) {
            return {};
        }

        std::vector<std::string> result;

        do {
            // "." と ".." をスキップ
            if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) {
                continue;
            }

            bool isDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            bool isHidden = (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
            bool isSystem = (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0;
            bool isHiddenOrSystem = isHidden || isSystem;

            // モードに応じてフィルタリング
            bool include = false;
            switch (dirMode) {
                case 0:  // すべてのファイル
                    include = true;
                    break;
                case 1:  // ディレクトリを除く
                    include = !isDir;
                    break;
                case 2:  // 隠し・システム属性を除く
                    include = !isHiddenOrSystem;
                    break;
                case 3:  // ディレクトリ・隠し・システム属性を除く
                    include = !isDir && !isHiddenOrSystem;
                    break;
                case 5:  // ディレクトリのみ
                    include = isDir;
                    break;
                case 6:  // 隠し・システム属性のみ
                    include = isHiddenOrSystem;
                    break;
                case 7:  // ディレクトリと隠し・システム属性のみ
                    include = isDir || isHiddenOrSystem;
                    break;
                default:
                    include = true;
                    break;
            }

            if (include) {
                result.push_back(internal::WideToUtf8(findData.cFileName));
            }
        } while (FindNextFileW(hFind, &findData));

        FindClose(hFind);
        return result;
    }

    // ============================================================
    // bload - バッファにファイルをロード（HSP互換）
    // ============================================================

    namespace {
        // 1回の読み込み/書き込み上限 (DWORD最大値)
        // Windows max マクロとの衝突を避けるため括弧で囲む
        constexpr int64_t kMaxChunkSize = static_cast<int64_t>((std::numeric_limits<DWORD>::max)());
        // 自動バッファ確保の上限 (2GB) - メモリ不足を防止
        constexpr int64_t kMaxAutoAllocSize = 2LL * 1024 * 1024 * 1024;

        template<typename BufferType>
        int64_t bload_impl(const std::string& filename, BufferType& buffer, OptInt64 size, OptInt64 offset,
                      const std::source_location& location) {
            std::wstring filenameW = internal::Utf8ToWide(filename);
            
            HANDLE hFile = CreateFileW(filenameW.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                       nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            
            if (hFile == INVALID_HANDLE_VALUE) {
                DWORD err = GetLastError();
                std::string msg = "ファイルを開けません (Windows error: " + std::to_string(err) + ")";
                throw HspError(12, msg, location);
            }

            // ファイルサイズを取得
            LARGE_INTEGER fileSize;
            if (!GetFileSizeEx(hFile, &fileSize)) {
                DWORD err = GetLastError();
                CloseHandle(hFile);
                std::string msg = "ファイルサイズの取得に失敗しました (Windows error: " + std::to_string(err) + ")";
                throw HspError(12, msg, location);
            }

            int64_t fileOffset = offset.value_or(0);
            if (fileOffset < 0) fileOffset = 0;

            // オフセット位置に移動
            if (fileOffset > 0) {
                LARGE_INTEGER li;
                li.QuadPart = fileOffset;
                if (!SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN)) {
                    DWORD err = GetLastError();
                    CloseHandle(hFile);
                    std::string msg = "ファイルオフセットの設定に失敗しました (Windows error: " + std::to_string(err) + ")";
                    throw HspError(12, msg, location);
                }
            }

            // 残りファイルサイズを計算
            int64_t fileRemaining = fileSize.QuadPart - fileOffset;
            if (fileRemaining < 0) fileRemaining = 0;

            // 読み込みサイズを決定
            int64_t readSize = size.value_or(-1);
            if (readSize < 0) {
                // 自動サイズ（バッファサイズまたはファイル残りサイズ）
                readSize = static_cast<int64_t>(buffer.size());
                if (readSize == 0) {
                    readSize = fileRemaining;
                    // 自動確保の上限チェック
                    if (readSize > kMaxAutoAllocSize) {
                        CloseHandle(hFile);
                        throw HspError(12, "ファイルが大きすぎます (自動確保上限: 2GB)", location);
                    }
                    buffer.resize(static_cast<size_t>(readSize));
                } else {
                    // バッファサイズがファイル残りを超える場合は残りサイズに制限
                    if (readSize > fileRemaining) {
                        readSize = fileRemaining;
                    }
                }
            }
            else {
                if (static_cast<size_t>(readSize) > buffer.size()) {
                    buffer.resize(static_cast<size_t>(readSize));
                }
            }

            // 読み込み (4GBを超える場合は分割読み込み)
            int64_t totalRead = 0;
            uint8_t* dst = reinterpret_cast<uint8_t*>(buffer.data());
            int64_t remaining = readSize;

            while (remaining > 0) {
                DWORD chunkSize = (remaining > kMaxChunkSize)
                    ? static_cast<DWORD>(kMaxChunkSize)
                    : static_cast<DWORD>(remaining);

                DWORD bytesRead = 0;
                if (!ReadFile(hFile, dst, chunkSize, &bytesRead, nullptr)) {
                    DWORD err = GetLastError();
                    CloseHandle(hFile);
                    std::string msg = "ファイルの読み込みに失敗しました (Windows error: " + std::to_string(err) + ")";
                    throw HspError(12, msg, location);
                }

                if (bytesRead == 0) {
                    // EOF
                    break;
                }

                totalRead += static_cast<int64_t>(bytesRead);
                remaining -= static_cast<int64_t>(bytesRead);
                dst += bytesRead;
            }

            CloseHandle(hFile);
            return totalRead;
        }
    }

    int64_t bload(const std::string& filename, std::string& buffer, OptInt64 size, OptInt64 offset,
              const std::source_location& location) {
        return bload_impl(filename, buffer, size, offset, location);
    }

    int64_t bload(const std::string& filename, std::vector<uint8_t>& buffer, OptInt64 size, OptInt64 offset,
              const std::source_location& location) {
        return bload_impl(filename, buffer, size, offset, location);
    }

    // ============================================================
    // bsave - バッファをファイルにセーブ（HSP互換）
    // ============================================================

    namespace {
        template<typename BufferType>
        int64_t bsave_impl(const std::string& filename, const BufferType& buffer, OptInt64 size, OptInt64 offset,
                       const std::source_location& location) {
            std::wstring filenameW = internal::Utf8ToWide(filename);
            
            int64_t fileOffset = offset.value_or(-1);
            
            DWORD createMode = CREATE_ALWAYS;
            if (fileOffset >= 0) {
                // オフセット指定時はファイルが存在する必要がある
                createMode = OPEN_EXISTING;
            }

            HANDLE hFile = CreateFileW(filenameW.c_str(), GENERIC_WRITE, 0,
                                       nullptr, createMode, FILE_ATTRIBUTE_NORMAL, nullptr);
            
            if (hFile == INVALID_HANDLE_VALUE) {
                DWORD err = GetLastError();
                std::string msg = "ファイルを開けません (Windows error: " + std::to_string(err) + ")";
                throw HspError(12, msg, location);
            }

            // オフセット位置に移動
            if (fileOffset > 0) {
                LARGE_INTEGER li;
                li.QuadPart = fileOffset;
                if (!SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN)) {
                    DWORD err = GetLastError();
                    CloseHandle(hFile);
                    std::string msg = "ファイルオフセットの設定に失敗しました (Windows error: " + std::to_string(err) + ")";
                    throw HspError(12, msg, location);
                }
            }

            // 書き込みサイズを決定
            int64_t writeSize = size.value_or(-1);
            if (writeSize < 0) {
                writeSize = static_cast<int64_t>(buffer.size());
            }
            else {
                writeSize = (std::min)(writeSize, static_cast<int64_t>(buffer.size()));
            }

            // 書き込み (4GBを超える場合は分割書き込み)
            int64_t totalWritten = 0;
            const uint8_t* src = reinterpret_cast<const uint8_t*>(buffer.data());
            int64_t remaining = writeSize;

            while (remaining > 0) {
                DWORD chunkSize = (remaining > kMaxChunkSize)
                    ? static_cast<DWORD>(kMaxChunkSize)
                    : static_cast<DWORD>(remaining);

                DWORD bytesWritten = 0;
                if (!WriteFile(hFile, src, chunkSize, &bytesWritten, nullptr)) {
                    DWORD err = GetLastError();
                    CloseHandle(hFile);
                    std::string msg = "ファイルの書き込みに失敗しました (Windows error: " + std::to_string(err) + ")";
                    throw HspError(12, msg, location);
                }

                totalWritten += static_cast<int64_t>(bytesWritten);
                remaining -= static_cast<int64_t>(bytesWritten);
                src += bytesWritten;

                if (bytesWritten == 0) {
                    // 書き込みが進まない場合は中断
                    break;
                }
            }

            CloseHandle(hFile);
            return totalWritten;
        }
    }

    int64_t bsave(const std::string& filename, const std::string& buffer, OptInt64 size, OptInt64 offset,
               const std::source_location& location) {
        return bsave_impl(filename, buffer, size, offset, location);
    }

    int64_t bsave(const std::string& filename, const std::vector<uint8_t>& buffer, OptInt64 size, OptInt64 offset,
               const std::source_location& location) {
        return bsave_impl(filename, buffer, size, offset, location);
    }

    // ============================================================
    // dialog - ダイアログを開く（HSP互換）
    // ============================================================
    // type 0-3:  メッセージボックス
    // type 16:   ファイルOPEN(開く)ダイアログ
    // type 17:   ファイルSAVE(保存)ダイアログ
    // type 32-33: カラー選択ダイアログ

    DialogResult dialog(const std::string& message, OptInt type, const std::string& option,
               const std::source_location& location) {
        int dialogType = type.value_or(0);
        
        // メッセージボックス (type 0-3)
        if (dialogType >= 0 && dialogType <= 3) {
            std::wstring messageW = internal::Utf8ToWide(message);
            std::wstring titleW = internal::Utf8ToWide(option);
            
            UINT mbType = MB_OK;
            if (dialogType == 1) {
                mbType = MB_OK | MB_ICONWARNING;
            }
            else if (dialogType == 2) {
                mbType = MB_YESNO;
            }
            else if (dialogType == 3) {
                mbType = MB_YESNO | MB_ICONWARNING;
            }

            int result = MessageBoxW(nullptr, messageW.c_str(), 
                                    titleW.empty() ? nullptr : titleW.c_str(), mbType);
            return { result, std::to_string(result) };
        }

        // ファイルOPEN/SAVEダイアログ (type 16-17)
        if (dialogType == dialog_open || dialogType == dialog_save) {
            wchar_t filenameBuffer[MAX_PATH] = {};
            
            // フィルタ文字列を構築
            // message: 拡張子（例: "txt" または "txt|log"）
            // option: 説明（例: "テキストファイル" または "テキストファイル|ログファイル"）
            std::wstring filterW;
            
            // ローカルでパイプ区切りを解析するラムダ（split への依存を排除）
            auto parsePipeSeparated = [](const std::string& input) {
                std::vector<std::string> parts;
                std::string current;
                for (char ch : input) {
                    if (ch == '|') {
                        parts.push_back(current);
                        current.clear();
                    } else {
                        current.push_back(ch);
                    }
                }
                parts.push_back(current);
                return parts;
            };
            
            if (message.empty() || message == "*") {
                filterW = L"すべてのファイル\0*.*\0\0";
            }
            else {
                try {
                    // HSP形式のフィルタを解析（|区切り）
                    auto exts = parsePipeSeparated(message);
                    auto descs = parsePipeSeparated(option);
                    
                    for (size_t i = 0; i < exts.size(); i++) {
                        std::wstring extW = internal::Utf8ToWide(exts[i]);
                        std::wstring descW = (i < descs.size()) ? internal::Utf8ToWide(descs[i]) : extW;
                        
                        filterW += descW;
                        filterW += L'\0';
                        filterW += L"*.";
                        filterW += extW;
                        filterW += L'\0';
                    }
                    filterW += L'\0';
                }
                catch (...) {
                    // 予期しない例外が発生した場合は汎用フィルタにフォールバック
                    filterW = L"すべてのファイル\0*.*\0\0";
                }
            }

            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = nullptr;
            ofn.lpstrFilter = filterW.c_str();
            ofn.lpstrFile = filenameBuffer;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
            
            if (dialogType == dialog_open) {
                ofn.Flags |= OFN_FILEMUSTEXIST;
            }
            else {
                ofn.Flags |= OFN_OVERWRITEPROMPT;
            }

            BOOL result;
            if (dialogType == dialog_open) {
                result = GetOpenFileNameW(&ofn);
            }
            else {
                result = GetSaveFileNameW(&ofn);
            }

            if (result) {
                std::string path = internal::WideToUtf8(filenameBuffer);
                return { 1, path };
            }
            else {
                return { 0, "" };
            }
        }

        // カラー選択ダイアログ (type 32-33)
        if (dialogType == dialog_color || dialogType == dialog_colorex) {
            static COLORREF customColors[16] = {};
            
            CHOOSECOLORW cc = {};
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = nullptr;
            cc.lpCustColors = customColors;
            cc.Flags = CC_RGBINIT;
            
            if (dialogType == dialog_colorex) {
                cc.Flags |= CC_FULLOPEN;  // 拡張ダイアログ（RGB自由選択）
            }

            // オーナーウィンドウを設定してダイアログのモーダル処理を安定させる
            // （オーナー未設定だとモードの扱いでアプリケーションが応答しなくなる環境がある）
            auto current = getCurrentSurface();
            auto pWindow = current ? std::dynamic_pointer_cast<internal::HspWindow>(current) : nullptr;
            if (pWindow && pWindow->getHwnd()) {
                cc.hwndOwner = pWindow->getHwnd();
            }

            if (ChooseColorW(&cc)) {
                // 選択された色をカレントサーフェスのcolorに設定
                // これにより ginfo_r/g/b で取得可能になる
                hsppp::color(
                    GetRValue(cc.rgbResult),
                    GetGValue(cc.rgbResult),
                    GetBValue(cc.rgbResult)
                );
                return { 1, "1" };
            }
            else {
                return { 0, "0" };
            }
        }

        // 未サポートのタイプ
        return { 0, "0" };
    }

} // namespace hsppp
