// HspppLib/src/core/hsppp_string.inl
// 文字列操作関数の実装（HSP互換）
// 注: cstdio, cctypeはhsppp.cppのグローバルモジュールフラグメントでインクルード済み

namespace hsppp {

    namespace {
        std::string* g_noteSelected = nullptr;
        std::vector<std::string*> g_noteSelectedStack;

        std::string& requireNoteSelected(const std::source_location& location) {
            if (!g_noteSelected) {
                throw HspError(ERR_ILLEGAL_FUNCTION, "noteselが必要です", location);
            }
            return *g_noteSelected;
        }

        // ============================================================
        // note系ヘルパー関数（最適化版）
        // ============================================================

        // 行数をカウント（vector生成なし）
        size_t countNoteLines(std::string_view buffer) {
            if (buffer.empty()) return 0;
            return static_cast<size_t>(std::count(buffer.begin(), buffer.end(), '\n')) + 1;
        }

        // n番目の行の開始位置と長さを取得（\r除去後の長さ）
        // 戻り値: 見つかった場合 true、範囲外は false
        bool findNoteLine(std::string_view buffer, size_t lineIndex, size_t& outStart, size_t& outLen) {
            if (buffer.empty()) return false;

            size_t currentLine = 0;
            size_t start = 0;

            while (start <= buffer.size()) {
                size_t end = buffer.find('\n', start);
                if (end == std::string_view::npos) {
                    end = buffer.size();
                }

                if (currentLine == lineIndex) {
                    size_t len = end - start;
                    // 末尾の \r を除去
                    if (len > 0 && buffer[start + len - 1] == '\r') {
                        --len;
                    }
                    outStart = start;
                    outLen = len;
                    return true;
                }

                if (end >= buffer.size()) break;
                start = end + 1;
                ++currentLine;
            }

            return false;
        }

        // n番目の行の範囲（改行含む）を取得（削除用）
        // 戻り値: 見つかった場合 true
        bool findNoteLineRange(std::string_view buffer, size_t lineIndex, size_t& outStart, size_t& outEnd) {
            if (buffer.empty()) return false;

            size_t currentLine = 0;
            size_t start = 0;
            size_t lineCount = countNoteLines(buffer);

            while (start <= buffer.size()) {
                size_t end = buffer.find('\n', start);
                if (end == std::string_view::npos) {
                    end = buffer.size();
                }

                if (currentLine == lineIndex) {
                    outStart = start;
                    // 最後の行でなければ改行も含める
                    if (lineIndex < lineCount - 1 && end < buffer.size()) {
                        outEnd = end + 1;  // \n を含む
                    } else {
                        outEnd = end;
                    }
                    // 先頭行以外で最後の行を削除する場合、直前の改行も削除
                    if (lineIndex > 0 && lineIndex == lineCount - 1 && outStart > 0) {
                        outStart--;  // 直前の \n を含める
                    }
                    return true;
                }

                if (end >= buffer.size()) break;
                start = end + 1;
                ++currentLine;
            }

            return false;
        }

        // n番目の行の挿入位置を取得
        // lineIndex == lineCount の場合は末尾への追加
        bool findNoteInsertPos(std::string_view buffer, size_t lineIndex, size_t& outPos) {
            size_t lineCount = countNoteLines(buffer);

            if (lineIndex > lineCount) return false;

            if (buffer.empty() || lineIndex == 0) {
                outPos = 0;
                return true;
            }

            if (lineIndex == lineCount) {
                // 末尾に追加
                outPos = buffer.size();
                return true;
            }

            // lineIndex 番目の行の先頭を探す
            size_t currentLine = 0;
            size_t start = 0;

            while (start < buffer.size()) {
                if (currentLine == lineIndex) {
                    outPos = start;
                    return true;
                }

                size_t end = buffer.find('\n', start);
                if (end == std::string_view::npos) break;
                start = end + 1;
                ++currentLine;
            }

            return false;
        }

        // 旧来のparseNoteLines（noteadd上書きモード等で必要な場合用）
        std::vector<std::string> parseNoteLines(std::string_view buffer) {
            std::vector<std::string> lines;
            if (buffer.empty()) return lines;

            size_t start = 0;
            while (start <= buffer.size()) {
                size_t end = buffer.find('\n', start);
                if (end == std::string_view::npos) {
                    end = buffer.size();
                }

                size_t len = end - start;
                if (len > 0 && buffer[start + len - 1] == '\r') {
                    --len;
                }

                lines.emplace_back(buffer.substr(start, len));

                if (end >= buffer.size()) break;
                start = end + 1;
            }

            return lines;
        }

        std::string joinNoteLines(const std::vector<std::string>& lines) {
            if (lines.empty()) return "";

            size_t total = 0;
            for (const auto& line : lines) {
                total += line.size();
            }
            total += (lines.size() - 1);

            std::string out;
            out.reserve(total);
            for (size_t i = 0; i < lines.size(); ++i) {
                if (i != 0) out.push_back('\n');
                out += lines[i];
            }
            return out;
        }
    }

    // ============================================================
    // NotePad クラス実装
    // ============================================================

    NotePad::NotePad(std::string_view text)
        : m_buffer(text) {}

    NotePad::NotePad(std::string&& text) noexcept
        : m_buffer(std::move(text)) {}

    size_t NotePad::count() const noexcept {
        if (m_buffer.empty()) return 0;
        
        size_t newlineCount = static_cast<size_t>(std::count(m_buffer.begin(), m_buffer.end(), '\n'));
        
        // 末尾が改行で終わっている場合は、その後の空行はカウントしない（HSP互換）
        // "test\ntest" → 2行
        // "test\ntest\n" → 2行（末尾改行は無視）
        if (!m_buffer.empty() && m_buffer.back() == '\n') {
            return newlineCount;
        }
        return newlineCount + 1;
    }

    std::string NotePad::get(size_t index) const {
        if (m_buffer.empty()) return "";

        size_t currentLine = 0;
        size_t start = 0;

        while (start <= m_buffer.size()) {
            size_t end = m_buffer.find('\n', start);
            if (end == std::string::npos) {
                end = m_buffer.size();
            }

            if (currentLine == index) {
                size_t len = end - start;
                // 末尾の \r を除去
                if (len > 0 && m_buffer[start + len - 1] == '\r') {
                    --len;
                }
                return m_buffer.substr(start, len);
            }

            if (end >= m_buffer.size()) break;
            start = end + 1;
            ++currentLine;
        }

        return "";
    }

    NotePad& NotePad::add(std::string_view text, int index, int overwrite) {
        size_t lineCount = count();

        // 末尾追加
        if (index < 0 || static_cast<size_t>(index) >= lineCount) {
            if (!m_buffer.empty() && m_buffer.back() != '\n') {
                m_buffer.push_back('\n');
            }
            m_buffer.append(text);
            return *this;
        }

        // 指定位置への挿入/上書き
        size_t currentLine = 0;
        size_t start = 0;

        while (start <= m_buffer.size()) {
            size_t end = m_buffer.find('\n', start);
            if (end == std::string::npos) {
                end = m_buffer.size();
            }

            if (currentLine == static_cast<size_t>(index)) {
                if (overwrite != 0) {
                    // 上書きモード：現在の行を置換
                    m_buffer.replace(start, end - start, text);
                } else {
                    // 挿入モード：現在の行の前に挿入
                    std::string insertText(text);
                    insertText.push_back('\n');
                    m_buffer.insert(start, insertText);
                }
                return *this;
            }

            if (end >= m_buffer.size()) break;
            start = end + 1;
            ++currentLine;
        }

        return *this;
    }

    NotePad& NotePad::del(size_t index) {
        if (m_buffer.empty()) return *this;

        size_t lineCount = count();
        if (index >= lineCount) return *this;

        size_t currentLine = 0;
        size_t start = 0;

        while (start <= m_buffer.size()) {
            size_t end = m_buffer.find('\n', start);
            if (end == std::string::npos) {
                end = m_buffer.size();
            }

            if (currentLine == index) {
                size_t eraseStart = start;
                size_t eraseEnd = end;

                // 最後の行でなければ改行も削除
                if (index < lineCount - 1 && eraseEnd < m_buffer.size()) {
                    eraseEnd++;  // \n を含む
                }
                // 先頭行以外で最後の行を削除する場合、直前の改行も削除
                else if (index > 0 && index == lineCount - 1 && eraseStart > 0) {
                    eraseStart--;  // 直前の \n を含める
                }

                m_buffer.erase(eraseStart, eraseEnd - eraseStart);
                return *this;
            }

            if (end >= m_buffer.size()) break;
            start = end + 1;
            ++currentLine;
        }

        return *this;
    }

    int NotePad::find(std::string_view search, int mode, size_t startIndex) const {
        if (m_buffer.empty()) return -1;

        size_t currentLine = 0;
        size_t start = 0;

        while (start <= m_buffer.size()) {
            size_t end = m_buffer.find('\n', start);
            if (end == std::string::npos) {
                end = m_buffer.size();
            }

            if (currentLine >= startIndex) {
                size_t len = end - start;
                // 末尾の \r を除去して比較
                if (len > 0 && m_buffer[start + len - 1] == '\r') {
                    --len;
                }
                std::string_view line(m_buffer.data() + start, len);

                bool found = false;
                switch (mode) {
                    case 0:  // 完全一致
                        found = (line == search);
                        break;
                    case 1:  // 先頭一致
                        found = (line.size() >= search.size() &&
                                 line.compare(0, search.size(), search) == 0);
                        break;
                    case 2:  // 部分一致
                        found = (line.find(search) != std::string_view::npos);
                        break;
                    default:
                        found = (line == search);
                        break;
                }

                if (found) {
                    return static_cast<int>(currentLine);
                }
            }

            if (end >= m_buffer.size()) break;
            start = end + 1;
            ++currentLine;
        }

        return -1;
    }

    NotePad& NotePad::load(std::string_view filename, size_t maxSize) {
        std::wstring wideFilename = internal::Utf8ToWide(filename);

        HANDLE hFile = CreateFileW(
            wideFilename.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            m_buffer.clear();
            return *this;
        }

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize)) {
            CloseHandle(hFile);
            m_buffer.clear();
            return *this;
        }

        size_t readSize = static_cast<size_t>(fileSize.QuadPart);
        if (maxSize > 0 && readSize > maxSize) {
            readSize = maxSize;
        }

        m_buffer.resize(readSize);
        DWORD bytesRead = 0;
        if (!ReadFile(hFile, m_buffer.data(), static_cast<DWORD>(readSize), &bytesRead, nullptr)) {
            CloseHandle(hFile);
            m_buffer.clear();
            return *this;
        }
        CloseHandle(hFile);

        m_buffer.resize(bytesRead);
        return *this;
    }

    [[nodiscard]] bool NotePad::save(std::string_view filename) const {
        std::wstring wideFilename = internal::Utf8ToWide(filename);

        HANDLE hFile = CreateFileW(
            wideFilename.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        DWORD bytesWritten = 0;
        BOOL success = WriteFile(hFile, m_buffer.data(), static_cast<DWORD>(m_buffer.size()), &bytesWritten, nullptr);
        CloseHandle(hFile);

        return success && (bytesWritten == m_buffer.size());
    }

    // ============================================================
    // instr - 文字列の検索
    // ============================================================
    
    int64_t instr(const std::string& p1, int64_t p2, const std::string& search) {
        // p2がマイナス値の場合は常に-1を返す（HSP仕様）
        if (p2 < 0) {
            return -1;
        }
        
        // p2が文字列長を超える場合は-1を返す
        if (static_cast<size_t>(p2) >= p1.size()) {
            return -1;
        }
        
        // 検索文字列が空の場合は0を返す
        if (search.empty()) {
            return 0;
        }
        
        // p2の位置から検索開始
        size_t pos = p1.find(search, static_cast<size_t>(p2));
        
        if (pos == std::string::npos) {
            return -1;
        }
        
        // 結果はp2を起点(0)とするインデックス
        return static_cast<int64_t>(pos - p2);
    }

    int64_t instr(const std::string& p1, const std::string& search) {
        return instr(p1, 0, search);
    }

    // ============================================================
    // strmid - 文字列の一部を取り出す
    // ============================================================
    
    std::string strmid(const std::string& p1, int64_t p2, int64_t p3) {
        if (p3 <= 0) {
            return "";
        }
        
        if (p2 == -1) {
            // 右からp3文字を取り出す
            if (static_cast<size_t>(p3) >= p1.size()) {
                return p1;
            }
            return p1.substr(p1.size() - p3);
        }
        
        // 通常の左からの取り出し
        if (p2 < 0) {
            return "";  // 負のインデックス（-1以外）は空文字列
        }
        
        if (static_cast<size_t>(p2) >= p1.size()) {
            return "";  // 開始位置が文字列長を超える
        }
        
        return p1.substr(static_cast<size_t>(p2), static_cast<size_t>(p3));
    }

    // ============================================================
    // strtrim - 指定した文字だけを取り除く
    // ============================================================
    
    std::string strtrim(const std::string& p1, int p2, int p3) {
        if (p1.empty()) {
            return "";
        }
        
        std::string result = p1;
        
        // p3が2バイト文字の場合（全角文字対応）
        // 1バイト文字（0-255）および2バイト文字（Shift-JIS等、上位バイト+下位バイト）に対応
        // 注意: UTF-8のような可変長マルチバイト文字列では正確に動作しない可能性あり
        char targetBytes[3] = {0};
        size_t targetLen = 1;
        
        if (p3 > 255) {
            // Shift-JIS等の2バイト文字: 上位バイトと下位バイトに分解
            targetBytes[0] = static_cast<char>((p3 >> 8) & 0xFF);
            targetBytes[1] = static_cast<char>(p3 & 0xFF);
            targetLen = 2;
        } else {
            targetBytes[0] = static_cast<char>(p3 & 0xFF);
        }
        
        std::string_view targetView(targetBytes, targetLen);
        
        auto matchTarget = [&](size_t pos) -> bool {
            if (pos + targetLen > result.size()) return false;
            for (size_t i = 0; i < targetLen; ++i) {
                if (result[pos + i] != targetBytes[i]) return false;
            }
            return true;
        };
        
        switch (p2) {
            case 0:  // 両端にある指定文字を除去する
            {
                // 左端
                while (!result.empty() && matchTarget(0)) {
                    result.erase(0, targetLen);
                }
                // 右端
                while (result.size() >= targetLen && matchTarget(result.size() - targetLen)) {
                    result.erase(result.size() - targetLen);
                }
                break;
            }
            case 1:  // 左端にある指定文字を除去する
            {
                while (!result.empty() && matchTarget(0)) {
                    result.erase(0, targetLen);
                }
                break;
            }
            case 2:  // 右端にある指定文字を除去する
            {
                while (result.size() >= targetLen && matchTarget(result.size() - targetLen)) {
                    result.erase(result.size() - targetLen);
                }
                break;
            }
            case 3:  // 文字列内にあるすべての指定文字を除去する
            {
                std::string newResult;
                newResult.reserve(result.size());
                
                size_t startPos = 0;
                while (startPos < result.size()) {
                    size_t foundPos = result.find(targetView, startPos);
                    if (foundPos == std::string::npos) {
                        // 残りを全て追加
                        newResult.append(result, startPos, std::string::npos);
                        break;
                    }
                    // ターゲットまでの部分を追加
                    newResult.append(result, startPos, foundPos - startPos);
                    startPos = foundPos + targetLen;
                }
                result = std::move(newResult);
                break;
            }
            default:
                // 不正なモードは何もしない
                break;
        }
        
        return result;
    }

    // ============================================================
    // strf - 書式付き文字列を変換
    // ============================================================
    // HSPのstrf互換実装
    // snprintfを使用してprintf形式の書式をサポート
    
    namespace detail {
        // 内部ヘルパー: バッファサイズを動的に調整してsnprintfを実行
        template<typename... Args>
        std::string sprintf_impl(const char* fmt, Args... args) {
            // 最初に必要なバッファサイズを計算
            int size = std::snprintf(nullptr, 0, fmt, args...);
            if (size < 0) {
                return "";  // エラー
            }
            
            std::string result(static_cast<size_t>(size) + 1, '\0');
            std::snprintf(result.data(), result.size(), fmt, args...);
            result.resize(static_cast<size_t>(size));  // null終端を除去
            return result;
        }
    }

    std::string strf(const std::string& format) {
        return format;  // 引数なしはそのまま返す
    }

    std::string strf(const std::string& format, int arg1) {
        return detail::sprintf_impl(format.c_str(), arg1);
    }

    std::string strf(const std::string& format, double arg1) {
        return detail::sprintf_impl(format.c_str(), arg1);
    }

    std::string strf(const std::string& format, const std::string& arg1) {
        return detail::sprintf_impl(format.c_str(), arg1.c_str());
    }

    std::string strf(const std::string& format, int arg1, int arg2) {
        return detail::sprintf_impl(format.c_str(), arg1, arg2);
    }

    std::string strf(const std::string& format, int arg1, double arg2) {
        return detail::sprintf_impl(format.c_str(), arg1, arg2);
    }

    std::string strf(const std::string& format, int arg1, const std::string& arg2) {
        return detail::sprintf_impl(format.c_str(), arg1, arg2.c_str());
    }

    std::string strf(const std::string& format, double arg1, int arg2) {
        return detail::sprintf_impl(format.c_str(), arg1, arg2);
    }

    std::string strf(const std::string& format, double arg1, double arg2) {
        return detail::sprintf_impl(format.c_str(), arg1, arg2);
    }

    std::string strf(const std::string& format, int arg1, int arg2, int arg3) {
        return detail::sprintf_impl(format.c_str(), arg1, arg2, arg3);
    }

    std::string strf(const std::string& format, int arg1, double arg2, const std::string& arg3) {
        return detail::sprintf_impl(format.c_str(), arg1, arg2, arg3.c_str());
    }

    // ============================================================
    // getpath - パスの一部を取得
    // ============================================================
    
    std::string getpath(const std::string& p1, int p2) {
        if (p1.empty()) {
            return "";
        }
        
        std::string result = p1;
        
        // パスの区切り文字を正規化（Windows/Unix両対応）
        auto findLastSeparator = [](const std::string& path) -> size_t {
            return path.find_last_of("/\\");
        };
        
        auto findExtension = [&findLastSeparator](const std::string& path) -> size_t {
            size_t dotPos = path.rfind('.');
            if (dotPos == std::string::npos) {
                return std::string::npos;
            }
            
            // 区切り文字より後にドットがある場合のみ拡張子とみなす
            size_t sepPos = findLastSeparator(path);
            if (sepPos != std::string::npos && dotPos < sepPos) {
                return std::string::npos;  // ドットはディレクトリ名の中
            }
            
            return dotPos;
        };
        
        // タイプ32: ディレクトリ情報のみ
        if (p2 & 32) {
            size_t sepPos = findLastSeparator(result);
            if (sepPos != std::string::npos) {
                result = result.substr(0, sepPos + 1);  // 区切り文字を含む
            } else {
                result = "";  // ディレクトリ情報なし
            }
        }
        
        // タイプ8: ディレクトリ情報を取り除く
        if (p2 & 8) {
            size_t sepPos = findLastSeparator(result);
            if (sepPos != std::string::npos) {
                result = result.substr(sepPos + 1);
            }
        }
        
        // タイプ1: 拡張子を除くファイル名
        if (p2 & 1) {
            size_t extPos = findExtension(result);
            if (extPos != std::string::npos) {
                result = result.substr(0, extPos);
            }
        }
        
        // タイプ2: 拡張子のみ（.???）
        if (p2 & 2) {
            size_t extPos = findExtension(result);
            if (extPos != std::string::npos) {
                result = result.substr(extPos);  // ドットを含む
            } else {
                result = "";  // 拡張子なし
            }
        }
        
        // タイプ16: 文字列を小文字に変換
        if (p2 & 16) {
            std::transform(result.begin(), result.end(), result.begin(),
                [](uint8_t c) { return static_cast<char>(std::tolower(c)); });
        }
        
        return result;
    }

    // ============================================================
    // strrep - 文字列の置換
    // ============================================================
    
    int64_t strrep(std::string& p1, const std::string& search, const std::string& replace, const std::source_location&) {
        if (search.empty()) {
            return 0;  // 検索文字列が空の場合は何もしない
        }
        
        int64_t count = 0;
        size_t pos = 0;
        
        while ((pos = p1.find(search, pos)) != std::string::npos) {
            p1.replace(pos, search.length(), replace);
            pos += replace.length();  // 置換後の文字列の後ろから検索を再開
            ++count;
        }
        
        return count;
    }

    // ============================================================
    // getstr - バッファから文字列読み出し
    // ============================================================
    
    int64_t getstr(std::string& dest, const std::string& src, int64_t index, int delimiter, int64_t maxLen, const std::source_location&) {
        dest.clear();
        
        if (index < 0 || static_cast<size_t>(index) >= src.size()) {
            return 0;
        }
        
        size_t startPos = static_cast<size_t>(index);
        size_t endPos = startPos;
        size_t srcSize = src.size();
        int64_t readCount = 0;
        
        // 区切り文字または改行を探す
        while (endPos < srcSize && readCount < maxLen) {
            uint8_t c = static_cast<uint8_t>(src[endPos]);
            
            // 改行コードのチェック（\r\n, \n, \r）
            if (c == '\r') {
                if (endPos + 1 < srcSize && src[endPos + 1] == '\n') {
                    // \r\n の場合は2バイト消費
                    dest = src.substr(startPos, endPos - startPos);
                    return static_cast<int64_t>(endPos - startPos + 2);
                }
                // \r のみ
                dest = src.substr(startPos, endPos - startPos);
                return static_cast<int64_t>(endPos - startPos + 1);
            }
            
            if (c == '\n') {
                dest = src.substr(startPos, endPos - startPos);
                return static_cast<int64_t>(endPos - startPos + 1);
            }
            
            // NULL終端（delimiter=0のデフォルト時のみ）
            if (c == 0 && delimiter == 0) {
                dest = src.substr(startPos, endPos - startPos);
                return static_cast<int64_t>(endPos - startPos + 1);
            }
            
            // 区切り文字のチェック（delimiter > 0の場合）
            if (delimiter > 0 && c == static_cast<uint8_t>(delimiter)) {
                dest = src.substr(startPos, endPos - startPos);
                return static_cast<int64_t>(endPos - startPos + 1);
            }
            
            ++endPos;
            ++readCount;
        }
        
        // 最大文字数に達した、または文字列の終端に達した
        dest = src.substr(startPos, endPos - startPos);
        return static_cast<int64_t>(endPos - startPos);
    }

    int64_t getstr(std::string& dest, const std::vector<uint8_t>& src, int64_t index, int delimiter, int64_t maxLen, const std::source_location& loc) {
        // vector<uint8_t>をstring_viewとして扱い、同じロジックを適用
        dest.clear();
        
        if (index < 0 || static_cast<size_t>(index) >= src.size()) {
            return 0;
        }
        
        size_t startPos = static_cast<size_t>(index);
        size_t endPos = startPos;
        size_t srcSize = src.size();
        int64_t readCount = 0;
        
        while (endPos < srcSize && readCount < maxLen) {
            uint8_t c = src[endPos];
            
            if (c == '\r') {
                if (endPos + 1 < srcSize && src[endPos + 1] == '\n') {
                    dest.assign(reinterpret_cast<const char*>(&src[startPos]), endPos - startPos);
                    return static_cast<int64_t>(endPos - startPos + 2);
                }
                dest.assign(reinterpret_cast<const char*>(&src[startPos]), endPos - startPos);
                return static_cast<int64_t>(endPos - startPos + 1);
            }
            
            if (c == '\n') {
                dest.assign(reinterpret_cast<const char*>(&src[startPos]), endPos - startPos);
                return static_cast<int64_t>(endPos - startPos + 1);
            }
            
            if (c == 0 && delimiter == 0) {
                dest.assign(reinterpret_cast<const char*>(&src[startPos]), endPos - startPos);
                return static_cast<int64_t>(endPos - startPos + 1);
            }
            
            if (delimiter > 0 && c == static_cast<uint8_t>(delimiter)) {
                dest.assign(reinterpret_cast<const char*>(&src[startPos]), endPos - startPos);
                return static_cast<int64_t>(endPos - startPos + 1);
            }
            
            ++endPos;
            ++readCount;
        }
        
        dest.assign(reinterpret_cast<const char*>(&src[startPos]), endPos - startPos);
        return static_cast<int64_t>(endPos - startPos);
    }

    // ============================================================
    // split - 文字列から分割された要素を取得
    // ============================================================
    
    std::vector<std::string> split(const std::string& src, const std::string& delimiter, const std::source_location&) {
        std::vector<std::string> result;
        
        if (delimiter.empty()) {
            // 区切り文字が空の場合は元の文字列をそのまま返す
            result.push_back(src);
            return result;
        }
        
        size_t start = 0;
        size_t end = src.find(delimiter);
        
        while (end != std::string::npos) {
            result.push_back(src.substr(start, end - start));
            start = end + delimiter.length();
            end = src.find(delimiter, start);
        }
        
        // 最後の要素を追加
        result.push_back(src.substr(start));
        
        return result;
    }

    // ============================================================
    // メモリノートパッド命令セット（HSP互換）
    // ============================================================

    void notesel(std::string& buffer, const std::source_location&) {
        g_noteSelectedStack.push_back(g_noteSelected);
        g_noteSelected = &buffer;
    }

    void noteunsel(const std::source_location&) {
        if (g_noteSelectedStack.empty()) {
            g_noteSelected = nullptr;
            return;
        }

        g_noteSelected = g_noteSelectedStack.back();
        g_noteSelectedStack.pop_back();
    }

    void noteadd(std::string_view text, OptInt index, OptInt overwrite, const std::source_location& location) {
        std::string& buffer = requireNoteSelected(location);

        const int idxRaw = index.is_default() ? -1 : index.value();
        const int overwriteMode = overwrite.is_default() ? 0 : overwrite.value();

        if (overwriteMode != 0 && overwriteMode != 1) {
            throw HspError(ERR_OUT_OF_RANGE, "noteadd: 上書きモードが不正です", location);
        }

        const size_t lineCount = countNoteLines(buffer);

        if (overwriteMode == 0) {
            // 追加（挿入）モード
            size_t targetLine = (idxRaw < 0) ? lineCount : static_cast<size_t>(idxRaw);
            if (targetLine > lineCount) {
                throw HspError(ERR_OUT_OF_RANGE, "noteadd: インデックスが範囲外です", location);
            }

            size_t insertPos = 0;
            if (!findNoteInsertPos(buffer, targetLine, insertPos)) {
                // 空バッファへの追加
                buffer = std::string(text);
                return;
            }

            std::string newContent;
            newContent.reserve(buffer.size() + text.size() + 1);

            if (targetLine == lineCount) {
                // 末尾追加
                newContent = buffer;
                if (!newContent.empty()) {
                    newContent.push_back('\n');
                }
                newContent.append(text);
            } else {
                // 途中挿入
                newContent.append(buffer, 0, insertPos);
                newContent.append(text);
                newContent.push_back('\n');
                newContent.append(buffer, insertPos);
            }
            buffer = std::move(newContent);
        } else {
            // 上書きモード
            if (lineCount == 0) {
                buffer = std::string(text);
                return;
            }

            size_t targetLine = (idxRaw < 0) ? (lineCount - 1) : static_cast<size_t>(idxRaw);
            if (targetLine >= lineCount) {
                throw HspError(ERR_OUT_OF_RANGE, "noteadd: インデックスが範囲外です", location);
            }

            size_t lineStart = 0, lineLen = 0;
            if (!findNoteLine(buffer, targetLine, lineStart, lineLen)) {
                throw HspError(ERR_OUT_OF_RANGE, "noteadd: インデックスが範囲外です", location);
            }

            // \r があった場合の考慮（lineLen は \r 除去後の長さ）
            size_t actualEnd = lineStart + lineLen;
            if (actualEnd < buffer.size() && buffer[actualEnd] == '\r') {
                actualEnd++;  // \r も置換対象に含める
            }

            std::string newContent;
            newContent.reserve(buffer.size() - lineLen + text.size());
            newContent.append(buffer, 0, lineStart);
            newContent.append(text);
            newContent.append(buffer, actualEnd);
            buffer = std::move(newContent);
        }
    }

    void notedel(int indexValue, const std::source_location& location) {
        std::string& buffer = requireNoteSelected(location);
        const size_t lineCount = countNoteLines(buffer);

        if (indexValue < 0 || static_cast<size_t>(indexValue) >= lineCount) {
            throw HspError(ERR_OUT_OF_RANGE, "notedel: インデックスが範囲外です", location);
        }

        size_t rangeStart = 0, rangeEnd = 0;
        if (!findNoteLineRange(buffer, static_cast<size_t>(indexValue), rangeStart, rangeEnd)) {
            throw HspError(ERR_OUT_OF_RANGE, "notedel: インデックスが範囲外です", location);
        }

        buffer.erase(rangeStart, rangeEnd - rangeStart);
    }

    void noteget(std::string& dest, OptInt index, const std::source_location& location) {
        const std::string& buffer = requireNoteSelected(location);

        const int idx = index.is_default() ? 0 : index.value();
        if (idx < 0) {
            throw HspError(ERR_OUT_OF_RANGE, "noteget: インデックスが範囲外です", location);
        }

        size_t lineStart = 0, lineLen = 0;
        if (!findNoteLine(buffer, static_cast<size_t>(idx), lineStart, lineLen)) {
            throw HspError(ERR_OUT_OF_RANGE, "noteget: インデックスが範囲外です", location);
        }

        dest = buffer.substr(lineStart, lineLen);
    }

    void noteload(std::string_view filename, OptInt maxSize, const std::source_location& location) {
        std::string& buffer = requireNoteSelected(location);

        const int64_t maxBytes = maxSize.is_default() ? -1 : static_cast<int64_t>(maxSize.value());
        if (maxBytes == 0) {
            buffer.clear();
            return;
        }
        if (maxBytes < -1) {
            throw HspError(ERR_OUT_OF_RANGE, "noteload: 最大サイズが不正です", location);
        }

        std::ifstream ifs(std::string(filename), std::ios::binary);
        if (!ifs) {
            throw HspError(ERR_FILE_IO, "noteload: ファイルを開けません", location);
        }

        ifs.seekg(0, std::ios::end);
        std::streamoff size = ifs.tellg();
        if (size < 0) size = 0;
        ifs.seekg(0, std::ios::beg);

        size_t readSize = static_cast<size_t>(size);
        if (maxBytes >= 0 && readSize > static_cast<size_t>(maxBytes)) {
            readSize = static_cast<size_t>(maxBytes);
        }

        std::string data(readSize, '\0');
        if (readSize > 0) {
            ifs.read(data.data(), static_cast<std::streamsize>(readSize));
            if (ifs.gcount() < static_cast<std::streamsize>(readSize)) {
                data.resize(static_cast<size_t>(ifs.gcount()));
            }
        }

        buffer = std::move(data);
    }

    void notesave(std::string_view filename, const std::source_location& location) {
        const std::string& buffer = requireNoteSelected(location);

        std::ofstream ofs(std::string(filename), std::ios::binary | std::ios::trunc);
        if (!ofs) {
            throw HspError(ERR_FILE_IO, "notesave: ファイルを開けません", location);
        }

        if (!buffer.empty()) {
            ofs.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        }
        if (!ofs) {
            throw HspError(ERR_FILE_IO, "notesave: 書き込みに失敗しました", location);
        }
    }

    int notefind(std::string_view search, OptInt mode, const std::source_location& location) {
        const std::string& buffer = requireNoteSelected(location);

        const int m = mode.is_default() ? 0 : mode.value();
        if (m < 0 || m > 2) {
            throw HspError(ERR_OUT_OF_RANGE, "notefind: 検索モードが不正です", location);
        }

        if (buffer.empty()) return -1;

        // 1行ずつ処理（vector生成なし）
        size_t lineIndex = 0;
        size_t start = 0;

        while (start <= buffer.size()) {
            size_t end = buffer.find('\n', start);
            if (end == std::string_view::npos) {
                end = buffer.size();
            }

            size_t len = end - start;
            // 末尾の \r を除去
            if (len > 0 && buffer[start + len - 1] == '\r') {
                --len;
            }

            std::string_view line(buffer.data() + start, len);
            bool matched = false;

            switch (m) {
                case 0: // 完全一致
                    matched = (line == search);
                    break;
                case 1: // 前方一致
                    matched = line.starts_with(search);
                    break;
                case 2: // 部分一致
                    matched = (line.find(search) != std::string_view::npos);
                    break;
            }

            if (matched) {
                return static_cast<int>(lineIndex);
            }

            if (end >= buffer.size()) break;
            start = end + 1;
            ++lineIndex;
        }

        return -1;
    }

    int noteinfo(OptInt mode, const std::source_location& location) {
        const std::string& buffer = requireNoteSelected(location);
        const int m = mode.is_default() ? 0 : mode.value();

        switch (m) {
            case 0:
                return static_cast<int>(countNoteLines(buffer));
            case 1:
                return static_cast<int>(buffer.size());
            default:
                throw HspError(ERR_OUT_OF_RANGE, "noteinfo: モードが不正です", location);
        }
    }

    // ============================================================
    // 文字列変換関数（HSP hsp3utf.as互換）
    // ============================================================
    // HSPPPは内部的にUTF-8を使用。外部DLLやCOMとの連携時に使用。
    // Windows API (MultiByteToWideChar / WideCharToMultiByte) を使用
    // ============================================================

    namespace {
        // wstring -> u16string 変換ヘルパー
        // wchar_tのサイズに応じて最適な変換を行う
        std::u16string wstringToU16string(const std::wstring& wideStr) {
            if (wideStr.empty()) {
                return std::u16string();
            }

            // Case 1: wchar_t が 2バイト (Windows / UTF-16)
            // メモリレイアウトが同じなので、ビット列をそのままコピーするだけでOK（爆速）
            if constexpr (sizeof(wchar_t) == 2) {
                return std::u16string(
                    reinterpret_cast<const char16_t*>(wideStr.data()),
                    wideStr.size()
                );
            }
            // Case 2: wchar_t が 4バイト (Linux, macOS / UTF-32)
            // UTF-32 -> UTF-16 への変換（サロゲートペア計算）が必要
            else {
                std::u16string u16str;
                u16str.reserve(wideStr.size());

                for (wchar_t wc : wideStr) {
                    uint32_t codepoint = static_cast<uint32_t>(wc);

                    if (codepoint <= 0xFFFF) {
                        // 基本多言語面 (BMP)
                        u16str.push_back(static_cast<char16_t>(codepoint));
                    } else if (codepoint <= 0x10FFFF) {
                        // サロゲートペアが必要な文字 (絵文字など)
                        codepoint -= 0x10000;
                        u16str.push_back(static_cast<char16_t>(0xD800 | (codepoint >> 10)));
                        u16str.push_back(static_cast<char16_t>(0xDC00 | (codepoint & 0x3FF)));
                    } else {
                        // 無効なコードポイント (置換文字 U+FFFD を入れる)
                        u16str.push_back(u'\uFFFD');
                    }
                }
                return u16str;
            }
        }

        // u16string -> wstring 変換ヘルパー
        std::wstring u16stringToWstring(const std::u16string& u16str) {
            if (u16str.empty()) {
                return std::wstring();
            }

            // Case 1: wchar_t が 2バイト (Windows / UTF-16)
            if constexpr (sizeof(wchar_t) == 2) {
                return std::wstring(
                    reinterpret_cast<const wchar_t*>(u16str.data()),
                    u16str.size()
                );
            }
            // Case 2: wchar_t が 4バイト (Linux, macOS / UTF-32)
            // UTF-16 -> UTF-32 への変換（サロゲートペアの結合）が必要
            else {
                std::wstring wstr;
                wstr.reserve(u16str.size());

                for (size_t i = 0; i < u16str.size(); ++i) {
                    char16_t c = u16str[i];

                    if (c >= 0xD800 && c <= 0xDBFF && i + 1 < u16str.size()) {
                        // ハイサロゲート
                        char16_t high = c;
                        char16_t low = u16str[i + 1];
                        if (low >= 0xDC00 && low <= 0xDFFF) {
                            // ローサロゲートと結合
                            uint32_t codepoint = 0x10000 +
                                ((static_cast<uint32_t>(high) - 0xD800) << 10) +
                                (static_cast<uint32_t>(low) - 0xDC00);
                            wstr.push_back(static_cast<wchar_t>(codepoint));
                            ++i;  // ローサロゲートをスキップ
                            continue;
                        }
                    }
                    // BMPまたは単独のサロゲート（エラーケース）
                    wstr.push_back(static_cast<wchar_t>(c));
                }
                return wstr;
            }
        }
    }

    std::u16string cnvstow(const std::string& str, const std::source_location& location) {
        if (str.empty()) {
            return std::u16string();
        }

        // UTF-8 -> UTF-16 (Windows: wchar_t = UTF-16 LE)
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
        if (wideLen <= 0) {
            throw HspError(ERR_TYPE_MISMATCH, "cnvstow: UTF-8からUTF-16への変換に失敗しました", location);
        }

        std::wstring wideStr(static_cast<size_t>(wideLen), L'\0');
        int result = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), wideStr.data(), wideLen);
        if (result <= 0) {
            throw HspError(ERR_TYPE_MISMATCH, "cnvstow: UTF-8からUTF-16への変換に失敗しました", location);
        }

        // wchar_t -> char16_t (constexpr ifで最適化)
        return wstringToU16string(wideStr);
    }

    std::string cnvwtos(const std::u16string& wstr, const std::source_location& location) {
        if (wstr.empty()) {
            return std::string();
        }

        // char16_t -> wchar_t (constexpr ifで最適化)
        std::wstring wideStr = u16stringToWstring(wstr);

        // UTF-16 -> UTF-8
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), static_cast<int>(wideStr.size()), nullptr, 0, nullptr, nullptr);
        if (utf8Len <= 0) {
            throw HspError(ERR_TYPE_MISMATCH, "cnvwtos: UTF-16からUTF-8への変換に失敗しました", location);
        }

        std::string utf8Str(static_cast<size_t>(utf8Len), '\0');
        int result = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), static_cast<int>(wideStr.size()), utf8Str.data(), utf8Len, nullptr, nullptr);
        if (result <= 0) {
            throw HspError(ERR_TYPE_MISMATCH, "cnvwtos: UTF-16からUTF-8への変換に失敗しました", location);
        }

        return utf8Str;
    }

    std::string cnvstoa(const std::string& str, const std::source_location& location) {
        if (str.empty()) {
            return std::string();
        }

        // UTF-8 -> UTF-16 -> ANSI (ShiftJIS)
        // Step 1: UTF-8 -> UTF-16 (cnvstowを再利用)
        std::u16string u16str = cnvstow(str, location);

        // Step 1.5: u16string -> wstring (constexpr ifで最適化)
        std::wstring wideStr = u16stringToWstring(u16str);

        // Step 2: UTF-16 -> ANSI (CP_ACP = system default ANSI code page, typically ShiftJIS on Japanese Windows)
        int ansiLen = WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), static_cast<int>(wideStr.size()), nullptr, 0, nullptr, nullptr);
        if (ansiLen <= 0) {
            throw HspError(ERR_TYPE_MISMATCH, "cnvstoa: UTF-16からANSIへの変換に失敗しました", location);
        }

        std::string ansiStr(static_cast<size_t>(ansiLen), '\0');
        int result = WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), static_cast<int>(wideStr.size()), ansiStr.data(), ansiLen, nullptr, nullptr);
        if (result <= 0) {
            throw HspError(ERR_TYPE_MISMATCH, "cnvstoa: UTF-16からANSIへの変換に失敗しました", location);
        }

        return ansiStr;
    }

    std::string cnvatos(const std::string& astr, const std::source_location& location) {
        if (astr.empty()) {
            return std::string();
        }

        // ANSI (ShiftJIS) -> UTF-16 -> UTF-8
        // Step 1: ANSI -> UTF-16
        int wideLen = MultiByteToWideChar(CP_ACP, 0, astr.c_str(), static_cast<int>(astr.size()), nullptr, 0);
        if (wideLen <= 0) {
            throw HspError(ERR_TYPE_MISMATCH, "cnvatos: ANSIからUTF-16への変換に失敗しました", location);
        }

        std::wstring wideStr(static_cast<size_t>(wideLen), L'\0');
        int result1 = MultiByteToWideChar(CP_ACP, 0, astr.c_str(), static_cast<int>(astr.size()), wideStr.data(), wideLen);
        if (result1 <= 0) {
            throw HspError(ERR_TYPE_MISMATCH, "cnvatos: ANSIからUTF-16への変換に失敗しました", location);
        }

        // Step 2: UTF-16 -> UTF-8 (cnvwtosを再利用)
        std::u16string u16str = wstringToU16string(wideStr);
        return cnvwtos(u16str, location);
    }

} // namespace hsppp
