// HspppLib/src/core/ImageLoader.cpp
// 画像読み込み・保存機能（WIC使用）

module;

#include <windows.h>
#include <d2d1_1.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <string>
#include <string_view>

#include "Internal.h"

#pragma comment(lib, "windowscodecs.lib")

module hsppp;

namespace hsppp {
namespace internal {

// グローバル変数定義
std::map<int, CelData> g_celDataMap;
int g_nextCelId = 1;

// WICで画像ファイルをロードしてD2Dビットマップを作成
ComPtr<ID2D1Bitmap1> loadImageFile(std::string_view filename, int& width, int& height) {
    auto& deviceMgr = D2DDeviceManager::getInstance();
    if (!deviceMgr.getWICFactory()) return nullptr;

    std::wstring wideFilename = Utf8ToWide(filename);

    // WICデコーダーを作成
    ComPtr<IWICBitmapDecoder> pDecoder;
    HRESULT hr = deviceMgr.getWICFactory()->CreateDecoderFromFilename(
        wideFilename.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        pDecoder.GetAddressOf()
    );
    if (FAILED(hr)) return nullptr;

    // フレームを取得
    ComPtr<IWICBitmapFrameDecode> pFrame;
    hr = pDecoder->GetFrame(0, pFrame.GetAddressOf());
    if (FAILED(hr)) return nullptr;

    // フォーマット変換器を作成（32bppPBGRAに統一）
    ComPtr<IWICFormatConverter> pConverter;
    hr = deviceMgr.getWICFactory()->CreateFormatConverter(pConverter.GetAddressOf());
    if (FAILED(hr)) return nullptr;

    hr = pConverter->Initialize(
        pFrame.Get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );
    if (FAILED(hr)) return nullptr;

    // サイズを取得
    UINT w, h;
    hr = pConverter->GetSize(&w, &h);
    if (FAILED(hr)) return nullptr;

    width = static_cast<int>(w);
    height = static_cast<int>(h);

    // Direct2D ビットマップを作成
    ComPtr<ID2D1Bitmap1> pBitmap;
    auto pDeviceContext = deviceMgr.createDeviceContext();
    if (!pDeviceContext) return nullptr;

    hr = pDeviceContext->CreateBitmapFromWicBitmap(
        pConverter.Get(),
        nullptr,
        pBitmap.GetAddressOf()
    );
    if (FAILED(hr)) return nullptr;

    return pBitmap;
}

// D2DビットマップをBMPファイルに保存
bool saveBitmapToFile(ID2D1Bitmap1* pBitmap, std::string_view filename) {
    if (!pBitmap) return false;

    auto& deviceMgr = D2DDeviceManager::getInstance();
    if (!deviceMgr.getWICFactory()) return false;

    std::wstring wideFilename = Utf8ToWide(filename);

    // ビットマップのサイズとピクセルデータを取得
    D2D1_SIZE_U size = pBitmap->GetPixelSize();
    UINT width = size.width;
    UINT height = size.height;

    // WICビットマップを作成
    ComPtr<IWICBitmap> pWICBitmap;
    HRESULT hr = deviceMgr.getWICFactory()->CreateBitmap(
        width,
        height,
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapCacheOnDemand,
        pWICBitmap.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    // D2Dビットマップからピクセルをコピー
    D2D1_POINT_2U destPoint = { 0, 0 };
    D2D1_RECT_U srcRect = { 0, 0, width, height };
    
    // まずWICビットマップのロックを取得
    ComPtr<IWICBitmapLock> pLock;
    WICRect lockRect = { 0, 0, static_cast<INT>(width), static_cast<INT>(height) };
    hr = pWICBitmap->Lock(&lockRect, WICBitmapLockWrite, pLock.GetAddressOf());
    if (FAILED(hr)) return false;

    UINT bufferSize = 0;
    BYTE* pData = nullptr;
    hr = pLock->GetDataPointer(&bufferSize, &pData);
    if (FAILED(hr)) return false;

    // D2Dビットマップからピクセルをコピー
    hr = pBitmap->CopyFromMemory(&srcRect, pData, width * 4);
    pLock.Reset();  // ロック解除
    if (FAILED(hr)) return false;

    // エンコーダーを作成
    ComPtr<IWICStream> pStream;
    hr = deviceMgr.getWICFactory()->CreateStream(pStream.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = pStream->InitializeFromFilename(wideFilename.c_str(), GENERIC_WRITE);
    if (FAILED(hr)) return false;

    ComPtr<IWICBitmapEncoder> pEncoder;
    hr = deviceMgr.getWICFactory()->CreateEncoder(
        GUID_ContainerFormatBmp,
        nullptr,
        pEncoder.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    hr = pEncoder->Initialize(pStream.Get(), WICBitmapEncoderNoCache);
    if (FAILED(hr)) return false;

    // フレームを作成
    ComPtr<IWICBitmapFrameEncode> pFrameEncode;
    hr = pEncoder->CreateNewFrame(pFrameEncode.GetAddressOf(), nullptr);
    if (FAILED(hr)) return false;

    hr = pFrameEncode->Initialize(nullptr);
    if (FAILED(hr)) return false;

    hr = pFrameEncode->SetSize(width, height);
    if (FAILED(hr)) return false;

    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    hr = pFrameEncode->SetPixelFormat(&format);
    if (FAILED(hr)) return false;

    // ビットマップを書き込み
    hr = pFrameEncode->WriteSource(pWICBitmap.Get(), nullptr);
    if (FAILED(hr)) return false;

    hr = pFrameEncode->Commit();
    if (FAILED(hr)) return false;

    hr = pEncoder->Commit();
    if (FAILED(hr)) return false;

    return true;
}

} // namespace internal
} // namespace hsppp
