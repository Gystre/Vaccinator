#pragma once

class Bypass {
private:
	//close all processes via their process name
	void closeProcesses(std::vector<std::wstring_view> vecProcesses);

	//little helper function to get the steam dir path
	std::wstring getSteamPath();
public:
	bool start();
};

inline std::unique_ptr<Bypass> bypass;