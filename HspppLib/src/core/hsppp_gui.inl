// Source: https://github.com/Velgail/HspppLib
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at
// https://www.boost.org/LICENSE_1_0.txt
// SPDX-License-Identifier: BSL-1.0

// HspppLib/src/core/hsppp_gui.inl
// GUIオブジェクト制御命令の実装

namespace hsppp {

// internal名前空間の関数・変数を使用
using namespace internal;

// ============================================================
// objsize - オブジェクトサイズ設定
// ============================================================
void objsize(OptInt sizeX, OptInt sizeY, OptInt spaceY, const std::source_location& location) {
    safe_call(location, [&] {
        // 現在の Surface に設定（OOP設計：Surfaceがデータを所有）
        auto surface = getCurrentSurface();
        if (surface) {
            surface->setObjSize(
                sizeX.value_or(64),
                sizeY.value_or(24),
                spaceY.value_or(0)
            );
        }
        
        // ObjectManager にも設定（後方互換性のため）
        auto& objMgr = internal::ObjectManager::getInstance();
        objMgr.setObjSize(
            sizeX.value_or(64),
            sizeY.value_or(24),
            spaceY.value_or(0)
        );
    });
}

// ============================================================
// objmode - オブジェクトモード設定
// ============================================================
void objmode(OptInt mode, OptInt tabMove, const std::source_location& location) {
    safe_call(location, [&] {
        auto& objMgr = internal::ObjectManager::getInstance();
        objMgr.setObjMode(
            mode.value_or(0),
            tabMove.is_default() ? -1 : tabMove.value()
        );
    });
}

// ============================================================
// objcolor - オブジェクトのカラー設定
// ============================================================
void objcolor(OptInt r, OptInt g, OptInt b, const std::source_location& location) {
    safe_call(location, [&] {
        auto& objMgr = internal::ObjectManager::getInstance();
        objMgr.setObjColor(
            r.value_or(0),
            g.value_or(0),
            b.value_or(0)
        );
    });
}

// ============================================================
// button - ボタン表示
// ============================================================
int button(std::string_view name, std::function<void()> callback, const std::source_location& location) {
    return safe_call(location, [&]() -> int {
        ensureDefaultScreen();

        auto surface = getCurrentSurface();
        if (!surface) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
        }

        return internal::button_impl(surface, g_currentScreenId, name, std::move(callback));
    });
}

// ============================================================
// input - 入力ボックス表示（文字列版）
// shared_ptrによりライフタイムが自動管理されるため、
// GUIオブジェクトより先に変数が破棄されるリスクがありません。
// ============================================================
// input (shared_ptr<std::string>版)
int input(std::shared_ptr<std::string> var, OptInt sizeX, OptInt sizeY, OptInt maxLen, const std::source_location& location) {
    return safe_call(location, [&]() -> int {
        ensureDefaultScreen();

        auto surface = getCurrentSurface();
        if (!surface) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
        }

        // Surface から objsize を取得
        int objW = surface->getObjSizeX();
        int objH = surface->getObjSizeY();
        int objSpace = surface->getObjSpaceY();

        int w = sizeX.value_or(objW);
        int h = sizeY.value_or(objH);
        int maxChars = maxLen.value_or(256);

        return internal::input_impl(surface, g_currentScreenId, var, maxChars, w, h, objSpace);
    });
}

// mesbox (shared_ptr<std::string>版)
int mesbox(std::shared_ptr<std::string> var, OptInt sizeX, OptInt sizeY, OptInt style, OptInt maxLen, const std::source_location& location) {
    return safe_call(location, [&]() -> int {
        ensureDefaultScreen();

        auto surface = getCurrentSurface();
        if (!surface) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
        }

        // Surface から objsize を取得
        int objW = surface->getObjSizeX();
        int objH = surface->getObjSizeY();
        int objSpace = surface->getObjSpaceY();

        int w = sizeX.value_or(objW);
        int h = sizeY.value_or(objH * 3);
        int styleVal = style.value_or(1);
        int maxChars = maxLen.value_or(32767);

        return internal::mesbox_impl(surface, g_currentScreenId, var, maxChars, styleVal, w, h, objSpace);
    });
}

// chkbox (shared_ptr<int>版)
int chkbox(std::string_view label, std::shared_ptr<int> var, const std::source_location& location) {
    return safe_call(location, [&]() -> int {
        auto& objMgr = internal::ObjectManager::getInstance();
        
        int windowId = g_currentScreenId;
        auto surface = getCurrentSurface();
        if (!surface) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
        }
        
        auto pHspWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (!pHspWindow) {
            throw HspError(ERR_UNSUPPORTED, "Cannot create chkbox on buffer", location);
        }
        
        // Surface から objsize を取得
        int objW = surface->getObjSizeX();
        int objH = surface->getObjSizeY();
        int objSpace = surface->getObjSpaceY();
        
        int posX = surface->getCurrentX();
        int posY = surface->getCurrentY();
        
        HWND hwndParent = pHspWindow->getHwnd();
        std::wstring wlabel = internal::Utf8ToWide(label);
        
        HWND hwndCheck = CreateWindowExW(
            WS_EX_NOPARENTNOTIFY,
            L"BUTTON",
            wlabel.c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | BS_AUTOCHECKBOX,
            posX, posY, objW, objH,
            hwndParent,
            (HMENU)(INT_PTR)(objMgr.getNextId()),
            GetModuleHandle(nullptr),
            nullptr
        );
        
        if (!hwndCheck) {
            throw HspError(ERR_SYSTEM_ERROR, "Failed to create checkbox", location);
        }
        
        SetWindowPos(hwndCheck, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        SendMessageW(hwndCheck, BM_SETCHECK, *var ? BST_CHECKED : BST_UNCHECKED, 0);
        
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessageW(hwndCheck, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        internal::ObjectInfo info;
        info.type = internal::ObjectType::Chkbox;
        info.hwnd.reset(hwndCheck);
        info.windowId = windowId;
        info.x = posX;
        info.y = posY;
        info.width = objW;
        info.height = objH;
        info.ownedStateVar = var;  // shared_ptrで保持
        info.enabled = true;
        info.focusSkipMode = 1;
        
        int objectId = objMgr.registerObject(std::move(info));
        
        int nextY = posY + std::max(objH, objSpace);
        surface->pos(posX, nextY);
        
        return objectId;
    });
}

// combox (shared_ptr<int>版)
int combox(std::shared_ptr<int> var, OptInt expandY, std::string_view items, const std::source_location& location) {
    return safe_call(location, [&]() -> int {
        auto& objMgr = internal::ObjectManager::getInstance();
        
        int windowId = g_currentScreenId;
        auto surface = getCurrentSurface();
        if (!surface) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
        }
        
        auto pHspWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (!pHspWindow) {
            throw HspError(ERR_UNSUPPORTED, "Cannot create combox on buffer", location);
        }
        
        // Surface から objsize を取得
        int objW = surface->getObjSizeX();
        int objH = surface->getObjSizeY();
        int objSpace = surface->getObjSpaceY();
        int expandYVal = expandY.value_or(100);
        
        int posX = surface->getCurrentX();
        int posY = surface->getCurrentY();
        
        HWND hwndParent = pHspWindow->getHwnd();
        
        HWND hwndCombo = CreateWindowExW(
            WS_EX_NOPARENTNOTIFY,
            L"COMBOBOX",
            L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | CBS_DROPDOWNLIST | WS_VSCROLL,
            posX, posY, objW, objH + expandYVal,
            hwndParent,
            (HMENU)(INT_PTR)(objMgr.getNextId()),
            GetModuleHandle(nullptr),
            nullptr
        );
        
        if (!hwndCombo) {
            throw HspError(ERR_SYSTEM_ERROR, "Failed to create combobox", location);
        }
        
        SetWindowPos(hwndCombo, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        
        // アイテムを追加
        std::string itemStr(items);
        size_t pos = 0;
        while (pos < itemStr.size()) {
            size_t nextPos = itemStr.find('\n', pos);
            if (nextPos == std::string::npos) {
                nextPos = itemStr.size();
            }
            std::string item = itemStr.substr(pos, nextPos - pos);
            std::wstring witem = internal::Utf8ToWide(item);
            SendMessageW(hwndCombo, CB_ADDSTRING, 0, (LPARAM)witem.c_str());
            pos = nextPos + 1;
        }
        
        if (*var >= 0) {
            SendMessageW(hwndCombo, CB_SETCURSEL, *var, 0);
        }
        
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessageW(hwndCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        internal::ObjectInfo info;
        info.type = internal::ObjectType::Combox;
        info.hwnd.reset(hwndCombo);
        info.windowId = windowId;
        info.x = posX;
        info.y = posY;
        info.width = objW;
        info.height = objH;
        info.ownedStateVar = var;  // shared_ptrで保持
        info.enabled = true;
        info.focusSkipMode = 1;
        
        int objectId = objMgr.registerObject(std::move(info));
        
        int nextY = posY + std::max(objH, objSpace);
        surface->pos(posX, nextY);
        
        return objectId;
    });
}

// listbox (shared_ptr<int>版)
int listbox(std::shared_ptr<int> var, OptInt expandY, std::string_view items, const std::source_location& location) {
    return safe_call(location, [&]() -> int {
        auto& objMgr = internal::ObjectManager::getInstance();
        
        int windowId = g_currentScreenId;
        auto surface = getCurrentSurface();
        if (!surface) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
        }
        
        auto pHspWindow = std::dynamic_pointer_cast<internal::HspWindow>(surface);
        if (!pHspWindow) {
            throw HspError(ERR_UNSUPPORTED, "Cannot create listbox on buffer", location);
        }
        
        // Surface から objsize を取得
        int objW = surface->getObjSizeX();
        int objSpace = surface->getObjSpaceY();
        int height = expandY.value_or(100);
        
        int posX = surface->getCurrentX();
        int posY = surface->getCurrentY();
        
        HWND hwndParent = pHspWindow->getHwnd();
        
        HWND hwndList = CreateWindowExW(
            WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
            L"LISTBOX",
            L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | WS_VSCROLL | LBS_NOTIFY,
            posX, posY, objW, height,
            hwndParent,
            (HMENU)(INT_PTR)(objMgr.getNextId()),
            GetModuleHandle(nullptr),
            nullptr
        );
        
        if (!hwndList) {
            throw HspError(ERR_SYSTEM_ERROR, "Failed to create listbox", location);
        }
        
        SetWindowPos(hwndList, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        
        // アイテムを追加
        std::string itemStr(items);
        size_t pos = 0;
        while (pos < itemStr.size()) {
            size_t nextPos = itemStr.find('\n', pos);
            if (nextPos == std::string::npos) {
                nextPos = itemStr.size();
            }
            std::string item = itemStr.substr(pos, nextPos - pos);
            std::wstring witem = internal::Utf8ToWide(item);
            SendMessageW(hwndList, LB_ADDSTRING, 0, (LPARAM)witem.c_str());
            pos = nextPos + 1;
        }
        
        if (*var >= 0) {
            SendMessageW(hwndList, LB_SETCURSEL, *var, 0);
        }
        
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessageW(hwndList, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        internal::ObjectInfo info;
        info.type = internal::ObjectType::Listbox;
        info.hwnd.reset(hwndList);
        info.windowId = windowId;
        info.x = posX;
        info.y = posY;
        info.width = objW;
        info.height = height;
        info.ownedStateVar = var;  // shared_ptrで保持
        info.enabled = true;
        info.focusSkipMode = 1;
        
        int objectId = objMgr.registerObject(std::move(info));
        
        int nextY = posY + std::max(height, objSpace);
        surface->pos(posX, nextY);
        
        return objectId;
    });
}

// ============================================================
// clrobj - オブジェクトをクリア
// ============================================================
void clrobj(OptInt startId, OptInt endId, const std::source_location& location) {
    safe_call(location, [&] {
        auto& objMgr = internal::ObjectManager::getInstance();
        
        int start = startId.value_or(0);
        int end = endId.value_or(-1);
        
        objMgr.removeObjects(start, end);
    });
}

// ============================================================
// objprm - オブジェクトの内容を変更（文字列版）
// ============================================================
void objprm(int objectId, std::string_view value, const std::source_location& location) {
    safe_call(location, [&] {
        auto& objMgr = internal::ObjectManager::getInstance();
        auto* pInfo = objMgr.getObject(objectId);
        
        if (!pInfo || !pInfo->hwnd) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid object ID", location);
        }
        
        std::wstring wvalue = internal::Utf8ToWide(value);
        
        switch (pInfo->type) {
            case internal::ObjectType::Button:
            case internal::ObjectType::Input:
            case internal::ObjectType::Mesbox:
                SetWindowTextW(pInfo->hwnd, wvalue.c_str());
                // 変数も更新
                if (auto* strVar = pInfo->getStrVar()) {
                    *strVar = std::string(value);
                }
                // input/mesboxの場合はフォーカスを設定
                if (pInfo->type != internal::ObjectType::Button) {
                    SetFocus(pInfo->hwnd);
                }
                break;
                
            case internal::ObjectType::Combox:
            case internal::ObjectType::Listbox: {
                // 内容をクリアして再設定
                UINT clearMsg = (pInfo->type == internal::ObjectType::Combox) ? CB_RESETCONTENT : LB_RESETCONTENT;
                UINT addMsg = (pInfo->type == internal::ObjectType::Combox) ? CB_ADDSTRING : LB_ADDSTRING;
                
                SendMessageW(pInfo->hwnd, clearMsg, 0, 0);
                
                std::string itemStr(value);
                size_t pos = 0;
                while (pos < itemStr.size()) {
                    size_t nextPos = itemStr.find('\n', pos);
                    if (nextPos == std::string::npos) {
                        nextPos = itemStr.size();
                    }
                    std::string item = itemStr.substr(pos, nextPos - pos);
                    std::wstring witem = internal::Utf8ToWide(item);
                    SendMessageW(pInfo->hwnd, addMsg, 0, (LPARAM)witem.c_str());
                    pos = nextPos + 1;
                }
                break;
            }
                
            default:
                break;
        }
    });
}

// ============================================================
// objprm - オブジェクトの内容を変更（整数版）
// ============================================================
void objprm(int objectId, int value, const std::source_location& location) {
    safe_call(location, [&] {
        auto& objMgr = internal::ObjectManager::getInstance();
        auto* pInfo = objMgr.getObject(objectId);
        
        if (!pInfo || !pInfo->hwnd) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid object ID", location);
        }
        
        switch (pInfo->type) {
            case internal::ObjectType::Input:
                if (auto* intVar = pInfo->getIntVar()) {
                    *intVar = value;
                    SetWindowTextW(pInfo->hwnd, std::to_wstring(value).c_str());
                } else if (auto* strVar = pInfo->getStrVar()) {
                    *strVar = std::to_string(value);
                    SetWindowTextW(pInfo->hwnd, std::to_wstring(value).c_str());
                }
                SetFocus(pInfo->hwnd);
                break;
                
            case internal::ObjectType::Chkbox:
                SendMessageW(pInfo->hwnd, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0);
                if (auto* stateVar = pInfo->getStateVar()) {
                    *stateVar = value ? 1 : 0;
                }
                break;
                
            case internal::ObjectType::Combox:
                SendMessageW(pInfo->hwnd, CB_SETCURSEL, value, 0);
                if (auto* stateVar = pInfo->getStateVar()) {
                    *stateVar = value;
                }
                break;
                
            case internal::ObjectType::Listbox:
                SendMessageW(pInfo->hwnd, LB_SETCURSEL, value, 0);
                if (auto* stateVar = pInfo->getStateVar()) {
                    *stateVar = value;
                }
                break;
                
            default:
                // ボタンなど：文字列として設定（直接SetWindowText）
                {
                    std::string str = std::to_string(value);
                    std::wstring wstr = internal::Utf8ToWide(str);
                    SetWindowTextW(pInfo->hwnd, wstr.c_str());
                }
                break;
        }
    });
}

// ============================================================
// objsel - オブジェクトに入力フォーカスを設定
// ============================================================
int objsel(OptInt objectId, const std::source_location& location) {
    return safe_call(location, [&]() -> int {
        auto& objMgr = internal::ObjectManager::getInstance();
        
        if (objectId.is_default() || objectId.value() == -1) {
            // 現在のフォーカスを取得
            HWND hwndFocus = GetFocus();
            if (hwndFocus) {
                return objMgr.findObjectByHwnd(hwndFocus);
            }
            return -1;
        }
        
        auto* pInfo = objMgr.getObject(objectId.value());
        if (!pInfo || !pInfo->hwnd) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid object ID", location);
        }
        
        SetFocus(pInfo->hwnd);
        return objectId.value();
    });
}

// ============================================================
// objenable - オブジェクトの有効・無効を設定
// ============================================================
void objenable(int objectId, OptInt enable, const std::source_location& location) {
    safe_call(location, [&] {
        auto& objMgr = internal::ObjectManager::getInstance();
        auto* pInfo = objMgr.getObject(objectId);
        
        if (!pInfo || !pInfo->hwnd) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid object ID", location);
        }
        
        bool isEnabled = enable.value_or(1) != 0;
        pInfo->enabled = isEnabled;
        EnableWindow(pInfo->hwnd, isEnabled ? TRUE : FALSE);
    });
}

// ============================================================
// objskip - オブジェクトのフォーカス移動モードを設定
// ============================================================
void objskip(int objectId, OptInt mode, const std::source_location& location) {
    safe_call(location, [&] {
        auto& objMgr = internal::ObjectManager::getInstance();
        auto* pInfo = objMgr.getObject(objectId);
        
        if (!pInfo || !pInfo->hwnd) {
            throw HspError(ERR_INVALID_HANDLE, "Invalid object ID", location);
        }
        
        int modeVal = mode.value_or(2);
        pInfo->focusSkipMode = modeVal;
        
        // WS_TABSTOPの設定/解除
        LONG_PTR style = GetWindowLongPtrW(pInfo->hwnd, GWL_STYLE);
        if ((modeVal & 3) == 1) {
            style |= WS_TABSTOP;
        } else {
            style &= ~WS_TABSTOP;
        }
        SetWindowLongPtrW(pInfo->hwnd, GWL_STYLE, style);
    });
}

}  // namespace hsppp