#pragma once

#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/external/rapidyaml/rapidyaml-0.9.0.hpp"
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
            // CORRECTED LINE: Use ConstNodeRef because this method is const
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

                if constexpr (std::is_convertible_v<T, std::string_view>)
                {
                    data.tree[ckey] = value;
                }
                else
                {
                    std::stringstream ss;
                    ss << value;
                    data.tree[ckey] = ryml::to_csubstr(ss.str());
                }
            }
            catch (const std::exception &e)
            {
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
        data.tree.rootref() |= ryml::MAP;
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