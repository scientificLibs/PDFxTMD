#pragma once
#include <PDFxTMDLib/Common/PartonUtils.h>
#include <optional>
#include <string>

namespace PDFxTMD
{
struct YamlErrorInfo
{
    double ErrorConfLevel;
    std::string ErrorType;
    size_t errorSize;
    size_t size;
};
std::pair<std::optional<YamlErrorInfo>, ErrorType> YamlErrorInfoReader(
    const std::string &yamlInfoPath);
} // namespace PDFxTMD