#pragma once

#include "resource.h"
#include "Dialog.h"
#include "Message.h"
#include "Config.h"
#include "Button.h"
#include "StatusBar.h"
#include "ListView.h"
#include <src/BlackBone/PE/PEImage.h>

typedef std::vector<std::shared_ptr<blackbone::pe::PEImage>> vecPEImages;
typedef std::vector<blackbone::pe::vecExports> vecImageExports;

class MainDlg : public Dialog
{
public:
    enum StartAction
    {
        Nothing = 0,
        LoadProfile,
        RunProfile
    };

public:
    MainDlg();
    ~MainDlg();

private:
    /// <summary>
	/// Load configuration from file
	/// </summary>
	/// <returns>Error code</returns>
    DWORD LoadConfig(const std::wstring& path = L"");

    /// <summary>
    /// Save Configuration.
    /// </summary>
    /// <returns>Error code</returns>
    DWORD SaveConfig(const std::wstring& path = L"");


    /// <summary>
    /// Update interface controls
    /// </summary>
    /// <returns>Error code</returns>
    DWORD UpdateInterface();

    /// <summary>
    /// Load selected image and do some validation
    /// </summary>
    /// <param name="path">Full qualified image path</param>
    /// <returns>Error code </returns>
    DWORD LoadImageFile(const std::wstring& path);

    /// <summary>
    /// Add module to module list
    /// </summary>
    /// <param name="path">Loaded image</param>
    void AddToModuleList(std::shared_ptr<blackbone::pe::PEImage>& img);

    /// <summary>
	/// Invoke Open/Save file dialog
	/// </summary>
	/// <param name="filter">File filter</param>
	/// <param name="defIndex">Default filter index</param>
	/// <param name="selectedPath">Target file path</param>
	/// <param name="bSave">true to Save file, false to open</param>
	/// <param name="defExt">Default file extension for file save</param>
	/// <returns>true on success, false if operation was canceled by user</returns>
    bool OpenSaveDialog(
        const wchar_t* filter,
        int defIndex,
        std::wstring& selectedPath,
        bool bSave = false,
        const std::wstring& defExt = L""
    );

    //
    // Message handlers
    //
    MSG_HANDLER(OnInit);
    MSG_HANDLER(OnClose);
    MSG_HANDLER(OnLoadImage);
    MSG_HANDLER(OnRemoveImage);
    MSG_HANDLER(OnClearImages);
    MSG_HANDLER(OnDragDrop);
    MSG_HANDLER(OnStart);

    MSG_HANDLER(OnAbout);

private:
    std::wstring    _defConfig;     // Profile to load on start
    vecPEImages     _images;        // Loaded module exports
    Config      _profileMgr;    // Configuration manager
    vecImageExports _exports;       // Exports list

    //
    // Interface controls
    //  
    ctrl::ListView _modules;        // Image list

    ctrl::StatusBar _status;        // Status bar

    ctrl::Button _start;           // Start button
};