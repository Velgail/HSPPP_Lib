// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/src/core/ObjectManager.cpp
// GUIオブジェクトマネージャーの実装

#include "Internal.h"
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace hsppp::internal {

// ============================================================
// ObjectManager シングルトン実装
// ============================================================

ObjectManager::ObjectManager()
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

int ObjectManager::registerObject(ObjectInfo info, const HspSurface& surface) {
    const int windowId = info.windowId;
    const int newId = getNextId(windowId);
    const ObjectKey key{windowId, newId};

    // objmodeは「以降に配置するオブジェクト」へ、配置時点の状態を反映する。
    const auto& settings = settingsFor(windowId);
    const int fontMode = settings.fontMode & 3;
    HFONT font = nullptr;
    if (fontMode == 0) {
        font = static_cast<HFONT>(GetStockObject(SYSTEM_FONT));
    } else if (fontMode == 1) {
        font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    } else {
        font = surface.createObjectFont();
        if (font) {
            info.ownedFont = std::shared_ptr<void>(font, [](void* handle) {
                DeleteObject(reinterpret_cast<HGDIOBJ>(handle));
            });
        }
    }
    if (font && info.hwnd) {
        SendMessageW(info.hwnd.get(), WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    }

    // HSP Win32版と同じく、objmode_usecolorはEDIT系に対して
    // color=背景色、objcolor=文字色として配置時に固定する。
    if ((settings.fontMode & 4) != 0 &&
        (info.type == ObjectType::Input || info.type == ObjectType::Mesbox)) {
        const auto color = surface.getCurrentColor();
        const int backgroundR = (std::clamp)(static_cast<int>(color.r * 255.0f + 0.5f), 0, 255);
        const int backgroundG = (std::clamp)(static_cast<int>(color.g * 255.0f + 0.5f), 0, 255);
        const int backgroundB = (std::clamp)(static_cast<int>(color.b * 255.0f + 0.5f), 0, 255);
        info.backgroundColor = RGB(backgroundR, backgroundG, backgroundB);
        info.textColor = RGB(settings.objColorR, settings.objColorG, settings.objColorB);
        if (HBRUSH brush = CreateSolidBrush(info.backgroundColor)) {
            info.ownedBackgroundBrush = std::shared_ptr<void>(brush, [](void* handle) {
                DeleteObject(reinterpret_cast<HGDIOBJ>(handle));
            });
            info.useCustomColors = true;
        }
    }

    // 逆引きマップに登録（ムーブ前に HWND を取得）
    HWND hwnd = info.hwnd.get();
    m_objects[key] = std::move(info);
    if (hwnd) {
        m_hwndMap[hwnd] = key;
    }
    return newId;
}

ObjectInfo* ObjectManager::getObject(int windowId, int objectId) {
    auto it = m_objects.find({windowId, objectId});
    if (it != m_objects.end()) {
        return &(it->second);
    }
    return nullptr;
}

ObjectInfo* ObjectManager::getObjectByHwnd(HWND hwnd) {
    auto keyIt = m_hwndMap.find(hwnd);
    if (keyIt == m_hwndMap.end()) return nullptr;
    auto it = m_objects.find(keyIt->second);
    return it != m_objects.end() ? &it->second : nullptr;
}

void ObjectManager::removeObject(int windowId, int objectId) {
    auto it = m_objects.find({windowId, objectId});
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

void ObjectManager::removeObjects(int windowId, int startId, int endId) {
    auto it = m_objects.lower_bound({windowId, startId});
    while (it != m_objects.end() && it->first.first == windowId &&
           (endId < 0 || it->first.second <= endId)) {
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
        return it->second.second;
    }
    return -1;
}

ObjectManager::ObjectSettings& ObjectManager::settingsFor(int windowId) {
    return m_settings[windowId];
}

const ObjectManager::ObjectSettings& ObjectManager::settingsFor(int windowId) const {
    static const ObjectSettings defaults{};
    auto it = m_settings.find(windowId);
    return it != m_settings.end() ? it->second : defaults;
}

int ObjectManager::getNextId(int windowId) const {
    int id = 0;
    while (m_objects.contains({windowId, id})) ++id;
    return id;
}

void ObjectManager::setObjSize(int windowId, int x, int y, int spaceY) {
    auto& settings = settingsFor(windowId);
    settings.objSizeX = x;
    settings.objSizeY = y;
    settings.objSpaceY = spaceY;
}

void ObjectManager::getObjSize(int windowId, int& x, int& y, int& spaceY) const {
    const auto& settings = settingsFor(windowId);
    x = settings.objSizeX;
    y = settings.objSizeY;
    spaceY = settings.objSpaceY;
}

void ObjectManager::setObjMode(int windowId, int fontMode, int tabEnabled) {
    auto& settings = settingsFor(windowId);
    settings.fontMode = fontMode;
    if (tabEnabled >= 0) {
        settings.tabEnabled = (tabEnabled != 0);
    }
}

void ObjectManager::getObjMode(int windowId, int& fontMode, bool& tabEnabled) const {
    const auto& settings = settingsFor(windowId);
    fontMode = settings.fontMode;
    tabEnabled = settings.tabEnabled;
}

void ObjectManager::setObjColor(int windowId, int r, int g, int b) {
    auto& settings = settingsFor(windowId);
    settings.objColorR = r;
    settings.objColorG = g;
    settings.objColorB = b;
}

void ObjectManager::getObjColor(int windowId, int& r, int& g, int& b) const {
    const auto& settings = settingsFor(windowId);
    r = settings.objColorR;
    g = settings.objColorG;
    b = settings.objColorB;
}

void ObjectManager::resetSettings(int windowId) {
    m_settings[windowId] = ObjectSettings{};
}

void ObjectManager::resetSettingsForCls(int windowId) {
    auto& settings = settingsFor(windowId);
    settings.objSizeX = 64;
    settings.objSizeY = 24;
    settings.objSpaceY = 0;
    settings.objColorR = 0;
    settings.objColorG = 0;
    settings.objColorB = 0;
    // objmodeとtabmoveはBmscr::Clsでも変更されないため維持する。
}

void ObjectManager::processTabKey(HWND messageWindow, UINT message, WPARAM wParam) {
    if (message != WM_KEYDOWN || wParam != VK_TAB) return;

    HWND focusWindow = GetFocus();
    auto focusKeyIt = m_hwndMap.find(focusWindow);
    int windowId = -1;
    int currentIndex = -1;
    ObjectInfo* currentObject = nullptr;
    if (focusKeyIt != m_hwndMap.end()) {
        windowId = focusKeyIt->second.first;
        currentObject = getObject(windowId, focusKeyIt->second.second);
    } else if (messageWindow) {
        HWND root = GetAncestor(messageWindow, GA_ROOT);
        windowId = getWindowIdFromHwnd(root ? root : messageWindow);
    }
    if (windowId < 0 || !settingsFor(windowId).tabEnabled) return;
    if (currentObject && (currentObject->focusSkipMode & 3) == 2) return;

    std::vector<ObjectInfo*> objects;
    auto it = m_objects.lower_bound({windowId, 0});
    for (; it != m_objects.end() && it->first.first == windowId; ++it) {
        if (&it->second == currentObject) currentIndex = static_cast<int>(objects.size());
        objects.push_back(&it->second);
    }
    if (objects.empty()) return;

    const int direction = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;
    int index = currentIndex;
    if (index < 0 && direction < 0) index = static_cast<int>(objects.size());
    for (size_t attempt = 0; attempt < objects.size(); ++attempt) {
        index += direction;
        if (index >= static_cast<int>(objects.size())) index = 0;
        if (index < 0) index = static_cast<int>(objects.size()) - 1;
        ObjectInfo* candidate = objects[static_cast<size_t>(index)];
        if (!candidate->hwnd || !candidate->enabled || !IsWindowEnabled(candidate->hwnd.get())) continue;
        if ((candidate->focusSkipMode & 3) == 3) continue;
        if ((candidate->focusSkipMode & 4) != 0) {
            SendMessageW(candidate->hwnd.get(), EM_SETSEL, 0, -1);
        }
        SetFocus(candidate->hwnd.get());
        return;
    }
}

void ObjectManager::syncSingleInputControl(HWND hwnd) {
    // HWNDからオブジェクトを検索
    auto it = m_hwndMap.find(hwnd);
    if (it == m_hwndMap.end()) {
        return;
    }
    
    ObjectInfo* pInfo = getObject(it->second.first, it->second.second);
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
