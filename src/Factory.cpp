#include "PDFxTMDLib/Factory.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlImpelemntationInfo.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h"
#include "PDFxTMDLib/GenericPDF.h"
#include "PDFxTMDLib/Implementation/Coupling/Analytic/AnalyticQCDCoupling.h"
#include "PDFxTMDLib/Implementation/Coupling/Interpolation/InterpolateQCDCoupling.h"
#include "PDFxTMDLib/Implementation/Coupling/ODE/ODEQCDCoupling.h"
#include "PDFxTMDLib/Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/Collinear/CErrExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/Collinear/CNearestPointExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/TMD/TErrExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/TMD/TZeroExtrapolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/Collinear/CLHAPDFBicubicInterpolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/Collinear/CLHAPDFBilinearInterpolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/TMD/TTrilinearInterpolator.h"
#include "PDFxTMDLib/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Implementation/Reader/TMD/TDefaultLHAPDF_TMDReader.h"
#include <memory>

namespace PDFxTMD
{

//////// start reader types
enum class TReader
{
    TDefaultLHAPDF_TMDReader,
};

enum class CReader
{
    UNKNOWN,
    CDefaultLHAPDFFileReader
};
TReader TReaderType(const std::string &type)
{
    if (type == "TDefaultLHAPDF_TMDReader")
    {
        return TReader::TDefaultLHAPDF_TMDReader;
    }

    throw NotSupportError("This file reader is not supported");
}

CReader CReaderType(const std::string &type)
{
    if (type == "CDefaultLHAPDFFileReader")
    {
        return CReader::CDefaultLHAPDFFileReader;
    }
    throw NotSupportError("This file reader is not supported");
}

///////end reader types
////// start interpolator type
enum class TInterpolator
{
    TTrilinearInterpolator,
};

// collinear supported interpolator type
enum class CInterpolator
{
    CLHAPDFBilinearInterpolator,
    CLHAPDFBicubicInterpolator,
};

CInterpolator CInterpolatorType(const std::string &type)
{
    if (type == "CLHAPDFBilinearInterpolator")
    {
        return CInterpolator::CLHAPDFBilinearInterpolator;
    }
    else if (type == "CLHAPDFBicubicInterpolator")
    {
        return CInterpolator::CLHAPDFBicubicInterpolator;
    }

    throw NotSupportError("This interpolator is not supported");
}

TInterpolator TInterpolatorType(const std::string &type)
{
    if (type == "TTrilinearInterpolator")
    {
        return TInterpolator::TTrilinearInterpolator;
    }

    throw NotSupportError("This interpolator is not supported");
}
////// end interpolator type
////// start extrapolator type
enum class CExtrapolator
{
    CContinuationExtrapolator,
    CErrExtrapolator,
    CNearestPointExtrapolator,
};

enum class TExtrapolator
{
    TZeroExtrapolator,
    TErrExtrapolator,
};
CExtrapolator CExtrapolatorype(const std::string &type)
{
    if (type == "CContinuationExtrapolator")
    {
        return CExtrapolator::CContinuationExtrapolator;
    }
    else if (type == "CErrExtrapolator")
    {
        return CExtrapolator::CErrExtrapolator;
    }
    else if (type == "CNearestPointExtrapolator")
    {
        return CExtrapolator::CNearestPointExtrapolator;
    }

    throw NotSupportError("This extrapolator is not supported");
}

TExtrapolator TExtrapolatorype(const std::string &type)
{
    if (type == "TZeroExtrapolator")
    {
        return TExtrapolator::TZeroExtrapolator;
    }
    else if (type == "TExtrapolator")
    {
        return TExtrapolator::TErrExtrapolator;
    }
    throw NotSupportError("This extrapolator is not supported");
}
////// end extrapolator type
ITMD GenericTMDFactory::mkTMD(const std::string &pdfSetName, int setMember)
{
    auto infoPathPair = StandardInfoFilePath(pdfSetName);
    if (infoPathPair.second != ErrorType::None)
    {
        throw FileLoadException("Unable to find info file of PDF set " + pdfSetName);
    }
    TReader readerType;
    auto [impelmentationInfo, error] = YamlImpelemntationInfoReader(*infoPathPair.first);
    if ((*impelmentationInfo).reader == "")
    {
        readerType = TReader::TDefaultLHAPDF_TMDReader;
    }
    else
    {
        readerType = TReaderType((*impelmentationInfo).reader);
    }

    TInterpolator interpolatorType;
    if ((*impelmentationInfo).interpolator == "")
    {
        interpolatorType = TInterpolator::TTrilinearInterpolator;
    }
    else
    {
        interpolatorType = TInterpolatorType((*impelmentationInfo).interpolator);
    }
    TExtrapolator extrapolatorType;
    if ((*impelmentationInfo).extrapolator == "")
    {
        extrapolatorType = TExtrapolator::TZeroExtrapolator;
    }
    else
    {
        extrapolatorType = TExtrapolatorype((*impelmentationInfo).extrapolator);
    }

    if (readerType == TReader::TDefaultLHAPDF_TMDReader)
    {
        if (extrapolatorType == TExtrapolator::TErrExtrapolator)
        {
            return ITMD(GenericPDF<TMDPDFTag, TDefaultLHAPDF_TMDReader, TTrilinearInterpolator,
                                   TErrExtrapolator>(pdfSetName, setMember));
        }
        else if (extrapolatorType == TExtrapolator::TZeroExtrapolator)
        {
            return ITMD(GenericPDF<TMDPDFTag, TDefaultLHAPDF_TMDReader, TTrilinearInterpolator,
                                   TZeroExtrapolator>(pdfSetName, setMember));
        }
    }
    throw NotSupportError("Not known combination of Reader, Interpolator, "
                          "Extrapolator is selected for this  UPDF");
}

ICPDF GenericCPDFFactory::mkCPDF(const std::string &pdfSetName, int setMember)
{
    auto infoPathPair = StandardInfoFilePath(pdfSetName);
    if (infoPathPair.second != ErrorType::None)
    {
        throw FileLoadException("Unable to find info file of PDF set " + pdfSetName);
    }
    CReader readerType;
    auto [impelmentationInfo, error] = YamlImpelemntationInfoReader(*infoPathPair.first);
    auto selectedReader = (*impelmentationInfo).reader;
    if (selectedReader == "")
    {
        readerType = CReader::CDefaultLHAPDFFileReader;
    }
    else
    {
        readerType = CReaderType(selectedReader);
    }
    CInterpolator interpolatorType;
    auto selectedInterpolator = (*impelmentationInfo).interpolator;
    if (selectedInterpolator == "")
    {
        interpolatorType = CInterpolator::CLHAPDFBicubicInterpolator;
    }
    else
    {
        interpolatorType = CInterpolatorType(selectedInterpolator);
    }
    CExtrapolator extrapolatorType;
    auto selectedExtrapolator = (*impelmentationInfo).extrapolator;
    if (selectedExtrapolator == "")
    {
        extrapolatorType = CExtrapolator::CContinuationExtrapolator;
    }
    else
    {
        extrapolatorType = CExtrapolatorype(selectedExtrapolator);
    }

    if (readerType == CReader::CDefaultLHAPDFFileReader)
    {
        if (interpolatorType == CInterpolator::CLHAPDFBicubicInterpolator)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return ICPDF(
                    std::move(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                         CLHAPDFBicubicInterpolator,
                                         CContinuationExtrapolator<CLHAPDFBicubicInterpolator>>(
                        pdfSetName, setMember)));
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBilinearInterpolator, CErrExtrapolator>(pdfSetName,
                                                                                       setMember));
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBilinearInterpolator,
                                        CNearestPointExtrapolator<CLHAPDFBilinearInterpolator>>(
                    pdfSetName, setMember));
            }
        }
        else if (interpolatorType == CInterpolator::CLHAPDFBilinearInterpolator)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBilinearInterpolator,
                                        CContinuationExtrapolator<CLHAPDFBilinearInterpolator>>(
                    pdfSetName, setMember));
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBilinearInterpolator, CErrExtrapolator>(pdfSetName,
                                                                                       setMember));
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return ICPDF(GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader,
                                        CLHAPDFBilinearInterpolator,
                                        CNearestPointExtrapolator<CLHAPDFBilinearInterpolator>>(
                    pdfSetName, setMember));
            }
        }
    }

    throw NotSupportError("Not known combination of Reader, Interpolator, "
                          "Extrapolator is selected for this collinear PDF");
}
IQCDCoupling CouplingFactory::mkCoupling(const std::string &pdfSetName)
{
    std::pair<std::optional<std::string>, ErrorType> pdfSetInfo = StandardInfoFilePath(pdfSetName);
    YamlCouplingInfo couplingInfo_;
    if (pdfSetInfo.second != ErrorType::None)
    {
        throw FileLoadException("PDFset info " + pdfSetName + " not found");
    }
    std::pair<std::optional<YamlCouplingInfo>, ErrorType> couplingInfo =
        YamlCouplingInfoReader(*pdfSetInfo.first);
    if (couplingInfo.second != ErrorType::None)
    {
        throw std::runtime_error("Coupling info not found!");
    }

    couplingInfo_ = *couplingInfo.first;
    if (couplingInfo_.alphaCalcMethod == AlphasType::ipol)
    {
        InterpolateQCDCoupling interpCoupling = InterpolateQCDCoupling();
        interpCoupling.initialize(couplingInfo_);
        return IQCDCoupling(interpCoupling);
    }
    if (couplingInfo_.alphaCalcMethod == AlphasType::analytic)
    {
        AnalyticQCDCoupling analyticCoupling = AnalyticQCDCoupling();
        analyticCoupling.initialize(couplingInfo_);
        return IQCDCoupling(analyticCoupling);
    }
    if (couplingInfo_.alphaCalcMethod == AlphasType::ode)
    {
        ODEQCDCoupling odeQCDCoupling = ODEQCDCoupling();
        odeQCDCoupling.initialize(couplingInfo_);
        return IQCDCoupling(odeQCDCoupling);
    }
    throw NotSupportError("The requested coupling approach is currently not supported");
}

} // namespace PDFxTMD
