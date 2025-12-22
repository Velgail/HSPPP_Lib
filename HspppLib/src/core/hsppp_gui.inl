// HspppLib/src/core/hsppp_gui.inl
// GUIオブジェクト制御命令の実装

namespace hsppp {

// internal名前空間の関数・変数を使用
using namespace internal;

// ============================================================
// objsize - オブジェクトサイズ設定
// ============================================================
void objsize(OptInt sizeX, OptInt sizeY, OptInt spaceY, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    objMgr.setObjSize(
        sizeX.value_or(64),
        sizeY.value_or(24),
        spaceY.value_or(0)
    );
}

// ============================================================
// objmode - オブジェクトモード設定
// ============================================================
void objmode(OptInt mode, OptInt tabMove, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    objMgr.setObjMode(
        mode.value_or(0),
        tabMove.is_default() ? -1 : tabMove.value()
    );
}

// ============================================================
// objcolor - オブジェクトのカラー設定
// ============================================================
void objcolor(OptInt r, OptInt g, OptInt b, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    objMgr.setObjColor(
        r.value_or(0),
        g.value_or(0),
        b.value_or(0)
    );
}

// ============================================================
// button - ボタン表示
// ============================================================
int button(std::string_view name, std::function<int()> callback, bool isGosub, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    
    // 現在のウィンドウを取得
    int windowId = g_currentScreenId;
    auto* pWindow = getSurface(windowId);
    if (!pWindow) {
        throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
    }
    
    // HspWindowにキャスト
    auto* pHspWindow = dynamic_cast<internal::HspWindow*>(pWindow);
    if (!pHspWindow) {
        throw HspError(ERR_UNSUPPORTED, "Cannot create button on buffer", location);
    }
    
    // オブジェクトサイズを取得
    int objW, objH, objSpace;
    objMgr.getObjSize(objW, objH, objSpace);
    
    // 現在のカレントポジションを取得
    int posX = pWindow->getCurrentX();
    int posY = pWindow->getCurrentY();
    
    // Win32ボタンを作成
    HWND hwndParent = pHspWindow->getHwnd();
    std::wstring wname = internal::Utf8ToWide(name);
    
    HWND hwndButton = CreateWindowExW(
        WS_EX_NOPARENTNOTIFY,  // WS_EX拡張スタイル
        L"BUTTON",
        wname.c_str(),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | BS_PUSHBUTTON,
        posX, posY, objW, objH,
        hwndParent,
        (HMENU)(INT_PTR)(objMgr.getNextId()),  // IDをHMENUとして埋め込み
        GetModuleHandle(nullptr),
        nullptr
    );
    
    // Z-orderを最前面に設定（Direct2D描画との競合対策）
    if (hwndButton) {
        SetWindowPos(hwndButton, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    
    if (!hwndButton) {
        throw HspError(ERR_SYSTEM_ERROR, "Failed to create button", location);
    }
    
    // フォント設定（objmodeに従う）
    int fontMode;
    bool tabEnabled;
    objMgr.getObjMode(fontMode, tabEnabled);
    
    // デフォルトGUIフォントを設定
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hwndButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // ObjectInfoを登録
    internal::ObjectInfo info;
    info.type = internal::ObjectType::Button;
    info.hwnd = hwndButton;
    info.windowId = windowId;
    info.x = posX;
    info.y = posY;
    info.width = objW;
    info.height = objH;
    info.callback = std::move(callback);
    info.isGosub = isGosub;
    info.enabled = true;
    info.focusSkipMode = 1;
    
    int objectId = objMgr.registerObject(info);
    
    // カレントポジションを次の行に移動
    int nextY = posY + std::max(objH, objSpace);
    pWindow->pos(posX, nextY);
    
    return objectId;
}

// ============================================================
// input - 入力ボックス表示（文字列版）
// ============================================================
int input(std::string& var, OptInt sizeX, OptInt sizeY, OptInt maxLen, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    
    // 現在のウィンドウを取得
    int windowId = g_currentScreenId;
    auto* pWindow = getSurface(windowId);
    if (!pWindow) {
        throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
    }
    
    auto* pHspWindow = dynamic_cast<internal::HspWindow*>(pWindow);
    if (!pHspWindow) {
        throw HspError(ERR_UNSUPPORTED, "Cannot create input on buffer", location);
    }
    
    // オブジェクトサイズを取得
    int objW, objH, objSpace;
    objMgr.getObjSize(objW, objH, objSpace);
    
    // サイズの上書き
    int w = sizeX.value_or(objW);
    int h = sizeY.value_or(objH);
    
    // 最大文字数
    int maxChars = maxLen.value_or(static_cast<int>(var.capacity()));
    if (maxChars <= 0) {
        maxChars = 32767;  // Windowsのデフォルト最大
    }
    
    // 現在のカレントポジションを取得
    int posX = pWindow->getCurrentX();
    int posY = pWindow->getCurrentY();
    
    // Win32 EDITを作成
    HWND hwndParent = pHspWindow->getHwnd();
    std::wstring wtext = internal::Utf8ToWide(var);
    
    HWND hwndEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
        L"EDIT",
        wtext.c_str(),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | ES_AUTOHSCROLL,
        posX, posY, w, h,
        hwndParent,
        (HMENU)(INT_PTR)(objMgr.getNextId()),
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (!hwndEdit) {
        throw HspError(ERR_SYSTEM_ERROR, "Failed to create input box", location);
    }
    
    // Z-orderを最前面に設定
    SetWindowPos(hwndEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    // 最大文字数設定
    SendMessageW(hwndEdit, EM_SETLIMITTEXT, (WPARAM)maxChars, 0);
    
    // デフォルトGUIフォントを設定
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // ObjectInfoを登録
    internal::ObjectInfo info;
    info.type = internal::ObjectType::Input;
    info.hwnd = hwndEdit;
    info.windowId = windowId;
    info.x = posX;
    info.y = posY;
    info.width = w;
    info.height = h;
    info.pStrVar = &var;
    info.maxLength = maxChars;
    info.enabled = true;
    info.focusSkipMode = 1;
    
    int objectId = objMgr.registerObject(info);
    
    // カレントポジションを次の行に移動
    int nextY = posY + std::max(h, objSpace);
    pWindow->pos(posX, nextY);
    
    return objectId;
}

// ============================================================
// input - 入力ボックス表示（整数版）
// ============================================================
int input(int& var, OptInt sizeX, OptInt sizeY, OptInt maxLen, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    
    // 現在のウィンドウを取得
    int windowId = g_currentScreenId;
    auto* pWindow = getSurface(windowId);
    if (!pWindow) {
        throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
    }
    
    auto* pHspWindow = dynamic_cast<internal::HspWindow*>(pWindow);
    if (!pHspWindow) {
        throw HspError(ERR_UNSUPPORTED, "Cannot create input on buffer", location);
    }
    
    // オブジェクトサイズを取得
    int objW, objH, objSpace;
    objMgr.getObjSize(objW, objH, objSpace);
    
    int w = sizeX.value_or(objW);
    int h = sizeY.value_or(objH);
    int maxChars = maxLen.value_or(32);
    
    int posX = pWindow->getCurrentX();
    int posY = pWindow->getCurrentY();
    
    HWND hwndParent = pHspWindow->getHwnd();
    std::wstring wtext = std::to_wstring(var);
    
    HWND hwndEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
        L"EDIT",
        wtext.c_str(),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | ES_AUTOHSCROLL | ES_NUMBER,
        posX, posY, w, h,
        hwndParent,
        (HMENU)(INT_PTR)(objMgr.getNextId()),
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (!hwndEdit) {
        throw HspError(ERR_SYSTEM_ERROR, "Failed to create input box", location);
    }
    
    // Z-orderを最前面に設定
    SetWindowPos(hwndEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    SendMessageW(hwndEdit, EM_SETLIMITTEXT, (WPARAM)maxChars, 0);
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    internal::ObjectInfo info;
    info.type = internal::ObjectType::Input;
    info.hwnd = hwndEdit;
    info.windowId = windowId;
    info.x = posX;
    info.y = posY;
    info.width = w;
    info.height = h;
    info.pIntVar = &var;
    info.maxLength = maxChars;
    info.enabled = true;
    info.focusSkipMode = 1;
    
    int objectId = objMgr.registerObject(info);
    
    int nextY = posY + std::max(h, objSpace);
    pWindow->pos(posX, nextY);
    
    return objectId;
}

// ============================================================
// mesbox - メッセージボックス表示
// ============================================================
int mesbox(std::string& var, OptInt sizeX, OptInt sizeY, OptInt style, OptInt maxLen, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    
    int windowId = g_currentScreenId;
    auto* pWindow = getSurface(windowId);
    if (!pWindow) {
        throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
    }
    
    auto* pHspWindow = dynamic_cast<internal::HspWindow*>(pWindow);
    if (!pHspWindow) {
        throw HspError(ERR_UNSUPPORTED, "Cannot create mesbox on buffer", location);
    }
    
    int objW, objH, objSpace;
    objMgr.getObjSize(objW, objH, objSpace);
    
    int w = sizeX.value_or(objW);
    int h = sizeY.value_or(objH);
    int styleVal = style.value_or(1);  // デフォルト: 編集可能
    int maxChars = maxLen.value_or(-1);
    
    if (maxChars < 0) {
        maxChars = static_cast<int>(var.capacity());
    }
    if (maxChars == 0) {
        maxChars = 32767;
    }
    
    int posX = pWindow->getCurrentX();
    int posY = pWindow->getCurrentY();
    
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_CLIPSIBLINGS | ES_MULTILINE;
    
    // スタイル解析
    if ((styleVal & 1) == 0) {
        dwStyle |= ES_READONLY;
    }
    if (styleVal & 4) {
        dwStyle |= WS_HSCROLL;
    }
    if ((styleVal & 8) == 0) {
        dwStyle |= ES_AUTOVSCROLL;
    }
    
    HWND hwndParent = pHspWindow->getHwnd();
    
    // 改行文字を\nから\r\nに変換（Windows標準）
    std::string convertedText = var;
    size_t pos = 0;
    while ((pos = convertedText.find('\n', pos)) != std::string::npos) {
        if (pos == 0 || convertedText[pos - 1] != '\r') {
            convertedText.insert(pos, "\r");
            pos += 2;  // \r\nの次へ
        } else {
            pos += 1;  // 既に\r\nなら次へ
        }
    }
    std::wstring wtext = internal::Utf8ToWide(convertedText);
    
    HWND hwndEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
        L"EDIT",
        wtext.c_str(),
        dwStyle,
        posX, posY, w, h,
        hwndParent,
        (HMENU)(INT_PTR)(objMgr.getNextId()),
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (!hwndEdit) {
        throw HspError(ERR_SYSTEM_ERROR, "Failed to create mesbox", location);
    }
    
    // Z-orderを最前面に設定
    SetWindowPos(hwndEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    SendMessageW(hwndEdit, EM_SETLIMITTEXT, (WPARAM)maxChars, 0);
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    internal::ObjectInfo info;
    info.type = internal::ObjectType::Mesbox;
    info.hwnd = hwndEdit;
    info.windowId = windowId;
    info.x = posX;
    info.y = posY;
    info.width = w;
    info.height = h;
    info.pStrVar = &var;
    info.maxLength = maxChars;
    info.enabled = true;
    info.focusSkipMode = 1;
    
    int objectId = objMgr.registerObject(info);
    
    int nextY = posY + std::max(h, objSpace);
    pWindow->pos(posX, nextY);
    
    return objectId;
}

// ============================================================
// chkbox - チェックボックス表示
// ============================================================
int chkbox(std::string_view label, int& var, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    
    int windowId = g_currentScreenId;
    auto* pWindow = getSurface(windowId);
    if (!pWindow) {
        throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
    }
    
    auto* pHspWindow = dynamic_cast<internal::HspWindow*>(pWindow);
    if (!pHspWindow) {
        throw HspError(ERR_UNSUPPORTED, "Cannot create chkbox on buffer", location);
    }
    
    int objW, objH, objSpace;
    objMgr.getObjSize(objW, objH, objSpace);
    
    int posX = pWindow->getCurrentX();
    int posY = pWindow->getCurrentY();
    
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
    
    // Z-orderを最前面に設定
    SetWindowPos(hwndCheck, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    // 初期状態を設定
    SendMessageW(hwndCheck, BM_SETCHECK, var ? BST_CHECKED : BST_UNCHECKED, 0);
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hwndCheck, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    internal::ObjectInfo info;
    info.type = internal::ObjectType::Chkbox;
    info.hwnd = hwndCheck;
    info.windowId = windowId;
    info.x = posX;
    info.y = posY;
    info.width = objW;
    info.height = objH;
    info.pStateVar = &var;
    info.enabled = true;
    info.focusSkipMode = 1;
    
    int objectId = objMgr.registerObject(info);
    
    int nextY = posY + std::max(objH, objSpace);
    pWindow->pos(posX, nextY);
    
    return objectId;
}

// ============================================================
// combox - コンボボックス表示
// ============================================================
int combox(int& var, OptInt expandY, std::string_view items, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    
    int windowId = g_currentScreenId;
    auto* pWindow = getSurface(windowId);
    if (!pWindow) {
        throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
    }
    
    auto* pHspWindow = dynamic_cast<internal::HspWindow*>(pWindow);
    if (!pHspWindow) {
        throw HspError(ERR_UNSUPPORTED, "Cannot create combox on buffer", location);
    }
    
    int objW, objH, objSpace;
    objMgr.getObjSize(objW, objH, objSpace);
    int expandYVal = expandY.value_or(100);
    
    int posX = pWindow->getCurrentX();
    int posY = pWindow->getCurrentY();
    
    HWND hwndParent = pHspWindow->getHwnd();
    
    // コンボボックスはドロップダウン時の高さを含めたサイズで作成
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
    
    // Z-orderを最前面に設定
    SetWindowPos(hwndCombo, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    // アイテムを追加（\n区切り）
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
    
    // 初期選択を設定
    if (var >= 0) {
        SendMessageW(hwndCombo, CB_SETCURSEL, var, 0);
    }
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hwndCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    internal::ObjectInfo info;
    info.type = internal::ObjectType::Combox;
    info.hwnd = hwndCombo;
    info.windowId = windowId;
    info.x = posX;
    info.y = posY;
    info.width = objW;
    info.height = objH;
    info.pStateVar = &var;
    info.enabled = true;
    info.focusSkipMode = 1;
    
    int objectId = objMgr.registerObject(info);
    
    int nextY = posY + std::max(objH, objSpace);
    pWindow->pos(posX, nextY);
    
    return objectId;
}

// ============================================================
// listbox - リストボックス表示
// ============================================================
int listbox(int& var, OptInt expandY, std::string_view items, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    
    int windowId = g_currentScreenId;
    auto* pWindow = getSurface(windowId);
    if (!pWindow) {
        throw HspError(ERR_INVALID_HANDLE, "Invalid window ID", location);
    }
    
    auto* pHspWindow = dynamic_cast<internal::HspWindow*>(pWindow);
    if (!pHspWindow) {
        throw HspError(ERR_UNSUPPORTED, "Cannot create listbox on buffer", location);
    }
    
    int objW, objH, objSpace;
    objMgr.getObjSize(objW, objH, objSpace);
    int height = expandY.value_or(100);
    
    int posX = pWindow->getCurrentX();
    int posY = pWindow->getCurrentY();
    
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
    
    // Z-orderを最前面に設定
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
    
    // 初期選択を設定
    if (var >= 0) {
        SendMessageW(hwndList, LB_SETCURSEL, var, 0);
    }
    
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hwndList, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    internal::ObjectInfo info;
    info.type = internal::ObjectType::Listbox;
    info.hwnd = hwndList;
    info.windowId = windowId;
    info.x = posX;
    info.y = posY;
    info.width = objW;
    info.height = height;
    info.pStateVar = &var;
    info.enabled = true;
    info.focusSkipMode = 1;
    
    int objectId = objMgr.registerObject(info);
    
    int nextY = posY + std::max(height, objSpace);
    pWindow->pos(posX, nextY);
    
    return objectId;
}

// ============================================================
// clrobj - オブジェクトをクリア
// ============================================================
void clrobj(OptInt startId, OptInt endId, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    
    int start = startId.value_or(0);
    int end = endId.value_or(-1);
    
    objMgr.removeObjects(start, end);
}

// ============================================================
// objprm - オブジェクトの内容を変更（文字列版）
// ============================================================
void objprm(int objectId, std::string_view value, const std::source_location& location) {
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
            if (pInfo->pStrVar) {
                *pInfo->pStrVar = std::string(value);
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
}

// ============================================================
// objprm - オブジェクトの内容を変更（整数版）
// ============================================================
void objprm(int objectId, int value, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    auto* pInfo = objMgr.getObject(objectId);
    
    if (!pInfo || !pInfo->hwnd) {
        throw HspError(ERR_INVALID_HANDLE, "Invalid object ID", location);
    }
    
    switch (pInfo->type) {
        case internal::ObjectType::Input:
            if (pInfo->pIntVar) {
                *pInfo->pIntVar = value;
                SetWindowTextW(pInfo->hwnd, std::to_wstring(value).c_str());
            } else if (pInfo->pStrVar) {
                *pInfo->pStrVar = std::to_string(value);
                SetWindowTextW(pInfo->hwnd, std::to_wstring(value).c_str());
            }
            SetFocus(pInfo->hwnd);
            break;
            
        case internal::ObjectType::Chkbox:
            SendMessageW(pInfo->hwnd, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0);
            if (pInfo->pStateVar) {
                *pInfo->pStateVar = value ? 1 : 0;
            }
            break;
            
        case internal::ObjectType::Combox:
            SendMessageW(pInfo->hwnd, CB_SETCURSEL, value, 0);
            if (pInfo->pStateVar) {
                *pInfo->pStateVar = value;
            }
            break;
            
        case internal::ObjectType::Listbox:
            SendMessageW(pInfo->hwnd, LB_SETCURSEL, value, 0);
            if (pInfo->pStateVar) {
                *pInfo->pStateVar = value;
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
}

// ============================================================
// objsel - オブジェクトに入力フォーカスを設定
// ============================================================
int objsel(OptInt objectId, const std::source_location& location) {
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
}

// ============================================================
// objenable - オブジェクトの有効・無効を設定
// ============================================================
void objenable(int objectId, OptInt enable, const std::source_location& location) {
    auto& objMgr = internal::ObjectManager::getInstance();
    auto* pInfo = objMgr.getObject(objectId);
    
    if (!pInfo || !pInfo->hwnd) {
        throw HspError(ERR_INVALID_HANDLE, "Invalid object ID", location);
    }
    
    bool isEnabled = enable.value_or(1) != 0;
    pInfo->enabled = isEnabled;
    EnableWindow(pInfo->hwnd, isEnabled ? TRUE : FALSE);
}

// ============================================================
// objskip - オブジェクトのフォーカス移動モードを設定
// ============================================================
void objskip(int objectId, OptInt mode, const std::source_location& location) {
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
}

}  // namespace hsppp