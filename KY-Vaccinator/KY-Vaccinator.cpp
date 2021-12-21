#include "pch.h"
#include "framework.h"
#include "Resource.h"
#include "MainDialog.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
#ifdef _DEBUG
    AllocConsole();
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    SetConsoleTitleA("ezezez");
#endif

    MainDlg mainDlg;
    int ret = mainDlg.RunModeless();

#ifdef _DEBUG
    fclose((FILE*)stdin);
    fclose((FILE*)stdout);
    FreeConsole();
#endif

    return ret;
}
