#include "Common/EnvUtils.h"
#include "Common/PartonUtils.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace PDFxTMD
{
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

} // namespace PDFxTMD