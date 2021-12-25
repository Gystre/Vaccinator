// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <Psapi.h>
#include <CommCtrl.h>
#include <commdlg.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdint.h>
#include <wchar.h>
#include <TlHelp32.h>

// C++ RunTime Header Files
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <thread>
#include <chrono>
#include <filesystem>

//can just type 25ms instead of longer stuff
using namespace std::chrono_literals;

// Manifest (for modern look on gui)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")


#endif //PCH_H
