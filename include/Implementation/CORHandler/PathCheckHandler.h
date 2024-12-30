#pragma once
#include "Common/PartonUtils.h"
#include "Interface/CORHandler.h"
#include <iostream>
#include <string>
#include <vector>

class PathCheckHandler : public Handler
{
  public:
    void handle(Context &context) override
    {
        const char *envPaths = std::getenv("ENV_PATH_VARIABLE");
        if (!envPaths)
        {
            std::cerr << "Environment variable not found: ENV_PATH_VARIABLE" << std::endl;
            context.set("error", "ENV_PATH_VARIABLE not set");
            return;
        }

        std::vector<std::string> paths = PDFxTMD::splitPaths(envPaths);
        for (const auto &path : paths)
        {
            if (PDFxTMD::hasWriteAccess(path))
            {
                std::cout << "Writable path found: " << path << std::endl;
                context.set("writable_path", path);
                if (nextHandler)
                {
                    nextHandler->handle(context);
                }
                return;
            }
        }

        std::cerr << "No writable path found in environment variable." << std::endl;
        context.set("error", "No writable path found");
    }
};