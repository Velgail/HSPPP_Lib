// HspppLib/src/core/Internal.h
// 内部実装用ヘッダ（モジュール間で共有するクラス定義）

#pragma once

#include <windows.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>
#include <string_view>
#include <memory>

// COMスマートポインタのエイリアス
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace hsppp {
namespace internal {

// UTF-8文字列をUTF-16(wchar_t)に変換（安全な string_view を使用）
std::wstring Utf8ToWide(std::string_view utf8str);

// 前方宣言
class HspSurface;
class HspWindow;
class HspBuffer;

// Direct2D 1.1 デバイスマネージャー（シングルトン）
// すべてのSurfaceで共有するD3D11/D2Dデバイスを管理
class D2DDeviceManager {
private:
    ComPtr<ID3D11Device> m_pD3DDevice;
    ComPtr<ID3D11DeviceContext> m_pD3DContext;
    ComPtr<IDXGIDevice1> m_pDxgiDevice;
    ComPtr<ID2D1Factory1> m_pD2DFactory;
    ComPtr<ID2D1Device> m_pD2DDevice;
    ComPtr<IDWriteFactory> m_pDWriteFactory;

    bool m_initialized;

    D2DDeviceManager();
    ~D2DDeviceManager();

    // シングルトンなのでコピー・ムーブを禁止
    D2DDeviceManager(const D2DDeviceManager&) = delete;
    D2DDeviceManager& operator=(const D2DDeviceManager&) = delete;

public:
    static D2DDeviceManager& getInstance();

    bool initialize();
    void shutdown();

    // デバイスコンテキストを作成
    ComPtr<ID2D1DeviceContext> createDeviceContext();

    // ゲッター
    ID2D1Factory1* getFactory() const { return m_pD2DFactory.Get(); }
    ID2D1Device* getDevice() const { return m_pD2DDevice.Get(); }
    IDWriteFactory* getDWriteFactory() const { return m_pDWriteFactory.Get(); }
    ID3D11Device* getD3DDevice() const { return m_pD3DDevice.Get(); }
    IDXGIDevice1* getDxgiDevice() const { return m_pDxgiDevice.Get(); }
    bool isInitialized() const { return m_initialized; }
};

// 基底クラス: HspSurface
// 描画対象を抽象化する（Direct2D 1.1対応）
class HspSurface {
protected:
    // デバイスコンテキスト（描画用）
    ComPtr<ID2D1DeviceContext> m_pDeviceContext;

    // 描画先ビットマップ（共有可能）
    ComPtr<ID2D1Bitmap1> m_pTargetBitmap;

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
    virtual ~HspSurface() = default;

    // 初期化（派生クラスで実装）
    virtual bool initialize() = 0;

    // 描画命令
    void boxf(int x1, int y1, int x2, int y2);
    void mes(std::string_view text);
    void color(int r, int g, int b);
    void pos(int x, int y);
    void line(int x2, int y2, int x1, int y1, bool useStartPos);
    void circle(int x1, int y1, int x2, int y2, int fillMode);
    void pset(int x, int y);
    bool pget(int x, int y, int& r, int& g, int& b);

    // フォント設定
    bool font(std::string_view fontName, int size, int style);
    bool sysfont(int type);

    // 描画制御
    void beginDraw();
    void endDraw();
    bool isDrawing() const { return m_isDrawing; }

    // ゲッター
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getCurrentX() const { return m_currentX; }
    int getCurrentY() const { return m_currentY; }
    D2D1_COLOR_F getCurrentColor() const { return m_currentColor; }
    ID2D1DeviceContext* getDeviceContext() const { return m_pDeviceContext.Get(); }
    ID2D1Bitmap1* getTargetBitmap() const { return m_pTargetBitmap.Get(); }
};

// 派生クラス: HspWindow
// ウィンドウを表すSurface（スワップチェーン使用）
class HspWindow : public HspSurface {
private:
    HWND m_hwnd;
    ComPtr<IDXGISwapChain1> m_pSwapChain;
    ComPtr<ID2D1Bitmap1> m_pBackBufferBitmap;  // スワップチェーンのバックバッファ
    std::wstring m_title;

public:
    HspWindow(int width, int height, std::string_view title);
    virtual ~HspWindow();

    // 初期化
    bool initialize() override;

    // ウィンドウ作成
    bool createWindow(
        HINSTANCE hInstance,
        std::wstring_view className,
        DWORD style,
        DWORD exStyle,
        int x,
        int y,
        int clientWidth,
        int clientHeight
    );

    // 画面に転送
    void present();

    // ゲッター
    HWND getHwnd() const { return m_hwnd; }

    // WM_PAINT処理
    void onPaint();

    // タイトル設定
    void setTitle(std::string_view title);

    // ウィンドウサイズ・位置設定
    void setClientSize(int clientW, int clientH);
    void setWindowPos(int x, int y);
};

// 派生クラス: HspBuffer
// 仮想画面を表すSurface（表示されない、共有可能ビットマップ）
class HspBuffer : public HspSurface {
public:
    HspBuffer(int width, int height);
    virtual ~HspBuffer() = default;

    // 初期化
    bool initialize() override;
};

// ウィンドウマネージャー
class WindowManager {
private:
    HINSTANCE m_hInstance;
    std::wstring m_className;
    bool m_classRegistered;

    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    WindowManager(WindowManager&&) = delete;
    WindowManager& operator=(WindowManager&&) = delete;

public:
    WindowManager();
    ~WindowManager();

    static WindowManager& getInstance();

    bool registerWindowClass();
    void unregisterWindowClass();

    HINSTANCE getHInstance() const { return m_hInstance; }
    std::wstring_view getClassName() const { return m_className; }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

} // namespace internal
} // namespace hsppp

// ============================================================
// 割り込みトリガー関数（WindowProcから呼び出し）
// ============================================================
namespace hsppp::internal {
    // クリック割り込みをトリガー
    void triggerOnClick(int buttonId, WPARAM wp, LPARAM lp);

    // キー割り込みをトリガー
    void triggerOnKey(int charCode, WPARAM wp, LPARAM lp);

    // Windowsメッセージ割り込みをトリガー
    // 戻り値: true=カスタム戻り値を使用, false=デフォルト処理
    bool triggerOnCmd(int messageId, WPARAM wp, LPARAM lp, int& returnValue);

    // 終了割り込みをトリガー
    // 戻り値: true=終了をブロック, false=終了を許可
    bool triggerOnExit(int windowId, int reason);
}
