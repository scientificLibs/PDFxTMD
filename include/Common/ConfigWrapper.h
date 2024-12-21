#pragma once
#include "yaml-cpp/yaml.h"

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "PartonUtils.h"

namespace PDFxTMD
{
class ConfigWrapper
{
  public:
    enum class Format
    {
        YAML
    };

    ConfigWrapper() = default;

    bool loadFromFile(const std::string &filename, Format format)
    {
        switch (format)
        {
        case Format::YAML:
            try
            {
                data.yaml = YAML::LoadFile(filename);
                data.format = format; // Set the format after loading
                return true;
            }
            catch (const YAML::Exception &e)
            {
                return false;
            }
        }
        return false;
    }

    bool loadFromString(const std::string &data_string, Format format)
    {
        switch (format)
        {
        case Format::YAML:
            try
            {
                data.yaml = YAML::Load(data_string);
                data.format = format; // Set the format after loading
                return true;
            }
            catch (const YAML::Exception &e)
            {
                return false;
            }
        }
        return false;
    }

    template <typename T> std::pair<std::optional<T>, ErrorType> get(const std::string &key) const
    {
        if (data.format == Format::YAML)
        {
            if (data.yaml[key])
            {
                try
                {
                    return {data.yaml[key].as<T>(), ErrorType::None};
                }
                catch (const YAML::Exception &)
                {
                    return {std::nullopt, ErrorType::CONFIG_ConversionFailed};
                }
            }
            else
            {
                return {std::nullopt, ErrorType::CONFIG_KeyNotFound};
            }
        }
        return {std::nullopt, ErrorType::CONFIG_KeyNotFound}; // Return if format
                                                              // is wrong
    }

    template <typename T>
    std::pair<std::optional<std::map<std::string, T>>, ErrorType> getMap(
        const std::string &key) const
    {
        std::map<std::string, T> result;
        if (data.format == Format::YAML)
        {
            if (data.yaml[key])
            {
                const YAML::Node &node = data.yaml[key];
                for (const auto &pair : node)
                {
                    try
                    {
                        result[pair.first.as<std::string>()] = pair.second.as<T>();
                    }
                    catch (const YAML::Exception &)
                    {
                        return {std::nullopt, ErrorType::CONFIG_ConversionFailed}; // Return
                                                                                   // if
                                                                                   // any
                                                                                   // conversion
                                                                                   // fails
                    }
                }
                return {result, ErrorType::None}; // Return the populated map
            }
            else
            {
                return {std::nullopt, ErrorType::CONFIG_KeyNotFound}; // Key does
                                                                      // not exist
            }
        }
        return {std::nullopt, ErrorType::CONFIG_KeyNotFound}; // Return if format
                                                              // is wrong
    }

  private:
    struct Data
    {
        Format format;
        YAML::Node yaml;

    } data;
};
} // namespace PDFxTMD