#include "Common/EnvUtils.h"
#include "Common/PartonUtils.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#if defined(_WIN32)
#include <Windows.h>
#endif

namespace PDFxTMD
{
#if defined(__linux__)
bool EnvUtils::AddPathToEnvironment(const std::string &newPath)
{
    const char *homeDir = std::getenv("HOME");
    if (!homeDir)
        return false;

    // Check if the environment variable already exists
    const char *currentPath = std::getenv(ENV_PATH_VARIABLE);
    std::string exportCommand;

    if (currentPath)
    {
        // Check if the new path is already in the variable
        std::string currentPathStr(currentPath);
        if (currentPathStr.find(newPath) != std::string::npos)
        {
            // Path already exists in the variable
            return true;
        }
        // Append new path to existing value
        exportCommand = "export " + std::string(ENV_PATH_VARIABLE) + "=\"" + currentPathStr + ":" +
                        newPath + "\"";
    }
    else
    {
        // Create new environment variable with just the new path
        exportCommand = "export " + std::string(ENV_PATH_VARIABLE) + "=\"" + newPath + "\"";
    }

    // Add to .bashrc
    std::string bashrc = std::string(homeDir) + "/.bashrc";

    // First check if this export already exists in .bashrc
    std::ifstream readFile(bashrc);
    if (readFile.is_open())
    {
        std::string line;
        while (std::getline(readFile, line))
        {
            if (line.find("export " + std::string(ENV_PATH_VARIABLE)) != std::string::npos)
            {
                // Found existing export, need to remove it
                readFile.close();

                // Read all content
                std::ifstream in(bashrc);
                std::stringstream buffer;
                buffer << in.rdbuf();
                in.close();

                // Remove the old export line
                std::string content = buffer.str();
                size_t pos = content.find(line);
                if (pos != std::string::npos)
                {
                    content.erase(pos, line.length() + 1); // +1 for newline
                }

                // Write updated content
                std::ofstream out(bashrc);
                out << content;
                out.close();
                break;
            }
        }
        readFile.close();
    }

    // Append new export command
    std::ofstream file(bashrc, std::ios::app);
    if (!file)
        return false;

    file << "\n# Added by PDFxTMD\n" << exportCommand << "\n";

    return true;
}
#elif defined(_WIN32)
bool EnvUtils::AddPathToEnvironment(const std::string &newPath)
{
    HKEY hKey;
    LONG result;

    // Open the registry key for the current user
    result =
        RegOpenKeyExA(HKEY_CURRENT_USER, PDF_X_TMD_REGISTRY_PATH, 0, KEY_READ | KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return false; // Failed to open registry key
    }

    // Read the existing value
    DWORD dataSize = 0;
    result = RegQueryValueExA(hKey, ENV_PATH_VARIABLE, nullptr, nullptr, nullptr, &dataSize);
    if (result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND)
    {
        RegCloseKey(hKey);
        return false; // Failed to query registry value
    }

    std::vector<char> buffer(dataSize);
    result = RegQueryValueExA(hKey, ENV_PATH_VARIABLE, nullptr, nullptr,
                              reinterpret_cast<LPBYTE>(buffer.data()), &dataSize);
    if (result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND)
    {
        RegCloseKey(hKey);
        return false; // Failed to read registry value
    }

    // Parse the MULTI_SZ value into a vector of strings
    std::vector<std::string> paths;
    const char *current = buffer.data();
    while (current && *current)
    {
        paths.emplace_back(current);
        current += strlen(current) + 1;
    }

    // Check if the new path is already in the list
    if (std::find(paths.begin(), paths.end(), newPath) != paths.end())
    {
        RegCloseKey(hKey);
        return true; // Path already exists
    }

    // Add the new path to the list
    paths.push_back(newPath);

    // Rebuild the MULTI_SZ value
    std::vector<char> newMultiSz;
    for (const auto &path : paths)
    {
        newMultiSz.insert(newMultiSz.end(), path.begin(), path.end());
        newMultiSz.push_back('\0');
    }
    newMultiSz.push_back('\0'); // Double null-terminate

    // Write the updated value back to the registry
    result = RegSetValueExA(hKey, ENV_PATH_VARIABLE, 0, REG_MULTI_SZ,
                            reinterpret_cast<const BYTE *>(newMultiSz.data()),
                            static_cast<DWORD>(newMultiSz.size()));
    RegCloseKey(hKey);

    return result == ERROR_SUCCESS;
}
#endif

} // namespace PDFxTMD