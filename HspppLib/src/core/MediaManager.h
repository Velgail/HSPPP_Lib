// HspppLib/src/core/MediaManager.h
// マルチメディア管理ヘッダ（XAudio2 + Media Foundation ハイブリッド実装）
//
// 設計方針：
//   - SE（WAV/短尺音声）: XAudio2（低遅延、多重再生）
//   - BGM/動画（MP3, MP4, 長尺）: Media Foundation（ストリーミング）
//   - ファイルの種類・サイズに応じて自動振り分け

#pragma once

#define NOMINMAX
#include <windows.h>
#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <wrl/client.h>

#include <string>
#include <string_view>
#include <memory>
#include <map>
#include <vector>
#include <mutex>
#include <atomic>

namespace hsppp {
namespace internal {

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

// ============================================================
// メディアの種類
// ============================================================
enum class MediaType {
    Unknown,
    WAV,        // XAudio2でオンメモリ再生
    MP3,        // Media Foundation or XAudio2
    OGG,        // 未実装（将来対応）
    MIDI,       // 現状winmm継続
    Video,      // Media Foundation（AVI, MP4, WMV等）
    Stream,     // Media Foundationでストリーミング
};

// ============================================================
// 再生モード（HSP互換）
// ============================================================
enum class PlayMode {
    Normal = 0,     // 通常再生
    Loop   = 1,     // 無限ループ
    Wait   = 2,     // 再生終了まで待機
    CDCont = 3,     // CDのみ：指定トラック以降を再生
};

// ============================================================
// メディアスロット状態
// ============================================================
enum class MediaState {
    Empty,          // 未ロード
    Loaded,         // ロード済み（停止中）
    Playing,        // 再生中
    Paused,         // 一時停止（将来用）
    Error,          // エラー
};

// ============================================================
// XAudio2用バッファデータ
// ============================================================
struct AudioBuffer {
    std::vector<BYTE> data;
    WAVEFORMATEX format;
    bool isValid = false;
};

// ============================================================
// RAII ラッパー: UniqueSourceVoice（IXAudio2SourceVoice の自動破棄）
// ============================================================

/// @brief IXAudio2SourceVoice を RAII で管理するラッパークラス
/// @details スコープを抜けるか例外が飛んだ際に自動的に DestroyVoice を呼び出す
class UniqueSourceVoice {
private:
    IXAudio2SourceVoice* m_voice = nullptr;

public:
    UniqueSourceVoice() noexcept = default;
    explicit UniqueSourceVoice(IXAudio2SourceVoice* voice) noexcept : m_voice(voice) {}
    
    ~UniqueSourceVoice() {
        reset();
    }
    
    // コピー禁止
    UniqueSourceVoice(const UniqueSourceVoice&) = delete;
    UniqueSourceVoice& operator=(const UniqueSourceVoice&) = delete;
    
    // ムーブ許可
    UniqueSourceVoice(UniqueSourceVoice&& other) noexcept : m_voice(other.m_voice) {
        other.m_voice = nullptr;
    }
    
    UniqueSourceVoice& operator=(UniqueSourceVoice&& other) noexcept {
        if (this != &other) {
            reset();
            m_voice = other.m_voice;
            other.m_voice = nullptr;
        }
        return *this;
    }
    
    /// @brief 保持しているボイスを破棄し、新しいボイスを設定
    void reset(IXAudio2SourceVoice* voice = nullptr) noexcept {
        if (m_voice && m_voice != voice) {
            m_voice->Stop();
            m_voice->DestroyVoice();
        }
        m_voice = voice;
    }
    
    /// @brief 所有権を放棄してボイスを返す
    [[nodiscard]] IXAudio2SourceVoice* release() noexcept {
        IXAudio2SourceVoice* voice = m_voice;
        m_voice = nullptr;
        return voice;
    }
    
    /// @brief 保持しているボイスを取得
    [[nodiscard]] IXAudio2SourceVoice* get() const noexcept { return m_voice; }
    
    /// @brief ボイスが有効かどうか
    [[nodiscard]] explicit operator bool() const noexcept { return m_voice != nullptr; }
    
    /// @brief ポインタとしてアクセス
    IXAudio2SourceVoice* operator->() const noexcept { return m_voice; }
    
    /// @brief アドレス取得（CreateSourceVoice用）
    IXAudio2SourceVoice** put() noexcept { 
        reset();
        return &m_voice; 
    }
};

// ============================================================
// RAII ラッパー: UniqueMasteringVoice（IXAudio2MasteringVoice の自動破棄）
// ============================================================

/// @brief IXAudio2MasteringVoice を RAII で管理するラッパークラス
class UniqueMasteringVoice {
private:
    IXAudio2MasteringVoice* m_voice = nullptr;

public:
    UniqueMasteringVoice() noexcept = default;
    explicit UniqueMasteringVoice(IXAudio2MasteringVoice* voice) noexcept : m_voice(voice) {}
    
    ~UniqueMasteringVoice() {
        reset();
    }
    
    // コピー禁止
    UniqueMasteringVoice(const UniqueMasteringVoice&) = delete;
    UniqueMasteringVoice& operator=(const UniqueMasteringVoice&) = delete;
    
    // ムーブ許可
    UniqueMasteringVoice(UniqueMasteringVoice&& other) noexcept : m_voice(other.m_voice) {
        other.m_voice = nullptr;
    }
    
    UniqueMasteringVoice& operator=(UniqueMasteringVoice&& other) noexcept {
        if (this != &other) {
            reset();
            m_voice = other.m_voice;
            other.m_voice = nullptr;
        }
        return *this;
    }
    
    /// @brief 保持しているボイスを破棄し、新しいボイスを設定
    void reset(IXAudio2MasteringVoice* voice = nullptr) noexcept {
        if (m_voice && m_voice != voice) {
            m_voice->DestroyVoice();
        }
        m_voice = voice;
    }
    
    /// @brief 所有権を放棄してボイスを返す
    [[nodiscard]] IXAudio2MasteringVoice* release() noexcept {
        IXAudio2MasteringVoice* voice = m_voice;
        m_voice = nullptr;
        return voice;
    }
    
    /// @brief 保持しているボイスを取得
    [[nodiscard]] IXAudio2MasteringVoice* get() const noexcept { return m_voice; }
    
    /// @brief ボイスが有効かどうか
    [[nodiscard]] explicit operator bool() const noexcept { return m_voice != nullptr; }
    
    /// @brief ポインタとしてアクセス
    IXAudio2MasteringVoice* operator->() const noexcept { return m_voice; }
    
    /// @brief アドレス取得（CreateMasteringVoice用）
    IXAudio2MasteringVoice** put() noexcept { 
        reset();
        return &m_voice; 
    }
};

// ============================================================
// XAudio2用ボイスコールバック
// ============================================================
class XAudio2VoiceCallback : public IXAudio2VoiceCallback {
public:
    std::atomic<bool> isPlaying{false};
    std::atomic<bool> hasEnded{false};

    void __stdcall OnStreamEnd() override { hasEnded = true; isPlaying = false; }
    void __stdcall OnVoiceProcessingPassEnd() override {}
    void __stdcall OnVoiceProcessingPassStart(UINT32) override {}
    void __stdcall OnBufferEnd(void*) override {}
    void __stdcall OnBufferStart(void*) override {}
    void __stdcall OnLoopEnd(void*) override {}
    void __stdcall OnVoiceError(void*, HRESULT) override {}
};

// ============================================================
// Media Foundation コールバック
// ============================================================
class MediaFoundationCallback : public IMFAsyncCallback {
public:
    MediaFoundationCallback() : refCount_(1), session_(nullptr) {}
    
    // IUnknown
    ULONG __stdcall AddRef() override { return ++refCount_; }
    ULONG __stdcall Release() override {
        ULONG count = --refCount_;
        if (count == 0) delete this;
        return count;
    }
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == IID_IMFAsyncCallback) {
            *ppv = static_cast<IMFAsyncCallback*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    
    // IMFAsyncCallback
    HRESULT __stdcall GetParameters(DWORD*, DWORD*) override { return E_NOTIMPL; }
    HRESULT __stdcall Invoke(IMFAsyncResult* pResult) override;
    
    void SetSession(IMFMediaSession* session) { session_ = session; }
    
    std::atomic<bool> isPlaying{false};
    std::atomic<bool> hasEnded{false};
    std::atomic<bool> hasClosed{false};
    
private:
    std::atomic<ULONG> refCount_;
    IMFMediaSession* session_;
};

// ============================================================
// メディアスロット（1つの読み込まれたメディアを管理）
// ============================================================
struct MediaSlot {
    // 共通
    MediaType type = MediaType::Unknown;
    MediaState state = MediaState::Empty;
    PlayMode playMode = PlayMode::Normal;
    std::string filename;
    float volume = 1.0f;         // 0.0〜1.0（内部はリニア）
    float pan = 0.0f;            // -1.0（左）〜1.0（右）
    bool isVideoFullscreen = false;  // AVI動画をウィンドウ全体で再生
    HWND parentWindow = nullptr;     // 動画表示対象の親ウィンドウ（ScreenのHWND）
    HWND targetWindow = nullptr;     // 動画レンダラーの描画先ウィンドウ（内部で使用）
    HWND videoWindow = nullptr;      // 動画表示用の子ウィンドウ（EVRはここに描画）
    int videoX = 0, videoY = 0;      // 動画再生位置

    // XAudio2用（SE/WAV）
    std::unique_ptr<AudioBuffer> audioBuffer;
    UniqueSourceVoice sourceVoice;   // RAII管理
    std::unique_ptr<XAudio2VoiceCallback> voiceCallback;

    // Media Foundation用（BGM/動画）
    ComPtr<IMFMediaSession> mediaSession;
    ComPtr<IMFMediaSource> mediaSource;
    ComPtr<MediaFoundationCallback> mfCallback;

    MediaSlot() = default;
    ~MediaSlot();
    
    // コピー禁止
    MediaSlot(const MediaSlot&) = delete;
    MediaSlot& operator=(const MediaSlot&) = delete;
    
    // ムーブ許可
    MediaSlot(MediaSlot&& other) noexcept;
    MediaSlot& operator=(MediaSlot&& other) noexcept;
};

// ============================================================
// メディアマネージャー（シングルトン）
// ============================================================
class MediaManager {
public:
    static MediaManager& getInstance();

    // システム初期化・終了
    bool initialize();
    void shutdown();

    // メディア操作（HSP互換API）
    bool mmload(std::string_view filename, int bufferId, int mode, HWND targetWindow = nullptr);
    bool mmplay(int bufferId);
    void mmstop(int bufferId = -1);  // -1 = 全停止
    void mmvol(int bufferId, int vol);   // -1000〜0 (HSP互換)
    void mmpan(int bufferId, int pan);   // -1000〜1000 (HSP互換)
    int  mmstat(int bufferId, int mode);

    // ユーティリティ
    bool isPlaying(int bufferId);
    MediaType detectMediaType(std::string_view filename);

private:
    MediaManager() = default;
    ~MediaManager() = default;

    // 禁止
    MediaManager(const MediaManager&) = delete;
    MediaManager& operator=(const MediaManager&) = delete;

    // XAudio2 関連
    bool initializeXAudio2();
    void shutdownXAudio2();
    bool loadWavFile(std::string_view filename, AudioBuffer& buffer);
    bool playXAudio2(MediaSlot& slot);
    void stopXAudio2(MediaSlot& slot);
    void updateXAudio2Volume(MediaSlot& slot);
    void updateXAudio2Pan(MediaSlot& slot);

    // Media Foundation 関連
    bool initializeMediaFoundation();
    void shutdownMediaFoundation();
    bool loadMediaFoundation(std::string_view filename, MediaSlot& slot);
    bool playMediaFoundation(MediaSlot& slot);
    void stopMediaFoundation(MediaSlot& slot);
    void releaseMediaFoundation(MediaSlot& slot);
    void updateMediaFoundationVolume(MediaSlot& slot);
    void updateMediaFoundationPan(MediaSlot& slot);

    // ヘルパー
    float hspVolumeToLinear(int hspVol);    // -1000〜0 -> 0.0〜1.0
    float hspPanToFloat(int hspPan);        // -1000〜1000 -> -1.0〜1.0
    std::string resolveFilePath(std::string_view filename);  // 相対パス→絶対パス

    // メンバ変数
    bool initialized_ = false;
    std::mutex mutex_;
    std::map<int, std::unique_ptr<MediaSlot>> slots_;

    // XAudio2
    ComPtr<IXAudio2> xaudio2_;
    UniqueMasteringVoice masterVoice_;   // RAII管理

    // Media Foundation
    bool mfInitialized_ = false;
};

} // namespace internal
} // namespace hsppp
