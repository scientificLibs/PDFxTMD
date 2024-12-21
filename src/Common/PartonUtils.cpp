#include "Common/PartonUtils.h"

#include "Common/Exception.h"
#include <cmath>
#include <iostream>
namespace fs = std::filesystem;
namespace PDFxTMD
{
std::string StandardPDFNaming(const std::string &pdfName, int set)
{
    return pdfName + "_" + std::to_string(set) + ".dat";
}

std::vector<std::string> GetEnviormentalVariablePaths(const std::string &envVariable,
                                                      char delimiter)
{
    std::vector<std::string> output;
    const char *PDFxTMDEnv = std::getenv(envVariable.c_str());
    if (!std::filesystem::exists(DEFAULT_ENV_PATH))
    {
        std::filesystem::create_directories(DEFAULT_ENV_PATH);
    }
    std::vector<std::string> result;
    result.push_back(DEFAULT_ENV_PATH);
    std::string currentPath = std::filesystem::current_path();
    result.push_back(currentPath);
    if (PDFxTMDEnv == nullptr)
    {
        return result;
    }
    result = split(PDFxTMDEnv, delimiter);
    result.push_back(DEFAULT_ENV_PATH);
    return result;
}

std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}
std::string to_str_zeropad(int val, size_t nchars)
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(static_cast<int>(nchars)) << val;
    return ss.str();
}
std::pair<std::optional<std::string>, ErrorType> StandardPDFSetPath(const std::string &pdfSetName,
                                                                    int set)
{
#if defined(__linux__)
    auto pdfPaths = GetEnviormentalVariablePaths();
    if (pdfPaths.size() == 0)
    {
        return {"", ErrorType::FILE_NOT_FOUND};
    }
    fs::path pdfPath_;
    std::string memname = pdfSetName + "_" + to_str_zeropad(set) + STD_PDF_DATA_EXTENSION;
    for (auto &&pdfPath : pdfPaths)
    {
        pdfPath_ = pdfPath + FOLDER_SEP + pdfSetName + FOLDER_SEP + memname;
        if (!fs::exists(pdfPath_))
        {
            continue;
        }

        return {pdfPath_, ErrorType::None};
    }
#endif
    return {"", ErrorType::FILE_NOT_FOUND};
}
std::pair<std::optional<std::string>, ErrorType> StandardInfoFilePath(const std::string &pdfSetName)
{
#if defined(__linux__)
    auto pdfPaths = GetEnviormentalVariablePaths();
    if (pdfPaths.size() == 0)
    {
        return {"", ErrorType::FILE_NOT_FOUND};
    }
    fs::path pdfPath_;
    for (auto &&pdfPath : pdfPaths)
    {
        pdfPath_ =
            pdfPath + FOLDER_SEP + pdfSetName + FOLDER_SEP + pdfSetName + STD_PDF_INFO_EXTENSION;
        if (!fs::exists(pdfPath_))
        {
            continue;
        }

        return {pdfPath_, ErrorType::None};
    }
#endif
    return {"", ErrorType::FILE_NOT_FOUND};
}

double _extrapolateLinear(double x, double xl, double xh, double yl, double yh)
{
    if (yl > 1e-3 && yh > 1e-3)
    {
        // If yl and yh are sufficiently positive, keep y positive
        // by extrapolating log(y).
        return exp(std::log(yl) + (std::log(x) - std::log(xl)) / (std::log(xh) - std::log(xl)) *
                                      (std::log(yh) - std::log(yl)));
    }
    else
    {
        // Otherwise just extrapolate y itself.
        return yl + (std::log(x) - std::log(xl)) / (std::log(xh) - std::log(xl)) * (yh - yl);
    }
}
} // namespace PDFxTMD
