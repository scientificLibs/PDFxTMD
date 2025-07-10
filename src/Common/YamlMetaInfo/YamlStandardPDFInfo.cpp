#include <PDFxTMDLib/Common/Logger.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h>
#include <iostream>

namespace PDFxTMD
{
std::pair<std::optional<YamlStandardTMDInfo>, ErrorType> YamlStandardPDFInfoReader(
    const std::string &yamlInfoPath)
{
    ErrorType errTot = ErrorType::None;
    ConfigWrapper ConfigWrapper;
    YamlStandardTMDInfo output;
    if (!ConfigWrapper.loadFromFile(yamlInfoPath, ConfigWrapper::Format::YAML))
    {
        return {std::nullopt, ErrorType::FILE_NOT_FOUND};
    }
    //////OrderQCD
    auto [orderQCD, errorOrderQCD] = ConfigWrapper.get<int>("OrderQCD");
    if (errorOrderQCD != ErrorType::None)
    {
        PDFxTMDLOG << "[PDFxTMD][YamlInfoReader] OrderQCD not "
                      "found in yaml file"
                   << std::endl;
        return {std::nullopt, errorOrderQCD};
    }
    output.orderQCD = static_cast<OrderQCD>(*orderQCD);

    //////NumMembers
    auto [NumMembers, errorNumMembers] = ConfigWrapper.get<int>("NumMembers");
    if (errorNumMembers != ErrorType::None)
    {
        PDFxTMDLOG << "[PDFxTMD][YamlInfoReader] NumMembers is not "
                      "found in yaml config file"
                   << std::endl;
        return {std::nullopt, errorNumMembers};
    }
    output.NumMembers = *NumMembers;
    //////flavors
    auto [Flavors, errorFlavors] = ConfigWrapper.get<std::vector<int>>("Flavors");
    if (errorFlavors != ErrorType::None)
    {
        PDFxTMDLOG << "[PDFxTMD][YamlInfoReader] Flavors is not "
                      "found in yaml config file"
                   << std::endl;
        return {std::nullopt, errorFlavors};
    }
    output.Flavors = *Flavors;
    //////xMin
    auto [XMin, errorXMin] = ConfigWrapper.get<double>("XMin");
    if (errorXMin != ErrorType::None)
    {
        PDFxTMDLOG << "[PDFxTMD][YamlInfoReader] XMin is not found "
                      "in yaml config file"
                   << std::endl;
        return {std::nullopt, errorXMin};
    }
    output.XMin = *XMin;
    //////XMax
    auto [XMax, errorXMax] = ConfigWrapper.get<double>("XMax");
    if (errorXMax != ErrorType::None)
    {
        PDFxTMDLOG << "[PDFxTMD][YamlInfoReader] XMax is not found "
                      "in yaml config file"
                   << std::endl;
        return {std::nullopt, errorXMin};
    }
    output.XMax = *XMax;

    //////QMin
    auto [QMin, errorQMin] = ConfigWrapper.get<double>("QMin");
    if (errorQMin != ErrorType::None)
    {
        PDFxTMDLOG << "[PDFxTMD][YamlInfoReader] QMin is not found "
                      "in yaml config file"
                   << std::endl;
        return {std::nullopt, errorQMin};
    }
    output.QMin = *QMin;
    //////QMax
    auto [QMax, errorQMax] = ConfigWrapper.get<double>("QMax");
    if (errorQMax != ErrorType::None)
    {
        PDFxTMDLOG << "[PDFxTMD][YamlInfoReader] QMax is not found "
                      "in yaml config file"
                   << std::endl;
        return {std::nullopt, errorQMax};
    }
    output.QMax = *QMax;
    auto [TMDScheme, errorTMDScheme] = ConfigWrapper.get<std::string>("TMDScheme");
    if (errorTMDScheme != ErrorType::None)
    {
        output.TMDScheme = "";
    }
    else
        output.TMDScheme = *TMDScheme;
    //////KtMin
    auto [KtMin, errorKtMin] = ConfigWrapper.get<double>("KtMin");
    if (errorKtMin != ErrorType::None)
    {
        output.KtMin = -1;
    }
    else
        output.KtMin = *KtMin;
    //////KtMax
    auto [KtMax, errorKtMax] = ConfigWrapper.get<double>("KtMax");
    if (errorKtMax != ErrorType::None)
    {
        output.KtMax = -1;
    }
    else
        output.KtMax = *KtMax;

    //////SetIndex
    auto [SetIndex, errorSetIndex] = ConfigWrapper.get<int>("SetIndex");
    if (errorSetIndex == ErrorType::None)
    {
        output.lhapdfID = *SetIndex;
    }
    return {output, errTot};
}
} // namespace PDFxTMD