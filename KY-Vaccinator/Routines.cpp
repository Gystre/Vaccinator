#include "pch.h"
#include "MainDialog.h"

/// <summary>
/// Load configuration from file
/// </summary>
/// <returns>Error code</returns>
DWORD MainDlg::LoadConfig(const std::wstring& path /*= L""*/)
{
    auto& cfg = _profileMgr.config();
    if (_profileMgr.Load(path))
    {
        // Image
        for (auto& path : cfg.images)
            LoadImageFile(path);

        // Update profile name
        if (!_defConfig.empty())
            _status.SetText(0, blackbone::Utils::StripPath(_defConfig));
    }

    UpdateInterface();
    return ERROR_SUCCESS;
}


/// <summary>
/// Save Configuration.
/// </summary>
/// <returns>Error code</returns>
DWORD MainDlg::SaveConfig(const std::wstring& path /*= L""*/)
{
    auto& cfg = _profileMgr.config();

    cfg.images.clear();
    for (auto& img : _images)
        cfg.images.emplace_back(img->path());

    _profileMgr.Save(path);
    return ERROR_SUCCESS;
}

/// <summary>
/// Load selected image and do some validation
/// </summary>
/// <param name="path">Full qualified image path</param>
/// <returns>Error code </returns>
DWORD MainDlg::LoadImageFile(const std::wstring& path)
{
    std::shared_ptr<blackbone::pe::PEImage> img(new blackbone::pe::PEImage);
    blackbone::pe::vecExports exports;

    // Check if image is already in the list
    if (std::find_if(_images.begin(), _images.end(),
        [&path](std::shared_ptr<blackbone::pe::PEImage>& img) { return path == img->path(); }) != _images.end())
    {
        Message::ShowInfo(_hwnd, L"Image '" + path + L"' is already in the list");
        return ERROR_ALREADY_EXISTS;
    }

    // Check if image is a PE file
    if (!NT_SUCCESS(img->Load(path)))
    {
        std::wstring errstr = std::wstring(L"File \"") + path + L"\" is not a valid PE image";
        Message::ShowError(_hwnd, errstr.c_str());

        img->Release();
        return ERROR_INVALID_IMAGE_HASH;
    }

    // In case of pure IL, list all methods
    if (img->pureIL() && img->net().Init(path))
    {
        blackbone::ImageNET::mapMethodRVA methods;
        img->net().Parse(&methods);

        for (auto& entry : methods)
        {
            std::wstring name = entry.first.first + L"." + entry.first.second;
            exports.push_back(blackbone::pe::ExportData(blackbone::Utils::WstringToAnsi(name), 0));
        }
    }
    // Simple exports otherwise
    else
        img->GetExports(exports);


    // Add to internal lists
    AddToModuleList(img);
    _exports.emplace_back(exports);

    //if (_procList.selection() != -1)
    //    _inject.enable();

    img->Release(true);
    return ERROR_SUCCESS;
}

/// <summary>
/// Add module to module list
/// </summary>
/// <param name="path">Loaded image</param>
/// <param name="exports">Module exports</param>
void MainDlg::AddToModuleList(std::shared_ptr<blackbone::pe::PEImage>& img)
{
    const wchar_t* platfom = nullptr;

    // Module platform
    if (img->mType() == blackbone::mt_mod32)
        platfom = L"32 bit";
    else if (img->mType() == blackbone::mt_mod64)
        platfom = L"64 bit";
    else
        platfom = L"Unknown";

    _images.emplace_back(img);
    _modules.AddItem(blackbone::Utils::StripPath(img->path()), 0, { platfom });
}

/// <summary>
/// Invoke Open/Save file dialog
/// </summary>
/// <param name="filter">File filter</param>
/// <param name="defIndex">Default filter index</param>
/// <param name="selectedPath">Target file path</param>
/// <param name="bSave">true to Save file, false to open</param>
/// <param name="defExt">Default file extension for file save</param>
/// <returns>true on success, false if operation was canceled by user</returns>
bool MainDlg::OpenSaveDialog(
    const wchar_t* filter,
    int defIndex,
    std::wstring& selectedPath,
    bool bSave /*= false*/,
    const std::wstring& defExt /*= L""*/
)
{
    OPENFILENAMEW ofn = { 0 };
    wchar_t path[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = path;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = defIndex;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    if (!bSave)
    {
        ofn.Flags = OFN_PATHMUSTEXIST;
    }
    else
    {
        ofn.Flags = OFN_OVERWRITEPROMPT;
        ofn.lpstrDefExt = defExt.c_str();
    }

    auto res = bSave ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
    if (res)
        selectedPath = path;

    return res != FALSE;
}

