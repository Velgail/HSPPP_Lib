// HspppLib/src/core/Surface.cpp
// HspSurface と HspWindow の実装

#include "Internal.h"
#include <cstring>

namespace hsppp {
namespace internal {

// UTF-8文字列をUTF-16(wchar_t)に変換（安全な string_view を使用）
std::wstring Utf8ToWide(std::string_view utf8str) {
    if (utf8str.empty()) {
        return L"";
    }

    // 必要なバッファサイズを取得
    int wideSize = MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8str.data(),
        static_cast<int>(utf8str.size()),
        nullptr,
        0
    );
    if (wideSize == 0) {
        return L"";
    }

    // 変換
    std::wstring wideStr(wideSize, 0);
    MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8str.data(),
        static_cast<int>(utf8str.size()),
        &wideStr[0],
        wideSize
    );

    return wideStr;
}

// ========== HspSurface 実装 ==========

HspSurface::HspSurface(int width, int height)
    : m_pBitmapTarget(nullptr)
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

// デストラクタはdefaultなので実装不要（ComPtrが自動解放）

bool HspSurface::initialize(const ComPtr<ID2D1Factory>& pD2DFactory,
                            const ComPtr<IDWriteFactory>& pDWriteFactory) {
    // 基底クラスでは何もしない（派生クラスで実装）
    return false;
}

void HspSurface::activate() {
    // 現在のサーフェスとして設定（hsppp.cppで管理）
}

void HspSurface::beginDraw() {
    if (m_pBitmapTarget && !m_isDrawing) {
        m_pBitmapTarget->BeginDraw();
        m_isDrawing = true;
    }
}

void HspSurface::endDraw() {
    if (m_pBitmapTarget && m_isDrawing) {
        HRESULT hr = m_pBitmapTarget->EndDraw();
        m_isDrawing = false;

        // デバイスロスト時の処理（今は無視）
        if (hr == D2DERR_RECREATE_TARGET) {
            // TODO: リソースの再作成
        }
    }
}

void HspSurface::boxf(int x1, int y1, int x2, int y2) {
    if (!m_pBitmapTarget || !m_pBrush) return;
    if (!m_isDrawing) return;

    D2D1_RECT_F rect = D2D1::RectF(
        static_cast<FLOAT>(x1),
        static_cast<FLOAT>(y1),
        static_cast<FLOAT>(x2),
        static_cast<FLOAT>(y2)
    );

    m_pBitmapTarget->FillRectangle(rect, m_pBrush.Get());
}

void HspSurface::mes(std::string_view text) {
    if (!m_pBitmapTarget || !m_pBrush || !m_pTextFormat) return;
    if (!m_isDrawing) return;

    // UTF-8からUTF-16に変換
    std::wstring wideText = Utf8ToWide(text);

    // テキスト描画領域
    D2D1_RECT_F layoutRect = D2D1::RectF(
        static_cast<FLOAT>(m_currentX),
        static_cast<FLOAT>(m_currentY),
        static_cast<FLOAT>(m_currentX + m_width),
        static_cast<FLOAT>(m_currentY + m_height)
    );

    m_pBitmapTarget->DrawText(
        wideText.c_str(),
        static_cast<UINT32>(wideText.length()),
        m_pTextFormat.Get(),
        layoutRect,
        m_pBrush.Get()
    );
}

void HspSurface::color(int r, int g, int b) {
    m_currentColor = D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);

    // ブラシの色を更新
    if (m_pBrush) {
        m_pBrush->SetColor(m_currentColor);
    }
}

void HspSurface::pos(int x, int y) {
    m_currentX = x;
    m_currentY = y;
}

// ========== HspWindow 実装 ==========

HspWindow::HspWindow(int width, int height, std::string_view title)
    : HspSurface(width, height)
    , m_hwnd(nullptr)
    , m_pHwndTarget(nullptr)
    , m_title(Utf8ToWide(title))
{
}

// デストラクタはdefaultだが、HWNDの破棄は手動で行う必要がある
HspWindow::~HspWindow() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    // ComPtrは自動解放される
}

bool HspWindow::initialize(const ComPtr<ID2D1Factory>& pD2DFactory,
                          const ComPtr<IDWriteFactory>& pDWriteFactory) {
    if (!pD2DFactory || !pDWriteFactory) return false;

    // HwndRenderTargetの作成（表示用）
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(
        m_hwnd,
        D2D1::SizeU(m_width, m_height)
    );

    HRESULT hr = pD2DFactory->CreateHwndRenderTarget(
        props,
        hwndProps,
        m_pHwndTarget.GetAddressOf()
    );

    if (FAILED(hr)) {
        return false;
    }

    // BitmapRenderTargetの作成（オフスクリーン描画用）
    D2D1_SIZE_F size = D2D1::SizeF(
        static_cast<FLOAT>(m_width),
        static_cast<FLOAT>(m_height)
    );

    hr = m_pHwndTarget->CreateCompatibleRenderTarget(
        size,
        m_pBitmapTarget.GetAddressOf()
    );

    if (FAILED(hr)) {
        return false;
    }

    // ブラシの作成
    hr = m_pBitmapTarget->CreateSolidColorBrush(
        m_currentColor,
        m_pBrush.GetAddressOf()
    );

    if (FAILED(hr)) {
        return false;
    }

    // テキストフォーマットの作成
    hr = pDWriteFactory->CreateTextFormat(
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
    // クライアント領域のサイズを指定されたサイズにするため、
    // ウィンドウ全体のサイズを計算
    RECT rect = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    // wstring_view から wstring に変換してnull終端を保証
    std::wstring classNameStr(className);

    // ウィンドウ位置の決定（-1の場合はシステム規定）
    int posX = (x < 0) ? CW_USEDEFAULT : x;
    int posY = (y < 0) ? CW_USEDEFAULT : y;

    // ウィンドウの作成
    m_hwnd = CreateWindowExW(
        exStyle,                // 拡張スタイル
        classNameStr.c_str(),   // ウィンドウクラス名（null終端保証）
        m_title.c_str(),        // ウィンドウタイトル
        style,                  // ウィンドウスタイル
        posX,                   // X座標
        posY,                   // Y座標
        windowWidth,            // 幅
        windowHeight,           // 高さ
        nullptr,                // 親ウィンドウ
        nullptr,                // メニュー
        hInstance,              // インスタンスハンドル
        this                    // 追加パラメータ（thisポインタを渡す）
    );

    if (!m_hwnd) {
        return false;
    }

    // ウィンドウの表示はscreen()側で行う（screen_hideフラグの処理のため）
    // ShowWindow/UpdateWindowは呼ばない

    return true;
}

void HspWindow::present() {
    if (!m_pHwndTarget || !m_pBitmapTarget) return;

    // オフスクリーンバッファから画面への転送
    m_pHwndTarget->BeginDraw();

    // ビットマップを取得（ComPtrで安全に管理）
    ComPtr<ID2D1Bitmap> pBitmap;
    HRESULT hr = m_pBitmapTarget->GetBitmap(pBitmap.GetAddressOf());

    if (SUCCEEDED(hr) && pBitmap) {
        // ビットマップを画面に描画
        D2D1_RECT_F destRect = D2D1::RectF(
            0.0f,
            0.0f,
            static_cast<FLOAT>(m_width),
            static_cast<FLOAT>(m_height)
        );

        m_pHwndTarget->DrawBitmap(
            pBitmap.Get(),
            destRect,
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
        );

        // ComPtrなので自動解放される
    }

    hr = m_pHwndTarget->EndDraw();

    // デバイスロスト時の処理
    if (hr == D2DERR_RECREATE_TARGET) {
        // TODO: リソースの再作成
    }
}

void HspWindow::onPaint() {
    // WM_PAINTメッセージを受け取った際、オフスクリーンバッファを再描画
    present();
}

} // namespace internal
} // namespace hsppp
