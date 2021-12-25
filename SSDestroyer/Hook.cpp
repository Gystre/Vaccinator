#include "pch.h"
#include "Hook.h"
#include "Memory.h"

bool Hook::start() {
	//stop the functions that load and call the vac modules
	auto* const h_steamservice = GetModuleHandleA("steamservice.dll");

	// LOADING
	{
		// 8B CF E8 ? ? ? ? 84 C0 75 0B + 2
		// memory::relative_address((std::uintptr_t)dw_loading, 1)

		auto* const dw_loading = Memory::patternScanIda(h_steamservice, "55 8B EC 83 EC 28 53 56 8B 75 08 8B");
		if (!dw_loading)
		{
			valve::msg("[ SSDestroyer ERROR ] dw_loading is nullptr...\n");
			return false;
		}

		SSDestroyer::o_loading = reinterpret_cast<SSDestroyer::loading_t>(Memory::trampHook(reinterpret_cast<std::uintptr_t*>(dw_loading),
			reinterpret_cast<std::uintptr_t>(SSDestroyer::hk_loading), 6));
	}

	// CALLING
	{
		// E8 ? ? ? ? 89 86 ? ? ? ? E8 ? ? ? ? 6A 00	
		// memory::relative_address((std::uintptr_t)dw_calling, 1)

		auto* const dw_calling = Memory::patternScanIda(h_steamservice, "55 8B EC 6A FF 68 ? ? ? ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 83 EC 6C 53 56");
		if (!dw_calling)
		{
			valve::msg("[ SSDestroyer ERROR ] dw_calling is nullptr...\n");
			return false;
		}

		SSDestroyer::o_calling = reinterpret_cast<SSDestroyer::calling_t>(Memory::trampHook(reinterpret_cast<std::uintptr_t*>(dw_calling),
			reinterpret_cast<std::uintptr_t>(SSDestroyer::hk_calling), 5));
	}

	//now patch the function that loads vac to force loadlibrary
	if (GetModuleHandleW(L"steamservice")) {
		//force it now
		if (!forceLoadLibrary()) {
			valve::msg("[ SSDestroyer ERROR ] couldn't find address for force load library!\n");
			return false;
		}
	}
	else {
		//force it later
		Memory::hookIAT(NULL, "kernel32.dll", "LoadLibraryExW", (PVOID)SSDestroyer::loadLibraryExW_SteamClient);
	}

	valve::msg("[ SSDestroyer ] hooked everything. phew \n");
	return true;
}

bool Hook::forceLoadLibrary() {
	PBYTE toPatch = Memory::patternScanIda(GetModuleHandle(L"steamservice"), "74 47 6A 01 6A");

	if (toPatch) {
		DWORD old;
		VirtualProtect(toPatch, 1, PAGE_EXECUTE_READWRITE, &old);
		*toPatch = 0xEB;
		VirtualProtect(toPatch, 1, old, &old);
		
		return true;
	}

	return false;
}

HMODULE WINAPI SSDestroyer::loadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	HMODULE result = LoadLibraryExW(lpLibFileName, hFile, dwFlags);

	Memory::hookIAT(lpLibFileName, "kernel32.dll", "GetProcAddress", getProcAddress);
	Memory::hookIAT(lpLibFileName, "kernel32.dll", "GetSystemInfo", getSystemInfo);

	return result;
}

HMODULE WINAPI SSDestroyer::loadLibraryExW_SteamClient(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	HMODULE result = LoadLibraryExW(lpLibFileName, hFile, dwFlags);

	if (wcsstr(lpLibFileName, L"steamui.dll")) {
		Memory::hookIAT(lpLibFileName, "kernel32.dll", "LoadLibraryExW", loadLibraryExW_SteamClient);
	}
	else if (wcsstr(lpLibFileName, L"steamservice.dll")) {
		if (hook->forceLoadLibrary()) {
			Memory::hookIAT(L"steamservice", "kernel32.dll", "LoadLibraryExW", loadLibraryExW);
			valve::msg("[ SSDestroyer ] hooked loadLibraryExW for steamservice.dll\n");
		}
	}
	return result;
}

FARPROC WINAPI SSDestroyer::getProcAddress(HMODULE hModule, LPCSTR lpProcName) {
	FARPROC result = GetProcAddress(hModule, lpProcName);

	if (result) {
		if (!strcmp(lpProcName, "GetProcAddress"))
			return (FARPROC)getProcAddress;
		else if (!strcmp(lpProcName, "GetSystemInfo"))
			return (FARPROC)getSystemInfo;
		else if (!strcmp(lpProcName, "GetVersionExA"))
			return (FARPROC)getVersionExA;
		else if (!strcmp(lpProcName, "GetSystemDirectoryW"))
			return (FARPROC)getSystemDirectoryW;
		else if (!strcmp(lpProcName, "GetWindowsDirectoryW"))
			return (FARPROC)getWindowsDirectoryW;
		else if (!strcmp(lpProcName, "GetCurrentProcessId"))
			return (FARPROC)getCurrentProcessId;
		else if (!strcmp(lpProcName, "GetCurrentThreadId"))
			return (FARPROC)getCurrentThreadId;
	}
	return result;
}

VOID WINAPI SSDestroyer::getSystemInfo(LPSYSTEM_INFO lpSystemInfo) {
	GetSystemInfo(lpSystemInfo);
	lpSystemInfo->dwPageSize = 6969;
}


BOOL WINAPI SSDestroyer::getVersionExA(LPOSVERSIONINFOEXA lpVersionInformation) {
	valve::msg("[ SSDestroyer ERROR ] GetVersionExA called! Exiting...\n");
	ExitProcess(1);
	return FALSE;
}

UINT WINAPI SSDestroyer::getSystemDirectoryW(LPWSTR lpBuffer, UINT uSize) {
	valve::msg("[ SSDestroyer ERROR ] GetVersionExA called! Exiting...\n");
	ExitProcess(1);
	return 0;
}

UINT WINAPI SSDestroyer::getWindowsDirectoryW(LPWSTR lpBuffer, UINT uSize) {
	valve::msg("[ SSDestroyer ERROR ] GetWindowsDirectoryW called! Exiting...\n");
	ExitProcess(1);
	return 0;
}

DWORD WINAPI SSDestroyer::getCurrentProcessId(VOID)
{
	return 0;
}

DWORD WINAPI SSDestroyer::getCurrentThreadId(VOID)
{
	return 0;
}

bool __stdcall SSDestroyer::hk_loading(valve::vac_buffer* h_mod, char injection_flags)
{
	const auto nt_header_crc32 = Memory::hashHeader(h_mod->m_pRawModule);
	const auto b_ret = o_loading(h_mod, injection_flags);

	if (nt_header_crc32)
	{
		valve::msg("[ SSDestroyer ] loading module crc32 [ 0x%.8X ].\n", nt_header_crc32);

		//these are the first 2 modules that load 
		if (nt_header_crc32 == 0xCC29049A || nt_header_crc32 == 0x2B8DD987)
			valve::uid_whitelist.push_back(h_mod->m_unCRC32);
	}

	if (h_mod->m_unCRC32 && std::find(valve::uid_whitelist.begin(), valve::uid_whitelist.end(), h_mod->m_unCRC32) != valve::uid_whitelist.end())
		return b_ret;

	//let the module load but fuck the call to runfunc
	if (h_mod->m_pRunFunc)
		h_mod->m_pRunFunc = nullptr;

	return b_ret;
}

int __fastcall SSDestroyer::hk_calling(void* ecx, void* edx, std::uint32_t crc_hash, char injection_mode, int unused1, int id, int param1, int unused2, int param2, int param3, int* param4, int* size_check)
{
	auto status = o_calling(ecx, edx, crc_hash, injection_mode, unused1, id, param1, unused2, param2, param3, param4, size_check);

	//make sure that it succeeds b/c it's not going to since we messed with the loading function
	if (status != valve::SUCCESS && status != valve::OTHER_SUCCESS)
		status = valve::SUCCESS;

	return status;
}
