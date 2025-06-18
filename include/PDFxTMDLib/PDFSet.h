///this class and all uncertainties related features are based on the LHAPDF library, but is heavily refactored to improve its readibility and even its performance!
#pragma once
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include "PDFxTMDLib/Common/Uncertainty.h"
#include "PDFxTMDLib/GenericPDF.h"
#include "PDFxTMDLib/ICPDF.h"
#include "PDFxTMDLib/ITMD.h"
#include <PDFxTMDLib/Common/PDFErrInfo.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlErrorInfo.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h>
#include <PDFxTMDLib/Factory.h>
#include <PDFxTMDLib/Interface/IUncertainty.h>
#include <PDFxTMDLib/Uncertainty/HessianStrategy.h>
#include <PDFxTMDLib/Uncertainty/ReplicasPercentileStrategy.h>
#include <PDFxTMDLib/Uncertainty/ReplicasStdDevStrategy.h>
#include <PDFxTMDLib/Uncertainty/SymmHessianStrategy.h>
#include <PDFxTMDLib/Common/MathUtils.h>
#include <PDFxTMDLib/Common/Exception.h>

namespace PDFxTMD
{

// Forward declarations
template <typename T>
struct PDFType;

template <>
struct PDFType<TMDPDFTag> {
    using type = ITMD;
};

template <>
struct PDFType<CollinearPDFTag> {
    using type = ICPDF;
};

template <typename Tag>
class PDFSet {
public:
    using PDF_t = typename PDFType<Tag>::type;

    explicit PDFSet(std::string pdfSetName, bool alternativeReplicaUncertainty = false)
        : m_pdfSetName(std::move(pdfSetName)),
          m_alternativeReplicaUncertainty(alternativeReplicaUncertainty) {
        Initialize();
    }

    std::vector<PDF_t> GetPDFSet() const {
        return m_PDFSet_;
    }

    template <typename T = Tag,
              typename = std::enable_if_t<std::is_same_v<T, TMDPDFTag>>>
    void Uncertainty(PartonFlavor flavor, double x, double kt2, double mu2,
                     double cl, PDFUncertainty& resUncertainty) {
        throw NotSupportError("PDFxTMD::PDFSet::Uncertainty is not defined for TMDs");
        // const auto pdfs = CalculatePDFValues(flavor, x, kt2, mu2);
        // m_uncertaintyStrategy_->Uncertainty(pdfs, m_pdfErrInfo.nmemCore(), cl, resUncertainty);
        // resUncertainty.central = pdfs[0];
    }

    template <typename T = Tag,
              typename = std::enable_if_t<std::is_same_v<T, CollinearPDFTag>>>
    void Uncertainty(PartonFlavor flavor, double x, double mu2, double cl,
                     PDFUncertainty& resUncertainty) {
        const auto pdfs = CalculatePDFValues(flavor, x, mu2);
        const double reqCL = ValidateAndGetCL(cl);

        m_uncertaintyStrategy_->Uncertainty(pdfs, m_pdfErrInfo.nmemCore(), reqCL, resUncertainty);
        resUncertainty.central = pdfs[0];

        ApplyConfidenceLevelScaling(resUncertainty, reqCL);
        StoreCoreVariationErros(resUncertainty);
        CalculateParameterVariationErrors(resUncertainty, pdfs);
    }
    template <typename T = Tag,
              typename = std::enable_if_t<std::is_same_v<T, CollinearPDFTag>>>
    PDFUncertainty Uncertainty(PartonFlavor flavor, double x, double mu2, double cl = NO_REQUESTED_CONFIDENCE_LEVEL) {
        PDFUncertainty resUncertainty;
        const auto pdfs = CalculatePDFValues(flavor, x, mu2);
        const double reqCL = ValidateAndGetCL(cl);

        m_uncertaintyStrategy_->Uncertainty(pdfs, m_pdfErrInfo.nmemCore(), reqCL, resUncertainty);
        resUncertainty.central = pdfs[0];

        ApplyConfidenceLevelScaling(resUncertainty, reqCL);
        StoreCoreVariationErros(resUncertainty);
        CalculateParameterVariationErrors(resUncertainty, pdfs);
        return resUncertainty;
    }
    void Uncertainty(const std::vector<double> &values, double cl, PDFUncertainty &resUncertainty)
    {
        if (values.size() != m_pdfSetErrorInfo.size())
            throw InvalidInputError("Error in PDFxTMD::PDFSet::Uncertainty. Input vector must contain values for all PDF members.");
        const double reqCL = ValidateAndGetCL(cl);

        m_uncertaintyStrategy_->Uncertainty(values, m_pdfErrInfo.nmemCore(), reqCL, resUncertainty);
        resUncertainty.central = values[0];

        ApplyConfidenceLevelScaling(resUncertainty, reqCL);
        StoreCoreVariationErros(resUncertainty);
        CalculateParameterVariationErrors(resUncertainty, values);
    }
    template <typename T = Tag,
              typename = std::enable_if_t<std::is_same_v<T, CollinearPDFTag>>>
    double Correlation(PartonFlavor flavorA, double xA, double mu2A, PartonFlavor flavorB, double xB, double mu2B) {
        const auto pdfsA = CalculatePDFValues(flavorA, xA, mu2A);
        const auto pdfsB = CalculatePDFValues(flavorB, xB, mu2B);
        return m_uncertaintyStrategy_->Correlation(pdfsA, pdfsB, m_pdfErrInfo.nmemCore());
    }
    double Correlation(const std::vector<double>& valuesA, const std::vector<double>& valuesB) const
    {
        if (valuesA.size() != m_pdfSetErrorInfo.size() || valuesB.size() != m_pdfSetErrorInfo.size())
            throw InvalidInputError("Error in PDFxTMD::PDFSet::Correlation. Input vectors must contain values for all PDF members.");
        return m_uncertaintyStrategy_->Correlation(valuesA, valuesB, m_pdfErrInfo.nmemCore());
    }
private:
    void Initialize() {
        ValidatePDFSetName();
        LoadYamlInfo();
        CreatePDFSet();
        InitializeUncertaintyStrategy();
    }

    void ValidatePDFSetName() {
        if (m_pdfSetName.empty()) {
            throw InvalidInputError("PDFxTMD::PDFSet::ValidatePDFSetName: PDF set name cannot be empty.");
        }
    }

    void LoadYamlInfo() {
        auto infoPathPair = StandardInfoFilePath(m_pdfSetName);
        if (infoPathPair.second != ErrorType::None)
            throw FileLoadException("PDFxTMD::PDFSet::LoadYamlInfo: Unable to find info file of PDF set " + m_pdfSetName);
        auto yamlInfoErrPair = YamlErrorInfoReader(*infoPathPair.first);
        if (yamlInfoErrPair.second != ErrorType::None || !yamlInfoErrPair.first.has_value()) {
            m_isValid = false;
        } else {
            m_pdfSetErrorInfo = yamlInfoErrPair.first.value();
            m_isValid = true;
        }

        auto yamlStdInfoErrPair = YamlStandardPDFInfoReader(*infoPathPair.first);
        if (yamlStdInfoErrPair.second != ErrorType::None || !yamlStdInfoErrPair.first.has_value()) {
            m_isValid = false;
        } else {
            m_pdfSetStdInfo = yamlStdInfoErrPair.first.value();
        }

        if (!m_isValid) {
            throw InvalidInputError("PDFSet: Invalid PDF set '" + m_pdfSetName + "'. Please check the YAML file.");
        }
    }

    void CreatePDFSet() {
        m_PDFSet_.reserve(m_pdfSetStdInfo.NumMembers);
        for (int i = 0; i < m_pdfSetStdInfo.NumMembers; ++i) {
            if constexpr (std::is_same_v<Tag, TMDPDFTag>) {
                m_PDFSet_.emplace_back(PDFxTMD::GenericTMDFactory().mkTMD(m_pdfSetName, i));
            } else if constexpr (std::is_same_v<Tag, CollinearPDFTag>) {
                m_PDFSet_.emplace_back(PDFxTMD::GenericCPDFFactory().mkCPDF(m_pdfSetName, i));
            } else {
                static_assert(!std::is_same_v<Tag, Tag>, "Unsupported Tag");
            }
        }
    }

    void InitializeUncertaintyStrategy() {
        m_pdfErrInfo = PDFErrInfo::CalculateErrorInfo(m_pdfSetErrorInfo);
        if (m_pdfErrInfo.nmemCore() <= 0) {
            throw InvalidInputError("Error in PDFxTMD::PDFSet::InitializeUncertaintyStrategy. PDF set must contain more than just the central value.");
        }

        const auto& coreType = m_pdfErrInfo.coreType();
        if (coreType == "replicas") {
            if (m_alternativeReplicaUncertainty) {
                m_uncertaintyStrategy_ = std::make_unique<ReplicasPercentileStrategy>();
            } else {
                m_uncertaintyStrategy_ = std::make_unique<ReplicasStdDevStrategy>();
            }
        } else if (coreType == "hessian") {
            m_uncertaintyStrategy_ = std::make_unique<HessianStrategy>();
        } else if (coreType == "symm-hessian") {
            m_uncertaintyStrategy_ = std::make_unique<SymmHessianStrategy>();
        } else {
            throw NotSupportError("PDFxTMD::PDFSet::InitializeUncertaintyStrategy: Unsupported error type '" + coreType +
                                     "' for PDF set " + m_pdfSetName + ".");
        }

        m_setCL = (coreType != "replicas") ? m_pdfSetErrorInfo.ErrorConfLevel / 100.0 : CL1SIGMA / 100.0;
    }

    template <typename... Args>
    std::vector<double> CalculatePDFValues(PartonFlavor flavor, Args... args) const {
        std::vector<double> pdfs;
        pdfs.reserve(m_pdfSetStdInfo.NumMembers);
        for (const auto& pdf : m_PDFSet_) {
            if constexpr (sizeof...(args) == 3) { // TMD case
                pdfs.emplace_back(pdf.tmd(flavor, args...));
            } else { // Collinear case
                pdfs.emplace_back(pdf.pdf(flavor, args...));
            }
        }
        return pdfs;
    }

    double ValidateAndGetCL(double cl) const {
        const double reqCL = (cl >= 0) ? cl / 100.0 : m_setCL;
        if (!in_range(reqCL, 0, 1) || !in_range(m_setCL, 0, 1)) {
            throw InvalidInputError("Error in PDFxTMD::PDFSet::ValidateAndGetCL. Requested or PDF set "
                                     "confidence level outside [0,1] range.");
        }
        return reqCL;
    }

    void ApplyConfidenceLevelScaling(PDFUncertainty& rtn, double reqCL) const {
        if (m_setCL != reqCL && !m_alternativeReplicaUncertainty) {
            double qsetCL = chisquared_quantile(m_setCL, 1);
            double qreqCL = chisquared_quantile(reqCL, 1);
            const double scale = std::sqrt(qreqCL / qsetCL);
            
            rtn.scale = scale;
            rtn.errplus *= scale;
            rtn.errminus *= scale;
            rtn.errsymm *= scale;
        }
    }
    void StoreCoreVariationErros(PDFUncertainty& resUncertainty)
    {
        // Store core variation uncertainties
        resUncertainty.errplus_pdf = resUncertainty.errplus;
        resUncertainty.errminus_pdf = resUncertainty.errminus;
        resUncertainty.errsymm_pdf = resUncertainty.errsymm;
        resUncertainty.errparts.push_back({resUncertainty.errplus_pdf, resUncertainty.errminus_pdf}); ///< @note (+,-) pair-ordering
    }
    void CalculateParameterVariationErrors(PDFUncertainty& rtn, const std::vector<double>& values) const {
        double errsq_par_plus = 0.0;
        double errsq_par_minus = 0.0;
        size_t index = m_pdfErrInfo.nmemCore();

        for (size_t iq = 1; iq < m_pdfErrInfo.qparts.size(); ++iq) {
            const auto& eparts = m_pdfErrInfo.qparts[iq];
            double vmin = rtn.central;
            double vmax = rtn.central;

            for (const auto& epart : eparts) {
                const bool symm = StartsWith(epart.first, "$");
                for (size_t ie = 0; ie < epart.second; ++ie) {
                    index++;
                    if (!symm) {
                        vmin = std::min(values[index], vmin);
                        vmax = std::max(values[index], vmax);
                    } else {
                        const double delta = values[index] - rtn.central;
                        vmin = std::min({values[index], rtn.central - delta, vmin});
                        vmax = std::max({values[index], rtn.central - delta, vmax});
                    }
                }
            }

            const double eplus = vmax - rtn.central;
            const double eminus = rtn.central - vmin;
            rtn.errparts.push_back({eplus, eminus});
            errsq_par_plus += SQR(eplus);
            errsq_par_minus += SQR(eminus);
        }
    }

    std::string m_pdfSetName;
    std::vector<PDF_t> m_PDFSet_;
    YamlErrorInfo m_pdfSetErrorInfo;
    YamlStandardPDFInfo m_pdfSetStdInfo;
    std::unique_ptr<IUncertainty> m_uncertaintyStrategy_;
    PDFErrInfo m_pdfErrInfo;
    double m_setCL;
    bool m_alternativeReplicaUncertainty;
    bool m_isValid = false;
};

} // namespace PDFxTMD