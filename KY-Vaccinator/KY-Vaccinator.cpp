#include "pch.h"
#include "framework.h"
#include "Resource.h"
#include "MainDialog.h"
#include "Logger.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    MainDlg mainDlg;

    return mainDlg.RunModeless();
}
