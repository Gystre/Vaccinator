#pragma once

#include <src/BlackBone/ManualMap/MMap.h>

class Bypass {
private:
	//close all processes via their process name
	void closeProcesses(std::vector<std::wstring_view> vecProcesses);

	//map the dll stored inside vecBuffer into the process strProc
	//wait for wstrModName to exist before continuing
	bool map(std::wstring_view strProc, std::wstring_view wstrModName, std::vector<std::uint8_t> vecBuffer, blackbone::eLoadFlags flags);

	//little helper function to get the steam dir path
	std::wstring getSteamPath();
public:
	bool start();
};

inline std::unique_ptr<Bypass> bypass;