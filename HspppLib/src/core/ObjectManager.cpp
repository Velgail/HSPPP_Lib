// HspppLib/src/core/ObjectManager.cpp
// GUIオブジェクトマネージャーの実装

#include "Internal.h"

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
    // すべてのオブジェクトのHWNDを破棄
    for (auto& [id, info] : m_objects) {
        if (info.hwnd && IsWindow(info.hwnd)) {
            DestroyWindow(info.hwnd);
        }
    }
    m_objects.clear();
}

ObjectManager& ObjectManager::getInstance() {
    static ObjectManager instance;
    return instance;
}

int ObjectManager::registerObject(const ObjectInfo& info) {
    int newId = m_nextId++;
    m_objects[newId] = info;
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
        if (it->second.hwnd && IsWindow(it->second.hwnd)) {
            DestroyWindow(it->second.hwnd);
        }
        m_objects.erase(it);
    }
}

void ObjectManager::removeObjects(int startId, int endId) {
    // endId が -1 の場合は最後まで
    if (endId < 0) {
        endId = m_nextId - 1;
    }
    
    for (int id = startId; id <= endId; ++id) {
        auto it = m_objects.find(id);
        if (it != m_objects.end()) {
            if (it->second.hwnd && IsWindow(it->second.hwnd)) {
                DestroyWindow(it->second.hwnd);
            }
            m_objects.erase(it);
        }
    }
}

void ObjectManager::removeObjectsByWindow(int windowId) {
    auto it = m_objects.begin();
    while (it != m_objects.end()) {
        if (it->second.windowId == windowId) {
            if (it->second.hwnd && IsWindow(it->second.hwnd)) {
                DestroyWindow(it->second.hwnd);
            }
            it = m_objects.erase(it);
        } else {
            ++it;
        }
    }
}

int ObjectManager::findObjectByHwnd(HWND hwnd) {
    for (const auto& [id, info] : m_objects) {
        if (info.hwnd == hwnd) {
            return id;
        }
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

} // namespace hsppp::internal
