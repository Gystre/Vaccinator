#include "pch.h"
#include "Bypass.h"
#include "Logger.h"
#include "ProcessUtil.h"

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

	//function hooking shit aka vac3 stop at root

	//inject hack dll

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
