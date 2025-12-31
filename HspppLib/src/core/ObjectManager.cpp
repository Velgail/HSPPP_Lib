// HspppLib/src/core/ObjectManager.cpp
// GUIオブジェクトマネージャーの実装

#include "Internal.h"
#include <stdexcept>

namespace hsppp::internal {

// ============================================================
// ObjectManager シングルトン実装
// ============================================================

ObjectManager::ObjectManager()
    : m_nextId(0)
    , m_objSizeX(64)
    , m_objSizeY(24)
    , m_objSpaceY(0)
    , m_fontMode(1)  // デフォルト: GUIフォント
    , m_tabEnabled(true)
    , m_objColorR(0)
    , m_objColorG(0)
    , m_objColorB(0)
{
}

ObjectManager::~ObjectManager() {
    // UniqueHwnd が RAII で自動的に DestroyWindow を呼び出すため、
    // 明示的な破棄は不要。マップをクリアするだけで OK。
    m_objects.clear();
    m_hwndMap.clear();
}

ObjectManager& ObjectManager::getInstance() {
    static ObjectManager instance;
    return instance;
}

int ObjectManager::registerObject(ObjectInfo info) {
    int newId = m_nextId++;
    // 逆引きマップに登録（ムーブ前に HWND を取得）
    HWND hwnd = info.hwnd.get();
    m_objects[newId] = std::move(info);
    if (hwnd) {
        m_hwndMap[hwnd] = newId;
    }
    return newId;
}

ObjectInfo* ObjectManager::getObject(int objectId) {
    auto it = m_objects.find(objectId);
    if (it != m_objects.end()) {
        return &(it->second);
    }
    return nullptr;
}

void ObjectManager::removeObject(int objectId) {
    auto it = m_objects.find(objectId);
    if (it != m_objects.end()) {
        // 逆引きマップから削除
        HWND hwnd = it->second.hwnd.get();
        if (hwnd) {
            m_hwndMap.erase(hwnd);
        }
        // UniqueHwnd が RAII で DestroyWindow を呼び出す
        m_objects.erase(it);
    }
}

void ObjectManager::removeObjects(int startId, int endId) {
    // 効率的なイテレータベースの実装
    auto it = m_objects.lower_bound(startId);
    
    // endId が -1 の場合は最後まで
    if (endId < 0 && !m_objects.empty()) {
        endId = m_objects.rbegin()->first;
    }
    
    while (it != m_objects.end() && it->first <= endId) {
        // 逆引きマップから削除
        HWND hwnd = it->second.hwnd.get();
        if (hwnd) {
            m_hwndMap.erase(hwnd);
        }
        // UniqueHwnd が RAII で DestroyWindow を呼び出す
        it = m_objects.erase(it);
    }
}

void ObjectManager::removeObjectsByWindow(int windowId) {
    auto it = m_objects.begin();
    while (it != m_objects.end()) {
        if (it->second.windowId == windowId) {
            // 逆引きマップから削除
            HWND hwnd = it->second.hwnd.get();
            if (hwnd) {
                m_hwndMap.erase(hwnd);
            }
            // UniqueHwnd が RAII で DestroyWindow を呼び出す
            it = m_objects.erase(it);
        } else {
            ++it;
        }
    }
}

int ObjectManager::findObjectByHwnd(HWND hwnd) {
    // O(log N)の逆引きマップを使用
    auto it = m_hwndMap.find(hwnd);
    if (it != m_hwndMap.end()) {
        return it->second;
    }
    return -1;
}

void ObjectManager::setObjSize(int x, int y, int spaceY) {
    m_objSizeX = x;
    m_objSizeY = y;
    m_objSpaceY = spaceY;
}

void ObjectManager::getObjSize(int& x, int& y, int& spaceY) const {
    x = m_objSizeX;
    y = m_objSizeY;
    spaceY = m_objSpaceY;
}

void ObjectManager::setObjMode(int fontMode, int tabEnabled) {
    m_fontMode = fontMode;
    if (tabEnabled >= 0) {
        m_tabEnabled = (tabEnabled != 0);
    }
}

void ObjectManager::getObjMode(int& fontMode, bool& tabEnabled) const {
    fontMode = m_fontMode;
    tabEnabled = m_tabEnabled;
}

void ObjectManager::setObjColor(int r, int g, int b) {
    m_objColorR = r;
    m_objColorG = g;
    m_objColorB = b;
}

void ObjectManager::getObjColor(int& r, int& g, int& b) const {
    r = m_objColorR;
    g = m_objColorG;
    b = m_objColorB;
}

void ObjectManager::resetSettings() {
    m_objSizeX = 64;
    m_objSizeY = 24;
    m_objSpaceY = 0;
    m_fontMode = 1;
    // m_tabEnabled はリセットしない（HSP仕様）
}

void ObjectManager::syncSingleInputControl(HWND hwnd) {
    // HWNDからオブジェクトを検索
    auto it = m_hwndMap.find(hwnd);
    if (it == m_hwndMap.end()) {
        return;
    }
    
    ObjectInfo* pInfo = getObject(it->second);
    if (!pInfo) {
        return;
    }
    
    // Input/Mesbox以外は無視
    if (pInfo->type != ObjectType::Input && pInfo->type != ObjectType::Mesbox) {
        return;
    }
    
    if (!hwnd || !IsWindow(hwnd)) {
        return;
    }
    
    // EDITコントロールからテキストを取得
    int len = GetWindowTextLengthW(hwnd);
    std::wstring wtext(len + 1, L'\0');
    GetWindowTextW(hwnd, wtext.data(), len + 1);
    wtext.resize(len);  // ヌル終端を除去
    
    // UTF-8に変換
    std::string utf8Text = WideToUtf8(wtext);
    
    // 文字列変数を更新
    if (pInfo->ownedStrVar) {
        *pInfo->ownedStrVar = utf8Text;
    }
}

void ObjectManager::syncInputControls() {
    for (auto& [id, info] : m_objects) {
        syncSingleInputControl(info.hwnd.get());
    }
}

} // namespace hsppp::internal
