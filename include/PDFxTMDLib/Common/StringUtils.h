#pragma once
// https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring
#include <algorithm>
#include <cctype>
#include <locale>

namespace PDFxTMD
{
inline void ltrim(std::string &s)
{
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}
inline void trim(std::string &s)
{
    rtrim(s);
    ltrim(s);
}

inline std::string ltrim_copy(std::string s)
{
    ltrim(s);
    return s;
}

inline std::string rtrim_copy(std::string s)
{
    rtrim(s);
    return s;
}

inline std::string trim_copy(std::string s)
{
    trim(s);
    return s;
}
inline std::string ToLower(const std::string &str)
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

// Function to convert a string to uppercase
inline std::string ToUpper(const std::string &str)
{
    std::string upperStr = str;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    return upperStr;
}
inline bool StartsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}
} // namespace PDFxTMD
