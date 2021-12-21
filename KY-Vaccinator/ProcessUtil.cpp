#include "pch.h"
#include "ProcessUtil.h"

namespace string
{
	std::wstring to_lower(std::wstring str)
	{
		std::transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(::tolower));
		return str;
	}
}

namespace ProcessUtil {
	bool openProcess(std::wstring_view path, const std::vector<std::wstring> arguments, PROCESS_INFORMATION& pi)
	{
		STARTUPINFO si;
		{
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
		}

		ZeroMemory(&pi, sizeof(pi));

		std::wstring params{};
		params += path;

		for (const auto& argument : arguments)
			params += (L" " + argument);

		return CreateProcess(nullptr, params.data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);
	}

	bool isProcessOpen(const std::vector<std::pair<std::uint32_t, std::wstring>>& vec_processes, std::wstring_view str_proc)
	{
		if (vec_processes.empty())
			return {};

		if (str_proc.empty())
			return {};

		auto target = string::to_lower(str_proc.data());
		for (const auto& ctx : vec_processes)
		{
			auto ep = string::to_lower(ctx.second);
			if (target.find(L".exe") == std::wstring::npos)
			{
				if (ep.find(target) == std::wstring::npos)
					continue;
			}
			else
			{
				if (ep != target)
					continue;
			}

			const auto h_process = OpenProcess(PROCESS_VM_READ, false, ctx.first);
			if (h_process != nullptr)
			{
				CloseHandle(h_process);
				return true;
			}
		}

		return {};
	}

	bool isProcessOpen(const std::wstring_view proc) {
		auto procList = ProcessUtil::getProcessList();
		return isProcessOpen(procList, proc);
	}

	bool killProcess(const std::vector<std::pair<std::uint32_t, std::wstring>>& vec_processes, std::wstring_view str_proc)
	{
		if (vec_processes.empty())
			return {};

		if (str_proc.empty())
			return {};

		auto executed = false;
		auto target = string::to_lower(str_proc.data());
		for (const auto& ctx : vec_processes)
		{
			auto ep = string::to_lower(ctx.second);
			if (target.find(L".exe") == std::wstring::npos)
			{
				if (ep.find(target) == std::wstring::npos)
					continue;
			}
			else
			{
				if (ep != target)
					continue;
			}

			const auto h_process = OpenProcess(PROCESS_TERMINATE, false, ctx.first);
			if (h_process != nullptr)
			{
				TerminateProcess(h_process, 9);
				CloseHandle(h_process);

				executed = true;
			}
		}

		return executed;
	}

	std::uint32_t getProcessIdByName(const std::vector<std::pair<std::uint32_t, std::wstring>>& vec_processes, std::wstring_view str_proc)
	{
		if (vec_processes.empty())
			return {};

		if (str_proc.empty())
			return {};

		auto target = string::to_lower(str_proc.data());
		for (const auto& ctx : vec_processes)
		{
			auto ep = string::to_lower(ctx.second);
			if (target.find(L".exe") == std::wstring::npos)
			{
				if (ep.find(target) == std::wstring::npos)
					continue;
			}
			else
			{
				if (ep != target)
					continue;
			}

			return ctx.first;
		}

		return {};
	}

	std::vector<std::pair<std::uint32_t, std::wstring>> getProcessList()
	{
		std::vector<std::pair<std::uint32_t, std::wstring>> vec_list{};

		const auto h_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		PROCESSENTRY32 m_entry{};
		m_entry.dwSize = sizeof(m_entry);

		if (!Process32First(h_handle, &m_entry))
			return {};

		while (Process32Next(h_handle, &m_entry))
			vec_list.emplace_back(m_entry.th32ProcessID, m_entry.szExeFile);

		CloseHandle(h_handle);

		return vec_list;
	}
}