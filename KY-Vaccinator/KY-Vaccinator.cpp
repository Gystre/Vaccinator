#include "pch.h"
#include "framework.h"
#include "Resource.h"
#include "MainDialog.h"
#include "Logger.h"
#include "Bypass.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    //init globals
    mainDlg = std::make_unique<MainDlg>();
    bypass = std::make_unique<Bypass>();

    return mainDlg->RunModeless();
}
