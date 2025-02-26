#include "Common/PartonUtils.h"
#include "Common/Exception.h"
#include "Common/FileUtils.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#if defined(_WIN32)
#include <Windows.h>
#endif

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
    try
    {
        // Get the file status
        std::filesystem::file_status file_status = std::filesystem::status(path);

        // Check if the file exists
        if (!std::filesystem::exists(file_status))
        {
            std::cout << "File does not exist." << std::endl;
            return false;
        }

        // Get the permissions
        std::filesystem::perms permissions = file_status.permissions();

        // Check if the write permission is set for the owner
        if ((permissions & std::filesystem::perms::owner_write) != std::filesystem::perms::none)
        {
            return true;
        }

        // Optionally, check for group and others write permissions
        if ((permissions & std::filesystem::perms::group_write) != std::filesystem::perms::none ||
            (permissions & std::filesystem::perms::others_write) != std::filesystem::perms::none)
        {
            return true;
        }

        return false;
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}
std::vector<std::string> GetEnviormentalVariablePaths(const std::string &envVariable,
                                                      char delimiter)
{
#if defined(__linux__)
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
    std::string currentPath = std::filesystem::current_path().string();
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
#elif defined(_WIN32)
    HKEY hKey;
    LONG result;

    // Create or open the registry key
    result =
        RegCreateKeyExA(HKEY_CURRENT_USER, PDF_X_TMD_REGISTRY_PATH, 0, nullptr,
                        REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, nullptr, &hKey, nullptr);
    if (result != ERROR_SUCCESS)
    {
        throw std::runtime_error("[PDFxTMD][GetEnviormentalVariablePaths] Fatal error: PDFxTMD "
                                 "registry key cannot be created!");
    }

    DWORD dataSize = 0;
    result = RegQueryValueExA(hKey, ENV_PATH_VARIABLE, nullptr, nullptr, nullptr, &dataSize);

    std::vector<char> buffer;

    if (result == ERROR_SUCCESS && dataSize > 0)
    {
        // The registry value exists and has data
        buffer.resize(dataSize);
        result = RegQueryValueExA(hKey, ENV_PATH_VARIABLE, nullptr, nullptr,
                                  reinterpret_cast<LPBYTE>(buffer.data()), &dataSize);
        if (result != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return {};
        }
    }
    else if (result == ERROR_FILE_NOT_FOUND)
    {
        // The registry value does not exist; initialize it with DEFAULT_ENV_PATH
        std::string defaultEnvPathString = DEFAULT_ENV_PATH;
        buffer.resize(defaultEnvPathString.size() + 2); // Include null terminators
        std::copy(defaultEnvPathString.begin(), defaultEnvPathString.end(), buffer.begin());
        buffer[defaultEnvPathString.size()] = '\0';
        buffer[defaultEnvPathString.size() + 1] = '\0';

        result = RegSetValueExA(hKey, ENV_PATH_VARIABLE, 0, REG_MULTI_SZ,
                                reinterpret_cast<const BYTE *>(buffer.data()),
                                static_cast<DWORD>(buffer.size()));
        if (result != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            throw std::runtime_error("[PDFxTMD][GetEnviormentalVariablePaths] Fatal error: Failed "
                                     "to initialize ENV_PATH_VARIABLE registry value!");
        }
    }

    // Parse the MULTI_SZ value into a vector of strings
    std::vector<std::string> paths;
    const char *current = buffer.data();
    while (current && *current)
    {
        paths.emplace_back(current);
        current += strlen(current) + 1;
    }

    // Ensure the DEFAULT_ENV_PATH is always included
    std::set<std::string> uniquePaths(paths.begin(), paths.end());
    if (uniquePaths.find(DEFAULT_ENV_PATH) == uniquePaths.end())
    {
        uniquePaths.emplace(DEFAULT_ENV_PATH);
    }

    // Remove inaccessible paths and ensure directories exist
    std::vector<std::string> validPaths;
    bool isDefaultPathFound = true;
    for (const auto &path : uniquePaths)
    {
        if (std::filesystem::exists(path) || std::filesystem::create_directories(path))
        {
            isDefaultPathFound = false;
            validPaths.push_back(path);
        }
    }

    // Rebuild the MULTI_SZ value with valid paths
    std::vector<char> newMultiSz;
    for (const auto &path : validPaths)
    {
        newMultiSz.insert(newMultiSz.end(), path.begin(), path.end());
        newMultiSz.push_back('\0');
    }
    newMultiSz.push_back('\0'); // Double null-terminate
    if (!isDefaultPathFound)
    {
        result = RegSetValueExA(hKey, ENV_PATH_VARIABLE, 0, REG_MULTI_SZ,
                                reinterpret_cast<const BYTE *>(newMultiSz.data()),
                                static_cast<DWORD>(newMultiSz.size()));
        if (result != ERROR_SUCCESS)
        {
            std::cerr << "Failed to write updated registry value: " << ENV_PATH_VARIABLE << '\n';
        }
    }

    RegCloseKey(hKey);
    return validPaths;
#endif
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

        return {pdfPath_.string(), ErrorType::None};
    }
    return {"", ErrorType::FILE_NOT_FOUND};
}
std::pair<std::optional<std::string>, ErrorType> StandardInfoFilePath(const std::string &pdfSetName)
{
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

        return {pdfPath_.string(), ErrorType::None};
    }
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

size_t indexbelow(double value, const std::vector<double> &knots)
{
    size_t i = std::upper_bound(knots.begin(), knots.end(), value) - knots.begin();
    if (i == knots.size())
        i -= 1; // can't return the last knot index
    i -= 1;     // step back to get the knot <= x behaviour
    return i;
}
} // namespace PDFxTMD
