// HspppLib/src/core/hsppp_easing.inl
// イージング関数、ソート関数、デバッグ出力の実装

namespace {
    // イージング関数の状態
    double g_easeMin = 0.0;
    double g_easeMax = 1.0;
    int g_easeType = 0;  // ease_linear

    // ソート関数のインデックス履歴
    // NOTE: HSP互換性のため、最後にソートされた配列のインデックスをグローバルに保持
    // sortget()は「最後に実行されたソート」の結果を返すHSPの仕様を再現している
    std::vector<int> g_sortIndices;
}

namespace hsppp {

    // ============================================================
    // イージング計算ヘルパー
    // ============================================================

    namespace {
        // バウンスイージング用定数
        namespace bounce {
            constexpr double N1 = 7.5625;
            constexpr double D1 = 2.75;
        }

        // バウンスアウトの共通実装
        double bounceOutImpl(double t) {
            if (t < 1.0 / bounce::D1) {
                return bounce::N1 * t * t;
            } else if (t < 2.0 / bounce::D1) {
                t -= 1.5 / bounce::D1;
                return bounce::N1 * t * t + 0.75;
            } else if (t < 2.5 / bounce::D1) {
                t -= 2.25 / bounce::D1;
                return bounce::N1 * t * t + 0.9375;
            } else {
                t -= 2.625 / bounce::D1;
                return bounce::N1 * t * t + 0.984375;
            }
        }

        // 正規化された時間 t (0.0〜1.0) から補間値を計算
        double calculateEase(double t, int type) {
            // ループフラグの処理
            bool isLoop = (type & ease_loop) != 0;
            int baseType = type & ~ease_loop;
            
            if (isLoop) {
                // ループ処理: 0→1→0→1...を繰り返す
                t = std::fmod(t, 2.0);
                if (t > 1.0) {
                    t = 2.0 - t;  // 折り返し
                }
            } else {
                // 範囲外はクランプ
                t = std::clamp(t, 0.0, 1.0);
            }

            switch (baseType) {
                case ease_linear:
                    return t;

                case ease_quad_in:
                    return t * t;

                case ease_quad_out:
                    return t * (2.0 - t);

                case ease_quad_inout:
                    if (t < 0.5) {
                        return 2.0 * t * t;
                    } else {
                        return -1.0 + (4.0 - 2.0 * t) * t;
                    }

                case ease_cubic_in:
                    return t * t * t;

                case ease_cubic_out: {
                    double t1 = t - 1.0;
                    return t1 * t1 * t1 + 1.0;
                }

                case ease_cubic_inout:
                    if (t < 0.5) {
                        return 4.0 * t * t * t;
                    } else {
                        double t1 = 2.0 * t - 2.0;
                        return 0.5 * t1 * t1 * t1 + 1.0;
                    }

                case ease_quartic_in:
                    return t * t * t * t;

                case ease_quartic_out: {
                    double t1 = t - 1.0;
                    return 1.0 - t1 * t1 * t1 * t1;
                }

                case ease_quartic_inout:
                    if (t < 0.5) {
                        return 8.0 * t * t * t * t;
                    } else {
                        double t1 = t - 1.0;
                        return 1.0 - 8.0 * t1 * t1 * t1 * t1;
                    }

                case ease_bounce_in:
                    return 1.0 - bounceOutImpl(1.0 - t);

                case ease_bounce_out:
                    return bounceOutImpl(t);

                case ease_bounce_inout:
                    if (t < 0.5) {
                        return (1.0 - bounceOutImpl(1.0 - 2.0 * t)) * 0.5;
                    } else {
                        return bounceOutImpl(2.0 * t - 1.0) * 0.5 + 0.5;
                    }

                case ease_shake_in: {
                    // シェイク効果（振動しながら収束）
                    double amplitude = 1.0 - t;
                    constexpr double frequency = 10.0;
                    return t + amplitude * std::sin(t * frequency * M_PI) * 0.1;
                }

                case ease_shake_out: {
                    // シェイク効果（振動しながら減衰）
                    double amplitude = t;
                    constexpr double frequency = 10.0;
                    return t + (1.0 - amplitude) * std::sin(t * frequency * M_PI) * 0.1;
                }

                case ease_shake_inout: {
                    // シェイク効果（中央でピーク）
                    double amplitude = t < 0.5 ? t : (1.0 - t);
                    constexpr double frequency = 10.0;
                    return t + amplitude * std::sin(t * frequency * M_PI) * 0.1;
                }

                default:
                    return t;  // 不明なタイプはリニア
            }
        }
    }

    // ============================================================
    // イージング関数の実装
    // ============================================================

    void setease(double p1, double p2, OptInt p3, const std::source_location& location) {
        g_easeMin = p1;
        g_easeMax = p2;
        if (!p3.is_default()) {
            g_easeType = p3.value();
        }
    }

    int getease(int p1, OptInt p2, const std::source_location& location) {
        int maxVal = p2.value_or(4096);
        if (maxVal <= 0) {
            return static_cast<int>(g_easeMin);
        }

        double t = static_cast<double>(p1) / static_cast<double>(maxVal);
        double eased = calculateEase(t, g_easeType);
        double result = g_easeMin + (g_easeMax - g_easeMin) * eased;
        return static_cast<int>(result);
    }

    double geteasef(double p1, OptDouble p2, const std::source_location& location) {
        double maxVal = p2.value_or(1.0);
        if (maxVal <= 0.0) {
            return g_easeMin;
        }

        double t = p1 / maxVal;
        double eased = calculateEase(t, g_easeType);
        return g_easeMin + (g_easeMax - g_easeMin) * eased;
    }

    // ============================================================
    // ソート関数の実装
    // ============================================================

    namespace {
        // ソート共通実装（インデックスソート方式で効率化）
        template<typename T>
        void sortImpl(std::vector<T>& arr, bool descending) {
            size_t n = arr.size();
            
            // インデックス配列を作成
            std::vector<int> indices(n);
            for (size_t i = 0; i < n; ++i) {
                indices[i] = static_cast<int>(i);
            }
            
            // インデックスをソート
            if (descending) {
                std::sort(indices.begin(), indices.end(),
                    [&arr](int a, int b) { return arr[a] > arr[b]; });
            } else {
                std::sort(indices.begin(), indices.end(),
                    [&arr](int a, int b) { return arr[a] < arr[b]; });
            }
            
            // ソート結果の配列を作成（moveで効率化）
            std::vector<T> sorted;
            sorted.reserve(n);
            g_sortIndices.resize(n);
            
            for (size_t i = 0; i < n; ++i) {
                sorted.push_back(std::move(arr[indices[i]]));
                g_sortIndices[i] = indices[i];
            }
            
            arr = std::move(sorted);
        }
    }

    void sortval(std::vector<int>& arr, OptInt order, const std::source_location& location) {
        sortImpl(arr, order.value_or(0) == 1);
    }

    void sortval(std::vector<double>& arr, OptInt order, const std::source_location& location) {
        sortImpl(arr, order.value_or(0) == 1);
    }

    void sortstr(std::vector<std::string>& arr, OptInt order, const std::source_location& location) {
        sortImpl(arr, order.value_or(0) == 1);
    }

    void sortnote(std::string& note, OptInt order, const std::source_location& location) {
        // メモリノート形式を行に分解
        std::vector<std::string> lines = split(note, "\n", location);

        // sortstrを使ってソート
        sortstr(lines, order, location);

        // 再度メモリノート形式に結合（reserveで効率化）
        if (lines.empty()) {
            note.clear();
            return;
        }
        
        size_t totalSize = 0;
        for (const auto& line : lines) {
            totalSize += line.size();
        }
        totalSize += lines.size() - 1;  // 改行文字分
        
        note.clear();
        note.reserve(totalSize);
        note += lines[0];
        for (size_t i = 1; i < lines.size(); ++i) {
            note += '\n';
            note += lines[i];
        }
    }

    int sortget(int index, const std::source_location& location) {
        return safe_call(location, [&]() -> int {
            if (index < 0 || static_cast<size_t>(index) >= g_sortIndices.size()) {
                throw HspError(ERR_OUT_OF_ARRAY, "sortgetのインデックスが範囲外です", location);
            }
            return g_sortIndices[index];
        });
    }

    // ============================================================
    // デバッグ出力の実装
    // ============================================================

    void logmes(std::string_view message, const std::source_location& location) {
        // UTF-8からワイド文字列への変換
        int wlen = MultiByteToWideChar(CP_UTF8, 0, message.data(), 
            static_cast<int>(message.size()), nullptr, 0);
        if (wlen > 0) {
            std::wstring wmsg(wlen, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, message.data(), 
                static_cast<int>(message.size()), wmsg.data(), wlen);
            wmsg += L"\n";
            OutputDebugStringW(wmsg.c_str());
        } else {
            // 変換に失敗した場合もW系を使用（空文字列として出力）
            OutputDebugStringW(L"[logmes: encoding error]\n");
        }
    }

    void logmes(int value, const std::source_location& location) {
        logmes(std::to_string(value), location);
    }

    void logmes(double value, const std::source_location& location) {
        logmes(std::to_string(value), location);
    }

} // namespace hsppp
