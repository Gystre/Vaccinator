#pragma once

#include "Valve.h"

class Hook {
public:
	// patch jnz to jmp byte in steamservice.dll to force vac modules to load via load library
	bool forceLoadLibrary();

	bool start();
};

namespace SSDestroyer {
	using loading_t = bool(__stdcall*)(valve::vac_buffer*, char);
	inline loading_t o_loading;
	inline bool __stdcall hk_loading(valve::vac_buffer*, char);

	using calling_t = int(__fastcall*)(void*, void*, std::uint32_t, char, int, int, int, int, int, int, int*, int*);
	inline calling_t o_calling;
	inline int __fastcall hk_calling(void*, void*, std::uint32_t, char, int, int, int, int, int, int, int*, int*);

	// force forceLoadLibrary() for steamui.dll as well as forceLoadLibrary() for 
	inline HMODULE WINAPI loadLibraryExW_SteamClient(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

	// inhibit GetProcAddress and GetSystemInfo for any modules loaded with loadLibrary inside of steam.exe
	inline HMODULE WINAPI loadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

	// GetProcAddress gives us access to a wide array of functions that we can get rid of
	inline FARPROC WINAPI getProcAddress(HMODULE hModule, LPCSTR lpProcName);

	// mess with the page size to stop
	inline VOID WINAPI getSystemInfo(LPSYSTEM_INFO lpSystemInfo);

	// not really sure why we break on these functions but I assume it's b/c vac has found our cheat and is trying to figure out some info about it
	inline BOOL WINAPI getVersionExA(LPOSVERSIONINFOEXA lpVersionInformation);
	inline UINT WINAPI getSystemDirectoryW(LPWSTR lpBuffer, UINT uSize);
	inline UINT WINAPI getWindowsDirectoryW(LPWSTR lpBuffer, UINT uSize);

	// mess with the return value of these functions to keep us hidden a little better
	inline DWORD WINAPI getCurrentProcessId(VOID);
	inline DWORD WINAPI getCurrentThreadId(VOID);

	inline std::unique_ptr<Hook> hook;
}

