#include "Common/PartonUtils.h"
#include "Common/Exception.h"
#include "Common/FileUtils.h"
#include <cmath>
#include <iostream>
#include <set>
#include <unistd.h>

namespace fs = std::filesystem;
namespace PDFxTMD
{
std::string StandardPDFNaming(const std::string &pdfName, int set)
{
    return pdfName + "_" + std::to_string(set) + ".dat";
}

std::vector<std::string> splitPaths(const std::string &paths)
{
    std::vector<std::string> result;
    size_t start = 0, end;
    while ((end = paths.find(':', start)) != std::string::npos)
    {
        result.push_back(paths.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(paths.substr(start));
    return result;
}
bool hasWriteAccess(const std::string &path)
{
    return std::filesystem::exists(path) && std::filesystem::is_directory(path) &&
           (access(path.c_str(), W_OK) == 0);
}
std::vector<std::string> GetEnviormentalVariablePaths(const std::string &envVariable,
                                                      char delimiter)
{
    std::vector<std::string> output;
    const char *PDFxTMDEnv = std::getenv(envVariable.c_str());
    if (!FileUtils::Exists(DEFAULT_ENV_PATH))
    {
        if (FileUtils::HasUserAccess(FileUtils::ParentDir(DEFAULT_ENV_PATH)))
        {
            FileUtils::CreateDirs(DEFAULT_ENV_PATH);
        }
    }
    std::set<std::string> result;
    result.emplace(DEFAULT_ENV_PATH);
    std::string currentPath = std::filesystem::current_path();
    result.emplace(currentPath);
    if (PDFxTMDEnv == nullptr)
    {
        return std::vector(result.begin(), result.end());
    }
    auto notDefaultPaths = split(PDFxTMDEnv, delimiter);
    for (auto &&notDefaultPath : notDefaultPaths)
    {
        if (FileUtils::HasUserAccess(FileUtils::ParentDir(notDefaultPath)))
        {
            if (!FileUtils::Exists(notDefaultPath))
                FileUtils::CreateDirs(notDefaultPath);
        }
        result.emplace(notDefaultPath);
    }
    return std::vector(result.begin(), result.end());
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
