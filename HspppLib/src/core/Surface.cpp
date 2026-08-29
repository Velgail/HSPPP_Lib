// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/src/core/Surface.cpp
// HspSurface, HspWindow, HspBuffer の実装（Direct2D 1.3対応）

// グローバルモジュールフラグメント
module;

#define NOMINMAX
#include <windows.h>
#include <d2d1_1.h>
#include <d2d1_3.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>
#include <string_view>
#include <cstring>
#include <algorithm>
#include <cmath>

#include "Internal.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwrite.lib")

// モジュール実装
module hsppp;

namespace hsppp {
namespace internal {

// ラジアンから度への変換用定数
constexpr double kRadToDeg = 180.0 / 3.14159265358979323846;

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

// UTF-16文字列をUTF-8に変換
std::string WideToUtf8(const std::wstring& wideStr) {
    if (wideStr.empty()) {
        return "";
    }

    int utf8Size = WideCharToMultiByte(
        CP_UTF8, 0,
        wideStr.c_str(), static_cast<int>(wideStr.size()),
        nullptr, 0, nullptr, nullptr
    );
    if (utf8Size == 0) {
        return "";
    }

    std::string utf8Str(utf8Size, 0);
    WideCharToMultiByte(
        CP_UTF8, 0,
        wideStr.c_str(), static_cast<int>(wideStr.size()),
        &utf8Str[0], utf8Size, nullptr, nullptr
    );

    return utf8Str;
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

    // WIC Factoryの作成
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(m_pWICFactory.GetAddressOf())
    );
    if (FAILED(hr)) return false;

    m_initialized = true;
    return true;
}

void D2DDeviceManager::shutdown() {
    m_pWICFactory.Reset();
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
    , m_redrawMode(1)  // デフォルト: 即時反映
    , m_gmodeMode(0)        // デフォルト: 通常コピー
    , m_gmodeSizeX(32)      // デフォルトコピーサイズ
    , m_gmodeSizeY(32)
    , m_gmodeBlendRate(0)   // ブレンドなし
    , m_celWidth(width)
    , m_celHeight(height)
    , m_celCenterX(0)
    , m_celCenterY(0)
    , m_objSizeX(64)        // デフォルトオブジェクトサイズ
    , m_objSizeY(24)
    , m_objSpaceY(0)        // デフォルト間隔
    , m_lastMesSizeX(0)     // 最後のmes出力サイズ
    , m_lastMesSizeY(0)
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

void HspSurface::endDrawAndPresent() {
    // 基底クラスでは endDraw() のみ（派生クラスでオーバーライド）
    endDraw();
}

void HspSurface::cls(int mode) {
    if (!m_pDeviceContext) return;

    // モードに応じてクリア色を設定
    D2D1_COLOR_F clearColor;
    switch (mode) {
    case 0:  // 白
        clearColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
        break;
    case 1:  // 明るい灰色
        clearColor = D2D1::ColorF(0.75f, 0.75f, 0.75f, 1.0f);
        break;
    case 2:  // 灰色
        clearColor = D2D1::ColorF(0.5f, 0.5f, 0.5f, 1.0f);
        break;
    case 3:  // 暗い灰色
        clearColor = D2D1::ColorF(0.25f, 0.25f, 0.25f, 1.0f);
        break;
    case 4:  // 黒
        clearColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f);
        break;
    default:
        clearColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);  // デフォルトは白
        break;
    }

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    // 画面をクリア
    m_pDeviceContext->Clear(clearColor);

    // フォント・カラー設定を初期状態に戻す
    m_currentColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f);  // 黒
    if (m_pBrush) {
        m_pBrush->SetColor(m_currentColor);
    }

    // カレントポジションをリセット
    m_currentX = 0;
    m_currentY = 0;

    // オブジェクト配置サイズも画面初期状態へ戻す
    m_objSizeX = 64;
    m_objSizeY = 24;
    m_objSpaceY = 0;

    // フォントを初期状態に戻す
    sysfont(0);

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

void HspSurface::boxf(int x1, int y1, int x2, int y2) {
    if (!m_pDeviceContext || !m_pBrush) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    D2D1_RECT_F rect = D2D1::RectF(
        static_cast<FLOAT>(x1),
        static_cast<FLOAT>(y1),
        static_cast<FLOAT>(x2),
        static_cast<FLOAT>(y2)
    );

    m_pDeviceContext->FillRectangle(rect, m_pBrush.Get());

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

bool HspSurface::picload(std::string_view filename, int mode) {
    if (!m_pDeviceContext) return false;

    // モードに応じて画面をクリア
    if (mode == 0 || mode == 2) {
        int clsMode = (mode == 0) ? 0 : 4;  // 0=白, 2=黒
        cls(clsMode);
    }

    // 画像読み込み
    int width = 0, height = 0;
    auto bitmap = loadImageFile(filename, width, height);
    if (!bitmap) {
        return false;
    }

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return false;

    // 現在位置に描画
    D2D1_RECT_F destRect = D2D1::RectF(
        static_cast<float>(m_currentX),
        static_cast<float>(m_currentY),
        static_cast<float>(m_currentX + width),
        static_cast<float>(m_currentY + height)
    );

    m_pDeviceContext->DrawBitmap(
        bitmap.Get(),
        &destRect,
        1.0f,  // opacity
        m_gmodeInterp,
        nullptr,  // sourceRect (全体)
        nullptr   // perspectiveTransform
    );

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }

    return true;
}

bool HspSurface::bmpsave(std::string_view filename) {
    if (!m_pTargetBitmap) return false;
    
    // 描画中の場合、描画をフラッシュしてからビットマップを保存
    // (EndDrawを呼ばないとD2Dの描画コマンドがビットマップに反映されない)
    bool wasDrawing = m_isDrawing;
    if (wasDrawing) {
        endDraw();
    }
    
    bool result = saveBitmapToFile(m_pTargetBitmap.Get(), filename);
    
    // 描画中だった場合は描画状態を復元
    if (wasDrawing) {
        beginDraw();
    }
    
    return result;
}

void HspSurface::setCelDivision(int width, int height, int centerX, int centerY) {
    m_celWidth = width > 0 ? width : m_width;
    m_celHeight = height > 0 ? height : m_height;
    m_celCenterX = centerX;
    m_celCenterY = centerY;
}

void HspSurface::resetCelDivision() {
    m_celWidth = m_width;
    m_celHeight = m_height;
    m_celCenterX = 0;
    m_celCenterY = 0;
}

void HspSurface::celput(ID2D1Bitmap1* pBitmap, const D2D1_RECT_F& srcRect, const D2D1_RECT_F& destRect) {
    if (!m_pDeviceContext || !pBitmap) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    m_pDeviceContext->DrawBitmap(
        pBitmap,
        &destRect,
        1.0f,  // opacity
        m_gmodeInterp,
        &srcRect,
        nullptr   // perspectiveTransform
    );

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

void HspSurface::celputHsp(
    ID2D1Bitmap1* pBitmap,
    const D2D1_RECT_F& srcRect,
    float cellWidth,
    float cellHeight,
    float centerX,
    float centerY,
    double zoomX,
    double zoomY,
    double angle
) {
    if (!m_pDeviceContext || !pBitmap) return;

    const bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    D2D1_MATRIX_3X2_F oldTransform;
    m_pDeviceContext->GetTransform(&oldTransform);

    const float destinationX = static_cast<float>(m_currentX);
    const float destinationY = static_cast<float>(m_currentY);
    const float angleDegrees = static_cast<float>(angle * kRadToDeg);
    const auto localTransform =
        D2D1::Matrix3x2F::Translation(-centerX, -centerY) *
        D2D1::Matrix3x2F::Scale(static_cast<float>(zoomX), static_cast<float>(zoomY)) *
        D2D1::Matrix3x2F::Rotation(angleDegrees) *
        D2D1::Matrix3x2F::Translation(destinationX, destinationY);
    m_pDeviceContext->SetTransform(localTransform * oldTransform);

    float opacity = 1.0f;
    if (m_gmodeMode >= 3 && m_gmodeMode <= 6) {
        opacity = static_cast<float>(m_gmodeBlendRate) / 256.0f;
    }

    const D2D1_PRIMITIVE_BLEND oldBlend = m_pDeviceContext->GetPrimitiveBlend();
    if (m_gmodeMode == 5) {
        m_pDeviceContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_ADD);
    } else if (m_gmodeMode == 6) {
        m_pDeviceContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_MIN);
    }

    const D2D1_RECT_F destinationRect = D2D1::RectF(0.0f, 0.0f, cellWidth, cellHeight);
    m_pDeviceContext->DrawBitmap(
        pBitmap,
        destinationRect,
        opacity,
        static_cast<D2D1_BITMAP_INTERPOLATION_MODE>(m_gmodeInterp),
        srcRect
    );

    m_pDeviceContext->SetPrimitiveBlend(oldBlend);
    m_pDeviceContext->SetTransform(oldTransform);
    m_currentX += static_cast<int>(std::round(cellWidth * zoomX));

    if (autoManage) {
        endDrawAndPresent();
    }
}

void HspSurface::mes(std::string_view text, int options) {
    // v2 統一テキスト経路（HiDPI 設計大改修以降）:
    //   - m_pTargetBitmap は HspWindow では物理 px サイズ（destW×destH）で生成され、
    //     beginDraw() で SetDpi(96.0f) 固定 + SetTransform(Scale(s)) が適用されている。
    //   - したがって本関数も他の描画 API と同経路で、論理 px 座標 (m_currentX, m_currentY) /
    //     論理 DIP 単位の CreateTextLayout を発行するだけで DWrite のサブピクセル AA が
    //     最終物理解像度に対して動作する（v1 §6.4 の SetTarget(BackBuffer) 副パイプライン廃止）。
    //   - font_mode_buffer 命令は PM Q-H 完全廃止裁定により新設しない（旧挙動オプトインも提供しない）。
    //   - HspBuffer 経路では m_pTargetBitmap が論理 = 物理 / SetDpi=96 / Identity Transform で
    //     動作するため、本関数の論理 px 引数解釈はそのまま正しい。
    if (!m_pDeviceContext || !m_pBrush || !m_pTextFormat) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    std::wstring wideText = Utf8ToWide(text);

    // オプションの解析
    bool nocr = (options & 1) != 0;        // mesopt_nocr: 改行しない
    bool shadow = (options & 2) != 0;      // mesopt_shadow: 影付き
    bool outline = (options & 4) != 0;     // mesopt_outline: 縁取り
    [[maybe_unused]] bool light = (options & 8) != 0;       // mesopt_light: 簡易描画
    // bool gmode = (options & 16) != 0;   // mesopt_gmode: gmode反映（未実装）

    D2D1_RECT_F layoutRect = D2D1::RectF(
        static_cast<FLOAT>(m_currentX),
        static_cast<FLOAT>(m_currentY),
        static_cast<FLOAT>(m_currentX + m_width),
        static_cast<FLOAT>(m_currentY + m_height)
    );

    // テキストサイズを計算するため、IDWriteTextLayout を作成
    ComPtr<IDWriteTextLayout> pTextLayout;
    IDWriteFactory* pDWriteFactory = D2DDeviceManager::getInstance().getDWriteFactory();
    if (pDWriteFactory) {
        HRESULT hr = pDWriteFactory->CreateTextLayout(
            wideText.c_str(),
            static_cast<UINT32>(wideText.length()),
            m_pTextFormat.Get(),
            static_cast<FLOAT>(m_width),
            static_cast<FLOAT>(m_height),
            pTextLayout.GetAddressOf()
        );

        if (SUCCEEDED(hr) && pTextLayout) {
            // テキストサイズを取得
            DWRITE_TEXT_METRICS metrics;
            pTextLayout->GetMetrics(&metrics);

            // 最後のmes出力サイズを記録（ginfo 14/15 用）
            // HSP仕様: 複数行ある文字列を出力した場合は、最後の行にあたるサイズを取得
            if (metrics.lineCount <= 1) {
                // 単一行の場合は全体のサイズ
                m_lastMesSizeX = static_cast<int>(metrics.width);
                m_lastMesSizeY = static_cast<int>(metrics.height);
            }
            else {
                // 複数行の場合は最後の行のサイズを取得
                // 改行位置を探して最後の行のテキストを抽出
                size_t lastNewline = wideText.rfind(L'\n');
                std::wstring lastLine = (lastNewline != std::wstring::npos) 
                    ? wideText.substr(lastNewline + 1) 
                    : wideText;
                
                if (lastLine.empty()) {
                    // 最後の行が空の場合（末尾が改行）
                    m_lastMesSizeX = 0;
                    m_lastMesSizeY = static_cast<int>(metrics.height / metrics.lineCount);
                }
                else {
                    // 最後の行だけのTextLayoutを作成して計測
                    ComPtr<IDWriteTextLayout> pLastLineLayout;
                    HRESULT hrLast = pDWriteFactory->CreateTextLayout(
                        lastLine.c_str(),
                        static_cast<UINT32>(lastLine.length()),
                        m_pTextFormat.Get(),
                        static_cast<FLOAT>(m_width),
                        static_cast<FLOAT>(m_height),
                        pLastLineLayout.GetAddressOf()
                    );
                    if (SUCCEEDED(hrLast) && pLastLineLayout) {
                        DWRITE_TEXT_METRICS lastLineMetrics;
                        pLastLineLayout->GetMetrics(&lastLineMetrics);
                        m_lastMesSizeX = static_cast<int>(lastLineMetrics.width);
                        m_lastMesSizeY = static_cast<int>(lastLineMetrics.height);
                    }
                    else {
                        m_lastMesSizeX = static_cast<int>(metrics.width);
                        m_lastMesSizeY = static_cast<int>(metrics.height / metrics.lineCount);
                    }
                }
            }

            // 影を描画（オプション指定時）
            if (shadow) {
                D2D1_RECT_F shadowRect = D2D1::RectF(
                    layoutRect.left + 1.0f,
                    layoutRect.top + 1.0f,
                    layoutRect.right + 1.0f,
                    layoutRect.bottom + 1.0f
                );
                D2D1_COLOR_F shadowColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.5f);
                ComPtr<ID2D1SolidColorBrush> pShadowBrush;
                m_pDeviceContext->CreateSolidColorBrush(shadowColor, pShadowBrush.GetAddressOf());
                if (pShadowBrush) {
                    m_pDeviceContext->DrawTextLayout(
                        D2D1::Point2F(shadowRect.left, shadowRect.top),
                        pTextLayout.Get(),
                        pShadowBrush.Get()
                    );
                }
            }

            // 縁取りを描画（オプション指定時）
            if (outline) {
                // 簡易的な縁取り（4方向にテキストを描画）
                for (float dx = -1.0f; dx <= 1.0f; dx += 1.0f) {
                    for (float dy = -1.0f; dy <= 1.0f; dy += 1.0f) {
                        if (dx == 0.0f && dy == 0.0f) continue;
                        D2D1_RECT_F outlineRect = D2D1::RectF(
                            layoutRect.left + dx,
                            layoutRect.top + dy,
                            layoutRect.right + dx,
                            layoutRect.bottom + dy
                        );
                        D2D1_COLOR_F outlineColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
                        ComPtr<ID2D1SolidColorBrush> pOutlineBrush;
                        m_pDeviceContext->CreateSolidColorBrush(outlineColor, pOutlineBrush.GetAddressOf());
                        if (pOutlineBrush) {
                            m_pDeviceContext->DrawTextLayout(
                                D2D1::Point2F(outlineRect.left, outlineRect.top),
                                pTextLayout.Get(),
                                pOutlineBrush.Get()
                            );
                        }
                    }
                }
            }

            // 通常テキストを描画
            m_pDeviceContext->DrawTextLayout(
                D2D1::Point2F(layoutRect.left, layoutRect.top),
                pTextLayout.Get(),
                m_pBrush.Get()
            );

            // カレントポジションを更新
            if (nocr) {
                // mesopt_nocr: テキストの右側に移動
                m_currentX += static_cast<int>(metrics.width);
            }
            else {
                // デフォルト: 次の行に移動
                m_currentX = 0;
                m_currentY += static_cast<int>(metrics.height) + 2;  // フォント高さ + 余白
            }
        }
    }

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

bool HspSurface::measureText(std::string_view text, int& width, int& height) const {
    if (!m_pTextFormat) {
        width = 0;
        height = 0;
        return false;
    }

    std::wstring wideText = Utf8ToWide(text);

    IDWriteFactory* pDWriteFactory = D2DDeviceManager::getInstance().getDWriteFactory();
    if (!pDWriteFactory) {
        width = 0;
        height = 0;
        return false;
    }

    ComPtr<IDWriteTextLayout> pTextLayout;
    HRESULT hr = pDWriteFactory->CreateTextLayout(
        wideText.c_str(),
        static_cast<UINT32>(wideText.length()),
        m_pTextFormat.Get(),
        static_cast<FLOAT>(m_width),
        static_cast<FLOAT>(m_height),
        pTextLayout.GetAddressOf()
    );

    if (FAILED(hr) || !pTextLayout) {
        width = 0;
        height = 0;
        return false;
    }

    DWRITE_TEXT_METRICS metrics;
    pTextLayout->GetMetrics(&metrics);
    width = static_cast<int>(metrics.width);
    height = static_cast<int>(metrics.height);
    return true;
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
    if (!m_pDeviceContext || !m_pBrush) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    // 始点を決定
    float startX = useStartPos ? static_cast<float>(x1) : static_cast<float>(m_currentX);
    float startY = useStartPos ? static_cast<float>(y1) : static_cast<float>(m_currentY);
    float endX = static_cast<float>(x2);
    float endY = static_cast<float>(y2);

    // 直線を描画（太さは m_lineWidth / gline_width 命令で制御 / 論理 px）
    m_pDeviceContext->DrawLine(
        D2D1::Point2F(startX, startY),
        D2D1::Point2F(endX, endY),
        m_pBrush.Get(),
        m_lineWidth
    );

    // カレントポジションを終点に更新
    m_currentX = x2;
    m_currentY = y2;

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

void HspSurface::circle(int x1, int y1, int x2, int y2, int fillMode) {
    if (!m_pDeviceContext || !m_pBrush) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

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
        // 輪郭のみ（太さは m_lineWidth / gline_width 命令で制御 / 論理 px）
        m_pDeviceContext->DrawEllipse(ellipse, m_pBrush.Get(), m_lineWidth);
    }

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

void HspSurface::pset(int x, int y) {
    if (!m_pDeviceContext || !m_pBrush) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    // m_lineWidth サイズの点を描画（m_lineWidth × m_lineWidth の矩形 / 左上 (x,y) アンカー）
    // デフォルト m_lineWidth = 1.0f のときは従来挙動 (1x1 矩形) と一致する。
    D2D1_RECT_F rect = D2D1::RectF(
        static_cast<FLOAT>(x),
        static_cast<FLOAT>(y),
        static_cast<FLOAT>(x) + m_lineWidth,
        static_cast<FLOAT>(y) + m_lineWidth
    );

    m_pDeviceContext->FillRectangle(rect, m_pBrush.Get());

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

bool HspSurface::pget(int x, int y, int& r, int& g, int& b) {
    if (!m_pDeviceContext || !m_pTargetBitmap) return false;

    // RAIIヘルパーで描画状態を管理
    struct DrawingStateGuard {
        HspSurface* self;
        bool wasDrawing;
        DrawingStateGuard(HspSurface* s) : self(s), wasDrawing(s->m_isDrawing) {
            if (wasDrawing) {
                self->m_pDeviceContext->EndDraw();
                self->m_isDrawing = false;
            }
        }
        ~DrawingStateGuard() {
            if (wasDrawing) {
                self->m_pDeviceContext->BeginDraw();
                self->m_isDrawing = true;
            }
        }
    };
    DrawingStateGuard guard(this);

    // CPU読み取り可能なビットマップを作成（1x1サイズで最適化）
    ComPtr<ID2D1Bitmap1> pReadBitmap;
    D2D1_BITMAP_PROPERTIES1 readProps = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    HRESULT hr = m_pDeviceContext->CreateBitmap(
        D2D1::SizeU(1, 1),  // パフォーマンス改善: サイズを1x1に
        nullptr, 0,
        readProps,
        pReadBitmap.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    // 指定座標のピクセルをコピー
    D2D1_POINT_2U destPoint = D2D1::Point2U(0, 0);
    D2D1_RECT_U srcRect = D2D1::RectU(x, y, x + 1, y + 1);
    hr = pReadBitmap->CopyFromBitmap(&destPoint, m_pTargetBitmap.Get(), &srcRect);
    if (FAILED(hr)) return false;

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

    return SUCCEEDED(hr);
}

void HspSurface::gradf(int x, int y, int w, int h, int mode, int color1, int color2) {
    if (!m_pDeviceContext) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    // RGBカラーコードを分解
    float r1 = ((color1 >> 16) & 0xFF) / 255.0f;
    float g1 = ((color1 >> 8) & 0xFF) / 255.0f;
    float b1 = (color1 & 0xFF) / 255.0f;
    float r2 = ((color2 >> 16) & 0xFF) / 255.0f;
    float g2 = ((color2 >> 8) & 0xFF) / 255.0f;
    float b2 = (color2 & 0xFF) / 255.0f;

    D2D1_COLOR_F colorStart = D2D1::ColorF(r1, g1, b1, 1.0f);
    D2D1_COLOR_F colorEnd = D2D1::ColorF(r2, g2, b2, 1.0f);

    // グラデーションブラシを作成
    D2D1_GRADIENT_STOP gradientStops[2];
    gradientStops[0].color = colorStart;
    gradientStops[0].position = 0.0f;
    gradientStops[1].color = colorEnd;
    gradientStops[1].position = 1.0f;

    ComPtr<ID2D1GradientStopCollection> pGradientStops;
    HRESULT hr = m_pDeviceContext->CreateGradientStopCollection(
        gradientStops,
        2,
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        pGradientStops.GetAddressOf()
    );
    if (FAILED(hr)) {
        if (autoManage) endDrawAndPresent();
        return;
    }

    D2D1_POINT_2F startPoint, endPoint;
    if (mode == 0) {
        // 横方向のグラデーション（左から右）
        startPoint = D2D1::Point2F(static_cast<float>(x), static_cast<float>(y));
        endPoint = D2D1::Point2F(static_cast<float>(x + w), static_cast<float>(y));
    } else {
        // 縦方向のグラデーション（上から下）
        startPoint = D2D1::Point2F(static_cast<float>(x), static_cast<float>(y));
        endPoint = D2D1::Point2F(static_cast<float>(x), static_cast<float>(y + h));
    }

    ComPtr<ID2D1LinearGradientBrush> pGradientBrush;
    hr = m_pDeviceContext->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(startPoint, endPoint),
        pGradientStops.Get(),
        pGradientBrush.GetAddressOf()
    );
    if (FAILED(hr)) {
        if (autoManage) endDrawAndPresent();
        return;
    }

    // 矩形を描画
    D2D1_RECT_F rect = D2D1::RectF(
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(x + w),
        static_cast<float>(y + h)
    );
    m_pDeviceContext->FillRectangle(rect, pGradientBrush.Get());

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

void HspSurface::grect(int cx, int cy, double angle, int w, int h) {
    if (!m_pDeviceContext || !m_pBrush) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    // 回転変換を適用
    float centerX = static_cast<float>(cx);
    float centerY = static_cast<float>(cy);
    float halfW = static_cast<float>(w) / 2.0f;
    float halfH = static_cast<float>(h) / 2.0f;

    // 現在の変換を保存
    D2D1_MATRIX_3X2_F oldTransform;
    m_pDeviceContext->GetTransform(&oldTransform);

    // 回転変換を設定（中心を指定）
    float angleInDegrees = static_cast<float>(angle * kRadToDeg);
    D2D1_MATRIX_3X2_F rotationMatrix = D2D1::Matrix3x2F::Rotation(
        angleInDegrees,
        D2D1::Point2F(centerX, centerY)
    );
    m_pDeviceContext->SetTransform(rotationMatrix * oldTransform);

    // 矩形を描画（中心を基準に）
    D2D1_RECT_F rect = D2D1::RectF(
        centerX - halfW,
        centerY - halfH,
        centerX + halfW,
        centerY + halfH
    );
    m_pDeviceContext->FillRectangle(rect, m_pBrush.Get());

    // 変換を元に戻す
    m_pDeviceContext->SetTransform(oldTransform);

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

void HspSurface::grotate(ID2D1Bitmap1* pSrcBitmap, int srcX, int srcY, int srcW, int srcH, double angle, int dstW, int dstH) {
    if (!m_pDeviceContext || !pSrcBitmap) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    // コピー先は現在のpos位置を中心とする
    float centerX = static_cast<float>(m_currentX);
    float centerY = static_cast<float>(m_currentY);
    float halfW = static_cast<float>(dstW) / 2.0f;
    float halfH = static_cast<float>(dstH) / 2.0f;

    // 現在の変換を保存
    D2D1_MATRIX_3X2_F oldTransform;
    m_pDeviceContext->GetTransform(&oldTransform);

    // 回転変換を設定（中心を指定）
    float angleInDegrees = static_cast<float>(angle * kRadToDeg);
    D2D1_MATRIX_3X2_F rotationMatrix = D2D1::Matrix3x2F::Rotation(
        angleInDegrees,
        D2D1::Point2F(centerX, centerY)
    );
    m_pDeviceContext->SetTransform(rotationMatrix * oldTransform);

    // コピー元とコピー先の矩形
    D2D1_RECT_F srcRect = D2D1::RectF(
        static_cast<float>(srcX),
        static_cast<float>(srcY),
        static_cast<float>(srcX + srcW),
        static_cast<float>(srcY + srcH)
    );
    D2D1_RECT_F dstRect = D2D1::RectF(
        centerX - halfW,
        centerY - halfH,
        centerX + halfW,
        centerY + halfH
    );

    // 画像を描画
    m_pDeviceContext->DrawBitmap(
        pSrcBitmap,
        dstRect,
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        srcRect
    );

    // 変換を元に戻す
    m_pDeviceContext->SetTransform(oldTransform);

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

void HspSurface::gsquare(const int (&dstX)[4], const int (&dstY)[4], ID2D1Bitmap1* pSrcBitmap, const int* srcX, const int* srcY) {
    if (!m_pDeviceContext) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    // 塗りつぶしモード（pSrcBitmap == nullptr）
    if (!pSrcBitmap) {
        // パスジオメトリを作成
        auto pFactory = D2DDeviceManager::getInstance().getFactory();
        if (!pFactory) {
            if (autoManage) endDrawAndPresent();
            return;
        }

        ComPtr<ID2D1PathGeometry> pPath;
        HRESULT hr = pFactory->CreatePathGeometry(pPath.GetAddressOf());
        if (FAILED(hr)) {
            if (autoManage) endDrawAndPresent();
            return;
        }

        ComPtr<ID2D1GeometrySink> pSink;
        hr = pPath->Open(pSink.GetAddressOf());
        if (FAILED(hr)) {
            if (autoManage) endDrawAndPresent();
            return;
        }

        // 四角形のパスを構築（左上から時計回り）
        pSink->BeginFigure(
            D2D1::Point2F(static_cast<float>(dstX[0]), static_cast<float>(dstY[0])),
            D2D1_FIGURE_BEGIN_FILLED
        );
        pSink->AddLine(D2D1::Point2F(static_cast<float>(dstX[1]), static_cast<float>(dstY[1])));
        pSink->AddLine(D2D1::Point2F(static_cast<float>(dstX[2]), static_cast<float>(dstY[2])));
        pSink->AddLine(D2D1::Point2F(static_cast<float>(dstX[3]), static_cast<float>(dstY[3])));
        pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        pSink->Close();

        m_pDeviceContext->FillGeometry(pPath.Get(), m_pBrush.Get());
    } else {
        // 画像コピーモード（アフィン変換で近似）
        // Direct2Dでは任意の四角形へのマッピングは直接サポートされないため、
        // ソース矩形からデスティネーション矩形へのアフィン変換で近似
        if (!srcX || !srcY) {
            if (autoManage) endDrawAndPresent();
            return;
        }

        // ソースのバウンディングボックスを計算
        float srcMinX = static_cast<float>((std::min)({srcX[0], srcX[1], srcX[2], srcX[3]}));
        float srcMinY = static_cast<float>((std::min)({srcY[0], srcY[1], srcY[2], srcY[3]}));
        float srcMaxX = static_cast<float>((std::max)({srcX[0], srcX[1], srcX[2], srcX[3]}));
        float srcMaxY = static_cast<float>((std::max)({srcY[0], srcY[1], srcY[2], srcY[3]}));
        float srcW = srcMaxX - srcMinX;
        float srcH = srcMaxY - srcMinY;

        // デスティネーションのバウンディングボックスを計算
        float dstMinX = static_cast<float>((std::min)({dstX[0], dstX[1], dstX[2], dstX[3]}));
        float dstMinY = static_cast<float>((std::min)({dstY[0], dstY[1], dstY[2], dstY[3]}));
        float dstMaxX = static_cast<float>((std::max)({dstX[0], dstX[1], dstX[2], dstX[3]}));
        float dstMaxY = static_cast<float>((std::max)({dstY[0], dstY[1], dstY[2], dstY[3]}));
        float dstW = dstMaxX - dstMinX;
        float dstH = dstMaxY - dstMinY;

        if (srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0) {
            if (autoManage) endDrawAndPresent();
            return;
        }

        // 現在の変換を保存
        D2D1_MATRIX_3X2_F oldTransform;
        m_pDeviceContext->GetTransform(&oldTransform);

        // ソースの中心とデスティネーションの中心を計算
        float srcCenterX = (srcMinX + srcMaxX) / 2.0f;
        float srcCenterY = (srcMinY + srcMaxY) / 2.0f;
        float dstCenterX = (dstMinX + dstMaxX) / 2.0f;
        float dstCenterY = (dstMinY + dstMaxY) / 2.0f;

        // スケール変換を計算
        float scaleX = dstW / srcW;
        float scaleY = dstH / srcH;

        // 変換行列を構築（スケールと平行移動）
        // ソース中心を原点に移動 → スケール → デスティネーション中心に移動
        D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Translation(-srcCenterX, -srcCenterY)
            * D2D1::Matrix3x2F::Scale(scaleX, scaleY)
            * D2D1::Matrix3x2F::Translation(dstCenterX, dstCenterY);

        m_pDeviceContext->SetTransform(transform * oldTransform);

        // ソース矩形
        D2D1_RECT_F srcRect = D2D1::RectF(srcMinX, srcMinY, srcMaxX, srcMaxY);
        // デスティネーション矩形（変換前の座標系なのでソースと同じ）
        D2D1_RECT_F destRect = srcRect;

        m_pDeviceContext->DrawBitmap(
            pSrcBitmap,
            destRect,
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            srcRect
        );

        // 変換を元に戻す
        m_pDeviceContext->SetTransform(oldTransform);
    }
    
    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

// ═══════════════════════════════════════════════════════════════════
// gsquareGrad - 4頂点バイリニア補間グラデーション
// 
// 【パフォーマンス警告】
// この関数はHSP互換のバイリニア補間を実現するため、ピクセル単位で
// CPU計算を行います。大きな四角形では処理が重くなる可能性があります。
// 
// GPU代替案を検討しましたが、以下の理由で採用していません：
// - ID2D1GradientMesh: Coonsパッチ補間であり、HSPのバイリニア補間と
//   見た目が異なる
// - 三角形分割: 対角線上で色の不連続が生じる
// 
// 将来的な最適化案：
// - SIMD (AVX2) による並列ピクセル処理
// - カスタムピクセルシェーダー（Direct3D連携）
// - サイズに応じた細分割へのフォールバック
// ═══════════════════════════════════════════════════════════════════
void HspSurface::gsquareGrad(const int (&dstX)[4], const int (&dstY)[4], const int (&colors)[4]) {
    if (!m_pDeviceContext) return;

    // モード1の場合、自動的にbeginDraw
    bool autoManage = (m_redrawMode == 1 && !m_isDrawing);
    if (autoManage) {
        beginDraw();
    }
    if (!m_isDrawing) return;

    // HSP頂点順序: 0=左上, 1=右上, 2=右下, 3=左下
    // バウンディングボックスを計算
    int minX = (std::min)({dstX[0], dstX[1], dstX[2], dstX[3]});
    int maxX = (std::max)({dstX[0], dstX[1], dstX[2], dstX[3]});
    int minY = (std::min)({dstY[0], dstY[1], dstY[2], dstY[3]});
    int maxY = (std::max)({dstY[0], dstY[1], dstY[2], dstY[3]});
    
    int bmpW = maxX - minX + 1;
    int bmpH = maxY - minY + 1;
    if (bmpW <= 0 || bmpH <= 0) {
        if (autoManage) endDrawAndPresent();
        return;
    }

    // 4頂点の色をfloatに変換
    float r[4], g[4], b[4];
    for (int i = 0; i < 4; i++) {
        r[i] = static_cast<float>((colors[i] >> 16) & 0xFF);
        g[i] = static_cast<float>((colors[i] >> 8) & 0xFF);
        b[i] = static_cast<float>(colors[i] & 0xFF);
    }

    // ローカル座標に変換（バウンディングボックス基準）
    float vx[4], vy[4];
    for (int i = 0; i < 4; i++) {
        vx[i] = static_cast<float>(dstX[i] - minX);
        vy[i] = static_cast<float>(dstY[i] - minY);
    }

    // ピクセルが四角形内にあるかチェック（外積法）
    auto isInsideQuad = [&](float px, float py) -> bool {
        // 4辺すべてについて、点が同じ側にあるかチェック
        auto cross = [](float ax, float ay, float bx, float by, float px, float py) {
            return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
        };
        float c0 = cross(vx[0], vy[0], vx[1], vy[1], px, py);
        float c1 = cross(vx[1], vy[1], vx[2], vy[2], px, py);
        float c2 = cross(vx[2], vy[2], vx[3], vy[3], px, py);
        float c3 = cross(vx[3], vy[3], vx[0], vy[0], px, py);
        // すべて同符号（または0）なら内部
        return (c0 >= 0 && c1 >= 0 && c2 >= 0 && c3 >= 0) ||
               (c0 <= 0 && c1 <= 0 && c2 <= 0 && c3 <= 0);
    };

    // ピクセル座標から(u,v)パラメータを逆算（ニュートン法）
    auto pixelToUV = [&](float px, float py, float& u, float& v) -> bool {
        // 初期値
        u = 0.5f; v = 0.5f;
        
        for (int iter = 0; iter < 10; iter++) {
            // バイリニア補間: P(u,v) = (1-u)(1-v)*P0 + u(1-v)*P1 + uv*P2 + (1-u)v*P3
            float x = (1-u)*(1-v)*vx[0] + u*(1-v)*vx[1] + u*v*vx[2] + (1-u)*v*vx[3];
            float y = (1-u)*(1-v)*vy[0] + u*(1-v)*vy[1] + u*v*vy[2] + (1-u)*v*vy[3];
            
            float dx = px - x;
            float dy = py - y;
            
            if (std::abs(dx) < 0.01f && std::abs(dy) < 0.01f) return true;
            
            // ヤコビアン
            float dxdu = -(1-v)*vx[0] + (1-v)*vx[1] + v*vx[2] - v*vx[3];
            float dxdv = -(1-u)*vx[0] - u*vx[1] + u*vx[2] + (1-u)*vx[3];
            float dydu = -(1-v)*vy[0] + (1-v)*vy[1] + v*vy[2] - v*vy[3];
            float dydv = -(1-u)*vy[0] - u*vy[1] + u*vy[2] + (1-u)*vy[3];
            
            float det = dxdu * dydv - dxdv * dydu;
            if (std::abs(det) < 1e-6f) break;
            
            float du = (dydv * dx - dxdv * dy) / det;
            float dv = (-dydu * dx + dxdu * dy) / det;
            
            u += du;
            v += dv;
            u = std::clamp(u, 0.0f, 1.0f);
            v = std::clamp(v, 0.0f, 1.0f);
        }
        return (u >= 0 && u <= 1 && v >= 0 && v <= 1);
    };

    // ビットマップデータを作成
    std::vector<uint32_t> pixels(bmpW * bmpH, 0);
    
    for (int py = 0; py < bmpH; py++) {
        for (int px = 0; px < bmpW; px++) {
            float fpx = static_cast<float>(px) + 0.5f;
            float fpy = static_cast<float>(py) + 0.5f;
            
            if (!isInsideQuad(fpx, fpy)) continue;
            
            float u, v;
            if (!pixelToUV(fpx, fpy, u, v)) continue;
            
            // バイリニア補間で色を計算
            float cr = (1-u)*(1-v)*r[0] + u*(1-v)*r[1] + u*v*r[2] + (1-u)*v*r[3];
            float cg = (1-u)*(1-v)*g[0] + u*(1-v)*g[1] + u*v*g[2] + (1-u)*v*g[3];
            float cb = (1-u)*(1-v)*b[0] + u*(1-v)*b[1] + u*v*b[2] + (1-u)*v*b[3];
            
            uint8_t rb = static_cast<uint8_t>(std::clamp(cr, 0.0f, 255.0f));
            uint8_t gb = static_cast<uint8_t>(std::clamp(cg, 0.0f, 255.0f));
            uint8_t bb = static_cast<uint8_t>(std::clamp(cb, 0.0f, 255.0f));
            
            // BGRA形式
            pixels[py * bmpW + px] = 0xFF000000 | (rb << 16) | (gb << 8) | bb;
        }
    }

    // D2Dビットマップを作成
    D2D1_BITMAP_PROPERTIES bmpProps = D2D1::BitmapProperties(
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    
    ComPtr<ID2D1Bitmap> pBitmap;
    HRESULT hr = m_pDeviceContext->CreateBitmap(
        D2D1::SizeU(bmpW, bmpH),
        pixels.data(),
        bmpW * sizeof(uint32_t),
        bmpProps,
        pBitmap.GetAddressOf()
    );
    
    if (SUCCEEDED(hr)) {
        m_pDeviceContext->DrawBitmap(
            pBitmap.Get(),
            D2D1::RectF(static_cast<float>(minX), static_cast<float>(minY),
                       static_cast<float>(maxX + 1), static_cast<float>(maxY + 1))
        );
    }

    // モード1の場合、自動的にendDraw + present
    if (autoManage) {
        endDrawAndPresent();
    }
}

// ========== LogicalRenderContext 実装 ==========

void LogicalRenderContext::update(int logW, int logH,
                                  int physClientW, int physClientH,
                                  UINT currentDpi,
                                  bool virtualEnabled) {
    m_logW    = (logW  > 0) ? logW  : 1;
    m_logH    = (logH  > 0) ? logH  : 1;
    m_physW   = (physClientW > 0) ? physClientW : 1;
    m_physH   = (physClientH > 0) ? physClientH : 1;
    m_dpi     = (currentDpi != 0) ? currentDpi : 96;
    m_virtual = virtualEnabled;
}

LogicalRenderContext::PresentMapping LogicalRenderContext::computePresentMapping() const {
    // v2: 仮想 ON / OFF を統一経路で算出する（旧二分岐を削除）。
    //   - scale は uniform = min(physW/logW, physH/logH)。
    //     * 仮想 OFF: 物理クライアントは「論理 × (DPI/96)」に追従するため sx == sy が原則成立。
    //       微小ズレは小さい方を採用してオフスクリーンが物理クライアントをはみ出さないようにする。
    //     * 仮想 ON : letterbox 中央寄せのための uniform スケール。
    //   - destW/H は logW/H * scale。
    //   - offsetX/Y のみが仮想 ON （中央寄せ）と仮想 OFF （原点寄せ）で異なる。
    PresentMapping m{};
    const float logW  = static_cast<float>((std::max)(1, m_logW));
    const float logH  = static_cast<float>((std::max)(1, m_logH));
    const float physW = static_cast<float>((std::max)(1, m_physW));
    const float physH = static_cast<float>((std::max)(1, m_physH));

    const float sx = physW / logW;
    const float sy = physH / logH;
    m.scale = (sx < sy) ? sx : sy;
    if (m.scale <= 0.0f) m.scale = 1.0f;
    m.destW = logW * m.scale;
    m.destH = logH * m.scale;

    if (m_virtual) {
        m.offsetX = (physW - m.destW) * 0.5f;
        m.offsetY = (physH - m.destH) * 0.5f;
    } else {
        m.offsetX = 0.0f;
        m.offsetY = 0.0f;
    }
    return m;
}

float LogicalRenderContext::getDpiScale() const {
    return static_cast<float>(m_dpi) / 96.0f;
}

void LogicalRenderContext::physToLogical(int physX, int physY, int& outLogX, int& outLogY) const {
    auto m = computePresentMapping();
    if (m.scale <= 0.0f) {
        outLogX = physX;
        outLogY = physY;
        return;
    }
    float fx = (static_cast<float>(physX) - m.offsetX) / m.scale;
    float fy = (static_cast<float>(physY) - m.offsetY) / m.scale;
    outLogX = static_cast<int>(std::floor(fx));
    outLogY = static_cast<int>(std::floor(fy));
}

void LogicalRenderContext::logicalToPhys(int logX, int logY, int& outPhysX, int& outPhysY) const {
    auto m = computePresentMapping();
    float px = m.offsetX + static_cast<float>(logX) * m.scale;
    float py = m.offsetY + static_cast<float>(logY) * m.scale;
    outPhysX = static_cast<int>(std::floor(px));
    outPhysY = static_cast<int>(std::floor(py));
}

// ========== HspWindow 実装 ==========

HspWindow::HspWindow(int width, int height, std::string_view title, int windowId)
    : HspSurface(width, height)
    , m_pSwapChain(nullptr)
    , m_title(Utf8ToWide(title))
    , m_clientWidth(width)      // 初期値はバッファサイズと同じ（initialize() で物理 px に再確定）
    , m_clientHeight(height)
    , m_currentDpi(96)          // HiDPI: createWindow / initialize 後に実 DPI で更新
    , m_physClientW(width)
    , m_physClientH(height)
    , m_virtualEnabled(false)
    , m_virtualScreenInterp(D2D1_BITMAP_INTERPOLATION_MODE_LINEAR)
    , m_letterboxColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f))
    , m_hwnd(nullptr)
    , m_scrollX(0)
    , m_scrollY(0)
    , m_windowId(windowId)
{
}

HspWindow::~HspWindow() {
    // UniqueHwnd handles destruction
}

bool HspWindow::initialize() {
    auto& deviceMgr = D2DDeviceManager::getInstance();
    if (!deviceMgr.isInitialized()) {
        return false;
    }

    HRESULT hr;

    // HiDPI: ウィンドウ作成後に実際の DPI と物理クライアントサイズを取得し、
    // SwapChain は物理 px ベースで初期化する（PerMonitorV2 で「ぼけない」描画の前提）
    if (m_hwnd) {
        using FnGetDpi = UINT(WINAPI*)(HWND);
        HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
        auto pGetDpi = hUser32
            ? reinterpret_cast<FnGetDpi>(GetProcAddress(hUser32, "GetDpiForWindow"))
            : nullptr;
        if (pGetDpi) {
            UINT dpi = pGetDpi(m_hwnd);
            if (dpi != 0) m_currentDpi = dpi;
        }

        RECT rcClient = {};
        if (GetClientRect(m_hwnd, &rcClient)) {
            int physW = rcClient.right - rcClient.left;
            int physH = rcClient.bottom - rcClient.top;
            if (physW > 0 && physH > 0) {
                m_physClientW = physW;
                m_physClientH = physH;
                m_clientWidth = physW;
                m_clientHeight = physH;
            }
        }
    }

    // 物理クライアントサイズが確定したので LogicalRenderContext を同期する。
    // 以降の SwapChain / m_pTargetBitmap 生成サイズ計算は m_logicalCtx に基づく。
    updateLogicalCtx();

    // スワップチェーンの作成
    ComPtr<IDXGIAdapter> pAdapter;
    hr = deviceMgr.getDxgiDevice()->GetAdapter(pAdapter.GetAddressOf());
    if (FAILED(hr)) return false;

    ComPtr<IDXGIFactory2> pFactory;
    hr = pAdapter->GetParent(IID_PPV_ARGS(pFactory.GetAddressOf()));
    if (FAILED(hr)) return false;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    // HiDPI: SwapChain は「物理クライアント px」基準で作成する。
    // 仮想画面 OFF 時は m_physClientW/H と m_width/m_height は通常一致するが、
    // OS 側の DPI 仮想化（Unaware fallback）等でズレ得るため物理 px を優先する。
    swapChainDesc.Width = static_cast<UINT>(m_physClientW);
    swapChainDesc.Height = static_cast<UINT>(m_physClientH);
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

    // オフスクリーンビットマップを作成
    // v2: 仮想 ON/OFF を問わず、letterbox 内有効描画領域 (mp.destW × mp.destH) 物理 px で生成する。
    auto mpInit = m_logicalCtx.computePresentMapping();
    UINT bw = static_cast<UINT>((std::max)(1.0f, std::round(mpInit.destW)));
    UINT bh = static_cast<UINT>((std::max)(1.0f, std::round(mpInit.destH)));
    D2D1_SIZE_U bitmapSize = D2D1::SizeU(bw, bh);
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

    // デフォルトはオフスクリーンビットマップに描画。SetDpi=96 + SetTransform(Scale(s)) を設定。
    m_pDeviceContext->SetTarget(m_pTargetBitmap.Get());
    m_pDeviceContext->SetDpi(96.0f, 96.0f);
    m_pDeviceContext->SetTransform(D2D1::Matrix3x2F::Scale(mpInit.scale, mpInit.scale));

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

void HspWindow::beginDraw() {
    if (!m_pDeviceContext || m_isDrawing) return;

    // v2: m_pTargetBitmap は常に (mp.destW × mp.destH) 物理 px とする。
    // 仮想 OFF + DPI≠96 でも DPI 倍率を吸収した物理サイズで保持されるため、
    // 描画は SetDpi=96 + SetTransform(Scale(s)) のみで物理解像度に展開される。
    auto mp = m_logicalCtx.computePresentMapping();

    auto ensureTargetBitmap = [&](UINT width, UINT height) -> bool {
        if (width < 1) width = 1;
        if (height < 1) height = 1;
        if (m_pTargetBitmap) {
            D2D1_SIZE_F s = m_pTargetBitmap->GetSize();
            UINT cw = static_cast<UINT>(std::round(s.width));
            UINT ch = static_cast<UINT>(std::round(s.height));
            if (cw == width && ch == height) return true;
        }

        D2D1_BITMAP_PROPERTIES1 offscreenProps = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
        );
        ComPtr<ID2D1Bitmap1> newBitmap;
        HRESULT hr = m_pDeviceContext->CreateBitmap(
            D2D1::SizeU(width, height), nullptr, 0, offscreenProps, newBitmap.GetAddressOf()
        );
        if (FAILED(hr) || !newBitmap) return false;
        // 旧 Bitmap が SetTarget 中なら一旦切り離してから差し替え
        m_pDeviceContext->SetTarget(nullptr);
        m_pTargetBitmap = newBitmap;
        m_pDeviceContext->SetTarget(m_pTargetBitmap.Get());
        return true;
    };

    UINT vw = static_cast<UINT>((std::max)(1.0f, std::round(mp.destW)));
    UINT vh = static_cast<UINT>((std::max)(1.0f, std::round(mp.destH)));
    if (!ensureTargetBitmap(vw, vh)) return;

    m_pDeviceContext->BeginDraw();
    m_isDrawing = true;

    // SetDpi=96 + SetTransform(Scale(s)) + groll 平行移動
    // 二重適用問題（§6.4.2）回避のため SetDpi は常に 96 固定とする。
    m_pDeviceContext->SetDpi(96.0f, 96.0f);
    D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Scale(mp.scale, mp.scale);
    transform._31 = -static_cast<float>(m_scrollX) * mp.scale;
    transform._32 = -static_cast<float>(m_scrollY) * mp.scale;
    m_pDeviceContext->SetTransform(transform);
}

void HspWindow::endDraw() {
    if (!m_pDeviceContext || !m_isDrawing) return;
    m_pDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
    HspSurface::endDraw();
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
    // v2: ユーザー指定の clientWidth/Height は「論理 px」(HSP3 公式準拠 / design §6.5)。
    // PerMonitorV2 では物理クライアントへの DPI 倍率変換を経由してウィンドウを作る。
    // 仮想画面 ON 時は initialize() 直後に再レイアウトされる前提のため、ここでは
    // 同じく論理 → 物理 換算で初期サイズを与える（過小ウィンドウ防止）。
    UINT initDpi = 96;
    {
        using FnGetDpiSys = UINT(WINAPI*)();
        HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
        auto pGetDpiSys = hUser32
            ? reinterpret_cast<FnGetDpiSys>(GetProcAddress(hUser32, "GetDpiForSystem"))
            : nullptr;
        if (pGetDpiSys) {
            UINT d = pGetDpiSys();
            if (d != 0) initDpi = d;
        }
    }
    int physClientW = clientWidth;
    int physClientH = clientHeight;
    if (initDpi != 96) {
        physClientW = MulDiv(clientWidth, static_cast<int>(initDpi), 96);
        physClientH = MulDiv(clientHeight, static_cast<int>(initDpi), 96);
    }
    if (physClientW < 1) physClientW = 1;
    if (physClientH < 1) physClientH = 1;

    RECT rect = { 0, 0, physClientW, physClientH };
    {
        using FnAdjForDpi = BOOL(WINAPI*)(LPRECT, DWORD, BOOL, DWORD, UINT);
        HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
        auto pAdjForDpi = hUser32
            ? reinterpret_cast<FnAdjForDpi>(GetProcAddress(hUser32, "AdjustWindowRectExForDpi"))
            : nullptr;
        if (pAdjForDpi && initDpi != 0) {
            pAdjForDpi(&rect, style, FALSE, exStyle, initDpi);
        } else {
            AdjustWindowRectEx(&rect, style, FALSE, exStyle);
        }
    }
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    std::wstring classNameStr(className);

    int posX = (x < 0) ? CW_USEDEFAULT : x;
    int posY = (y < 0) ? CW_USEDEFAULT : y;

    m_hwnd.reset(CreateWindowExW(
        exStyle,
        classNameStr.c_str(),
        m_title.c_str(),
        style,
        posX, posY,
        windowWidth, windowHeight,
        nullptr, nullptr,
        hInstance,
        this
    ));

    return m_hwnd.get() != nullptr;
}

void HspWindow::present() {
    presentInternal(0, 0);  // VSync待機なし
}

void HspWindow::presentInternal(UINT syncInterval, UINT flags) {
    if (!m_pSwapChain || !m_pDeviceContext || !m_pTargetBitmap || !m_pBackBufferBitmap) {
        return;
    }

    // v2 (DN 第 3 項): present は SwapChain への単純転送 + 配置オフセットのみ。
    // 拡縮ロジック (Matrix3x2F::Scale や dst≠src の DrawBitmap) は置かない。
    // m_pTargetBitmap は (mp.destW × mp.destH) 物理 px で生成済みのため、
    // src と dst のピクセル寸法は完全一致し、dot-by-dot 転送となる。
    auto mp = m_logicalCtx.computePresentMapping();

    m_pDeviceContext->SetTarget(m_pBackBufferBitmap.Get());
    m_pDeviceContext->BeginDraw();
    // BackBuffer に対する SetTransform / SetDpi は present 時のみ Identity / 96 にリセット
    m_pDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pDeviceContext->SetDpi(96.0f, 96.0f);

    // letterbox 余白の塗り潰し。仮想 ON はユーザー指定色、仮想 OFF は黒。
    m_pDeviceContext->Clear(m_logicalCtx.isVirtualEnabled()
                                ? m_letterboxColor
                                : D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f));

    // groll は論理 px 指定。物理 px の m_pTargetBitmap から取り出すために mp.scale を乗じる。
    int srcRightL  = (std::min)(m_scrollX + m_width,  m_width);
    int srcBottomL = (std::min)(m_scrollY + m_height, m_height);

    D2D1_SIZE_F tgtSize = m_pTargetBitmap->GetSize();
    float srcLeft   = static_cast<float>(m_scrollX) * mp.scale;
    float srcTop    = static_cast<float>(m_scrollY) * mp.scale;
    float srcRight  = static_cast<float>(srcRightL)  * mp.scale;
    float srcBottom = static_cast<float>(srcBottomL) * mp.scale;
    if (srcRight  > tgtSize.width)  srcRight  = tgtSize.width;
    if (srcBottom > tgtSize.height) srcBottom = tgtSize.height;

    float srcW = (std::max)(0.0f, srcRight  - srcLeft);
    float srcH = (std::max)(0.0f, srcBottom - srcTop);

    // 配置オフセット（平行移動のみ / 拡縮なし）。src と dst のピクセル寸法は完全一致。
    D2D1_RECT_F srcRect = D2D1::RectF(srcLeft, srcTop, srcRight, srcBottom);
    D2D1_RECT_F dstRect = D2D1::RectF(
        mp.offsetX,
        mp.offsetY,
        mp.offsetX + srcW,
        mp.offsetY + srcH
    );

    // dot-by-dot のため NEAREST 相当でよいが、サブピクセル整合誤差吸収のため LINEAR を採用。
    // m_virtualScreenInterp はユーザー指定があれば尊重（縮退仕様: 視覚効果は微小）。
    m_pDeviceContext->DrawBitmap(
        m_pTargetBitmap.Get(),
        dstRect,
        1.0f,
        m_virtualScreenInterp,
        srcRect
    );

    m_pDeviceContext->EndDraw();

    // 画面に表示（syncInterval: 0=即座, 1=VSync同期）
    if (m_pSwapChain) {
        HRESULT hr = m_pSwapChain->Present(syncInterval, flags);
        // ウィンドウ破棄等によるエラーは静かに無視
        (void)hr;
    }

    // 描画先をオフスクリーンに戻し、次フレームの描画 API 用に SetDpi=96 + SetTransform(Scale(s)) を再設定。
    // v2 §6.4.2: SetDpi は 96 固定。論理→物理スケールは SetTransform でのみ表現（二重適用回避）。
    if (m_pDeviceContext && m_pTargetBitmap) {
        m_pDeviceContext->SetTarget(m_pTargetBitmap.Get());
        m_pDeviceContext->SetDpi(96.0f, 96.0f);
        D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Scale(mp.scale, mp.scale);
        transform._31 = -static_cast<float>(m_scrollX) * mp.scale;
        transform._32 = -static_cast<float>(m_scrollY) * mp.scale;
        m_pDeviceContext->SetTransform(transform);
    }
}

void HspWindow::presentVsync() {
    presentInternal(1, 0);  // VSync同期
}

void HspWindow::endDrawAndPresent() {
    endDraw();
    present();
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

    // v2: 仮想 OFF 時の clientW/H は「論理 px」(HSP3 公式 width 仕様準拠)。
    // 仮想 ON 時は従来通り「物理 px」として扱う (PM Q-2 / design §6.5)。
    int physW, physH;
    if (!m_virtualEnabled) {
        // 論理 px → 物理 px = 論理 × (DPI/96)。バッファサイズ (m_width/m_height) は論理 px。
        if (clientW < 1) clientW = 1;
        if (clientH < 1) clientH = 1;
        clientW = (std::min)(clientW, m_width);
        clientH = (std::min)(clientH, m_height);
        if (m_currentDpi != 0 && m_currentDpi != 96) {
            physW = MulDiv(clientW, m_currentDpi, 96);
            physH = MulDiv(clientH, m_currentDpi, 96);
        } else {
            physW = clientW;
            physH = clientH;
        }
    } else {
        // 仮想 ON: clientW/H は物理 px。クランプは行わない。
        physW = (std::max)(1, clientW);
        physH = (std::max)(1, clientH);
    }

    // 現在のウィンドウスタイルを取得
    DWORD style = static_cast<DWORD>(GetWindowLongPtr(m_hwnd, GWL_STYLE));
    DWORD exStyle = static_cast<DWORD>(GetWindowLongPtr(m_hwnd, GWL_EXSTYLE));

    // クライアントサイズ（物理 px）からウィンドウサイズを計算（DPI 対応版を優先）
    RECT rect = { 0, 0, physW, physH };
    {
        using FnAdjForDpi = BOOL(WINAPI*)(LPRECT, DWORD, BOOL, DWORD, UINT);
        HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
        auto pAdjForDpi = hUser32
            ? reinterpret_cast<FnAdjForDpi>(GetProcAddress(hUser32, "AdjustWindowRectExForDpi"))
            : nullptr;
        if (pAdjForDpi && m_currentDpi != 0) {
            pAdjForDpi(&rect, style, FALSE, exStyle, m_currentDpi);
        } else {
            AdjustWindowRectEx(&rect, style, FALSE, exStyle);
        }
    }
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    // ウィンドウサイズを変更（位置は維持）
    SetWindowPos(m_hwnd, nullptr, 0, 0, windowWidth, windowHeight,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    // スワップチェーンをリサイズ
    if (resizeSwapChain(physW, physH)) {
        m_clientWidth = physW;
        m_clientHeight = physH;
        m_physClientW = physW;
        m_physClientH = physH;
        updateLogicalCtx();
    }
}

void HspWindow::setScroll(int x, int y) {
    // スクロール位置を設定（groll命令用）
    // バッファ範囲内にクランプ
    m_scrollX = (std::max)(0, (std::min)(x, m_width - 1));
    m_scrollY = (std::max)(0, (std::min)(y, m_height - 1));
}

void HspWindow::onSize(int newWidth, int newHeight) {
    // ウィンドウサイズ変更時の処理（WM_SIZE の lParam は物理 px）
    if (newWidth < 1) newWidth = 1;
    if (newHeight < 1) newHeight = 1;

    // HSP仕様：仮想画面 OFF 時はクライアントサイズをバッファサイズ（m_width, m_height）以下にクランプ。
    // HiDPI 対応: バッファサイズは論理 px なので、現在の DPI に応じて物理 px に換算してクランプする。
    // 仮想画面 ON 時は論理→物理 自動拡縮するため、ユーザーリサイズによる任意の物理サイズを許容する。
    if (!m_virtualEnabled) {
        int maxPhysW = m_width;
        int maxPhysH = m_height;
        if (m_currentDpi != 96 && m_currentDpi != 0) {
            maxPhysW = MulDiv(m_width, m_currentDpi, 96);
            maxPhysH = MulDiv(m_height, m_currentDpi, 96);
        }
        newWidth = (std::min)(newWidth, maxPhysW);
        newHeight = (std::min)(newHeight, maxPhysH);
    }

    // スワップチェーンをリサイズ（描画中の場合は一旦終了）
    bool wasDrawing = m_isDrawing;
    if (wasDrawing) {
        endDraw();
    }

    if (resizeSwapChain(newWidth, newHeight)) {
        m_clientWidth = newWidth;
        m_clientHeight = newHeight;
        m_physClientW = newWidth;
        m_physClientH = newHeight;
        updateLogicalCtx();
    }

    // 描画を再開
    if (wasDrawing) {
        beginDraw();  // beginDraw 内で m_pTargetBitmap を新 destW×destH に再生成する
    }
}

void HspWindow::onDpiChanged(UINT newDpi, const RECT* suggested) {
    // PerMonitorV2 の WM_DPICHANGED ハンドリング
    // 1. OS 提案矩形に従ってウィンドウを移動・リサイズ
    // 2. 新しい物理クライアントサイズで SwapChain を再構築（m_pBackBufferBitmap 再生成）
    // 3. 論理バッファ m_pTargetBitmap を防御的に再生成（D2D デバイス紐付けの確実な再確立）
    //    既存内容は CopyFromBitmap で引き継ぐ（ベストエフォート、失敗時は空バッファで継続）
    if (!m_hwnd) return;
    if (newDpi == 0) newDpi = 96;

    m_currentDpi = newDpi;

    if (suggested) {
        SetWindowPos(
            m_hwnd,
            nullptr,
            suggested->left,
            suggested->top,
            suggested->right - suggested->left,
            suggested->bottom - suggested->top,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
    }

    // 新しい物理クライアントサイズを取得
    RECT rcClient = {};
    if (!GetClientRect(m_hwnd, &rcClient)) return;
    int newPhysW = rcClient.right - rcClient.left;
    int newPhysH = rcClient.bottom - rcClient.top;
    if (newPhysW < 1) newPhysW = 1;
    if (newPhysH < 1) newPhysH = 1;

    // 描画中なら一旦終了して SwapChain を作り直す
    bool wasDrawing = m_isDrawing;
    if (wasDrawing) {
        endDraw();
    }

    if (resizeSwapChain(newPhysW, newPhysH)) {
        m_clientWidth = newPhysW;
        m_clientHeight = newPhysH;
        m_physClientW = newPhysW;
        m_physClientH = newPhysH;
        updateLogicalCtx();

        // m_pTargetBitmap 防御的再生成
        // v2: 仮想 ON/OFF を問わず、新 mp.destW × mp.destH 物理 px で再生成する。
        // ResizeBuffers では D2D デバイス自体は失われないが、device-context との
        // 紐付け再確立を明示することで K2/R-B 潜在不具合（onDpiChanged 後の
        // SetTarget / DrawBitmap / pget / bmpsave での device-bind ロスト）を排除する。
        if (m_pDeviceContext && m_pTargetBitmap) {
            ComPtr<ID2D1Bitmap1> oldBitmap = m_pTargetBitmap;
            auto mpDpi = m_logicalCtx.computePresentMapping();
            D2D1_SIZE_U targetSize = D2D1::SizeU(
                static_cast<UINT>((std::max)(1.0f, std::round(mpDpi.destW))),
                static_cast<UINT>((std::max)(1.0f, std::round(mpDpi.destH)))
            );
            D2D1_BITMAP_PROPERTIES1 offscreenProps = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
            );

            // 旧 Bitmap が現在のターゲットの可能性があるため、一旦切り離す
            m_pDeviceContext->SetTarget(nullptr);
            m_pTargetBitmap.Reset();

            ComPtr<ID2D1Bitmap1> newBitmap;
            HRESULT hrBmp = m_pDeviceContext->CreateBitmap(
                targetSize, nullptr, 0, offscreenProps, newBitmap.GetAddressOf()
            );
            if (SUCCEEDED(hrBmp) && newBitmap) {
                D2D1_SIZE_F oldSizeF = oldBitmap->GetSize();
                UINT oldW = static_cast<UINT>(std::round(oldSizeF.width));
                UINT oldH = static_cast<UINT>(std::round(oldSizeF.height));
                if (oldW == targetSize.width && oldH == targetSize.height) {
                    // サイズが偶然一致した場合のみピクセル単位コピー（厳密保持）。
                    // サイズ変動時の D2D ダウン/アップサンプリングは現スコープでは
                    // 行わず、空バッファで再生成する（HSP 慣行: 描画ループで毎フレーム再描画）。
                    D2D1_POINT_2U dstPt = D2D1::Point2U(0, 0);
                    D2D1_RECT_U srcRect = D2D1::RectU(0, 0, targetSize.width, targetSize.height);
                    HRESULT hrCopy = newBitmap->CopyFromBitmap(&dstPt, oldBitmap.Get(), &srcRect);
                    (void)hrCopy;
                }
                m_pTargetBitmap = newBitmap;
            } else {
                // 再生成失敗時は旧 Bitmap を復元してデバイスコンテキストに再バインド
                m_pTargetBitmap = oldBitmap;
            }
            m_pDeviceContext->SetTarget(m_pTargetBitmap.Get());
        }
    }

    if (wasDrawing) {
        beginDraw();
    }
}

bool HspWindow::resizeSwapChain(int newWidth, int newHeight) {
    if (!m_pSwapChain || !m_pDeviceContext) return false;

    // バックバッファビットマップを解放
    m_pBackBufferBitmap.Reset();
    m_pDeviceContext->SetTarget(nullptr);

    // スワップチェーンをリサイズ
    HRESULT hr = m_pSwapChain->ResizeBuffers(
        0,  // バッファ数を維持
        static_cast<UINT>(newWidth),
        static_cast<UINT>(newHeight),
        DXGI_FORMAT_UNKNOWN,  // フォーマットを維持
        0
    );
    if (FAILED(hr)) return false;

    // 新しいバックバッファからビットマップを再作成
    ComPtr<IDXGISurface> pBackBuffer;
    hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
    if (FAILED(hr)) return false;

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

    // 描画ターゲットをオフスクリーンビットマップに戻す
    m_pDeviceContext->SetTarget(m_pTargetBitmap.Get());

    return true;
}

void HspWindow::setWindowPos(int x, int y) {
    if (!m_hwnd) return;
    SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, 
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void HspWindow::getCurrentClientSize(int& outWidth, int& outHeight) const {
    // メンバ変数から取得（実際のウィンドウサイズではなく、HSPの論理クライアントサイズ）
    outWidth = m_clientWidth;
    outHeight = m_clientHeight;
}

// ========== 仮想画面 (Virtual Screen) 関連実装 ==========

void HspWindow::updateLogicalCtx() {
    m_logicalCtx.update(
        m_width, m_height,
        m_physClientW, m_physClientH,
        m_currentDpi,
        m_virtualEnabled
    );
}

void HspWindow::setVirtualScreenEnabled(bool enabled) {
    m_virtualEnabled = enabled;
    updateLogicalCtx();
}

void HspWindow::setVirtualInterpolation(D2D1_BITMAP_INTERPOLATION_MODE mode) {
    m_virtualScreenInterp = mode;
}

void HspWindow::setLetterboxColor(int r, int g, int b) {
    auto clamp01 = [](int v) -> float {
        if (v < 0)   v = 0;
        if (v > 255) v = 255;
        return static_cast<float>(v) / 255.0f;
    };
    m_letterboxColor = D2D1::ColorF(clamp01(r), clamp01(g), clamp01(b), 1.0f);
}

void HspWindow::physToLogical(int physX, int physY, int& outLogX, int& outLogY) const {
    // v2: LogicalRenderContext に委譲。仮想 OFF + DPI≠96 も DPI 倍率を吸収する（R-A5 解消）。
    m_logicalCtx.physToLogical(physX, physY, outLogX, outLogY);
}

void HspWindow::logicalToPhys(int logX, int logY, int& outPhysX, int& outPhysY) const {
    // v2: LogicalRenderContext に委譲。
    m_logicalCtx.logicalToPhys(logX, logY, outPhysX, outPhysY);
}

// ========== HspWindow: pget / bmpsave 互換ラッパ (§6.10) ==========

bool HspWindow::pget(int x, int y, int& r, int& g, int& b) {
    // v2: 引数 (x, y) は論理 px。m_pTargetBitmap は物理 px (mp.destW × mp.destH)。
    // 中心 1 物理 px を代表値として取得する（PM Q-G 暫定 / 非整数倍率時は近似動作）。
    if (!m_pDeviceContext || !m_pTargetBitmap) return false;
    auto mp = m_logicalCtx.computePresentMapping();
    float fx = (static_cast<float>(x) + 0.5f) * mp.scale;
    float fy = (static_cast<float>(y) + 0.5f) * mp.scale;
    int px = static_cast<int>(std::floor(fx));
    int py = static_cast<int>(std::floor(fy));
    return HspSurface::pget(px, py, r, g, b);
}

bool HspWindow::bmpsave(std::string_view filename) {
    // v2: HSP 仕様「論理サイズの bmp として保存」。
    // 物理 px の m_pTargetBitmap を論理サイズ (m_width × m_height) へ
    // D2D DrawBitmap (LINEAR) でダウンサンプリングしてから WIC で BMP 出力する (§6.10)。
    if (!m_pDeviceContext || !m_pTargetBitmap) return false;

    bool wasDrawing = m_isDrawing;
    if (wasDrawing) {
        endDraw();
    }

    UINT logW = static_cast<UINT>((std::max)(1, m_width));
    UINT logH = static_cast<UINT>((std::max)(1, m_height));

    D2D1_BITMAP_PROPERTIES1 logicalProps = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
    );
    ComPtr<ID2D1Bitmap1> pLogicalBitmap;
    HRESULT hr = m_pDeviceContext->CreateBitmap(
        D2D1::SizeU(logW, logH), nullptr, 0, logicalProps, pLogicalBitmap.GetAddressOf()
    );
    bool result = false;
    if (SUCCEEDED(hr) && pLogicalBitmap) {
        m_pDeviceContext->SetTarget(pLogicalBitmap.Get());
        m_pDeviceContext->SetDpi(96.0f, 96.0f);
        m_pDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
        m_pDeviceContext->BeginDraw();
        m_pDeviceContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f));

        D2D1_SIZE_F srcSize = m_pTargetBitmap->GetSize();
        D2D1_RECT_F srcRect = D2D1::RectF(0.0f, 0.0f, srcSize.width, srcSize.height);
        D2D1_RECT_F dstRect = D2D1::RectF(0.0f, 0.0f, static_cast<float>(logW), static_cast<float>(logH));
        m_pDeviceContext->DrawBitmap(
            m_pTargetBitmap.Get(),
            dstRect, 1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            srcRect
        );
        HRESULT hrEnd = m_pDeviceContext->EndDraw();
        (void)hrEnd;
        // 描画ターゲットを物理 px のオフスクリーンに戻す
        m_pDeviceContext->SetTarget(m_pTargetBitmap.Get());

        result = saveBitmapToFile(pLogicalBitmap.Get(), filename);
    }

    if (wasDrawing) {
        beginDraw();
    }
    return result;
}

// ========== HspSurface フォント関連実装 ==========

bool HspSurface::font(std::string_view fontName, int size, int style) {
    // v2 統一テキスト経路（HiDPI 設計大改修以降）:
    //   引数 `size` は論理 DIP (= 論理 px) として扱う。SetDpi=96 固定 + SetTransform(Scale(s))
    //   により最終描画は size*s 物理 px となり、HiDPI 環境でも自然なスケールでフォントが描画される。
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

HFONT HspSurface::createObjectFont() const {
    if (!m_pTextFormat) return nullptr;

    const UINT32 familyLength = m_pTextFormat->GetFontFamilyNameLength();
    std::wstring family(static_cast<size_t>(familyLength) + 1, L'\0');
    if (FAILED(m_pTextFormat->GetFontFamilyName(family.data(), familyLength + 1))) {
        return nullptr;
    }
    family.resize(familyLength);

    const int height = -(std::max)(1, static_cast<int>(std::round(m_pTextFormat->GetFontSize())));
    const int weight = m_pTextFormat->GetFontWeight() >= DWRITE_FONT_WEIGHT_BOLD ? FW_BOLD : FW_NORMAL;
    const BOOL italic = m_pTextFormat->GetFontStyle() != DWRITE_FONT_STYLE_NORMAL;
    return CreateFontW(
        height, 0, 0, 0, weight, italic, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, family.c_str());
}

bool HspSurface::sysfont(int type) {
    // v2 統一テキスト経路（HiDPI 設計大改修以降）:
    //   システム LOGFONT から得たフォントサイズ（ポイント値換算）を論理 DIP として CreateTextFormat に渡す。
    //   描画時は SetDpi=96 固定 + SetTransform(Scale(s)) が適用されるため、物理解像度に対する
    //   DWrite サブピクセル AA が自動で得られる（副パイプライン不要）。
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

    // v2 統一テキスト経路（HspBuffer 防御的 SetDpi 設定）:
    //   HspBuffer は m_pTargetBitmap が論理=物理 px / Identity Transform で動作する。
    //   D2D Factory の既定 DPI は 96 のため CreateDeviceContext 直後の値も実質 96 だが、
    //   将来の Factory 構成変更や DeviceContext 再利用に対する防御として明示的に 96 を設定する。
    //   これにより mes() 冒頭コメント「HspBuffer 経路は SetDpi=96 / Identity Transform」が
    //   実装裏付けを持つ。挙動上は no-op（既存挙動と同等）。
    m_pDeviceContext->SetDpi(96.0f, 96.0f);

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
