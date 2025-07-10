#define RYML_SINGLE_HDR_DEFINE_NOW
#include "PDFxTMDLib/external/rapidyaml/rapidyaml-0.9.0.hpp"
#include "PDFxTMDLib/Common/ConfigWrapper.h"
#include <filesystem>

namespace PDFxTMD
{
// Your saveToFile is correct, no changes needed here.
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
            // FIX: Use parse_in_arena to copy the data into the tree's own memory.
            // We can parse directly from the string without needing a separate buffer.
            data.tree = ryml::parse_in_arena(ryml::to_csubstr(data_string));

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
        {
            return false;
        }

        std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)),
                                 std::istreambuf_iterator<char>());

        // No need to null-terminate when passing size, but it doesn't hurt.

        try
        {
            // FIX: Use parse_in_arena to copy the data into the tree's own memory.
            data.tree = ryml::parse_in_arena(ryml::csubstr(buffer.data(), buffer.size()));

            if (!data.tree.rootref().is_map())
            {
                initializeEmptyYAML();
                return false;
            }

            // NOTE: This block below is strange. It modifies the input file on a read
            // operation if it's empty, which is unexpected. Consider removing it.
            if (data.tree.rootref().num_children() == 0)
            {
                // This logic is questionable. You might want to remove it.
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
} // namespace PDFxTMD