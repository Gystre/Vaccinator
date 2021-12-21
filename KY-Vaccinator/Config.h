#pragma once

#include "rapidxml_wrap.hpp";

class Config {
public:
    typedef std::vector<std::wstring> vecPaths;
    struct ConfigData
    {
        vecPaths images;                // Dll paths
    };

    bool Save(const std::wstring& path = L"");
    bool Load(const std::wstring& path = L"");

    inline ConfigData& config() { return _config; }

private:
    ConfigData _config;
};
