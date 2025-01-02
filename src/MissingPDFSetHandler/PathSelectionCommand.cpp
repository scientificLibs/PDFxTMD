#include "MissingPDFSetHandler/PathSelectionCommand.h"
#include "Common/EnvUtils.h"
#include "Common/FileUtils.h"
#include "Common/PartonUtils.h"
#include "Common/StringUtils.h"
#include <iostream>
#include <optional>
#include <regex>

namespace PDFxTMD
{

KeyValueStore DownloadPathSelection()
{
    auto paths = GetEnviormentalVariablePaths();
    std::cout << "\nPlease select one of the following options:\n" << std::endl;

    std::cout << "1. Choose existing path (C:number)" << std::endl;
    std::cout << "   Example: C:1 to select the first path\n" << std::endl;

    std::cout << "2. Add new path (A:path)" << std::endl;
    std::cout << "   Example: A:/home/user/documents\n" << std::endl;

    std::cout << "3. Exit (Q)" << std::endl;
    std::cout << "\nCurrent paths in " << ENV_PATH_VARIABLE << ":" << std::endl;
    for (int i = 0; i < paths.size(); i++)
    {
        std::cout << std::to_string(i + 1) << ": " << paths[i] << std::endl;
    }
    std::string userInput;
    std::getline(std::cin, userInput);
    trim(userInput);

    KeyValueStore keyValObj;
    if (userInput == "Q")
    {
        keyValObj.set("Error", "Q");
        return keyValObj;
    }
    // Regex patterns for both cases
    std::regex choosePattern(R"(^C:\s*(\d+)\s*$)"); // Matches "C: <number>"
    std::regex addPattern(R"(^A:\s*(.+?)\s*$)");    // Matches "A: <path>"

    std::smatch matches;
    if (std::regex_match(userInput, matches, choosePattern))
    {
        // Handle choosing existing path
        try
        {
            int index = std::stoi(matches[1]);
            if (index >= 1 && index <= paths.size())
            {
                keyValObj.set("Selected Path", paths[index - 1]);
            }
            else
            {
                keyValObj.set("Error", "Index out of range");
            }
        }
        catch (const std::exception &)
        {
            keyValObj.set("Error", "Invalid number format");
        }
    }
    else if (std::regex_match(userInput, matches, addPattern))
    {
        // Handle adding new path
        keyValObj.set("Added Path", matches[1]);
    }
    else
    {
        keyValObj.set("Error", "Invalid input format");
    }

    return keyValObj;
}

bool ResponseKeyValueErrorHandler(const KeyValueStore &response)
{
    std::optional<StandardType> errorVal = response.get("Error");
    if (errorVal)
    {
        StandardType errorVal_ = *errorVal;
        if (const auto reasonStrPTr = std::get_if<std::string>(&errorVal_))
        {
            std::cout << "Error: " << *reasonStrPTr << std::endl;
        }
        else
        {
            std::cout << "Error: something went wrong" << std::endl;
        }
        return true;
    }
    return false;
}

bool CheckPathRequirements(const std::string &path)
{
    std::string parentPath = FileUtils::ParentDir(path);
    bool isPathOK = false;
    if (FileUtils::HasUserAccess(parentPath))
    {
        if (FileUtils::FreeSize(parentPath) > 1)
        {
            if (!FileUtils::Exists(path))
            {
                isPathOK = FileUtils::CreateDir(path);
                if (!isPathOK)
                {
                    std::cerr << "path: " << path << " failed to be created" << std::endl;
                    return false;
                }
            }
            isPathOK = true;
        }
    }
    return isPathOK;
}

std::pair<bool, std::string> DownloadPathSelectionResponseHandler(const KeyValueStore &response)
{
    auto response_C = response.get("Selected Path");
    if (response_C)
    {
        StandardType response_C_ = *response_C;
        if (const auto pathStr = std::get_if<std::string>(&response_C_))
        {
            if (CheckPathRequirements(*pathStr))
            {
                return {true, *pathStr};
            }
        }
    }
    else
    {
        auto response_A = response.get("Added Path");
        StandardType response_A_ = *response_A;
        if (const auto pathStr = std::get_if<std::string>(&response_A_))
        {
            if (CheckPathRequirements(*pathStr))
            {
                if (EnvUtils::AddPathToEnvironment(*pathStr))
                {
                    const char *currentPaths = getenv(ENV_PATH_VARIABLE);
                    std::string updatedPaths =
                        currentPaths ? std::string(currentPaths) + ":" + *pathStr : *pathStr;
                    setenv(ENV_PATH_VARIABLE, updatedPaths.c_str(), 1);
                    return {true, *pathStr};
                }
                else
                {
                    std::cerr << "Failed to add new path to environment variable "
                              << ENV_PATH_VARIABLE << std::endl;
                }
            }
        }
    }
    return {false, ""};
}

bool pathSelectionCommnand::execute(StandardTypeMap &context)
{
    auto response = DownloadPathSelection();
    if (ResponseKeyValueErrorHandler(response))
    {
        context["Error"] = *response.get("Error");
        return false;
    }

    auto [success, path] = DownloadPathSelectionResponseHandler(response);
    if (success)
    {
        context["SelectedPath"] = path;
        return true;
    }
    context["Error"] = "Unkown error. Selected path is not accepted";

    return false;
}

} // namespace PDFxTMD
