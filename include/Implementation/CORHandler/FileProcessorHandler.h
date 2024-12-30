#pragma once
#include "Interface/CORHandler.h"
#include "nlohmann/json.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>

class FileProcessorHandler : public Handler
{
  public:
    void handle(Context &context) override
    {
        if (!context.contains("file_path"))
        {
            std::cerr << "File path not found in context." << std::endl;
            context.set("error", "File path missing");
            return;
        }

        std::string filePath = *context.get("file_path");
        std::cout << "Processing file: " << filePath << std::endl;

        std::string command = "tar -xzf " + filePath + " -C " +
                              std::filesystem::path(filePath).parent_path().string();
        if (std::system(command.c_str()) == 0)
        {
            std::cout << "File extracted successfully." << std::endl;
            std::filesystem::remove(filePath);
        }
        else
        {
            std::cerr << "Failed to extract the file!" << std::endl;
            context.set("error", "File extraction failed");
        }
    }
};
