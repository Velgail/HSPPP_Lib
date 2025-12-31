// HspppSample/DemoState.h
// ═══════════════════════════════════════════════════════════════════
// HSPPP デモアプリケーション - 状態と定義
// ═══════════════════════════════════════════════════════════════════

#pragma once

#include <memory>
import hsppp;
import <string>;
import <optional>;

// ═══════════════════════════════════════════════════════════════════
// デモカテゴリとモード定義
// ═══════════════════════════════════════════════════════════════════

enum class DemoCategory {
    Basic,      // 基本デモ (1-9)
    Extended,   // 拡張デモ (Ctrl + 1-8)
    Image,      // 画像デモ (Shift + 1-3)
    Interrupt,  // 割り込みデモ (Alt + 1-3)
    GUI,        // GUIデモ (Win + 1-2)
    Media       // マルチメディアデモ (Alt+Shift + 1)
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
    InputMouse,     // Ctrl+=: マウス入力関数
    Easing,         // Ctrl+[: イージング関数
    Sorting,        // Ctrl+]: ソート関数
    COUNT
};

enum class ImageDemo {
    Bmpsave = 0,    // Shift+1: BMP保存
    Picload,        // Shift+2: 画像ロード
    Celload,        // Shift+3: セルロード
    Bgscr,          // Shift+4: 枠なしウィンドウ
    COUNT
};

enum class InterruptDemo {
    OnClick = 0,    // Alt+1: クリック割り込み
    OnKey,          // Alt+2: キー割り込み
    OnExit,         // Alt+3: 終了割り込み
    OnCmd,          // Alt+4: Windowsメッセージ割り込み
    OnError,        // Alt+5: エラーハンドリング
    COUNT
};

enum class GUIDemo {
    Button = 0,     // Win+1: ボタン・入力系
    ChoiceBox,      // Win+2: チェック・コンボ・リスト
    COUNT
};

enum class MediaDemo {
    AudioPlayback = 0,  // Alt+Shift+1: 音声再生デモ
    COUNT
};

// ═══════════════════════════════════════════════════════════════════
// 仮想キーコード定数
// ═══════════════════════════════════════════════════════════════════

namespace VK {
    constexpr int CONTROL = 0x11;
    constexpr int SHIFT = 0x10;
    constexpr int MENU = 0x12;  // Alt key
    constexpr int LWIN = 0x5B;  // Left Windows key
    constexpr int RWIN = 0x5C;  // Right Windows key
    constexpr int ESCAPE = 0x1B;
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
extern DemoCategory g_prevCategory;
extern int g_prevDemoIndex;

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
extern bool g_bgscrVisible;

// バッファ用変数
extern bool g_bufferCreated;
extern hsppp::OptInt g_srcBufferId;  // 未作成時は std::nullopt

// 割り込みデモ用変数
extern int g_clickCount;
extern int g_keyCount;
extern int g_lastKey;
extern int g_cmdMessageCount;
extern int g_lastCmdMessage;

// onerror デモ用変数
extern bool g_errorHandlerEnabled;
extern int g_lastErrorCode;
extern std::string g_lastErrorMessage;

// GUIデモ用変数
extern bool g_guiObjectsCreated;
extern int g_buttonClickCount;
// 入力系GUIコントロール用の状態変数（shared_ptrでライフタイム安全）
extern std::shared_ptr<std::string> g_inputText;
extern std::shared_ptr<std::string> g_inputNumber;  // 文字列で受け取り、必要時にtoInt()で変換
extern std::shared_ptr<std::string> g_mesboxText;
// 選択系GUIコントロール用の状態変数（shared_ptrでライフタイム安全）
extern std::shared_ptr<int> g_checkState;
extern std::shared_ptr<int> g_comboxState;
extern std::shared_ptr<int> g_listboxState;

// マルチメディアデモ用変数
extern bool g_mediaLoaded;
extern int g_mediaVolume;
extern int g_mediaPan;
extern bool g_mediaIsPlaying;
extern int g_lastLoadResult;   // mmloadの戻り値
extern int g_lastPlayResult;   // mmplayの戻り値
extern int g_mediaType;        // 0=WAV, 1=MP3, 2=MP4
extern bool g_videoMode;       // 動画再生モード中か（描画スキップ用）

// アクション実行結果表示用
extern std::string g_actionLog;

// ═══════════════════════════════════════════════════════════════════
// 修飾キー状態チェック
// ═══════════════════════════════════════════════════════════════════

/// @brief 修飾キー（Ctrl/Alt/Shift/Win）が押されているかチェック
/// @return いずれかの修飾キーが押されていればtrue
inline bool isModifierKeyPressed() {
    return hsppp::getkey(VK::CONTROL) || hsppp::getkey(VK::SHIFT) || 
           hsppp::getkey(VK::MENU) || hsppp::getkey(VK::LWIN) || hsppp::getkey(VK::RWIN);
}

// ═══════════════════════════════════════════════════════════════════
// デモ関数（各cppファイルで実装）
// ═══════════════════════════════════════════════════════════════════

// 描画関数
void drawBasicDemo(hsppp::Screen& win);
void drawExtendedDemo(hsppp::Screen& win);
void drawImageDemo(hsppp::Screen& win);
void drawInterruptDemo(hsppp::Screen& win);
void drawGUIDemo(hsppp::Screen& win);
void drawMediaDemo(hsppp::Screen& win);

// アクション処理関数（各デモ固有の入力処理）
void processBasicAction(hsppp::Screen& win);
void processExtendedAction(hsppp::Screen& win);
void processImageAction(hsppp::Screen& win);
void processInterruptAction(hsppp::Screen& win);
void processGUIAction(hsppp::Screen& win);
void processMediaAction(hsppp::Screen& win);

// GUIオブジェクトクリア関数
void clearGUIObjects();

// デモ切り替え時のリセット処理
void onDemoChanged(hsppp::Screen& win);

// ヘルパー関数
std::string getCategoryName();
std::string getDemoName();
