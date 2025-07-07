#pragma once

#include <fstream>
#include <map>
#include <optional>
#include <sstream> // Required for stringstream, though we change its usage
#include <string>
#include <variant>
#include <vector>

#include "PDFxTMDLib/Common/PartonUtils.h" // Assuming ErrorType is defined here

#include "ryml.hpp"
#include "ryml_std.hpp"
#include <iostream>

namespace PDFxTMD
{
class ConfigWrapper
{
  public:
    enum class Format
    {
        YAML
    };

    // Constructor initializes an empty YAML map
    ConfigWrapper()
    {
        initializeEmptyYAML();
    }

    bool loadFromFile(const std::filesystem::path &filepath, Format format);

    bool loadFromString(const std::string &data_string, Format format);

    template <typename T> std::pair<std::optional<T>, ErrorType> get(const std::string &key) const
    {
        if (data.format == Format::YAML)
        {
            // BUG FIX 1: Use ConstNodeRef because this method is const.
            // This ensures const-correctness.
            ryml::ConstNodeRef root = data.tree.rootref();

            if (!root.is_map())
            {
                return {std::nullopt, ErrorType::CONFIG_KeyNotFound};
            }

            ryml::csubstr ckey(key.data(), key.size());
            if (!root.has_child(ckey))
            {
                return {std::nullopt, ErrorType::CONFIG_KeyNotFound};
            }

            try
            {
                T value;
                // Use the streaming operator to extract the value
                root[ckey] >> value;
                return {value, ErrorType::None};
            }
            catch (const std::exception &)
            {
                return {std::nullopt, ErrorType::CONFIG_ConversionFailed};
            }
        }
        return {std::nullopt, ErrorType::CONFIG_KeyNotFound};
    }

    template <typename T> bool set(const std::string &key, const T &value)
    {
        if (data.format == Format::YAML)
        {
            if (!data.tree.rootref().is_map())
            {
                return false;
            }
            try
            {
                ryml::csubstr ckey(key.data(), key.size());

                // BUG FIX 2: Stream directly to the node.
                // This avoids creating a temporary string and the resulting
                // dangling pointer. ryml will manage the memory.
                data.tree[ckey] << value;
            }
            catch (const std::exception &e)
            {
                // It's generally better to let exceptions propagate or handle them more gracefully
                // than re-throwing as a different type, but we'll keep the original logic.
                throw std::runtime_error(e.what());
            }
            return true;
        }
        return false;
    }

    bool saveToFile(const std::string &filename) const;

    void initializeEmptyYAML()
    {
        data.tree.clear();
        data.tree.rootref() |= ryml::MAP; // Set the root to be a map
        data.format = Format::YAML;
    }

  private:
    struct Data
    {
        Format format;
        ryml::Tree tree;
    } data;
};
} // namespace PDFxTMD