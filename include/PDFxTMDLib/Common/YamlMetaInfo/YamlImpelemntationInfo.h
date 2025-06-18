#pragma once
#include <string>
#include <optional>
#include <PDFxTMDLib/Common/PartonUtils.h>

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
}