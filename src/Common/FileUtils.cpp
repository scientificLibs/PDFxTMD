#include "Common/FileUtils.h"
namespace PDFxTMD
{
bool FileUtils::HasUserAccess(const std::string &path)
{
    try
    {
        // Create a test file to verify write permissions
        auto testPath = std::filesystem::path(path) / ".write_test";
        std::ofstream file(testPath);
        bool hasAccess = file.is_open();
        file.close();
        if (std::filesystem::exists(testPath))
        {
            std::filesystem::remove(testPath);
        }
        return hasAccess;
    }
    catch (const std::filesystem::filesystem_error &)
    {
        return false;
    }
}

double FileUtils::FreeSize(const std::string &path)
{
    std::error_code ec;
    auto space = std::filesystem::space(path, ec);

    if (ec)
    {
        return -1.0; // Return negative value to indicate error
    }

    // Convert bytes to GB (1 GB = 1024^3 bytes)
    return static_cast<double>(space.available) / (1024 * 1024 * 1024);
}
bool FileUtils::Exists(const std::string &path)
{
    return std::filesystem::exists(path);
}

std::string FileUtils::ParentDir(const std::string &path)
{
    std::filesystem::path fsPath(path);
    return fsPath.parent_path().string();
}

bool FileUtils::CreateDirs(const std::string &path)
{
    std::error_code ec;
    return std::filesystem::create_directories(path, ec);
}
} // namespace PDFxTMD