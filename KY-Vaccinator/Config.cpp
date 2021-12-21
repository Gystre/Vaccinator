#include "pch.h"
#include "Config.h"
#include <src/BlackBone/Misc/Utils.h>

#define CURRENT_PROFILE L"\\Vaccinator_default.xml"

bool Config::Save(const std::wstring& path /*= L""*/)
{
    try
    {
        auto filepath = path.empty() ? (blackbone::Utils::GetExeDirectory() + CURRENT_PROFILE) : path;

        acut::XmlDoc<wchar_t> xml;
        xml.create_document();

        for (auto& imgpath : _config.images)
            xml.append(L"Vaccinator.imagePath").value(imgpath);


        xml.create_document();
        xml.write_document(filepath);

        return true;
    }
    catch (const std::runtime_error&)
    {
        return false;
    }
}

bool Config::Load(const std::wstring& path /*= L""*/)
{
    try
    {
        auto filepath = path.empty() ? (blackbone::Utils::GetExeDirectory() + CURRENT_PROFILE) : path;
        if (!acut::file_exists(filepath))
            return false;

        acut::XmlDoc<wchar_t> xml;
        xml.read_from_file(filepath);

        // Load images in a safe way
        if (xml.has(L"Vaccinator.imagePath"))
        {
            auto nodes = xml.all_nodes_named(L"Vaccinator.imagePath");
            for (auto node : nodes)
                _config.images.emplace_back(node.value());
        }
        return true;
    }
    catch (const std::runtime_error&)
    {
        return false;
    }

}
