// HspppLib/src/core/Internal.h
// 内部実装用ヘッダ（モジュール間で共有するクラス定義）

#pragma once

#include <windows.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <string>
#include <string_view>
#include <memory>
#include <map>

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

// ============================================================
// 画像素材管理構造体（celシリーズ用）
// ============================================================
struct CelData {
    ComPtr<ID2D1Bitmap1> pBitmap;     // 画像ビットマップ
    int width;                         // 画像の幅
    int height;                        // 画像の高さ
    int divX;                          // 横方向の分割サイズ
    int divY;                          // 縦方向の分割サイズ
    int centerX;                       // 中心X座標
    int centerY;                       // 中心Y座標
    std::string filename;              // ファイル名（再利用チェック用）
    
    CelData() : width(0), height(0), divX(0), divY(0), centerX(0), centerY(0) {}
};

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
    ComPtr<IWICImagingFactory> m_pWICFactory;

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
    IWICImagingFactory* getWICFactory() const { return m_pWICFactory.Get(); }
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

    // 描画モード (0: バッチモード, 1: 即時反映)
    int m_redrawMode;

public:
    HspSurface(int width, int height);
    virtual ~HspSurface() = default;

    // 初期化（派生クラスで実装）
    virtual bool initialize() = 0;

    // 描画命令
    void cls(int mode = 0);
    void boxf(int x1, int y1, int x2, int y2);
    void mes(std::string_view text, int options = 0);
    void color(int r, int g, int b);
    void pos(int x, int y);
    void line(int x2, int y2, int x1, int y1, bool useStartPos);
    void circle(int x1, int y1, int x2, int y2, int fillMode);
    void pset(int x, int y);
    bool pget(int x, int y, int& r, int& g, int& b);

    // 拡張描画命令
    void gradf(int x, int y, int w, int h, int mode, int color1, int color2);
    void grect(int cx, int cy, double angle, int w, int h);
    void grotate(ID2D1Bitmap1* pSrcBitmap, int srcX, int srcY, int srcW, int srcH, double angle, int dstW, int dstH);
    void gsquare(int* dstX, int* dstY, ID2D1Bitmap1* pSrcBitmap = nullptr, int* srcX = nullptr, int* srcY = nullptr);
    void gsquareGrad(int* dstX, int* dstY, int* colors);

    // フォント設定
    bool font(std::string_view fontName, int size, int style);
    bool sysfont(int type);

    // 画像操作
    bool picload(std::string_view filename, int mode);
    bool bmpsave(std::string_view filename);
    void celput(ID2D1Bitmap1* pBitmap, const D2D1_RECT_F& srcRect, const D2D1_RECT_F& destRect);

    // 描画制御
    void beginDraw();
    void endDraw();
    virtual void endDrawAndPresent();  // 派生クラスでオーバーライド
    bool isDrawing() const { return m_isDrawing; }

    // 描画モード制御
    void setRedrawMode(int mode) { m_redrawMode = mode; }
    int getRedrawMode() const { return m_redrawMode; }

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
    
    // クライアントサイズ（実際のウィンドウ表示サイズ、m_width/m_height以下）
    int m_clientWidth;
    int m_clientHeight;
    
    // スクロール位置（groll用）
    int m_scrollX;
    int m_scrollY;

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
    void endDrawAndPresent() override;

    // ゲッター
    HWND getHwnd() const { return m_hwnd; }
    int getClientWidth() const { return m_clientWidth; }
    int getClientHeight() const { return m_clientHeight; }
    int getScrollX() const { return m_scrollX; }
    int getScrollY() const { return m_scrollY; }

    // WM_PAINT処理
    void onPaint();

    // タイトル設定
    void setTitle(std::string_view title);

    // ウィンドウサイズ・位置設定（width命令用）
    // クライアントサイズは画面サイズ（m_width, m_height）以下にクランプ
    void setClientSize(int clientW, int clientH);
    void setWindowPos(int x, int y);
    
    // スクロール位置設定（groll命令用）
    void setScroll(int x, int y);

    // 現在のクライアントサイズを取得（width関数用）
    void getCurrentClientSize(int& outWidth, int& outHeight) const;
    
private:
    // スワップチェーンをリサイズ（内部用）
    bool resizeSwapChain(int newWidth, int newHeight);
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

    // 画像読み込み・保存（ImageLoader.cpp）
    ComPtr<ID2D1Bitmap1> loadImageFile(std::string_view filename, int& width, int& height);
    bool saveBitmapToFile(ID2D1Bitmap1* pBitmap, std::string_view filename);

    // cel素材管理（ImageLoader.cpp）
    extern std::map<int, CelData> g_celDataMap;
    extern int g_nextCelId;
}
