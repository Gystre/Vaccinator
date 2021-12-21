#include "pch.h"
#include "MainDialog.h"

#include <shellapi.h>

#include <src/BlackBone/Misc/Utils.h>


MainDlg::MainDlg()
    : Dialog(IDD_MAIN)
{
    _messages[WM_INITDIALOG] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnInit);
    _messages[WM_COMMAND] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnCommand);
    _messages[WM_CLOSE] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnClose);
    _messages[WM_DROPFILES] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnDragDrop);

    _events[IDC_ADD_MOD] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnLoadImage);
    _events[IDC_REMOVE_MOD] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnRemoveImage);
    _events[IDC_CLEAR_MODS] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnClearImages);

    _events[IDM_ABOUT] = static_cast<Dialog::fnDlgProc>(&MainDlg::OnAbout);
}

MainDlg::~MainDlg()
{
}

INT_PTR MainDlg::OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    Dialog::OnInit(hDlg, message, wParam, lParam);

    //
    // Setup controls
    //
    _modules.Attach(_hwnd, IDC_MODS);

    // Setup modules view
    _modules.AddColumn(L"Name", 150, 0);
    _modules.AddColumn(L"Architecture", 100, 1);

    ListView_SetExtendedListViewStyle(_modules.hwnd(), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    // Set dialog title
    SetWindowTextW(_hwnd, blackbone::Utils::RandomANString().c_str());

    // Set icon
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    DestroyIcon(hIcon);


    // Setup status bar
    _status.Attach(CreateWindowW(STATUSCLASSNAME, L"", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 10, hDlg, NULL, GetModuleHandle(NULL), NULL));

    _status.SetParts({ 150, 200, -1 });
    _status.SetText(0, L"Default profile");

#ifdef USE64
    _status.SetText(1, L"x64");
#else
    _status.SetText(1, L"x86");
#endif
    _status.SetText(2, L"Idle");

    LoadConfig(_defConfig);

    return TRUE;
}

INT_PTR MainDlg::OnDragDrop(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    wchar_t path[MAX_PATH] = { 0 };
    HDROP hDrop = (HDROP)wParam;

    if (DragQueryFile(hDrop, 0, path, ARRAYSIZE(path)) != 0)
    {
        LoadImageFile(path);
    }

    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR MainDlg::OnAbout(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), _hwnd, About);

    return 0;
}

/// <summary>
/// Update interface controls
/// </summary>
/// <returns>Error code</returns>
DWORD MainDlg::UpdateInterface()
{
    // Disable inject button if no images available 
    if (_images.empty())
        _start.disable();
    else
       _start.enable();

    return ERROR_SUCCESS;
}

INT_PTR MainDlg::OnClose(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!_images.empty())
        SaveConfig();

    return Dialog::OnClose(hDlg, message, wParam, lParam);
}

INT_PTR MainDlg::OnLoadImage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::wstring path;
    EnableWindow(GetDlgItem(hDlg, IDC_ADD_MOD), FALSE);
    bool res = OpenSaveDialog(
        L"All (*.*)\0*.*\0Dynamic link library (*.dll)\0*.dll\0System driver (*.sys)\0*.sys\0",
        2,
        path
    );

    // Reset init routine upon load
    LoadImageFile(path);

    EnableWindow(GetDlgItem(hDlg, IDC_ADD_MOD), TRUE);
    return TRUE;
}

INT_PTR MainDlg::OnRemoveImage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto idx = _modules.selection();
    if (idx >= 0)
    {
        _modules.RemoveItem(idx);
        _images.erase(_images.begin() + idx);
        _exports.erase(_exports.begin() + idx);
    }

    // Disable inject if no images
    if (_images.empty())
        _start.disable();

    return TRUE;
}

INT_PTR MainDlg::OnClearImages(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    _profileMgr.config().images.clear();
    _images.clear();
    _exports.clear();
    _modules.reset();
    _start.disable();

    return TRUE;
}
