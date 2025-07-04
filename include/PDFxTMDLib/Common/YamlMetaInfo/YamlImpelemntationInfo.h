#pragma once
#include <PDFxTMDLib/Common/PartonUtils.h>
#include <optional>
#include <string>

namespace PDFxTMD
{
struct YamlImpelemntationInfo
{
    std::string reader = "";
    std::string interpolator = "";
    std::string extrapolator = "";
};

std::pair<std::optional<YamlImpelemntationInfo>, ErrorType> YamlImpelemntationInfoReader(
    const std::string &yamlInfoPath);
} // namespace PDFxTMD