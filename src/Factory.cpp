#include "Factory.h"
#include "Common/YamlInfoReader.h"
#include "GenericPDF.h"
#include "Implementation/Coupling/Interpolation/YamlCouplingInterp.h"
#include "Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h"
#include "Implementation/Extrapolator/Collinear/CErrExtrapolator.h"
#include "Implementation/Extrapolator/Collinear/CNearestPointExtrapolator.h"
#include "Implementation/Extrapolator/TMD/TErrExtrapolator.h"
#include "Implementation/Extrapolator/TMD/TZeroExtrapolator.h"
#include "Implementation/Interpolator/Collinear/CBilinearInterpolator.h"
#include "Implementation/Interpolator/Collinear/GSL/CGSLBicubic.h"
#include "Implementation/Interpolator/Collinear/GSL/CGSLBilinear.h"
#include "Implementation/Interpolator/TMD/TTrilinearInterpolator.h"
#include "Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "Implementation/Reader/TMD/TDefaultAllFlavorReader.h"
#include <memory>

namespace PDFxTMD
{

//////// start reader types
enum class TReader
{
    TDefaultAllFlavorReader,
};

enum class CReader
{
    UNKNOWN,
    CDefaultLHAPDFFileReader
};
TReader TReaderType(const std::string &type)
{
    if (type == "TDefaultAllFlavorReader")
    {
        return TReader::TDefaultAllFlavorReader;
    }

    throw NotSupportError("This file reader is not supported");
}

CReader CReaderType(const std::string &type)
{
    if (type == "DefaultLHAPDFFileReader")
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
    CBilinearInterpolator,
    CGSLBilinear,
    CGSLBicubic
};

CInterpolator CInterpolatorType(const std::string &type)
{
    if (type == "CBilinearInterpolator")
    {
        return CInterpolator::CBilinearInterpolator;
    }
    else if (type == "CGSLBilinear")
    {
        return CInterpolator::CGSLBilinear;
    }
    else if (type == "CGSLBicubic")
    {
        return CInterpolator::CGSLBicubic;
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
    PDFSetDownloadHandler downloadHandler;
    if (!downloadHandler.Start(pdfSetName))
    {
        throw FileLoadException("Unable to find, or download pdf set path!");
    }
    auto infoPathPair = StandardInfoFilePath(pdfSetName);
    if (infoPathPair.second != ErrorType::None)
    {
        throw FileLoadException("Unable to find info file of PDF set " + pdfSetName);
    }
    TReader readerType;
    auto [impelmentationInfo, error] = YamlImpelemntationInfoReader(*infoPathPair.first);
    if ((*impelmentationInfo).reader == "")
    {
        readerType = TReader::TDefaultAllFlavorReader;
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
        extrapolatorType = TExtrapolator::TErrExtrapolator;
    }
    else
    {
        extrapolatorType = TExtrapolatorype((*impelmentationInfo).extrapolator);
    }

    if (readerType == TReader::TDefaultAllFlavorReader)
    {
        if (extrapolatorType == TExtrapolator::TErrExtrapolator)
        {
            return ITMD(
                GenericPDF<TMDPDFTag, TDefaultAllFlavorReader, TTrilinearInterpolator,
                           TErrExtrapolator<TDefaultAllFlavorReader>>(pdfSetName, setMember));
        }
        else if (extrapolatorType == TExtrapolator::TZeroExtrapolator)
        {
            return ITMD(
                GenericPDF<TMDPDFTag, TDefaultAllFlavorReader, TTrilinearInterpolator,
                           TZeroExtrapolator<TDefaultAllFlavorReader>>(pdfSetName, setMember));
        }
    }
    throw NotSupportError("Not known combination of Reader, Interpolator, "
                          "Extrapolator is selected for this  UPDF");
}

ICPDF GenericCPDFFactory::mkCPDF(const std::string &pdfSetName, int setMember)
{
    PDFSetDownloadHandler downloadHandler;
    if (!downloadHandler.Start(pdfSetName))
    {
        throw FileLoadException("Unable to find, or download pdf set path!");
    }
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
        interpolatorType = CInterpolator::CBilinearInterpolator;
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
        if (interpolatorType == CInterpolator::CBilinearInterpolator)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return ICPDF(
                    GenericPDF<
                        CollinearPDFTag, CDefaultLHAPDFFileReader, CBilinearInterpolator,
                        CContinuationExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>(
                        pdfSetName, setMember));
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return ICPDF(
                    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CBilinearInterpolator,
                               CErrExtrapolator<CDefaultLHAPDFFileReader>>(pdfSetName, setMember));
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return ICPDF(
                    GenericPDF<
                        CollinearPDFTag, CDefaultLHAPDFFileReader, CBilinearInterpolator,
                        CNearestPointExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>(
                        pdfSetName, setMember));
            }
        }
        else if (interpolatorType == CInterpolator::CGSLBilinear)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return ICPDF(
                    GenericPDF<
                        CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBilinearInterpolator,
                        CContinuationExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>(
                        pdfSetName, setMember));
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return ICPDF(
                    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBilinearInterpolator,
                               CErrExtrapolator<CDefaultLHAPDFFileReader>>(pdfSetName, setMember));
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return ICPDF(
                    GenericPDF<
                        CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBilinearInterpolator,
                        CNearestPointExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>(
                        pdfSetName, setMember));
            }
        }
        else if (interpolatorType == CInterpolator::CGSLBicubic)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return ICPDF(
                    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBicubicInterpolator,
                               CContinuationExtrapolator<CDefaultLHAPDFFileReader,
                                                         CGSLBicubicInterpolator>>(pdfSetName,
                                                                                   setMember));
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return ICPDF(
                    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBicubicInterpolator,
                               CErrExtrapolator<CDefaultLHAPDFFileReader>>(pdfSetName, setMember));
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return ICPDF(
                    GenericPDF<
                        CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBicubicInterpolator,
                        CNearestPointExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>(
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
    m_alphaType = couplingInfo_.alphaCalcMethod;
    if (m_alphaType == AlphasType::ipol)
    {
        std::vector<double> mu2Vec;
        mu2Vec.resize(couplingInfo_.mu_vec.size());
        std::transform(couplingInfo_.mu_vec.begin(), couplingInfo_.mu_vec.end(), mu2Vec.begin(),
                       [](double q) { return q * q; });
        YamlCouplingInterp test = YamlCouplingInterp();
        test.SetParamters(mu2Vec, couplingInfo_.alphas_vec);
        return IQCDCoupling(test);
    }
    throw NotSupportError("The requested coupling approach is currently not supported");
}

} // namespace PDFxTMD
