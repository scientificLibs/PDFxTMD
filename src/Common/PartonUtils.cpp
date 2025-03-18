#include "PDFxTMDLib/Common/ConfigWrapper.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/FileUtils.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <fstream>

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
std::vector<std::string> GetPDFxTMDPathsAsVector()
{
    std::cout << "[RAMIN] salam" << std::endl;
    std::vector<std::string> output;
    auto PDFxTMDPath = GetPDFxTMDPaths();
    std::cout << "[RAMIN] GetPDFxTMDPathsAsVector " << PDFxTMDPath;
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
    auto notDefaultPaths = split(PDFxTMDPath, '|');
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
    auto pdfPaths = GetPDFxTMDPathsAsVector();
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
    auto pdfPaths = GetPDFxTMDPathsAsVector();
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

std::string GetPDFxTMDPaths()
{
    std::string rootPath;
#if defined(_WIN32)
    rootPath = "C:/ProgramData/PDFxTMDLib";
#else

    const char *homeDir = std::getenv("HOME");
    if (!homeDir)
    {
        return false;
    }
    rootPath = homeDir + "/.PDFxTMDLib";
#endif
    std::string configFilePath = rootPath + "/config.yaml";
    if (!std::filesystem::exists(rootPath) && !std::filesystem::create_directories(rootPath))
    {
        throw std::runtime_error("[PDFxTMD] The program is not functional. It needs path" +
                                 rootPath + " directory to be created.");
    }

    PDFxTMD::ConfigWrapper config;

    if (!std::filesystem::exists(configFilePath) || std::filesystem::file_size(configFilePath) == 0)
    {

        std::ofstream ofs(configFilePath);
        ofs << "paths: " << std::endl;
        ofs.close();
        return "";
    }
    config.loadFromFile(configFilePath, PDFxTMD::ConfigWrapper::Format::YAML);

    auto pathsPair = config.get<std::string>("paths");
    std::string delimitedPaths;
    if (pathsPair.first.has_value())
    {
        delimitedPaths = pathsPair.first.value();
    }
    
    return delimitedPaths;
}

bool AddPathToEnvironment(const std::string &newPath)
{
    std::string rootPath;
#if defined(_WIN32)
    rootPath = "C:/ProgramData/PDFxTMDLib";
#else

    const char *homeDir = std::getenv("HOME");
    if (!homeDir)
    {
        return false;
    }
    rootPath = homeDir + "/.PDFxTMDLib";
#endif
    std::string configFilePath = rootPath + "/config.yaml";
    std::string delimitedPaths = GetPDFxTMDPaths();
    PDFxTMD::ConfigWrapper config;

    if (delimitedPaths.find(newPath) != std::string::npos)
    {
        return true;
    }
    if (delimitedPaths.empty())
        delimitedPaths += newPath;
    else
        delimitedPaths += "|" + newPath;

    config.set("paths", delimitedPaths.c_str());
    return config.saveToFile(configFilePath);
}
} // namespace PDFxTMD
