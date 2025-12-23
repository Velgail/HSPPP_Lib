// HspppLib/src/core/hsppp_media.inl
// マルチメディア制御命令の実装（HSP互換）
//
// このファイルは hsppp.cpp から #include される
// MediaManager を使用した近代的な実装（XAudio2 + Media Foundation）
//
// 注意: MediaManager.h を直接インクルードするとC++モジュールと衝突するため、
// 前方宣言と外部関数を使用する。

namespace hsppp {
namespace internal {

// MediaManager への外部関数宣言（MediaManager.cpp で定義）
// 戻り値: 0=成功, 非0=失敗
int  MediaManager_mmload(std::string_view filename, int bufferId, int mode, void* targetWindow);
int  MediaManager_mmplay(int bufferId);
void MediaManager_mmstop(int bufferId);
void MediaManager_mmvol(int bufferId, int vol);
void MediaManager_mmpan(int bufferId, int pan);
int  MediaManager_mmstat(int bufferId, int mode);
void MediaManager_initialize();
void MediaManager_shutdown();

// HWND取得用関数（このファイル内で定義）
void* getWindowHwndById(int id);

} // namespace internal

// ============================================================
// mmload - メディアファイル読み込み
// ============================================================
int mmload(std::string_view filename, OptInt bufferId, OptInt mode,
           const std::source_location& location) {
    int id = bufferId.value_or(0);
    int modeVal = mode.value_or(0);

    // 動画再生用に現在のウィンドウのHWNDを取得
    void* hwnd = internal::getWindowHwndById(g_currentScreenId);

    return internal::MediaManager_mmload(filename, id, modeVal, hwnd);
}

// ============================================================
// mmplay - メディア再生
// ============================================================
int mmplay(OptInt bufferId, const std::source_location& location) {
    int id = bufferId.value_or(0);

    return internal::MediaManager_mmplay(id);
}

// ============================================================
// mmstop - メディア再生の停止
// ============================================================
void mmstop(OptInt bufferId, const std::source_location& location) {
    int id = bufferId.is_default() ? -1 : bufferId.value();

    internal::MediaManager_mmstop(id);
}

// ============================================================
// mmvol - 音量の設定
// ============================================================
void mmvol(int bufferId, int vol, const std::source_location& location) {
    internal::MediaManager_mmvol(bufferId, vol);
}

// ============================================================
// mmpan - パンニングの設定
// ============================================================
void mmpan(int bufferId, int pan, const std::source_location& location) {
    internal::MediaManager_mmpan(bufferId, pan);
}

// ============================================================
// mmstat - メディアの状態取得
// ============================================================
int mmstat(int bufferId, OptInt mode, const std::source_location& location) {
    int modeVal = mode.value_or(0);

    return internal::MediaManager_mmstat(bufferId, modeVal);
}

} // namespace hsppp
