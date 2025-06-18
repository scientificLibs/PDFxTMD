#include <PDFxTMDLib/Common/YamlMetaInfo/YamlImpelemntationInfo.h>
#include <PDFxTMDLib/Common/ConfigWrapper.h>

namespace PDFxTMD
{
std::pair<std::optional<YamlImpelemntationInfo>, ErrorType> YamlImpelemntationInfoReader(
    const std::string &yamlInfoPath)
{
    ConfigWrapper ConfigWrapper;
    YamlImpelemntationInfo output;
    if (!ConfigWrapper.loadFromFile(yamlInfoPath, ConfigWrapper::Format::YAML))
    {
        return {std::nullopt, ErrorType::FILE_NOT_FOUND};
    }
    auto [reader, errorReader] = ConfigWrapper.get<std::string>("Reader");
    if (errorReader == ErrorType::None)
    {
        output.reader = *reader;
    }
    auto [interpolator, errorInterpolator] = ConfigWrapper.get<std::string>("Interpolator");
    if (errorInterpolator == ErrorType::None)
    {
        output.interpolator = *interpolator;
    }
    auto [extrapolator_, errExtrapolator] = ConfigWrapper.get<std::string>("Extrapolator");
    if (errExtrapolator == ErrorType::None)
    {
        output.extrapolator = *extrapolator_;
    }
    return {output, ErrorType::None};
}
} // namespace PDFxTMD