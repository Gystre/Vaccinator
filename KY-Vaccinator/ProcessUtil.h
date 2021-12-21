#pragma once

namespace ProcessUtil {
	// Open process with CreateProcess
	bool openProcess(std::wstring_view path, const std::vector<std::wstring> arguments, PROCESS_INFORMATION& pi);

	// Checks if process is opened
	bool isProcessOpen(const std::vector<std::pair<std::uint32_t, std::wstring>>& vecProcesses, std::wstring_view strProc);

	//check if one process is open
	bool isProcessOpen(const std::wstring_view proc);

	// Kills target process
	bool killProcess(const std::vector<std::pair<std::uint32_t, std::wstring>>& vecProcesses, std::wstring_view strProc);

	// Returns the process id from process name
	std::uint32_t getProcessIdByName(const std::vector<std::pair<std::uint32_t, std::wstring>>& vecProcceses, std::wstring_view strProc);

	// Gets the process list
	std::vector<std::pair<std::uint32_t, std::wstring>> getProcessList();
}