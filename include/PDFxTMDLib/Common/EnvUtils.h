#pragma once

#include <cstddef>
#include <cstdlib>
#include <string>

namespace PDFxTMD
{
inline bool env_enabled(const char *name, bool default_value = false)
{
    const char *value = std::getenv(name);
    if (value == nullptr || *value == '\0')
    {
        return default_value;
    }
    const std::string text(value);
    return text != "0" && text != "false" && text != "FALSE" && text != "off" && text != "OFF";
}

inline std::size_t env_positive_size(const char *name, std::size_t fallback)
{
    const char *value = std::getenv(name);
    if (value == nullptr || *value == '\0')
    {
        return fallback;
    }
    try
    {
        const unsigned long long parsed = std::stoull(value);
        return parsed > 0 ? static_cast<std::size_t>(parsed) : fallback;
    }
    catch (...)
    {
        return fallback;
    }
}

inline std::size_t env_size_allow_zero(const char *name, std::size_t fallback)
{
    const char *value = std::getenv(name);
    if (value == nullptr || *value == '\0')
    {
        return fallback;
    }
    try
    {
        return static_cast<std::size_t>(std::stoull(value));
    }
    catch (...)
    {
        return fallback;
    }
}

inline std::string env_string(const char *name)
{
    const char *value = std::getenv(name);
    return value == nullptr ? std::string{} : std::string(value);
}
} // namespace PDFxTMD
