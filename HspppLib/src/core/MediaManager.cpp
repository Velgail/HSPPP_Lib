// HspppLib/src/core/MediaManager.cpp
// マルチメディア管理実装（XAudio2 + Media Foundation ハイブリッド）

#define NOMINMAX
#include <windows.h>
#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <propvarutil.h>
#include <evr.h>
#include <wrl/client.h>

#include <fstream>
#include <algorithm>
#include <cmath>

#include "MediaManager.h"
#include "Internal.h"

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "evr.lib")

namespace hsppp {
namespace internal {

// ============================================================
// MediaFoundationCallback 実装
// ============================================================
HRESULT MediaFoundationCallback::Invoke(IMFAsyncResult* pResult) {
    if (!session_) return S_OK;
    
    ComPtr<IMFMediaEvent> pEvent;
    HRESULT hr = session_->EndGetEvent(pResult, &pEvent);
    if (FAILED(hr)) return hr;

    MediaEventType meType;
    hr = pEvent->GetType(&meType);
    if (FAILED(hr)) return hr;

    switch (meType) {
        case MESessionStarted:
            isPlaying = true;
            hasEnded = false;
            break;
        case MESessionStopped:
        case MESessionEnded:
            isPlaying = false;
            hasEnded = true;
            break;
        case MEError:
            isPlaying = false;
            break;
        default:
            break;
    }

    // 次のイベントを待つ
    if (isPlaying) {
        hr = session_->BeginGetEvent(this, nullptr);
    }
    return S_OK;
}

// ============================================================
// MediaSlot デストラクタ・ムーブ
// ============================================================
MediaSlot::~MediaSlot() {
    // XAudio2 voice 解放
    if (sourceVoice) {
        sourceVoice->Stop();
        sourceVoice->DestroyVoice();
        sourceVoice = nullptr;
    }
    
    // Media Foundation セッション解放
    if (mediaSession) {
        mediaSession->Close();
        mediaSession.Reset();
    }
    
    if (mfCallback) {
        mfCallback->Release();
        mfCallback = nullptr;
    }
    
    mediaSource.Reset();
}

MediaSlot::MediaSlot(MediaSlot&& other) noexcept
    : type(other.type)
    , state(other.state)
    , playMode(other.playMode)
    , filename(std::move(other.filename))
    , volume(other.volume)
    , pan(other.pan)
    , isVideoFullscreen(other.isVideoFullscreen)
    , targetWindow(other.targetWindow)
    , videoX(other.videoX)
    , videoY(other.videoY)
    , audioBuffer(std::move(other.audioBuffer))
    , sourceVoice(other.sourceVoice)
    , voiceCallback(std::move(other.voiceCallback))
    , mediaSession(std::move(other.mediaSession))
    , mediaSource(std::move(other.mediaSource))
    , mfCallback(other.mfCallback)
{
    other.sourceVoice = nullptr;
    other.mfCallback = nullptr;
}

MediaSlot& MediaSlot::operator=(MediaSlot&& other) noexcept {
    if (this != &other) {
        // 既存リソース解放
        if (sourceVoice) {
            sourceVoice->Stop();
            sourceVoice->DestroyVoice();
        }
        if (mediaSession) {
            mediaSession->Close();
        }
        if (mfCallback) {
            mfCallback->Release();
        }
        
        // 移動
        type = other.type;
        state = other.state;
        playMode = other.playMode;
        filename = std::move(other.filename);
        volume = other.volume;
        pan = other.pan;
        isVideoFullscreen = other.isVideoFullscreen;
        targetWindow = other.targetWindow;
        videoX = other.videoX;
        videoY = other.videoY;
        audioBuffer = std::move(other.audioBuffer);
        sourceVoice = other.sourceVoice;
        voiceCallback = std::move(other.voiceCallback);
        mediaSession = std::move(other.mediaSession);
        mediaSource = std::move(other.mediaSource);
        mfCallback = other.mfCallback;
        
        other.sourceVoice = nullptr;
        other.mfCallback = nullptr;
    }
    return *this;
}

// ============================================================
// MediaManager シングルトン
// ============================================================
MediaManager& MediaManager::getInstance() {
    static MediaManager instance;
    return instance;
}

// ============================================================
// システム初期化・終了
// ============================================================
bool MediaManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) return true;

    // XAudio2 初期化
    if (!initializeXAudio2()) {
        return false;
    }

    // Media Foundation 初期化
    if (!initializeMediaFoundation()) {
        shutdownXAudio2();
        return false;
    }

    initialized_ = true;
    return true;
}

void MediaManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return;

    // 全スロット解放
    slots_.clear();

    shutdownMediaFoundation();
    shutdownXAudio2();

    initialized_ = false;
}

// ============================================================
// XAudio2 初期化・終了
// ============================================================
bool MediaManager::initializeXAudio2() {
    HRESULT hr = XAudio2Create(&xaudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) {
        return false;
    }

    // ステレオ出力のマスタリングボイスを作成（パンニング対応のため）
    hr = xaudio2_->CreateMasteringVoice(
        &masterVoice_,
        2,                          // 出力チャンネル数（ステレオ）
        XAUDIO2_DEFAULT_SAMPLERATE,
        0,
        nullptr,
        nullptr
    );
    if (FAILED(hr)) {
        xaudio2_.Reset();
        return false;
    }

    return true;
}

void MediaManager::shutdownXAudio2() {
    if (masterVoice_) {
        masterVoice_->DestroyVoice();
        masterVoice_ = nullptr;
    }
    xaudio2_.Reset();
}

// ============================================================
// Media Foundation 初期化・終了
// ============================================================
bool MediaManager::initializeMediaFoundation() {
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        return false;
    }
    mfInitialized_ = true;
    return true;
}

void MediaManager::shutdownMediaFoundation() {
    if (mfInitialized_) {
        MFShutdown();
        mfInitialized_ = false;
    }
}

// ============================================================
// メディア種類判定
// ============================================================
MediaType MediaManager::detectMediaType(std::string_view filename) {
    std::string ext;
    auto pos = filename.rfind('.');
    if (pos != std::string_view::npos) {
        ext = std::string(filename.substr(pos + 1));
        // 小文字化
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    }

    if (ext == "wav") return MediaType::WAV;
    if (ext == "mp3") return MediaType::MP3;
    if (ext == "ogg") return MediaType::OGG;
    if (ext == "mid" || ext == "midi") return MediaType::MIDI;
    if (ext == "avi" || ext == "mp4" || ext == "wmv" || ext == "mpg" || ext == "mpeg" || ext == "mkv") {
        return MediaType::Video;
    }
    if (ext == "wma" || ext == "aac" || ext == "m4a" || ext == "flac") {
        return MediaType::Stream;  // Media Foundationでストリーミング
    }

    return MediaType::Unknown;
}

// ============================================================
// mmload - メディアファイル読み込み
// ============================================================
bool MediaManager::mmload(std::string_view filename, int bufferId, int mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        initialize();
    }

    // 既存スロットを削除
    auto it = slots_.find(bufferId);
    if (it != slots_.end()) {
        slots_.erase(it);
    }

    // 相対パスをexeディレクトリ基準で絶対パスに変換
    std::string resolvedPath = resolveFilePath(filename);

    // 新規スロット作成
    auto slot = std::make_unique<MediaSlot>();
    slot->filename = resolvedPath;
    slot->playMode = static_cast<PlayMode>(mode & 0x03);
    slot->isVideoFullscreen = (mode & 16) != 0;
    slot->type = detectMediaType(filename);

    // ファイルサイズチェック（2MB以下のWAVはオンメモリ）
    bool loadToMemory = false;
    if (slot->type == MediaType::WAV) {
        std::ifstream file(resolvedPath, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            auto size = file.tellg();
            loadToMemory = (size <= 2 * 1024 * 1024);  // 2MB以下
            file.close();
        }
    }

    bool success = false;
    switch (slot->type) {
        case MediaType::WAV:
            if (loadToMemory) {
                slot->audioBuffer = std::make_unique<AudioBuffer>();
                success = loadWavFile(resolvedPath, *slot->audioBuffer);
            } else {
                // 大きいWAVはMedia Foundationで
                success = loadMediaFoundation(resolvedPath, *slot);
            }
            break;

        case MediaType::MP3:
        case MediaType::Stream:
            // MP3等はMedia Foundationでストリーミング
            success = loadMediaFoundation(resolvedPath, *slot);
            break;

        case MediaType::Video:
            // 動画もMedia Foundation
            success = loadMediaFoundation(resolvedPath, *slot);
            break;

        case MediaType::MIDI:
            // MIDI は現状winmm (mciSendString) を使用するか、未実装
            // 将来的には FluidSynth や Windows.Devices.Midi を検討
            // 今回は簡易的にMedia Foundationを試す
            success = loadMediaFoundation(resolvedPath, *slot);
            break;

        default:
            // 不明な形式もMedia Foundationで試行
            success = loadMediaFoundation(resolvedPath, *slot);
            break;
    }

    if (success) {
        slot->state = MediaState::Loaded;
        slots_[bufferId] = std::move(slot);
        return true;
    }

    return false;
}

// ============================================================
// WAVファイル読み込み
// ============================================================
bool MediaManager::loadWavFile(std::string_view filename, AudioBuffer& buffer) {
    std::ifstream file(std::string(filename), std::ios::binary);
    if (!file.is_open()) return false;

    // RIFFヘッダ
    char riff[4];
    file.read(riff, 4);
    if (std::memcmp(riff, "RIFF", 4) != 0) return false;

    uint32_t chunkSize;
    file.read(reinterpret_cast<char*>(&chunkSize), 4);

    char wave[4];
    file.read(wave, 4);
    if (std::memcmp(wave, "WAVE", 4) != 0) return false;

    // チャンク読み込み
    bool foundFmt = false;
    bool foundData = false;

    while (!file.eof() && (!foundFmt || !foundData)) {
        char chunkId[4];
        file.read(chunkId, 4);
        if (file.gcount() < 4) break;

        uint32_t chunkDataSize;
        file.read(reinterpret_cast<char*>(&chunkDataSize), 4);

        if (std::memcmp(chunkId, "fmt ", 4) == 0) {
            // フォーマットチャンク
            // WAVEFORMATEXを0クリアしてから、実際のサイズ分だけ読み込む
            std::memset(&buffer.format, 0, sizeof(WAVEFORMATEX));
            size_t bytesToRead = (std::min)(static_cast<size_t>(chunkDataSize), sizeof(WAVEFORMATEX));
            file.read(reinterpret_cast<char*>(&buffer.format), bytesToRead);
            // 残りをスキップ
            if (chunkDataSize > bytesToRead) {
                file.seekg(chunkDataSize - bytesToRead, std::ios::cur);
            }
            foundFmt = true;
        }
        else if (std::memcmp(chunkId, "data", 4) == 0) {
            // データチャンク
            buffer.data.resize(chunkDataSize);
            file.read(reinterpret_cast<char*>(buffer.data.data()), chunkDataSize);
            foundData = true;
        }
        else {
            // 他のチャンクはスキップ
            file.seekg(chunkDataSize, std::ios::cur);
        }
    }

    buffer.isValid = foundFmt && foundData;
    return buffer.isValid;
}

// ============================================================
// Media Foundation でメディア読み込み（セッション準備）
// ============================================================
bool MediaManager::loadMediaFoundation(std::string_view filename, MediaSlot& slot) {
    HRESULT hr;

    // ファイルパスをワイド文字に変換
    std::wstring wpath = Utf8ToWide(filename);

    // メディアソース作成
    ComPtr<IMFSourceResolver> resolver;
    hr = MFCreateSourceResolver(&resolver);
    if (FAILED(hr)) return false;

    MF_OBJECT_TYPE objectType;
    ComPtr<IUnknown> source;
    hr = resolver->CreateObjectFromURL(
        wpath.c_str(),
        MF_RESOLUTION_MEDIASOURCE,
        nullptr,
        &objectType,
        &source
    );
    if (FAILED(hr)) return false;

    hr = source.As(&slot.mediaSource);
    if (FAILED(hr)) return false;

    // メディアセッション作成
    hr = MFCreateMediaSession(nullptr, &slot.mediaSession);
    if (FAILED(hr)) {
        slot.mediaSource.Reset();
        return false;
    }

    // コールバック設定
    slot.mfCallback = new MediaFoundationCallback();
    slot.mfCallback->SetSession(slot.mediaSession.Get());

    return true;
}

// ============================================================
// mmplay - メディア再生
// ============================================================
bool MediaManager::mmplay(int bufferId) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = slots_.find(bufferId);
    if (it == slots_.end()) return false;

    MediaSlot& slot = *it->second;
    
    // 既に再生中なら停止
    if (slot.state == MediaState::Playing) {
        if (slot.sourceVoice) {
            stopXAudio2(slot);
        } else if (slot.mediaSession) {
            stopMediaFoundation(slot);
        }
    }

    bool success = false;
    if (slot.audioBuffer && slot.audioBuffer->isValid) {
        // XAudio2 で再生
        success = playXAudio2(slot);
    }
    else if (slot.mediaSession) {
        // Media Foundation で再生
        success = playMediaFoundation(slot);
    }

    if (success) {
        slot.state = MediaState::Playing;
        
        // Wait モード: 再生終了まで待機
        if (slot.playMode == PlayMode::Wait) {
            // ロックを解除して待機
            mutex_.unlock();
            while (isPlaying(bufferId)) {
                Sleep(10);
            }
            mutex_.lock();
        }
    }

    return success;
}

// ============================================================
// XAudio2 再生
// ============================================================
bool MediaManager::playXAudio2(MediaSlot& slot) {
    if (!xaudio2_ || !slot.audioBuffer || !slot.audioBuffer->isValid) return false;

    // 既存の voice を破棄
    if (slot.sourceVoice) {
        slot.sourceVoice->Stop();
        slot.sourceVoice->DestroyVoice();
        slot.sourceVoice = nullptr;
    }

    // コールバック作成
    slot.voiceCallback = std::make_unique<XAudio2VoiceCallback>();

    // Source Voice 作成
    HRESULT hr = xaudio2_->CreateSourceVoice(
        &slot.sourceVoice,
        &slot.audioBuffer->format,
        0,
        XAUDIO2_DEFAULT_FREQ_RATIO,
        slot.voiceCallback.get()
    );
    if (FAILED(hr)) return false;

    // バッファ設定
    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = static_cast<UINT32>(slot.audioBuffer->data.size());
    buffer.pAudioData = slot.audioBuffer->data.data();
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    
    if (slot.playMode == PlayMode::Loop) {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }

    hr = slot.sourceVoice->SubmitSourceBuffer(&buffer);
    if (FAILED(hr)) {
        slot.sourceVoice->DestroyVoice();
        slot.sourceVoice = nullptr;
        return false;
    }

    // 音量・パン設定
    updateXAudio2Volume(slot);
    updateXAudio2Pan(slot);

    // 再生開始
    slot.voiceCallback->isPlaying = true;
    slot.voiceCallback->hasEnded = false;
    hr = slot.sourceVoice->Start();
    
    return SUCCEEDED(hr);
}

void MediaManager::stopXAudio2(MediaSlot& slot) {
    if (slot.sourceVoice) {
        slot.sourceVoice->Stop();
        slot.sourceVoice->FlushSourceBuffers();
        if (slot.voiceCallback) {
            slot.voiceCallback->isPlaying = false;
        }
    }
    slot.state = MediaState::Loaded;
}

void MediaManager::updateXAudio2Volume(MediaSlot& slot) {
    if (slot.sourceVoice) {
        slot.sourceVoice->SetVolume(slot.volume);
    }
}

void MediaManager::updateXAudio2Pan(MediaSlot& slot) {
    if (!slot.sourceVoice || !slot.audioBuffer) return;

    // パンニング: 出力マトリクス設定
    XAUDIO2_VOICE_DETAILS voiceDetails;
    slot.sourceVoice->GetVoiceDetails(&voiceDetails);

    XAUDIO2_VOICE_DETAILS masterDetails;
    masterVoice_->GetVoiceDetails(&masterDetails);

    UINT32 srcCh = voiceDetails.InputChannels;
    UINT32 dstCh = masterDetails.InputChannels;

    // デバッグ出力
    char dbg[256];
    sprintf_s(dbg, "[XA2 Pan] pan=%.2f, srcCh=%u, dstCh=%u\n", slot.pan, srcCh, dstCh);
    OutputDebugStringA(dbg);

    // ステレオ出力を想定
    if (dstCh >= 2) {
        // pan: -1.0 (左) 〜 0 (中央) 〜 +1.0 (右)
        // 左出力: pan≤0で1.0、pan>0で減少
        // 右出力: pan≥0で1.0、pan<0で減少
        float leftGain = (slot.pan <= 0.0f) ? 1.0f : (1.0f - slot.pan);
        float rightGain = (slot.pan >= 0.0f) ? 1.0f : (1.0f + slot.pan);

        sprintf_s(dbg, "[XA2 Pan] leftGain=%.2f, rightGain=%.2f\n", leftGain, rightGain);
        OutputDebugStringA(dbg);

        // 出力マトリクス
        // XAudio2: pLevelMatrix[SourceChannels * DestChannel + SourceChannel]
        // つまり destination-major order
        std::vector<float> matrix(srcCh * dstCh, 0.0f);
        
        if (srcCh == 1) {
            // モノラル入力 → ステレオ出力
            // matrix[srcCh * dstIdx + srcIdx]
            matrix[1 * 0 + 0] = leftGain;   // src0 → dst0 (L)
            matrix[1 * 1 + 0] = rightGain;  // src0 → dst1 (R)
            
            sprintf_s(dbg, "[XA2 Pan] Mono: matrix[0]=%.2f (->L), matrix[1]=%.2f (->R)\n", 
                matrix[0], matrix[1]);
            OutputDebugStringA(dbg);
        }
        else if (srcCh >= 2) {
            // ステレオ入力 → ステレオ出力
            // pan=0: L→L, R→R (通常ステレオ)
            // pan=+1: 全部右から（L→R, R→R）
            // pan=-1: 全部左から（L→L, R→L）
            matrix[2 * 0 + 0] = leftGain;           // srcL → dstL
            matrix[2 * 0 + 1] = 1.0f - rightGain;   // srcR → dstL
            matrix[2 * 1 + 0] = 1.0f - leftGain;    // srcL → dstR
            matrix[2 * 1 + 1] = rightGain;          // srcR → dstR
        }

        HRESULT hr = slot.sourceVoice->SetOutputMatrix(
            masterVoice_,
            srcCh,
            dstCh,
            matrix.data()
        );
        sprintf_s(dbg, "[XA2 Pan] SetOutputMatrix hr=0x%08X\n", hr);
        OutputDebugStringA(dbg);
    }
}

// ============================================================
// Media Foundation 再生
// ============================================================
bool MediaManager::playMediaFoundation(MediaSlot& slot) {
    if (!slot.mediaSession || !slot.mediaSource) return false;

    HRESULT hr;

    // トポロジ作成
    ComPtr<IMFTopology> topology;
    hr = MFCreateTopology(&topology);
    if (FAILED(hr)) return false;

    // プレゼンテーション記述子取得
    ComPtr<IMFPresentationDescriptor> pd;
    hr = slot.mediaSource->CreatePresentationDescriptor(&pd);
    if (FAILED(hr)) return false;

    DWORD streamCount;
    hr = pd->GetStreamDescriptorCount(&streamCount);
    if (FAILED(hr)) return false;

    // 各ストリームをトポロジに追加
    for (DWORD i = 0; i < streamCount; i++) {
        BOOL selected;
        ComPtr<IMFStreamDescriptor> sd;
        hr = pd->GetStreamDescriptorByIndex(i, &selected, &sd);
        if (FAILED(hr) || !selected) continue;

        ComPtr<IMFMediaTypeHandler> handler;
        hr = sd->GetMediaTypeHandler(&handler);
        if (FAILED(hr)) continue;

        GUID majorType;
        hr = handler->GetMajorType(&majorType);
        if (FAILED(hr)) continue;

        // ソースノード作成
        ComPtr<IMFTopologyNode> sourceNode;
        hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &sourceNode);
        if (FAILED(hr)) continue;

        hr = sourceNode->SetUnknown(MF_TOPONODE_SOURCE, slot.mediaSource.Get());
        hr = sourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pd.Get());
        hr = sourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, sd.Get());
        hr = topology->AddNode(sourceNode.Get());

        // 出力ノード作成
        ComPtr<IMFTopologyNode> outputNode;
        hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &outputNode);
        if (FAILED(hr)) continue;

        ComPtr<IMFActivate> activate;
        if (majorType == MFMediaType_Audio) {
            // オーディオ出力
            hr = MFCreateAudioRendererActivate(&activate);
        }
        else if (majorType == MFMediaType_Video && slot.targetWindow) {
            // ビデオ出力（EVR）
            hr = MFCreateVideoRendererActivate(slot.targetWindow, &activate);
        }
        else {
            continue;  // サポートしないストリームタイプ
        }

        if (SUCCEEDED(hr) && activate) {
            hr = outputNode->SetObject(activate.Get());
            hr = topology->AddNode(outputNode.Get());
            hr = sourceNode->ConnectOutput(0, outputNode.Get(), 0);
        }
    }

    // トポロジをセッションに設定
    hr = slot.mediaSession->SetTopology(0, topology.Get());
    if (FAILED(hr)) return false;

    // イベント取得開始
    if (slot.mfCallback) {
        slot.mfCallback->isPlaying = false;
        slot.mfCallback->hasEnded = false;
        hr = slot.mediaSession->BeginGetEvent(slot.mfCallback, nullptr);
    }

    // 再生開始
    PROPVARIANT var;
    PropVariantInit(&var);
    hr = slot.mediaSession->Start(nullptr, &var);
    PropVariantClear(&var);

    if (SUCCEEDED(hr) && slot.mfCallback) {
        slot.mfCallback->isPlaying = true;
    }

    return SUCCEEDED(hr);
}

void MediaManager::stopMediaFoundation(MediaSlot& slot) {
    if (slot.mediaSession) {
        slot.mediaSession->Stop();
        if (slot.mfCallback) {
            slot.mfCallback->isPlaying = false;
        }
    }
    slot.state = MediaState::Loaded;
}

void MediaManager::updateMediaFoundationVolume(MediaSlot& slot) {
    if (!slot.mediaSession) return;

    // IMFAudioStreamVolumeを取得（チャンネル別ボリューム＝パンも可能）
    ComPtr<IMFAudioStreamVolume> streamVolume;
    HRESULT hr = MFGetService(slot.mediaSession.Get(), MR_STREAM_VOLUME_SERVICE,
                               IID_PPV_ARGS(&streamVolume));
    if (SUCCEEDED(hr) && streamVolume) {
        UINT32 channelCount = 0;
        streamVolume->GetChannelCount(&channelCount);
        
        if (channelCount >= 2) {
            // ステレオ: パンとボリュームを考慮
            float leftPan = (slot.pan <= 0.0f) ? 1.0f : (1.0f - slot.pan);
            float rightPan = (slot.pan >= 0.0f) ? 1.0f : (1.0f + slot.pan);
            
            streamVolume->SetChannelVolume(0, slot.volume * leftPan);   // L
            streamVolume->SetChannelVolume(1, slot.volume * rightPan);  // R
        } else if (channelCount == 1) {
            // モノラル
            streamVolume->SetChannelVolume(0, slot.volume);
        }
        return;
    }
    
    // フォールバック: IMFSimpleAudioVolume（パンなし）
    ComPtr<IMFSimpleAudioVolume> audioVolume;
    hr = MFGetService(slot.mediaSession.Get(), MR_POLICY_VOLUME_SERVICE, 
                       IID_PPV_ARGS(&audioVolume));
    if (SUCCEEDED(hr) && audioVolume) {
        audioVolume->SetMasterVolume(slot.volume);
    }
}

void MediaManager::updateMediaFoundationPan(MediaSlot& slot) {
    // パン変更時もボリューム関数で一括処理
    updateMediaFoundationVolume(slot);
}

// ============================================================
// mmstop - メディア停止
// ============================================================
void MediaManager::mmstop(int bufferId) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (bufferId < 0) {
        // 全停止
        for (auto& pair : slots_) {
            MediaSlot& slot = *pair.second;
            if (slot.sourceVoice) {
                stopXAudio2(slot);
            }
            if (slot.mediaSession) {
                stopMediaFoundation(slot);
            }
        }
    }
    else {
        auto it = slots_.find(bufferId);
        if (it != slots_.end()) {
            MediaSlot& slot = *it->second;
            if (slot.sourceVoice) {
                stopXAudio2(slot);
            }
            if (slot.mediaSession) {
                stopMediaFoundation(slot);
            }
        }
    }
}

// ============================================================
// mmvol - 音量設定
// ============================================================
void MediaManager::mmvol(int bufferId, int vol) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = slots_.find(bufferId);
    if (it == slots_.end()) return;

    MediaSlot& slot = *it->second;
    slot.volume = hspVolumeToLinear(vol);

    if (slot.sourceVoice) {
        updateXAudio2Volume(slot);
    }
    if (slot.mediaSession) {
        updateMediaFoundationVolume(slot);
    }
}

// ============================================================
// mmpan - パンニング設定
// ============================================================
void MediaManager::mmpan(int bufferId, int pan) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = slots_.find(bufferId);
    if (it == slots_.end()) return;

    MediaSlot& slot = *it->second;
    slot.pan = hspPanToFloat(pan);

    char dbg[256];
    sprintf_s(dbg, "[mmpan] bufferId=%d, pan=%d -> %.2f, hasVoice=%d, hasSession=%d\n",
        bufferId, pan, slot.pan, slot.sourceVoice != nullptr, slot.mediaSession != nullptr);
    OutputDebugStringA(dbg);

    if (slot.sourceVoice) {
        updateXAudio2Pan(slot);
    }
    if (slot.mediaSession) {
        updateMediaFoundationPan(slot);
    }
}

// ============================================================
// mmstat - 状態取得
// ============================================================
int MediaManager::mmstat(int bufferId, int mode) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = slots_.find(bufferId);
    if (it == slots_.end()) return 0;

    MediaSlot& slot = *it->second;

    switch (mode) {
        case 0:   // 設定フラグ値
            return static_cast<int>(slot.playMode);
        case 1:   // ボリューム値
            return static_cast<int>(slot.volume * -1000.0f + 0.0f);  // 近似逆変換
        case 2:   // パンニング値
            return static_cast<int>(slot.pan * 1000.0f);
        case 3:   // 再生レート（未実装）
            return 0;
        case 16:  // 再生中フラグ
            return isPlaying(bufferId) ? 1 : 0;
        default:
            return 0;
    }
}

// ============================================================
// isPlaying - 再生状態チェック
// ============================================================
bool MediaManager::isPlaying(int bufferId) {
    // ロックなしで呼び出される場合があるため注意
    auto it = slots_.find(bufferId);
    if (it == slots_.end()) return false;

    MediaSlot& slot = *it->second;
    
    if (slot.voiceCallback) {
        return slot.voiceCallback->isPlaying.load();
    }
    if (slot.mfCallback) {
        return slot.mfCallback->isPlaying.load();
    }
    
    return false;
}

// ============================================================
// ヘルパー関数
// ============================================================
float MediaManager::hspVolumeToLinear(int hspVol) {
    // HSP: -1000（無音）〜 0（最大）
    // リニア: 0.0（無音）〜 1.0（最大）
    hspVol = std::clamp(hspVol, -1000, 0);
    return (hspVol + 1000) / 1000.0f;
}

float MediaManager::hspPanToFloat(int hspPan) {
    // HSP: -1000（左）〜 1000（右）
    // float: -1.0（左）〜 1.0（右）
    hspPan = std::clamp(hspPan, -1000, 1000);
    return hspPan / 1000.0f;
}

std::string MediaManager::resolveFilePath(std::string_view filename) {
    // 絶対パスならそのまま
    if (filename.length() >= 2 && filename[1] == ':') {
        return std::string(filename);
    }
    if (filename.length() >= 1 && (filename[0] == '\\' || filename[0] == '/')) {
        return std::string(filename);
    }

    // exeのディレクトリを取得
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring wExePath(exePath);
    auto lastSlash = wExePath.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        wExePath = wExePath.substr(0, lastSlash + 1);
    }

    // 相対パスを連結
    std::wstring wFilename = Utf8ToWide(filename);
    std::wstring fullPath = wExePath + wFilename;
    return WideToUtf8(fullPath);
}

// ============================================================
// 外部関数（hsppp_media.inl から呼び出される）
// 戻り値: 0=成功, 非0=失敗
// ============================================================
int MediaManager_mmload(std::string_view filename, int bufferId, int mode) {
    return MediaManager::getInstance().mmload(filename, bufferId, mode) ? 0 : 1;
}

int MediaManager_mmplay(int bufferId) {
    return MediaManager::getInstance().mmplay(bufferId) ? 0 : 1;
}

void MediaManager_mmstop(int bufferId) {
    MediaManager::getInstance().mmstop(bufferId);
}

void MediaManager_mmvol(int bufferId, int vol) {
    MediaManager::getInstance().mmvol(bufferId, vol);
}

void MediaManager_mmpan(int bufferId, int pan) {
    MediaManager::getInstance().mmpan(bufferId, pan);
}

int MediaManager_mmstat(int bufferId, int mode) {
    return MediaManager::getInstance().mmstat(bufferId, mode);
}

void MediaManager_initialize() {
    MediaManager::getInstance().initialize();
}

void MediaManager_shutdown() {
    MediaManager::getInstance().shutdown();
}

} // namespace internal
} // namespace hsppp
