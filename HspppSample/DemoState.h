// HspppSample/DemoState.h
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 状態と定義
// ═══════════════════════════════════════════════════════════════════

#pragma once

import hsppp;
import <string>;

// ═══════════════════════════════════════════════════════════════════
// デモカテゴリとモード定義
// ═══════════════════════════════════════════════════════════════════

enum class DemoCategory {
    Basic,      // 基本デモ (1-9)
    Extended,   // 拡張デモ (Ctrl + 1-8)
    Image,      // 画像デモ (Shift + 1-3)
    Interrupt   // 割り込みデモ (Alt + 1-3)
};

enum class BasicDemo {
    Line = 0,       // 1: 直線描画
    Circle,         // 2: 円描画
    Pset,           // 3: 点描画
    Boxf,           // 4: 矩形塗りつぶし
    Cls,            // 5: 画面クリア
    Font,           // 6: フォント設定
    Title,          // 7: タイトル設定
    Width,          // 8: ウィンドウサイズ
    Groll,          // 9: スクロール
    COUNT
};

enum class ExtendedDemo {
    Math = 0,       // Ctrl+1: 数学関数
    Color,          // Ctrl+2: 色関連関数
    Gradf,          // Ctrl+3: グラデーション
    Grect,          // Ctrl+4: 回転矩形
    Gsquare,        // Ctrl+5: 任意四角形
    Gcopy,          // Ctrl+6: 画面コピー
    Gzoom,          // Ctrl+7: 変倍コピー
    Grotate,        // Ctrl+8: 回転コピー
    StringFunc,     // Ctrl+9: 文字列操作関数
    SystemInfo,     // Ctrl+0: システム情報関数
    FileOps,        // Ctrl+-: ファイル操作関数
    COUNT
};

enum class ImageDemo {
    Bmpsave = 0,    // Shift+1: BMP保存
    Picload,        // Shift+2: 画像ロード
    Celload,        // Shift+3: セルロード
    COUNT
};

enum class InterruptDemo {
    OnClick = 0,    // Alt+1: クリック割り込み
    OnKey,          // Alt+2: キー割り込み
    OnExit,         // Alt+3: 終了割り込み
    COUNT
};

// ═══════════════════════════════════════════════════════════════════
// 仮想キーコード定数
// ═══════════════════════════════════════════════════════════════════

namespace VK {
    constexpr int CONTROL = 0x11;
    constexpr int SHIFT = 0x10;
    constexpr int MENU = 0x12;  // Alt key
    constexpr int F1 = 0x70;
    constexpr int UP = 0x26;
    constexpr int DOWN = 0x28;
    constexpr int LEFT = 0x25;
    constexpr int RIGHT = 0x27;
}

// ═══════════════════════════════════════════════════════════════════
// グローバル状態（extern宣言）
// ═══════════════════════════════════════════════════════════════════

// 現在のデモ状態
extern DemoCategory g_category;
extern int g_demoIndex;

// ヘルプウィンドウ状態
extern bool g_helpVisible;

// 描画デモ用変数
extern int g_clsMode;
extern int g_fontStyle;
extern int g_fontSize;
extern int g_scrollX;
extern int g_scrollY;
extern double g_angle;

// 画像デモ用変数
extern bool g_testImageSaved;
extern bool g_testImageLoaded;
extern int g_celId;
extern int g_celIndex;

// バッファ用変数
extern bool g_bufferCreated;

// 割り込みデモ用変数
extern int g_clickCount;
extern int g_keyCount;
extern int g_lastKey;

// ═══════════════════════════════════════════════════════════════════
// デモ描画関数（各cppファイルで実装）
// ═══════════════════════════════════════════════════════════════════

void drawBasicDemo(hsppp::Screen& win);
void drawExtendedDemo(hsppp::Screen& win);
void drawImageDemo(hsppp::Screen& win);
void drawInterruptDemo(hsppp::Screen& win);

// ヘルパー関数
std::string getCategoryName();
std::string getDemoName();
