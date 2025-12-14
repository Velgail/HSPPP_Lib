// HspppLib/src/core/hsppp_math.inl
// 数学関数・型変換関数・色関連関数の実装

// 注: ヘッダーのインポートは hsppp.ixx モジュールで行われています

namespace {
    // 乱数生成器（スレッドローカル）
    thread_local std::mt19937 g_randomEngine;
    thread_local bool g_randomInitialized = false;
}

namespace hsppp {

    // ============================================================
    // 数学関数
    // ============================================================
    // 注: 三角関数等は std::cmath から直接再エクスポートされます。
    //     度数法対応は deg2rad/rad2deg() で行ってください。
    //
    // エクスポート対象:
    //   - std::abs (int, float, double, long double)
    //   - std::sin, std::cos, std::tan (ラジアン単位)
    //   - std::atan2
    //   - std::sqrt, std::pow, std::exp, std::log
    //
    // 新規追加:
    //   - deg2rad(double) : 度 → ラジアン変換
    //   - rad2deg(double) : ラジアン → 度 変換

    int rnd(int p1) {
        if (!g_randomInitialized) {
            // 初期化されていない場合は固定シードを使用（HSP互換: 一定パターン）
            g_randomEngine.seed(0);
            g_randomInitialized = true;
        }
        if (p1 <= 0) return 0;
        std::uniform_int_distribution<int> dist(0, p1 - 1);
        return dist(g_randomEngine);
    }

    void randomize(OptInt p1, const std::source_location& location) {
        unsigned int seed;
        if (p1.is_default()) {
            // 時刻ベースのシード
            auto now = std::chrono::high_resolution_clock::now();
            seed = static_cast<unsigned int>(now.time_since_epoch().count());
        } else {
            seed = static_cast<unsigned int>(p1.value());
        }
        g_randomEngine.seed(seed);
        g_randomInitialized = true;
    }

    int limit(int p1, OptInt p2, OptInt p3) {
        int result = p1;
        if (!p2.is_default() && result < p2.value()) {
            result = p2.value();
        }
        if (!p3.is_default() && result > p3.value()) {
            result = p3.value();
        }
        return result;
    }

    double limitf(double p1, OptDouble p2, OptDouble p3) {
        double result = p1;
        if (!p2.is_default() && result < p2.value()) {
            result = p2.value();
        }
        if (!p3.is_default() && result > p3.value()) {
            result = p3.value();
        }
        return result;
    }

    // ============================================================
    // 型変換関数
    // ============================================================

    int toInt(double p1) {
        return static_cast<int>(p1);
    }

    int toInt(const std::string& p1) {
        try {
            return std::stoi(p1);
        } catch (const std::invalid_argument&) {
            return 0;  // 変換失敗時は0を返す（HSP互換）
        } catch (const std::out_of_range&) {
            return 0;  // 変換失敗時は0を返す（HSP互換）
        }
    }

    double toDouble(int p1) {
        return static_cast<double>(p1);
    }

    double toDouble(const std::string& p1) {
        try {
            return std::stod(p1);
        } catch (const std::invalid_argument&) {
            return 0.0;  // 変換失敗時は0を返す（HSP互換）
        } catch (const std::out_of_range&) {
            return 0.0;  // 変換失敗時は0を返す（HSP互換）
        }
    }

    std::string str(double value, const std::source_location& location) {
        return std::to_string(value);
    }

    std::string str(int value, const std::source_location& location) {
        return std::to_string(value);
    }

    int strlen(const std::string& p1) {
        return static_cast<int>(p1.size());
    }

    // ============================================================
    // 色関連関数
    // ============================================================

    void hsvcolor(int p1, int p2, int p3, const std::source_location& location) {
        // HSVをRGBに変換
        // H: 0-191 (色相), S: 0-255 (彩度), V: 0-255 (明度)
        
        int h = p1;
        int s = p2;
        int v = p3;
        
        // Hを0-191から0-360にマッピング
        float hue = h * 360.0f / 192.0f;
        float sat = s / 255.0f;
        float val = v / 255.0f;
        
        int r, g, b;
        
        if (s == 0) {
            // 彩度0の場合はグレースケール
            r = g = b = v;
        } else {
            float h6 = hue / 60.0f;
            int hi = static_cast<int>(h6) % 6;
            float f = h6 - static_cast<int>(h6);
            float p = val * (1.0f - sat);
            float q = val * (1.0f - sat * f);
            float t = val * (1.0f - sat * (1.0f - f));
            
            switch (hi) {
                case 0: r = static_cast<int>(val * 255); g = static_cast<int>(t * 255); b = static_cast<int>(p * 255); break;
                case 1: r = static_cast<int>(q * 255); g = static_cast<int>(val * 255); b = static_cast<int>(p * 255); break;
                case 2: r = static_cast<int>(p * 255); g = static_cast<int>(val * 255); b = static_cast<int>(t * 255); break;
                case 3: r = static_cast<int>(p * 255); g = static_cast<int>(q * 255); b = static_cast<int>(val * 255); break;
                case 4: r = static_cast<int>(t * 255); g = static_cast<int>(p * 255); b = static_cast<int>(val * 255); break;
                default: r = static_cast<int>(val * 255); g = static_cast<int>(p * 255); b = static_cast<int>(q * 255); break;
            }
        }
        
        // color関数を呼び出してカレントカラーを設定
        color(r, g, b, location);
    }

    void rgbcolor(int p1, const std::source_location& location) {
        // $rrggbb形式のカラーコードをRGBに分解
        int r = (p1 >> 16) & 0xFF;
        int g = (p1 >> 8) & 0xFF;
        int b = p1 & 0xFF;
        
        // color関数を呼び出してカレントカラーを設定
        color(r, g, b, location);
    }

    void syscolor(int p1, const std::source_location& location) {
        // Windowsシステムカラーを取得
        DWORD sysColor = GetSysColor(p1);
        
        int r = GetRValue(sysColor);
        int g = GetGValue(sysColor);
        int b = GetBValue(sysColor);
        
        // color関数を呼び出してカレントカラーを設定
        color(r, g, b, location);
    }

} // namespace hsppp
