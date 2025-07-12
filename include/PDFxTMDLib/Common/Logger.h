#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

namespace PDFxTMD
{
class Logger
{
  public:
    static Logger &getInstance()
    {
        static Logger instance;
        return instance;
    }

    void setEnabled(bool enable)
    {
        enabled = enable;
    }

    // Set the output stream (e.g., std::cout, std::cerr, or a file stream)
    void setLogStream(std::ostream &os)
    {
        logStream = &os;
    }

    template <typename T> Logger &operator<<(T &&value)
    {
        if (enabled)
        {
            *logStream << std::forward<T>(value);
        }
        return *this;
    }

    Logger &operator<<(std::ostream &(*manip)(std::ostream &))
    {
        if (enabled)
        {
            manip(*logStream);
        }
        return *this;
    }

    // Prevent copying of the singleton
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

  private:
    Logger() : enabled(false), logStream(&std::cout)
    {
    }

    bool enabled;
    std::ostream *logStream;
};
} // namespace PDFxTMD
#define PDFxTMDLOG PDFxTMD::Logger::getInstance()
