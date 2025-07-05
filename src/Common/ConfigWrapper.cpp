#define RYML_SINGLE_HDR_DEFINE_NOW
#include "PDFxTMDLib/Common/ConfigWrapper.h"
namespace PDFxTMD
{
bool ConfigWrapper::saveToFile(const std::string &filename) const
{
    std::ofstream ofs(filename);
    if (!ofs.is_open())
    {
        return false;
    }
    try
    {
        ofs << data.tree;
        ofs.close();
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
    }
    bool ConfigWrapper::loadFromString(const std::string &data_string, Format format)
    {
        if (format == Format::YAML)
        {
            try
            {
                std::vector<char> buffer(data_string.begin(), data_string.end());
                buffer.push_back('\0');

                data.tree = ryml::parse_in_place(ryml::to_substr(buffer));

                if (!data.tree.rootref().is_map())
                {
                    initializeEmptyYAML();
                    return false;
                }

                data.format = format;
                return true;
            }
            catch (const std::exception &)
            {
                initializeEmptyYAML();
                return false;
            }
        }
        return false;
    }
    bool ConfigWrapper::loadFromFile(const std::filesystem::path &filepath, Format format)
    {
        if (format == Format::YAML)
        {
            std::ifstream ifs(filepath, std::ios::binary);
            if (!ifs.is_open())
                return false;

            // Read the entire file into a buffer for in-place parsing
            std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)),
                                     std::istreambuf_iterator<char>());
            buffer.push_back('\0'); // Null-terminate for safety

            try
            {
                data.tree = ryml::parse_in_place(ryml::to_substr(buffer));

                if (!data.tree.rootref().is_map())
                {
                    initializeEmptyYAML();
                    return false;
                }

                if (data.tree.rootref().num_children() == 0)
                {
                    ifs.close();
                    std::ofstream ofs(filepath, std::ios::trunc);
                    ofs << "{}";
                    return false;
                }

                data.format = format;
                return true;
            }
            catch (const std::exception &e)
            {
                std::cout << "[PDFxTMD] " << e.what() << std::endl;
                initializeEmptyYAML();
                return false;
            }
        }
        return false;
    }
}