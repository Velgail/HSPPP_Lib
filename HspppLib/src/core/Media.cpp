// HspppLib/src/core/Media.cpp
// OOP版メディアクラス実装

#include "Media.h"
#include "MediaManager.h"
#include <atomic>

namespace hsppp {

// HWND取得用外部関数（hsppp.cppで定義）
namespace internal {
    void* getWindowHwndById(int id);
}

// ============================================================
// 静的バッファID管理
// ============================================================
static std::atomic<int> s_nextBufferId{1000};  // mm系と被らないよう1000から開始

static int allocateBufferId() {
    return s_nextBufferId.fetch_add(1);
}

// ============================================================
// Media::Impl
// ============================================================
class Media::Impl {
public:
    int m_bufferId = -1;
    std::string m_filename;
    int m_vol = 0;        // -1000 ~ 0
    int m_pan = 0;        // -1000 ~ 1000
    int m_mode = 0;       // 再生モード
    bool m_loop = false;
    bool m_loaded = false;
    void* m_targetWindow = nullptr;  // 動画再生用ターゲットウィンドウ

    Impl() : m_bufferId(allocateBufferId()) {}
    
    ~Impl() {
        if (m_loaded) {
            internal::MediaManager::getInstance().mmstop(m_bufferId);
        }
    }
};

// ============================================================
// Media コンストラクタ・デストラクタ
// ============================================================
Media::Media() : m_impl(std::make_unique<Impl>()) {}

Media::Media(std::string_view filename) : m_impl(std::make_unique<Impl>()) {
    load(filename);
}

Media::~Media() = default;

Media::Media(Media&& other) noexcept = default;
Media& Media::operator=(Media&& other) noexcept = default;

// ============================================================
// ファイル操作
// ============================================================
bool Media::load(std::string_view filename) {
    m_impl->m_filename = std::string(filename);
    
    int loadMode = m_impl->m_loop ? 1 : m_impl->m_mode;
    bool result = internal::MediaManager::getInstance().mmload(
        filename, m_impl->m_bufferId, loadMode,
        static_cast<HWND>(m_impl->m_targetWindow));
    
    m_impl->m_loaded = result;
    
    if (m_impl->m_loaded) {
        // 初期設定を適用
        internal::MediaManager::getInstance().mmvol(m_impl->m_bufferId, m_impl->m_vol);
        internal::MediaManager::getInstance().mmpan(m_impl->m_bufferId, m_impl->m_pan);
    }
    
    return m_impl->m_loaded;
}

void Media::unload() {
    if (m_impl->m_loaded) {
        internal::MediaManager::getInstance().mmstop(m_impl->m_bufferId);
        m_impl->m_loaded = false;
        m_impl->m_filename.clear();
    }
}

// ============================================================
// 再生制御
// ============================================================
bool Media::play() {
    if (!m_impl->m_loaded) return false;
    return internal::MediaManager::getInstance().mmplay(m_impl->m_bufferId);
}

void Media::stop() {
    if (m_impl->m_loaded) {
        internal::MediaManager::getInstance().mmstop(m_impl->m_bufferId);
    }
}

// ============================================================
// 設定（メソッドチェーン）
// ============================================================
Media& Media::vol(int v) {
    m_impl->m_vol = v;
    if (m_impl->m_loaded) {
        internal::MediaManager::getInstance().mmvol(m_impl->m_bufferId, v);
    }
    return *this;
}

Media& Media::pan(int p) {
    m_impl->m_pan = p;
    if (m_impl->m_loaded) {
        internal::MediaManager::getInstance().mmpan(m_impl->m_bufferId, p);
    }
    return *this;
}

Media& Media::loop(bool l) {
    m_impl->m_loop = l;
    if (l) {
        m_impl->m_mode = 1;
    } else if (m_impl->m_mode == 1) {
        m_impl->m_mode = 0;  // ループ解除時はモードもリセット
    }
    return *this;
}

Media& Media::mode(int m) {
    m_impl->m_mode = m;
    m_impl->m_loop = (m == 1);
    return *this;
}

Media& Media::target(int screenId) {
    m_impl->m_targetWindow = internal::getWindowHwndById(screenId);
    return *this;
}

// ============================================================
// 取得
// ============================================================
int Media::get_vol() const { return m_impl->m_vol; }
int Media::get_pan() const { return m_impl->m_pan; }
bool Media::get_loop() const { return m_impl->m_loop; }
int Media::get_mode() const { return m_impl->m_mode; }

int Media::stat() const {
    if (!m_impl->m_loaded) return 0;
    return internal::MediaManager::getInstance().mmstat(m_impl->m_bufferId, 16);
}

bool Media::playing() const {
    return stat() == 1;
}

bool Media::loaded() const {
    return m_impl->m_loaded;
}

const std::string& Media::filename() const {
    return m_impl->m_filename;
}

int Media::id() const {
    return m_impl->m_bufferId;
}

} // namespace hsppp
