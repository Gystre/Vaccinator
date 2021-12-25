#include "pch.h"
#include "FileUtil.h"
#include "StringUtil.h"

#include <fstream>

#include <ShlObj.h>

template <typename Container>
bool read_helper(const std::wstring& path, Container& container)
{
    std::basic_ifstream<typename Container::value_type> f(path, std::ios_base::binary);

    if (!f.good())
        return false;

    container.assign((std::istreambuf_iterator<typename Container::value_type>(f)),
                      std::istreambuf_iterator<typename Container::value_type>());
    container.push_back(FileUtil::ensure_tchar<typename Container::value_type>('\0'));

    return true;
}


bool FileUtil::read_file(const std::wstring& path, std::string& buffer)
{
    return read_helper(path, buffer);
}

bool FileUtil::read_file(const std::wstring& path, std::vector<char>& buffer)
{
    return read_helper(path, buffer);
}

bool FileUtil::read_file(const std::wstring& path, std::wstring& buffer)
{
    return read_helper(path, buffer);
}

bool FileUtil::read_file(const std::wstring& path, std::vector<wchar_t>& buffer)
{
    return read_helper(path, buffer);
}

bool FileUtil::file_exists( const std::wstring& filename )
{
    return (GetFileAttributesW( filename.c_str() ) != INVALID_FILE_ATTRIBUTES);
}

bool FileUtil::readFile(const std::filesystem::path& path, std::vector<std::uint8_t>* out_buffer) {
    std::ifstream file(path, std::ios::binary);
    if (file.fail())
        return false;

    out_buffer->assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    return true;

}

std::wstring FileUtil::getCwd() {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}