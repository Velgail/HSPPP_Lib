// HspppLib/src/core/Surface.cpp
// HspSurface, HspWindow, HspBuffer の実装（Direct2D 1.1対応）

// グローバルモジュールフラグメント
module;

#include <windows.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>
#include <string_view>
#include <cstring>

#include "Internal.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwrite.lib")

// モジュール実装
module hsppp;

namespace hsppp {
namespace internal {

// UTF-8文字列をUTF-16(wchar_t)に変換
std::wstring Utf8ToWide(std::string_view utf8str) {
    if (utf8str.empty()) {
        return L"";
    }

    int wideSize = MultiByteToWideChar(
        CP_UTF8, 0,
        utf8str.data(), static_cast<int>(utf8str.size()),
        nullptr, 0
    );
    if (wideSize == 0) {
        return L"";
    }

    std::wstring wideStr(wideSize, 0);
    MultiByteToWideChar(
        CP_UTF8, 0,
        utf8str.data(), static_cast<int>(utf8str.size()),
        &wideStr[0], wideSize
    );

    return wideStr;
}

// ========== D2DDeviceManager 実装 ==========

D2DDeviceManager::D2DDeviceManager()
    : m_initialized(false)
{
}

D2DDeviceManager::~D2DDeviceManager() {
    shutdown();
}

D2DDeviceManager& D2DDeviceManager::getInstance() {
    static D2DDeviceManager instance;
    return instance;
}

bool D2DDeviceManager::initialize() {
    if (m_initialized) return true;

    HRESULT hr;

    // D3D11デバイスの作成
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    // デバッグビルドでもデバッグレイヤーは任意（インストールされていない場合があるため）
    // creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        m_pD3DDevice.GetAddressOf(),
        &featureLevel,
        m_pD3DContext.GetAddressOf()
    );

    if (FAILED(hr)) {
        // ハードウェアが使えない場合はWARPを試す
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            m_pD3DDevice.GetAddressOf(),
            &featureLevel,
            m_pD3DContext.GetAddressOf()
        );
        if (FAILED(hr)) return false;
    }

    // DXGIデバイスを取得
    hr = m_pD3DDevice.As(&m_pDxgiDevice);
    if (FAILED(hr)) return false;

    // Direct2D Factoryの作成
    D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
    // options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        options,
        m_pD2DFactory.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    // Direct2D Deviceの作成
    hr = m_pD2DFactory->CreateDevice(
        m_pDxgiDevice.Get(),
        m_pD2DDevice.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    // DirectWrite Factoryの作成
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(m_pDWriteFactory.GetAddressOf())
    );
    if (FAILED(hr)) return false;

    m_initialized = true;
    return true;
}

void D2DDeviceManager::shutdown() {
    m_pDWriteFactory.Reset();
    m_pD2DDevice.Reset();
    m_pD2DFactory.Reset();
    m_pDxgiDevice.Reset();
    m_pD3DContext.Reset();
    m_pD3DDevice.Reset();
    m_initialized = false;
}

ComPtr<ID2D1DeviceContext> D2DDeviceManager::createDeviceContext() {
    if (!m_initialized || !m_pD2DDevice) return nullptr;

    ComPtr<ID2D1DeviceContext> pContext;
    HRESULT hr = m_pD2DDevice->CreateDeviceContext(
        D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
        pContext.GetAddressOf()
    );

    return SUCCEEDED(hr) ? pContext : nullptr;
}

// ========== HspSurface 実装 ==========

HspSurface::HspSurface(int width, int height)
    : m_pDeviceContext(nullptr)
    , m_pTargetBitmap(nullptr)
    , m_pBrush(nullptr)
    , m_pTextFormat(nullptr)
    , m_currentX(0)
    , m_currentY(0)
    , m_currentColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f))
    , m_width(width)
    , m_height(height)
    , m_isDrawing(false)
{
}

void HspSurface::beginDraw() {
    if (m_pDeviceContext && !m_isDrawing) {
        m_pDeviceContext->BeginDraw();
        m_isDrawing = true;
    }
}

void HspSurface::endDraw() {
    if (m_pDeviceContext && m_isDrawing) {
        HRESULT hr = m_pDeviceContext->EndDraw();
        m_isDrawing = false;

        if (hr == D2DERR_RECREATE_TARGET) {
            // TODO: デバイスロスト時のリソース再作成
        }
    }
}

void HspSurface::boxf(int x1, int y1, int x2, int y2) {
    if (!m_pDeviceContext || !m_pBrush || !m_isDrawing) return;

    D2D1_RECT_F rect = D2D1::RectF(
        static_cast<FLOAT>(x1),
        static_cast<FLOAT>(y1),
        static_cast<FLOAT>(x2),
        static_cast<FLOAT>(y2)
    );

    m_pDeviceContext->FillRectangle(rect, m_pBrush.Get());
}

void HspSurface::mes(std::string_view text) {
    if (!m_pDeviceContext || !m_pBrush || !m_pTextFormat || !m_isDrawing) return;

    std::wstring wideText = Utf8ToWide(text);

    D2D1_RECT_F layoutRect = D2D1::RectF(
        static_cast<FLOAT>(m_currentX),
        static_cast<FLOAT>(m_currentY),
        static_cast<FLOAT>(m_currentX + m_width),
        static_cast<FLOAT>(m_currentY + m_height)
    );

    m_pDeviceContext->DrawText(
        wideText.c_str(),
        static_cast<UINT32>(wideText.length()),
        m_pTextFormat.Get(),
        layoutRect,
        m_pBrush.Get()
    );
}

void HspSurface::color(int r, int g, int b) {
    m_currentColor = D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    if (m_pBrush) {
        m_pBrush->SetColor(m_currentColor);
    }
}

void HspSurface::pos(int x, int y) {
    m_currentX = x;
    m_currentY = y;
}

void HspSurface::line(int x2, int y2, int x1, int y1, bool useStartPos) {
    if (!m_pDeviceContext || !m_pBrush || !m_isDrawing) return;

    // 始点を決定
    float startX = useStartPos ? static_cast<float>(x1) : static_cast<float>(m_currentX);
    float startY = useStartPos ? static_cast<float>(y1) : static_cast<float>(m_currentY);
    float endX = static_cast<float>(x2);
    float endY = static_cast<float>(y2);

    // 直線を描画（太さ1.0f）
    m_pDeviceContext->DrawLine(
        D2D1::Point2F(startX, startY),
        D2D1::Point2F(endX, endY),
        m_pBrush.Get(),
        1.0f
    );

    // カレントポジションを終点に更新
    m_currentX = x2;
    m_currentY = y2;
}

void HspSurface::circle(int x1, int y1, int x2, int y2, int fillMode) {
    if (!m_pDeviceContext || !m_pBrush || !m_isDrawing) return;

    // 楕円のパラメータを計算
    float centerX = (static_cast<float>(x1) + static_cast<float>(x2)) / 2.0f;
    float centerY = (static_cast<float>(y1) + static_cast<float>(y2)) / 2.0f;
    float radiusX = (static_cast<float>(x2) - static_cast<float>(x1)) / 2.0f;
    float radiusY = (static_cast<float>(y2) - static_cast<float>(y1)) / 2.0f;

    // 負の半径を正に（座標の順序が逆でも対応）
    if (radiusX < 0) radiusX = -radiusX;
    if (radiusY < 0) radiusY = -radiusY;

    D2D1_ELLIPSE ellipse = D2D1::Ellipse(
        D2D1::Point2F(centerX, centerY),
        radiusX,
        radiusY
    );

    if (fillMode == 1) {
        // 塗りつぶし
        m_pDeviceContext->FillEllipse(ellipse, m_pBrush.Get());
    } else {
        // 輪郭のみ
        m_pDeviceContext->DrawEllipse(ellipse, m_pBrush.Get(), 1.0f);
    }
}

void HspSurface::pset(int x, int y) {
    if (!m_pDeviceContext || !m_pBrush || !m_isDrawing) return;

    // 1ドットの点を描画（1x1の矩形）
    D2D1_RECT_F rect = D2D1::RectF(
        static_cast<FLOAT>(x),
        static_cast<FLOAT>(y),
        static_cast<FLOAT>(x + 1),
        static_cast<FLOAT>(y + 1)
    );

    m_pDeviceContext->FillRectangle(rect, m_pBrush.Get());
}

bool HspSurface::pget(int x, int y, int& r, int& g, int& b) {
    if (!m_pDeviceContext || !m_pTargetBitmap) return false;

    // 描画中の場合は一旦終了してピクセルを読み取る
    bool wasDrawing = m_isDrawing;
    if (wasDrawing) {
        m_pDeviceContext->EndDraw();
        m_isDrawing = false;
    }

    // CPU読み取り可能なビットマップを作成
    ComPtr<ID2D1Bitmap1> pReadBitmap;
    D2D1_BITMAP_PROPERTIES1 readProps = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    HRESULT hr = m_pDeviceContext->CreateBitmap(
        D2D1::SizeU(m_width, m_height),
        nullptr, 0,
        readProps,
        pReadBitmap.GetAddressOf()
    );

    if (FAILED(hr)) {
        if (wasDrawing) {
            m_pDeviceContext->BeginDraw();
            m_isDrawing = true;
        }
        return false;
    }

    // 指定座標のピクセルをコピー
    D2D1_POINT_2U destPoint = D2D1::Point2U(0, 0);
    D2D1_RECT_U srcRect = D2D1::RectU(x, y, x + 1, y + 1);
    hr = pReadBitmap->CopyFromBitmap(&destPoint, m_pTargetBitmap.Get(), &srcRect);

    if (FAILED(hr)) {
        if (wasDrawing) {
            m_pDeviceContext->BeginDraw();
            m_isDrawing = true;
        }
        return false;
    }

    // ピクセルデータをマップして読み取る
    D2D1_MAPPED_RECT mappedRect;
    hr = pReadBitmap->Map(D2D1_MAP_OPTIONS_READ, &mappedRect);

    if (SUCCEEDED(hr)) {
        // BGRA形式なので順序に注意
        BYTE* pixel = mappedRect.bits;
        b = pixel[0];
        g = pixel[1];
        r = pixel[2];
        // pixel[3] はアルファ値

        pReadBitmap->Unmap();

        // 取得した色を選択色として設定
        m_currentColor = D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
        if (m_pBrush) {
            m_pBrush->SetColor(m_currentColor);
        }
    }

    // 描画状態を復元
    if (wasDrawing) {
        m_pDeviceContext->BeginDraw();
        m_isDrawing = true;
    }

    return SUCCEEDED(hr);
}

// ========== HspWindow 実装 ==========

HspWindow::HspWindow(int width, int height, std::string_view title)
    : HspSurface(width, height)
    , m_hwnd(nullptr)
    , m_pSwapChain(nullptr)
    , m_title(Utf8ToWide(title))
{
}

HspWindow::~HspWindow() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

bool HspWindow::initialize() {
    auto& deviceMgr = D2DDeviceManager::getInstance();
    if (!deviceMgr.isInitialized()) {
        return false;
    }

    HRESULT hr;

    // スワップチェーンの作成
    ComPtr<IDXGIAdapter> pAdapter;
    hr = deviceMgr.getDxgiDevice()->GetAdapter(pAdapter.GetAddressOf());
    if (FAILED(hr)) return false;

    ComPtr<IDXGIFactory2> pFactory;
    hr = pAdapter->GetParent(IID_PPV_ARGS(pFactory.GetAddressOf()));
    if (FAILED(hr)) return false;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = 0;

    hr = pFactory->CreateSwapChainForHwnd(
        deviceMgr.getD3DDevice(),
        m_hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        m_pSwapChain.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    // デバイスコンテキストの作成
    m_pDeviceContext = deviceMgr.createDeviceContext();
    if (!m_pDeviceContext) return false;

    // スワップチェーンのバックバッファからビットマップを作成（描画ターゲット用）
    ComPtr<IDXGISurface> pBackBuffer;
    hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
    if (FAILED(hr)) return false;

    // バックバッファビットマップ（CANNOT_DRAWが必要 - スワップチェーンの制約）
    D2D1_BITMAP_PROPERTIES1 backBufferProps = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
    );

    hr = m_pDeviceContext->CreateBitmapFromDxgiSurface(
        pBackBuffer.Get(),
        backBufferProps,
        m_pBackBufferBitmap.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    // オフスクリーンビットマップを作成（gcopy/gzoomソースとして使用可能）
    D2D1_SIZE_U bitmapSize = D2D1::SizeU(m_width, m_height);
    D2D1_BITMAP_PROPERTIES1 offscreenProps = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET,  // CANNOT_DRAWなし - gcopy可能
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
    );

    hr = m_pDeviceContext->CreateBitmap(
        bitmapSize,
        nullptr, 0,
        offscreenProps,
        m_pTargetBitmap.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    // デフォルトはオフスクリーンビットマップに描画
    m_pDeviceContext->SetTarget(m_pTargetBitmap.Get());

    // ブラシの作成
    hr = m_pDeviceContext->CreateSolidColorBrush(
        m_currentColor,
        m_pBrush.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    // テキストフォーマットの作成
    hr = deviceMgr.getDWriteFactory()->CreateTextFormat(
        L"MS Gothic",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"ja-jp",
        m_pTextFormat.GetAddressOf()
    );

    return SUCCEEDED(hr);
}

bool HspWindow::createWindow(
    HINSTANCE hInstance,
    std::wstring_view className,
    DWORD style,
    DWORD exStyle,
    int x,
    int y,
    int clientWidth,
    int clientHeight
) {
    RECT rect = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    std::wstring classNameStr(className);

    int posX = (x < 0) ? CW_USEDEFAULT : x;
    int posY = (y < 0) ? CW_USEDEFAULT : y;

    m_hwnd = CreateWindowExW(
        exStyle,
        classNameStr.c_str(),
        m_title.c_str(),
        style,
        posX, posY,
        windowWidth, windowHeight,
        nullptr, nullptr,
        hInstance,
        this
    );

    return m_hwnd != nullptr;
}

void HspWindow::present() {
    if (!m_pSwapChain || !m_pDeviceContext || !m_pTargetBitmap || !m_pBackBufferBitmap) {
        return;
    }

    // オフスクリーンビットマップの内容をバックバッファにコピー
    m_pDeviceContext->SetTarget(m_pBackBufferBitmap.Get());
    m_pDeviceContext->BeginDraw();

    // オフスクリーンビットマップをバックバッファ全体にコピー
    D2D1_RECT_F destRect = D2D1::RectF(0, 0, static_cast<float>(m_width), static_cast<float>(m_height));
    m_pDeviceContext->DrawBitmap(m_pTargetBitmap.Get(), destRect);

    m_pDeviceContext->EndDraw();

    // 画面に表示（垂直同期を待たない - redraw(1)での高速描画用）
    // SyncInterval=0 で即座に表示
    m_pSwapChain->Present(0, 0);

    // 描画先をオフスクリーンビットマップに戻す
    m_pDeviceContext->SetTarget(m_pTargetBitmap.Get());
}

void HspWindow::onPaint() {
    // WM_PAINTでは何もしない（スワップチェーンが自動的に処理）
}

void HspWindow::setTitle(std::string_view title) {
    if (!m_hwnd) return;
    m_title = Utf8ToWide(title);
    SetWindowTextW(m_hwnd, m_title.c_str());
}

void HspWindow::setClientSize(int clientW, int clientH) {
    if (!m_hwnd) return;

    // 現在のウィンドウスタイルを取得
    DWORD style = static_cast<DWORD>(GetWindowLongPtr(m_hwnd, GWL_STYLE));
    DWORD exStyle = static_cast<DWORD>(GetWindowLongPtr(m_hwnd, GWL_EXSTYLE));

    // クライアントサイズからウィンドウサイズを計算
    RECT rect = { 0, 0, clientW, clientH };
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    // ウィンドウサイズを変更（位置は維持）
    SetWindowPos(m_hwnd, nullptr, 0, 0, windowWidth, windowHeight, 
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void HspWindow::setWindowPos(int x, int y) {
    if (!m_hwnd) return;
    SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, 
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

// ========== HspSurface フォント関連実装 ==========

bool HspSurface::font(std::string_view fontName, int size, int style) {
    auto& deviceMgr = D2DDeviceManager::getInstance();
    if (!deviceMgr.getDWriteFactory()) return false;

    std::wstring wideFontName = Utf8ToWide(fontName);

    // スタイルフラグの解析
    DWRITE_FONT_WEIGHT weight = (style & 1) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STYLE fontStyle = (style & 2) ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
    // 下線(4)、打ち消し線(8)はDrawTextではなくTextLayoutで処理が必要
    // アンチエイリアス(16)はDirect2Dではデフォルトで有効

    // 新しいTextFormatを作成
    ComPtr<IDWriteTextFormat> pNewFormat;
    HRESULT hr = deviceMgr.getDWriteFactory()->CreateTextFormat(
        wideFontName.c_str(),
        nullptr,
        weight,
        fontStyle,
        DWRITE_FONT_STRETCH_NORMAL,
        static_cast<float>(size),
        L"ja-jp",
        pNewFormat.GetAddressOf()
    );

    if (SUCCEEDED(hr)) {
        m_pTextFormat = pNewFormat;
        return true;
    }
    return false;
}

bool HspSurface::sysfont(int type) {
    auto& deviceMgr = D2DDeviceManager::getInstance();
    if (!deviceMgr.getDWriteFactory()) return false;

    // システムフォントの情報を取得
    HFONT hFont = nullptr;
    LOGFONTW lf = {};

    switch (type) {
    case 0:  // HSP標準システムフォント
    default:
        // MS Gothicを使用
        wcscpy_s(lf.lfFaceName, L"MS Gothic");
        lf.lfHeight = -14;
        break;
    case 10: // OEM固定幅フォント
        hFont = static_cast<HFONT>(GetStockObject(OEM_FIXED_FONT));
        break;
    case 11: // Windows固定幅システムフォント
        hFont = static_cast<HFONT>(GetStockObject(ANSI_FIXED_FONT));
        break;
    case 12: // Windows可変幅システムフォント
        hFont = static_cast<HFONT>(GetStockObject(ANSI_VAR_FONT));
        break;
    case 13: // 標準システムフォント
        hFont = static_cast<HFONT>(GetStockObject(SYSTEM_FONT));
        break;
    case 17: // デフォルトGUIフォント
        hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        break;
    }

    // ストックフォントから情報を取得
    if (hFont) {
        GetObjectW(hFont, sizeof(LOGFONTW), &lf);
    }

    // フォント名が空の場合はデフォルト
    if (lf.lfFaceName[0] == L'\0') {
        wcscpy_s(lf.lfFaceName, L"MS Gothic");
    }

    // フォントサイズの計算（論理単位からポイントへ）
    float fontSize = 14.0f;
    if (lf.lfHeight != 0) {
        HDC hdc = GetDC(nullptr);
        fontSize = static_cast<float>(abs(lf.lfHeight) * 72 / GetDeviceCaps(hdc, LOGPIXELSY));
        ReleaseDC(nullptr, hdc);
    }

    // DWriteのウェイトとスタイル
    DWRITE_FONT_WEIGHT weight = (lf.lfWeight >= FW_BOLD) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STYLE fontStyle = lf.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;

    // 新しいTextFormatを作成
    ComPtr<IDWriteTextFormat> pNewFormat;
    HRESULT hr = deviceMgr.getDWriteFactory()->CreateTextFormat(
        lf.lfFaceName,
        nullptr,
        weight,
        fontStyle,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"ja-jp",
        pNewFormat.GetAddressOf()
    );

    if (SUCCEEDED(hr)) {
        m_pTextFormat = pNewFormat;
        return true;
    }
    return false;
}

// ========== HspBuffer 実装 ==========

HspBuffer::HspBuffer(int width, int height)
    : HspSurface(width, height)
{
}

bool HspBuffer::initialize() {
    auto& deviceMgr = D2DDeviceManager::getInstance();
    if (!deviceMgr.isInitialized()) return false;

    HRESULT hr;

    // デバイスコンテキストの作成
    m_pDeviceContext = deviceMgr.createDeviceContext();
    if (!m_pDeviceContext) return false;

    // 描画可能なビットマップを作成（D2D1_BITMAP_OPTIONS_TARGETのみ）
    // D2D1_BITMAP_OPTIONS_CANNOT_DRAWがないので、他のDeviceContextから描画可能
    D2D1_BITMAP_PROPERTIES1 bitmapProps = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    D2D1_SIZE_U size = D2D1::SizeU(m_width, m_height);

    hr = m_pDeviceContext->CreateBitmap(
        size,
        nullptr,
        0,
        bitmapProps,
        m_pTargetBitmap.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    m_pDeviceContext->SetTarget(m_pTargetBitmap.Get());

    // ブラシの作成
    hr = m_pDeviceContext->CreateSolidColorBrush(
        m_currentColor,
        m_pBrush.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    // テキストフォーマットの作成
    hr = deviceMgr.getDWriteFactory()->CreateTextFormat(
        L"MS Gothic",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"ja-jp",
        m_pTextFormat.GetAddressOf()
    );

    return SUCCEEDED(hr);
}

} // namespace internal
} // namespace hsppp
