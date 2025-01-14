#include "Common/YamlInfoReader.h"
#include "Common/Exception.h"
#include "Common/StringUtils.h"
#include <iostream>

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

std::pair<std::optional<YamlCouplingInfo>, ErrorType> YamlCouplingInfoReader(
    const std::string &yamlInfoPath)
{
    ConfigWrapper ConfigWrapper;
    YamlCouplingInfo output;
    if (!ConfigWrapper.loadFromFile(yamlInfoPath, ConfigWrapper::Format::YAML))
    {
        return {std::nullopt, ErrorType::FILE_NOT_FOUND};
    }
    auto [mu_vec, mu_vecError] = ConfigWrapper.get<std::vector<double>>("AlphaS_Qs");
    if (mu_vecError == ErrorType::None)
    {
        output.mu_vec = std::move(*mu_vec);
    }
    auto [alphas_vec, alphas_vecError] = ConfigWrapper.get<std::vector<double>>("AlphaS_Vals");
    if (alphas_vecError == ErrorType::None)
    {
        output.alphas_vec = std::move(*alphas_vec);
    }

    auto [alphasLambda3, alphasLambda3Error] = ConfigWrapper.get<double>("AlphaS_Lambda3");
    if (alphasLambda3Error == ErrorType::None)
    {
        output.alphasLambda3 = *alphasLambda3;
    }
    auto [alphasLambda4, alphasLambda4Error] = ConfigWrapper.get<double>("AlphaS_Lambda4");
    if (alphasLambda4Error != ErrorType::None)
    {
        output.alphasLambda4 = *alphasLambda4;
    }
    auto [alphasLambda5, alphasLambda5Error] = ConfigWrapper.get<double>("AlphaS_Lambda5");
    if (alphasLambda5Error == ErrorType::None)
    {
        output.alphasLambda5 = *alphasLambda5;
    }

    auto [AlphaS_ThresholdDown, AlphaS_ThresholdDownError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdDown");
    auto [AlphaS_ThresholdUp, AlphaS_ThresholdUpError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdUp");
    auto [AlphaS_ThresholdStrange, AlphaS_ThresholdStrangeError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdStrange");
    auto [AlphaS_ThresholdCharm, AlphaS_ThresholdCharmError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdCharm");
    auto [AlphaS_ThresholdBottom, AlphaS_ThresholdBottomError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdBottom");
    auto [AlphaS_ThresholdTop, AlphaS_ThresholdTopError] =
        ConfigWrapper.get<double>("AlphaS_ThresholdTop");
    bool isThreshholdObtained = false;
    if (all(AlphaS_ThresholdDownError == ErrorType::None,
            AlphaS_ThresholdUpError == ErrorType::None,
            AlphaS_ThresholdStrangeError == ErrorType::None,
            AlphaS_ThresholdCharmError == ErrorType::None,
            AlphaS_ThresholdBottomError == ErrorType::None,
            AlphaS_ThresholdTopError == ErrorType::None))
    {
        output.quarkThreshhold[PartonFlavor::d] = *AlphaS_ThresholdDown;
        output.quarkThreshhold[PartonFlavor::u] = *AlphaS_ThresholdUp;
        output.quarkThreshhold[PartonFlavor::s] = *AlphaS_ThresholdStrange;
        output.quarkThreshhold[PartonFlavor::c] = *AlphaS_ThresholdCharm;
        output.quarkThreshhold[PartonFlavor::b] = *AlphaS_ThresholdBottom;
        output.quarkThreshhold[PartonFlavor::t] = *AlphaS_ThresholdTop;
        isThreshholdObtained = true;
    }
    auto [ThresholdDown, ThresholdDownError] = ConfigWrapper.get<double>("ThresholdDown");
    auto [ThresholdUp, ThresholdUpError] = ConfigWrapper.get<double>("ThresholdUp");
    auto [ThresholdStrange, ThresholdStrangeError] = ConfigWrapper.get<double>("ThresholdStrange");
    auto [ThresholdCharm, ThresholdCharmError] = ConfigWrapper.get<double>("ThresholdCharm");
    auto [ThresholdBottom, ThresholdBottomError] = ConfigWrapper.get<double>("ThresholdBottom");
    auto [ThresholdTop, ThresholdTopError] = ConfigWrapper.get<double>("ThresholdTop");

    if (all(ThresholdDownError == ErrorType::None, ThresholdUpError == ErrorType::None,
            ThresholdStrangeError == ErrorType::None, ThresholdCharmError == ErrorType::None,
            ThresholdBottomError == ErrorType::None, ThresholdTopError == ErrorType::None))
    {
        output.quarkThreshhold[PartonFlavor::d] = *ThresholdDown;
        output.quarkThreshhold[PartonFlavor::u] = *ThresholdUp;
        output.quarkThreshhold[PartonFlavor::s] = *ThresholdStrange;
        output.quarkThreshhold[PartonFlavor::c] = *ThresholdCharm;
        output.quarkThreshhold[PartonFlavor::b] = *ThresholdBottom;
        output.quarkThreshhold[PartonFlavor::t] = *ThresholdTop;
        isThreshholdObtained = true;
    }
    if (!isThreshholdObtained)
    {
        output.quarkThreshhold[PartonFlavor::d] = -1;
        output.quarkThreshhold[PartonFlavor::u] = -1;
        output.quarkThreshhold[PartonFlavor::s] = -1;
        output.quarkThreshhold[PartonFlavor::c] = -1;
        output.quarkThreshhold[PartonFlavor::b] = -1;
        output.quarkThreshhold[PartonFlavor::t] = -1;
    }
    bool isMasssObtained = false;
    auto [AlphaS_MDown, AlphaS_MDownError] = ConfigWrapper.get<double>("AlphaS_MDown");
    auto [AlphaS_MUp, AlphaS_MUpError] = ConfigWrapper.get<double>("AlphaS_MUp");
    auto [AlphaS_MStrange, AlphaS_MStrangeError] = ConfigWrapper.get<double>("AlphaS_MStrange");
    auto [AlphaS_MCharm, AlphaS_MCharmError] = ConfigWrapper.get<double>("AlphaS_MCharm");
    auto [AlphaS_MBottom, AlphaS_MBottomError] = ConfigWrapper.get<double>("AlphaS_MBottom");
    auto [AlphaS_MTop, AlphaS_MTopError] = ConfigWrapper.get<double>("AlphaS_MTop");
    if (all(AlphaS_MDownError == ErrorType::None, AlphaS_MUpError == ErrorType::None,
            AlphaS_MStrangeError == ErrorType::None, AlphaS_MCharmError == ErrorType::None,
            AlphaS_MBottomError == ErrorType::None, AlphaS_MTopError == ErrorType::None))
    {
        output.quarkMasses[PartonFlavor::d] = *AlphaS_MDown;
        output.quarkMasses[PartonFlavor::u] = *AlphaS_MUp;
        output.quarkMasses[PartonFlavor::s] = *AlphaS_MStrange;
        output.quarkMasses[PartonFlavor::c] = *AlphaS_MCharm;
        output.quarkMasses[PartonFlavor::b] = *AlphaS_MBottom;
        output.quarkMasses[PartonFlavor::t] = *AlphaS_MTop;
        isMasssObtained = true;
    }

    if (!isMasssObtained)
    {
        auto [MDown, MDownError] = ConfigWrapper.get<double>("MDown");
        auto [MUp, MUpError] = ConfigWrapper.get<double>("MUp");
        auto [MStrange, MStrangeError] = ConfigWrapper.get<double>("MStrange");
        auto [MCharm, MCharmError] = ConfigWrapper.get<double>("MCharm");
        auto [MBottom, MBottomError] = ConfigWrapper.get<double>("MBottom");
        auto [MTop, MTopError] = ConfigWrapper.get<double>("MTop");
        if (all(MDownError == ErrorType::None, MUpError == ErrorType::None,
                MStrangeError == ErrorType::None, MCharmError == ErrorType::None,
                MBottomError == ErrorType::None, MTopError == ErrorType::None))
        {
            output.quarkMasses[PartonFlavor::d] = *MDown;
            output.quarkMasses[PartonFlavor::u] = *MUp;
            output.quarkMasses[PartonFlavor::s] = *MStrange;
            output.quarkMasses[PartonFlavor::c] = *MCharm;
            output.quarkMasses[PartonFlavor::b] = *MBottom;
            output.quarkMasses[PartonFlavor::t] = *MTop;
            isMasssObtained = true;
        }
    }

    if (!isMasssObtained)
    {
        output.quarkMasses[PartonFlavor::d] = 0.005;
        output.quarkMasses[PartonFlavor::u] = 0.002;
        output.quarkMasses[PartonFlavor::s] = 0.10;
        output.quarkMasses[PartonFlavor::c] = 1.29;
        output.quarkMasses[PartonFlavor::b] = 4.19;
        output.quarkMasses[PartonFlavor::t] = 172.9;
    }

    bool isFlavorSchemeObtained = false;
    std::string flavorScheme = "";
    auto [AlphaS_FlavorScheme, AlphaS_FlavorSchemeError] =
        ConfigWrapper.get<std::string>("AlphaS_FlavorScheme");
    if (AlphaS_FlavorSchemeError == ErrorType::None)
    {
        flavorScheme = ToLower(*AlphaS_FlavorScheme);
        isFlavorSchemeObtained = true;
    }
    if (!isFlavorSchemeObtained)
    {
        auto [FlavorScheme, FlavorSchemeError] = ConfigWrapper.get<std::string>("FlavorScheme");
        if (FlavorSchemeError != ErrorType::None)
        {
            flavorScheme = ToLower(*FlavorScheme);
            isFlavorSchemeObtained = true;
        }
    }

    bool isNumFlavorsObtained = false;
    int numFlavors = DEFULT_NUM_FLAVORS;
    auto [AlphaS_NumFlavors, AlphaS_NumFlavorsError] = ConfigWrapper.get<int>("AlphaS_NumFlavors");
    if (AlphaS_NumFlavorsError == ErrorType::None)
    {
        numFlavors = *AlphaS_NumFlavors;
        isNumFlavorsObtained = true;
    }
    if (!isNumFlavorsObtained)
    {
        auto [NumFlavors, NumFlavorsError] = ConfigWrapper.get<int>("NumFlavors");
        if (NumFlavorsError != ErrorType::None)
        {
            numFlavors = *NumFlavors;
            isNumFlavorsObtained = true;
        }
    }
    if (!isNumFlavorsObtained)
    {
        numFlavors = DEFULT_NUM_FLAVORS;
    }

    if (!isFlavorSchemeObtained)
    {
        flavorScheme = "variable";
    }
    output.flavorScheme =
        (flavorScheme == "variable" ? AlphasFlavorScheme::variable : AlphasFlavorScheme::fixed);
    output.numFlavors = numFlavors;

    auto [MassZ, MassZError] = ConfigWrapper.get<double>("MZ");
    if (MassZError == ErrorType::None)
    {
        output.MZ = *MassZ;
    }

    auto [AlphaS_OrderQCD, AlphaS_OrderQCDError] = ConfigWrapper.get<int>("AlphaS_OrderQCD");
    if (AlphaS_OrderQCDError == ErrorType::None)
    {
        output.alphasOrder = (AlphasType)*AlphaS_OrderQCD;
    }
    auto [AlphaS_Type, AlphaS_TypeError] = ConfigWrapper.get<std::string>("AlphaS_Type");
    if (AlphaS_TypeError == ErrorType::None)
    {
        if (AlphaS_Type == "ipol")
        {
            output.alphaCalcMethod = AlphasType::ipol;
        }
        else if (AlphaS_Type == "analytic")
        {
            output.alphaCalcMethod = AlphasType::analytic;
        }
        else
        {
            throw NotSupportError(*AlphaS_Type +
                                  " is currently not support for AlphaQCD calculation");
        }
    }
    return {output, ErrorType::None};
}
} // namespace PDFxTMD