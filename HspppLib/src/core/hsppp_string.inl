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
        std::vector<std::string> lines = parseNoteLines(buffer);

        const int idxRaw = index.is_default() ? -1 : index.value();
        const int overwriteMode = overwrite.is_default() ? 0 : overwrite.value();

        if (overwriteMode != 0 && overwriteMode != 1) {
            throw HspError(ERR_OUT_OF_RANGE, "noteadd: 上書きモードが不正です", location);
        }

        if (overwriteMode == 0) {
            // 追加（挿入）
            size_t insertPos = 0;
            if (idxRaw < 0) {
                insertPos = lines.size();
            } else {
                insertPos = static_cast<size_t>(idxRaw);
                if (insertPos > lines.size()) {
                    // HSPは範囲外をエラーとする前提で扱う
                    throw HspError(ERR_OUT_OF_RANGE, "noteadd: インデックスが範囲外です", location);
                }
            }

            lines.insert(lines.begin() + static_cast<ptrdiff_t>(insertPos), std::string(text));
        } else {
            // 上書き
            if (lines.empty()) {
                lines.push_back(std::string(text));
            } else if (idxRaw < 0) {
                lines.back() = std::string(text);
            } else {
                size_t pos = static_cast<size_t>(idxRaw);
                if (pos >= lines.size()) {
                    throw HspError(ERR_OUT_OF_RANGE, "noteadd: インデックスが範囲外です", location);
                }
                lines[pos] = std::string(text);
            }
        }

        buffer = joinNoteLines(lines);
    }

    void notedel(int indexValue, const std::source_location& location) {
        std::string& buffer = requireNoteSelected(location);
        std::vector<std::string> lines = parseNoteLines(buffer);

        if (indexValue < 0 || static_cast<size_t>(indexValue) >= lines.size()) {
            throw HspError(ERR_OUT_OF_RANGE, "notedel: インデックスが範囲外です", location);
        }

        lines.erase(lines.begin() + indexValue);
        buffer = joinNoteLines(lines);
    }

    void noteget(std::string& dest, OptInt index, const std::source_location& location) {
        const std::string& buffer = requireNoteSelected(location);
        std::vector<std::string> lines = parseNoteLines(buffer);

        const int idx = index.is_default() ? 0 : index.value();
        if (idx < 0 || static_cast<size_t>(idx) >= lines.size()) {
            throw HspError(ERR_OUT_OF_RANGE, "noteget: インデックスが範囲外です", location);
        }
        dest = lines[static_cast<size_t>(idx)];
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
        std::vector<std::string> lines = parseNoteLines(buffer);

        const int m = mode.is_default() ? 0 : mode.value();
        if (m < 0 || m > 2) {
            throw HspError(ERR_OUT_OF_RANGE, "notefind: 検索モードが不正です", location);
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            const std::string& line = lines[i];
            bool matched = false;
            switch (m) {
                case 0: // 完全一致
                    matched = (line == search);
                    break;
                case 1: // 前方一致
                    matched = line.starts_with(search);
                    break;
                case 2: // 部分一致
                    matched = (line.find(search) != std::string::npos);
                    break;
            }
            if (matched) {
                return static_cast<int>(i);
            }
        }

        return -1;
    }

    int noteinfo(OptInt mode, const std::source_location& location) {
        const std::string& buffer = requireNoteSelected(location);
        const int m = mode.is_default() ? 0 : mode.value();

        switch (m) {
            case 0: {
                std::vector<std::string> lines = parseNoteLines(buffer);
                return static_cast<int>(lines.size());
            }
            case 1:
                return static_cast<int>(buffer.size());
            default:
                throw HspError(ERR_OUT_OF_RANGE, "noteinfo: モードが不正です", location);
        }
    }

} // namespace hsppp
