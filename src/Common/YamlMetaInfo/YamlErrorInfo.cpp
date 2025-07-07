#include <PDFxTMDLib/Common/ConfigWrapper.h>
#include <PDFxTMDLib/Common/Logger.h>
#include <PDFxTMDLib/Common/MathUtils.h>
#include <PDFxTMDLib/Common/StringUtils.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlErrorInfo.h>
#include <iostream>

namespace PDFxTMD
{
std::pair<std::optional<YamlErrorInfo>, ErrorType> YamlErrorInfoReader(
    const std::string &yamlInfoPath)
{
    ConfigWrapper ConfigWrapper;
    YamlErrorInfo output;
    if (!ConfigWrapper.loadFromFile(yamlInfoPath, ConfigWrapper::Format::YAML))
    {
        return {std::nullopt, ErrorType::FILE_NOT_FOUND};
    }

    auto [ErrorType, ErrorTypeError] = ConfigWrapper.get<std::string>("ErrorType");
    if (ErrorTypeError == ErrorType::None)
    {
        output.ErrorType = ToLower(*ErrorType);
    }
    else
    {
        output.ErrorType = "unkown";
    }

    auto [ErrorConfLevel, ErrorConfLevelError] = ConfigWrapper.get<double>("ErrorConfLevel");
    if (ErrorConfLevelError == ErrorType::None)
    {
        output.ErrorConfLevel = *ErrorConfLevel;
    }
    else
    {
        if (StartsWith(output.ErrorType, "replicas"))
        {
            output.ErrorConfLevel = -1;
        }
        else
        {
            output.ErrorConfLevel = CL1SIGMA;
            PDFxTMDLOG << "[PDFxTMD][YamlErrorInfoReader] ErrorConfLevel is not found "
                          "in yaml config file, set to default 68%"
                       << std::endl;
        }
    }

    auto [NumMembers, NumMembersError] = ConfigWrapper.get<size_t>("NumMembers");
    if (NumMembersError == ErrorType::None)
    {
        output.size = *NumMembers;
        output.errorSize = *NumMembers - 1;
    }
    else
    {
        output.size = 0;
        output.errorSize = 0;
    }
    return {output, ErrorType::None};
}
} // namespace PDFxTMD