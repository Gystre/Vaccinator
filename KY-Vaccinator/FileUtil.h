#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include <vector>
#include <memory>

#define FileUtil_SLASH L'\\'
#define FileUtil_SLASH_STR L"\\"
#define FileUtil_SLASH_UTF8 '\\'
#define FileUtil_SLASH_STR_UTF8 "\\"

namespace FileUtil
{
    // suitable only for reading small text files into a buffer
    bool read_file(const std::wstring& path, std::string& buffer);
    bool read_file(const std::wstring& path, std::vector<char>& buffer);
    bool read_file(const std::wstring& path, std::wstring& buffer);
    bool read_file(const std::wstring& path, std::vector<wchar_t>& buffer);

    bool file_exists(const std::wstring& filename);

    bool readFile(const std::filesystem::path& path, std::vector<std::uint8_t>* out_buffer);
    std::wstring getCwd();
}


