// HspppLib/src/core/Internal.h
// 内部実装用ヘッダ（モジュール間で共有するクラス定義）

#pragma once

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>
#include <memory>

// COMスマートポインタのエイリアス
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace hsppp {
namespace internal {

// UTF-8文字列をUTF-16(wchar_t)に変換
std::wstring Utf8ToWide(const char* utf8str);

// 前方宣言
class HspSurface;
class HspWindow;

// 基底クラス: HspSurface
// 描画対象を抽象化する
class HspSurface {
protected:
    // オフスクリーンバッファ（描画先）
    ComPtr<ID2D1BitmapRenderTarget> m_pBitmapTarget;

    // 描画リソース
    ComPtr<ID2D1SolidColorBrush> m_pBrush;
    ComPtr<IDWriteTextFormat> m_pTextFormat;

    // HSPのステート
    int m_currentX;
    int m_currentY;
    D2D1_COLOR_F m_currentColor;

    // サイズ
    int m_width;
    int m_height;

    // 描画中フラグ
    bool m_isDrawing;

public:
    HspSurface(int width, int height);
    virtual ~HspSurface() = default;  // ComPtrが自動解放するのでdefault

    // 初期化（派生クラスで実装する必要がある）
    virtual bool initialize(const ComPtr<ID2D1Factory>& pD2DFactory,
                           const ComPtr<IDWriteFactory>& pDWriteFactory);

    // 描画ターゲットとして設定する
    virtual void activate();

    // 描画命令
    void boxf(int x1, int y1, int x2, int y2);
    void mes(const char* text);
    void color(int r, int g, int b);
    void pos(int x, int y);

    // 描画制御
    void beginDraw();
    void endDraw();
    bool isDrawing() const { return m_isDrawing; }

    // ゲッター
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const ComPtr<ID2D1BitmapRenderTarget>& getBitmapTarget() const { return m_pBitmapTarget; }
};

// 派生クラス: HspWindow
// ウィンドウを表すSurface
class HspWindow : public HspSurface {
private:
    HWND m_hwnd;
    ComPtr<ID2D1HwndRenderTarget> m_pHwndTarget; // 表示用ターゲット
    std::wstring m_title;

public:
    HspWindow(int width, int height, const char* title);
    virtual ~HspWindow();  // HWNDの破棄が必要なので実装する

    // 初期化
    bool initialize(const ComPtr<ID2D1Factory>& pD2DFactory,
                   const ComPtr<IDWriteFactory>& pDWriteFactory) override;

    // ウィンドウ作成
    bool createWindow(HINSTANCE hInstance, const wchar_t* className, DWORD style);

    // オフスクリーンバッファの内容を画面に転送
    void present();

    // ゲッター
    HWND getHwnd() const { return m_hwnd; }

    // ウィンドウプロシージャから呼ばれるWM_PAINT処理
    void onPaint();
};

// ウィンドウマネージャー
class WindowManager {
private:
    HINSTANCE m_hInstance;
    const wchar_t* m_className;
    bool m_classRegistered;

    // シングルトンなのでコピー・ムーブを禁止
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    WindowManager(WindowManager&&) = delete;
    WindowManager& operator=(WindowManager&&) = delete;

public:
    WindowManager();
    ~WindowManager();

    // シングルトンインスタンスの取得（静的ローカル変数で安全に管理）
    static WindowManager& getInstance();

    bool registerWindowClass();
    void unregisterWindowClass();

    HINSTANCE getHInstance() const { return m_hInstance; }
    const wchar_t* getClassName() const { return m_className; }

    // ウィンドウプロシージャ
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

} // namespace internal
} // namespace hsppp
