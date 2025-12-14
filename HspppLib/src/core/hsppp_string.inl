// HspppLib/src/core/hsppp_string.inl
// 文字列操作関数の実装（HSP互換）
// 注: cstdio, cctypeはhsppp.cppのグローバルモジュールフラグメントでインクルード済み

namespace hsppp {

    // ============================================================
    // instr - 文字列の検索
    // ============================================================
    
    int instr(const std::string& p1, int p2, const std::string& search) {
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
        return static_cast<int>(pos - p2);
    }

    int instr(const std::string& p1, const std::string& search) {
        return instr(p1, 0, search);
    }

    // ============================================================
    // strmid - 文字列の一部を取り出す
    // ============================================================
    
    std::string strmid(const std::string& p1, int p2, int p3) {
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
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        }
        
        return result;
    }

} // namespace hsppp
