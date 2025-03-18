#pragma once

#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/external/fkYAML/node.hpp"

namespace PDFxTMD
{
class ConfigWrapper
{
  public:
    enum class Format
    {
        YAML
    };

    // Modified constructor to initialize data
    ConfigWrapper() : data{Format::YAML, fkyaml::node::mapping()} {}

    bool loadFromFile(const std::filesystem::path &filepath, Format format)
    {
        if (format == Format::YAML)
        {
            std::ifstream ifs(filepath);
            if (!ifs.is_open())
                return false;
            try
            {
                data.yaml = fkyaml::node::deserialize(ifs);
                if (!data.yaml.is_mapping())
                {
                    data.yaml = fkyaml::node::mapping();
                    data.format = format;
                    return false;
                }
                if (data.yaml.size() == 0)
                {
                    ifs.close();
                    std::ofstream ofs(filepath, std::ios::trunc);
                    ofs << "{}";
                    return false;
                }
                data.format = format;
                return true;
            }
            catch (const std::exception &)
            {
                data.yaml = fkyaml::node::mapping();
                data.format = format;
                return false;
            }
        }
        return false;
    }

    bool loadFromString(const std::string &data_string, Format format)
    {
        if (format == Format::YAML)
        {
            try
            {
                std::istringstream iss(data_string);
                data.yaml = fkyaml::node::deserialize(iss);
                // Check if the parsed node is a mapping
                if (!data.yaml.is_mapping())
                {
                    data.yaml = fkyaml::node::mapping(); // Reset to empty mapping
                    data.format = format;
                    return false; // Indicate failure
                }
                data.format = format;
                return true; // Success: it’s a mapping
            }
            catch (const std::exception &)
            {
                data.yaml = fkyaml::node::mapping();
                data.format = format;
                return false; // Exception means failure
            }
        }
        return false;
    }
    template <typename T>
    std::pair<std::optional<T>, ErrorType> get(const std::string &key) const
    {
        if (data.format == Format::YAML)
        {
            try
            {
                if (data.yaml.contains(key))
                {
                    T value = data.yaml[key].get_value<T>();
                    return {value, ErrorType::None};
                }
                return {std::nullopt, ErrorType::CONFIG_KeyNotFound};
            }
            catch (const std::exception &)
            {
                return {std::nullopt, ErrorType::CONFIG_ConversionFailed};
            }
        }
        return {std::nullopt, ErrorType::CONFIG_KeyNotFound};
    }

    // New method to set a value for a key
    template <typename T>
    bool set(const std::string& key, const T& value)
    {
        if (data.format == Format::YAML)
        {
            try
            {
                if (data.yaml == nullptr)
                    return false;
                data.yaml[key] = value;
            }
            catch (const std::exception & e)
            {
                throw std::runtime_error(e.what());
            }
            return true;
        }
        return false;
    }

    // New method to save the configuration to a YAML file
    bool saveToFile(const std::string& filename) const
    {
        std::ofstream ofs(filename);
        if (!ofs.is_open())
        {
            return false; // Failed to open file
        }
        try
        {
            auto serializedContent = fkyaml::node::serialize(data.yaml);
            ofs << serializedContent;
            ofs.close();
            return true; // Successfully wrote to file
        }
        catch (const std::exception&)
        {
            return false; // Serialization failed
        }
    }

    void initializeEmptyYAML()
    {
        data.yaml = fkyaml::node::mapping();
        data.format = Format::YAML;
    }

  private:
    struct Data
    {
        Format format;
        fkyaml::node yaml;
    } data;
};
} // namespace PDFxTMD