#include "Factory.h"

#include "Common/YamlInfoReader.h"
#include "GenericPDF.h"
#include "Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h"
#include "Implementation/Extrapolator/Collinear/CErrExtrapolator.h"
#include "Implementation/Extrapolator/Collinear/CNearestPointExtrapolator.h"
#include "Implementation/Extrapolator/TMD/TErrExtrapolator.h"
#include "Implementation/Extrapolator/TMD/TZeroExtrapolator.h"
#include "Implementation/Interpolator/Collinear/CBilinearInterpolator.h"
#include "Implementation/Interpolator/TMD/TTrilinearInterpolator.h"
#include "Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "Implementation/Reader/TMD/TDefaultAllFlavorReader.h"
#ifdef USE_GSL
#include "Implementation/Interpolator/Collinear/GSL/CGSLBicubic.h"
#include "Implementation/Interpolator/Collinear/GSL/CGSLBilinear.h"
#endif

namespace PDFxTMD
{

//////// start reader types
enum class TReader
{
    TDefaultAllFlavorReader,
};

enum class CReader
{
    CDefaultLHAPDFFileReader,
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
te::poly<ITMD> GenericTMDFactory::mkTMD(const std::string &pdfSetName, int setMember)
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
        readerType = TReader::TDefaultAllFlavorReader;
    }

    TInterpolator interpolatorType;
    if ((*impelmentationInfo).interpolator == "")
    {
        interpolatorType = TInterpolator::TTrilinearInterpolator;
    }
    TExtrapolator extrapolatorType;
    if ((*impelmentationInfo).extrapolator == "")
    {
        extrapolatorType = TExtrapolator::TErrExtrapolator;
    }

    if (readerType == TReader::TDefaultAllFlavorReader)
    {
        if (extrapolatorType == TExtrapolator::TErrExtrapolator)
        {
            return std::make_unique<
                GenericPDF<TMDPDFTag, TDefaultAllFlavorReader, TTrilinearInterpolator,
                           TErrExtrapolator<TDefaultAllFlavorReader>>>(pdfSetName, m_setMember);
        }
        else if (extrapolatorType == TExtrapolator::TZeroExtrapolator)
        {
            return std::make_unique<
                GenericPDF<TMDPDFTag, TDefaultAllFlavorReader, TTrilinearInterpolator,
                           TZeroExtrapolator<TDefaultAllFlavorReader>>>(m_pdfSetName, m_setMember);
        }
    }
    throw NotSupportError("Not known combination of Reader, Interpolator, "
                          "Extrapolator is selected for this  UPDF");
}

te::poly<ICPDF> GenericCPDFFactory::mkCPDF(const std::string &pdfSetName, int setMember)
{
    auto infoPathPair = StandardInfoFilePath(pdfSetName);
    if (infoPathPair.second != ErrorType::None)
    {
        throw FileLoadException("Unable to find info file of PDF set " + pdfSetName);
    }
    CReader readerType;
    auto [impelmentationInfo, error] = YamlImpelemntationInfoReader(*infoPathPair.first);
    if ((*impelmentationInfo).reader == "")
    {
        readerType = CReader::CDefaultLHAPDFFileReader;
    }

    CInterpolator interpolatorType;
    if ((*impelmentationInfo).interpolator == "")
    {
        interpolatorType = CInterpolator::CBilinearInterpolator;
    }
    CExtrapolator extrapolatorType;
    if ((*impelmentationInfo).extrapolator == "")
    {
        extrapolatorType = CExtrapolator::CContinuationExtrapolator;
    }

    if (readerType == CReader::CDefaultLHAPDFFileReader)
    {
        if (interpolatorType == CInterpolator::CBilinearInterpolator)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return std::make_unique<GenericPDF<
                    CollinearPDFTag, CDefaultLHAPDFFileReader, CBilinearInterpolator,
                    CContinuationExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>>(
                    pdfSetName, setMember);
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return std::make_unique<
                    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CBilinearInterpolator,
                               CErrExtrapolator<CDefaultLHAPDFFileReader>>>(pdfSetName, setMember);
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return std::make_unique<GenericPDF<
                    CollinearPDFTag, CDefaultLHAPDFFileReader, CBilinearInterpolator,
                    CNearestPointExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>>(
                    pdfSetName, setMember);
            }
        }
#ifdef USE_GSL
        else if (interpolatorType == CInterpolator::CGSLBilinear)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return std::make_unique<GenericPDF<
                    CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBilinearInterpolator,
                    CContinuationExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>>(
                    pdfSetName, setMember);
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return std::make_unique<
                    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBilinearInterpolator,
                               CErrExtrapolator<CDefaultLHAPDFFileReader>>>(pdfSetName, setMember);
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return std::make_unique<GenericPDF<
                    CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBilinearInterpolator,
                    CNearestPointExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>>(
                    pdfSetName, setMember);
            }
        }
        else if (interpolatorType == CInterpolator::CGSLBicubic)
        {
            if (extrapolatorType == CExtrapolator::CContinuationExtrapolator)
            {
                return std::make_unique<GenericPDF<
                    CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBicubicInterpolator,
                    CContinuationExtrapolator<CDefaultLHAPDFFileReader, CGSLBicubicInterpolator>>>(
                    pdfSetName, setMember);
            }
            else if (extrapolatorType == CExtrapolator::CErrExtrapolator)
            {
                return std::make_unique<
                    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBicubicInterpolator,
                               CErrExtrapolator<CDefaultLHAPDFFileReader>>>(pdfSetName, setMember);
            }
            else if (extrapolatorType == CExtrapolator::CNearestPointExtrapolator)
            {
                return std::make_unique<GenericPDF<
                    CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBicubicInterpolator,
                    CNearestPointExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>>>(
                    pdfSetName, setMember);
            }
        }
#endif
    }

    throw NotSupportError("Not known combination of Reader, Interpolator, "
                          "Extrapolator is selected for this collinear PDF");
}
} // namespace PDFxTMD
