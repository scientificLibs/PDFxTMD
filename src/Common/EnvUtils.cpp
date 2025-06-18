#include "PDFxTMDLib/Common/EnvUtils.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#if defined(_WIN32)
#include <Windows.h>
#endif
#include <filesystem>

namespace PDFxTMD
{
bool AddPathToEnvironment(const std::string &newPath)
{
    std::string rootPath;
#if defined(WIN32__)
    const char *homeDir = std::getenv("HOME");
    if (!homeDir)
    {
        rootPath = "./";
    }
    else
        rootPath = homeDir;
    std::string configDir = rootPath + ".PDFxTMDLib";
    std::string configFilePath = configDir + "/config.yaml";

    std::filesystem::create_directories(configDir);

    PDFxTMD::ConfigWrapper config;

    if (std::filesystem::exists(configFilePath) && std::filesystem::file_size(configFilePath) > 0)
    {
        config.loadFromFile(configFilePath, PDFxTMD::ConfigWrapper::Format::YAML);
    }

    auto pathsPair = config.get<std::string>("paths");
    std::string delimitedPaths;

    if (pathsPair.first.has_value())
    {
        delimitedPaths = pathsPair.first.value();
    }

    if (delimitedPaths.find(newPath) != std::string::npos)
    {
        return true;
    }
    if (delimitedPaths.empty())
        delimitedPaths += newPath;
    else
        delimitedPaths += "|" + newPath;

    config.set("paths", delimitedPaths.c_str());
    PDFxTMDLOG << "delimitedPaths " << delimitedPaths << std::endl;
    return config.saveToFile(configFilePath);
}

} // namespace PDFxTMD