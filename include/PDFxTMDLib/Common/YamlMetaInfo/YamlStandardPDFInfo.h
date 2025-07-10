#pragma once
#include <PDFxTMDLib/Common/ConfigWrapper.h>
#include <PDFxTMDLib/Common/PartonUtils.h>
#include <map>
#include <optional>
#include <type_traits>

namespace PDFxTMD
{

struct YamlStandardPDFInfo
{
    OrderQCD orderQCD;
    double XMin = 0;
    double XMax = 0;
    double QMin = 0;
    double QMax = 0;
    int NumMembers = 0;
    std::vector<int> Flavors;
    int lhapdfID = -1;
};

struct YamlStandardTMDInfo : YamlStandardPDFInfo
{
    double KtMin = 0;
    double KtMax = 0;
    std::string TMDScheme = "";
};

std::pair<std::optional<YamlStandardTMDInfo>, ErrorType> YamlStandardPDFInfoReader(
    const std::string &yamlInfoPath);
} // namespace PDFxTMD
