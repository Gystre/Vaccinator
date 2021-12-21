#pragma once

#include <consoleapi.h>
#include <shared_mutex>
#include <processthreadsapi.h>
#include <consoleapi2.h>
#include <WinUser.h>
#include <iostream>

namespace string {
	// Format string
	template<typename ... arg>
	static std::wstring format(std::wstring_view fmt, arg ... args)
	{
		const int size = std::swprintf(nullptr, NULL, fmt.data(), args ...) + 1;
		const auto buf = std::make_unique<wchar_t[]>(size);
		std::swprintf(buf.get(), size, fmt.data(), args ...);

		return std::wstring(buf.get(), (buf.get() + size) - 1);
	}
}

enum class msg_type_t : std::uint32_t
{
	LNONE = 0,
	LDEBUG = 9,		/* blue */
	LSUCCESS = 10,	/* green */
	LERROR = 12,	/* red */
	LPROMPT = 11,	/* pink */
	LWARN = 14		/* yellow */
};

inline std::ostream& operator<< (std::ostream& os, const msg_type_t type)
{
	switch (type)
	{
	case msg_type_t::LDEBUG:	return os << ".";
	case msg_type_t::LSUCCESS:	return os << "+";
	case msg_type_t::LERROR:	return os << "!";
	case msg_type_t::LPROMPT:	return os << ">";
	case msg_type_t::LWARN:		return os << "*";
	default: return os << "";
	}
}

class Logger
{
private:
	std::shared_timed_mutex mutex{};

public:
	Logger(std::wstring_view title_name = {})
	{
		AllocConsole();
		AttachConsole(GetCurrentProcessId());

		if (!title_name.empty())
			SetConsoleTitle(title_name.data());

		FILE* conin, * conout;

		freopen_s(&conin, "conin$", "r", stdin);
		freopen_s(&conout, "conout$", "w", stdout);
		freopen_s(&conout, "conout$", "w", stderr);
	}

	~Logger()
	{
		const auto handle = FindWindow(L"ConsoleWindowClass", nullptr);
		ShowWindow(handle, SW_HIDE);
		FreeConsole();
	}

	template< typename ... arg >
	void print(const msg_type_t type, const std::string_view func, std::wstring_view format, arg ... args)
	{
		static auto* h_console = GetStdHandle(STD_OUTPUT_HANDLE);
		std::unique_lock<decltype(mutex)> lock(mutex);

		const auto formatted = string::format(format, args ...);

		if (type != msg_type_t::LNONE)
		{
			SetConsoleTextAttribute(h_console, static_cast<WORD>(type));
			std::wcout << L"[";
			std::cout << type;
			std::wcout << L"] ";

#ifdef _DEBUG
			SetConsoleTextAttribute(h_console, 15 /* white */);
			std::wcout << L"[ ";
			SetConsoleTextAttribute(h_console, static_cast<WORD>(type));
			std::cout << func;
			SetConsoleTextAttribute(h_console, 15 /* white */);
			std::wcout << L" ] ";
#endif
		}

		if (type == msg_type_t::LDEBUG)
			SetConsoleTextAttribute(h_console, 8 /* gray */);
		else
			SetConsoleTextAttribute(h_console, 15 /* white */);

		if (type == msg_type_t::LPROMPT)
			std::wcout << formatted;
		else
			std::wcout << formatted << L"\n";
	}
};

inline auto g_logger = Logger(L"ezezez");
#define logDebug(...)	g_logger.print( msg_type_t::LDEBUG, __FUNCTION__, __VA_ARGS__ )
#define logOk(...)		g_logger.print( msg_type_t::LSUCCESS, __FUNCTION__, __VA_ARGS__ )
#define logError(...)	g_logger.print( msg_type_t::LERROR, __FUNCTION__, __VA_ARGS__ )
#define logPrompt(...) g_logger.print( msg_type_t::LPROMPT, __FUNCTION__, __VA_ARGS__ )
#define logWarn(...)	g_logger.print( msg_type_t::LWARN, __FUNCTION__, __VA_ARGS__ )
#define logRaw(...)	g_logger.print( msg_type_t::LNONE, __FUNCTION__, __VA_ARGS__ )
