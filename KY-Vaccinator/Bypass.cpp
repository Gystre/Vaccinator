#include "pch.h"
#include "Bypass.h"
#include "Logger.h"
#include "ProcessUtil.h"
#include "FileUtil.h"
#include "MainDialog.h"
#include <src/BlackBone/Process/Process.h>

bool Bypass::start() {
	logDebug(L"Closing processes");

	//close all procs that are related to vac, add more procs here if needed
	std::vector<std::wstring_view> steamProcs = { L"steam.exe", L"steamservice.exe", L"steamwebhelper.exe" };
	std::vector<std::wstring_view> gameProcs = { L"csgo.exe", L"tf2.exe" };

	//combine them lol
	std::vector<std::wstring_view> procs;
	procs.reserve(steamProcs.size() + gameProcs.size());
	procs.insert(procs.end(), steamProcs.begin(), steamProcs.end());
	procs.insert(procs.end(), gameProcs.begin(), gameProcs.end());
	closeProcesses(procs);

	const auto steamPath = getSteamPath();

	if (steamPath.empty()) {
		logError(L"Failed to get steam path!");
		return false;
	}

	//open the game automatically, TODO: implement via gui
	std::wstring launchAppend = {};

	//get the ids of the games we want to launch automatically ex. {730, "csgo.exe"}
	//if (vars::b_open_game_automatically)
	//{
	//	for (const auto& it : this->vec_app_ids)
	//	{
	//		if (it.second.find(string::to_unicode(vars::str_process_name)) != std::wstring::npos)
	//			launch_append = string::format(L"-applaunch %d", it.first);
	//	}
	//}

	//open steam
	logDebug(L"Opening steam at - %s", steamPath.data());

	PROCESS_INFORMATION pi; // Could use the current handle instead of closing it for steam, might do it in the future...
	bool openedSteam = ProcessUtil::openProcess(steamPath, { L"-console", launchAppend }, pi);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	if (!openedSteam) {
		logError(L"Failed to open Steam!");
		return false;
	}

	if (ProcessUtil::isProcessOpen(L"steamservice.exe")) {
		logError(L"steamservice is running, steam did not run as admin");
		return false;
	}

	//steamservice.dll hooking
	std::filesystem::path bypassPath = FileUtil::getCwd() + L"\\SSDestroyer.dll";

	if (!std::filesystem::exists(bypassPath)) {
		logError(L"Couldn't find SSDestroyer.dll, make sure it's in the same directory as the exe");
		return false;
	}

	std::vector<std::uint8_t> bypassBuffer;
	if (!FileUtil::readFile(bypassPath, &bypassBuffer)) {
		logError(L"Failed to load SSDestroyer.dll to memory");
		return false;
	}

	if (!map(L"steam.exe", L"tier0_s.dll", bypassBuffer, blackbone::eLoadFlags::WipeHeader)) {
		logError(L"Mapping the SSDestroyer.dll failed");
		return false;
	}

	//wait for game to load
	ProcessUtil::waitForProcess(L"csgo.exe");

	logDebug(L"Found process, injecting...");

	//load and write all the modules into memory
	auto images = mainDlg->getImages();
	std::vector<std::vector<std::uint8_t>> hackBuffer(images.size());

	for (int i = 0; i < images.size(); i++) {
		if (!FileUtil::readFile(images[i]->path(), &hackBuffer[i])) {
			logError(L"Failed to load %s to memory", images[i]->name());
		}
		if (!map(L"csgo.exe", L"serverbrowser.dll", hackBuffer[i], blackbone::eLoadFlags::WipeHeader)) {
			logError(L"Failed to map %s to memory", images[i]->name());
		}
	}
	
	logOk(L"Done!");

    //we're done here, time to leave :D
    exit(EXIT_SUCCESS);

	return true;
}

void Bypass::closeProcesses(std::vector<std::wstring_view> vecProcesses) {
	auto procList = ProcessUtil::getProcessList();
	for (const auto& proc : vecProcesses)
	{
		do
		{
			ProcessUtil::killProcess(procList, proc);

			procList = ProcessUtil::getProcessList();
			std::this_thread::sleep_for(25ms);
		} while (ProcessUtil::isProcessOpen(procList, proc));
	}
}

const auto getSystem32Directory = []() -> std::wstring
{
	wchar_t buf[MAX_PATH];
	GetSystemDirectory(buf, sizeof(buf) / 4);

	return std::wstring(buf);
};

bool Bypass::map(std::wstring_view strProc, std::wstring_view wstrModName, std::vector<std::uint8_t> vecBuffer, blackbone::eLoadFlags flags) {
	// Update process list while process is not opened
	ProcessUtil::waitForProcess(strProc);

	auto procList = ProcessUtil::getProcessList();
	blackbone::Process bb_proc;
	bb_proc.Attach(ProcessUtil::getProcessIdByName(procList, strProc), PROCESS_ALL_ACCESS);

	if (bb_proc.core().handle() == nullptr) {
		logError(L"Handle to %s was null! Restart the process as an administrator.", strProc);
		return false;
	}

	// Wait for a process module so we can continue with injection
	logDebug(L"Waiting for module %s in %s...", wstrModName.data(), strProc.data());

	auto mod_ready = false;
	while (!mod_ready)
	{
		for (const auto& mod : bb_proc.modules().GetAllModules())
		{
			if (mod.first.first == wstrModName)
			{
				mod_ready = true;
				break;
			}
		}

		if (mod_ready)
			break;

		std::this_thread::sleep_for(500ms); // 1s? fixes 0xC34... (i think i was calling the patch too early now)
	}

	// modifies .text section, ez vac ban LOL
	// plus don't need to LoadLibrary anything after this
	// Bypassing injection block by csgo (-allow_third_party_software)
	//if (strProc.find(L"csgo") != std::wstring::npos)
	//{
	//	const auto patch_nt_open_file = [&]()
	//	{
	//		const auto ntdll_path = string::format(L"%s\\ntdll.dll", getSystem32Directory().data());
	//		const auto ntdll = LoadLibrary(ntdll_path.data());

	//		if (!ntdll) {
	//			logError(L"Failed to load ntdll?");
	//			return false;
	//		}
	//		

	//		void* ntopenfile_ptr = GetProcAddress(ntdll, "NtOpenFile");

	//		if (!ntopenfile_ptr) {
	//			logError(L"Failed to get NtOpenFile proc address?");
	//			return false;
	//		}

	//		std::uint8_t restore[5];
	//		std::memcpy(restore, ntopenfile_ptr, sizeof(restore));

	//		const auto result = bb_proc.memory().Write((std::uintptr_t)ntopenfile_ptr, restore);

	//		if (!NT_SUCCESS(result)) {
	//			logError(L"Failed to write patch memory!");
	//			return false;
	//		}

	//		return true;
	//	};

	//	if (!patch_nt_open_file()) {
	//		logError(L"Failed to patch NtOpenFile!");
	//		return false;
	//	}
	//}

	// Resolve PE imports
	const auto mod_callback = [](blackbone::CallbackType type, void*, blackbone::Process&, const blackbone::ModuleData& modInfo)
	{
		if (type == blackbone::PreCallback)
		{
			if (modInfo.name == L"user32.dll")
				return blackbone::LoadData(blackbone::MT_Native, blackbone::Ldr_Ignore);
		}

		return blackbone::LoadData(blackbone::MT_Default, blackbone::Ldr_Ignore);
	};

	// Mapping dll to the process
	const auto call_result = bb_proc.mmap().MapImage(vecBuffer.size(), vecBuffer.data(), false, flags, mod_callback);

	if (!call_result.success())
	{
		// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
		logError(L"Failed to inject into %s. NT Status: 0x%.8X", strProc.data(), call_result.status);
		bb_proc.Detach();

		return false;
	}

	// Free memory and detach from process
	bb_proc.Detach();
	logOk(L"Injected into %s successfully!", strProc.data());

	return true;
}

std::wstring Bypass::getSteamPath() {
	HKEY h_key{};
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &h_key) != ERROR_SUCCESS)
	{
		RegCloseKey(h_key);
		return {};
	}

	wchar_t steam_path_reg[MAX_PATH]{}; steam_path_reg[0] = '"';
	DWORD steam_path_size = sizeof(steam_path_reg) - sizeof(char);

	if (RegQueryValueEx(h_key, L"SteamExe", nullptr, nullptr, (LPBYTE)(steam_path_reg + 1), &steam_path_size) != ERROR_SUCCESS)
	{
		RegCloseKey(h_key);
		return {};
	}

	RegCloseKey(h_key);

	return std::wstring(steam_path_reg) + L"\"";

}
