#pragma once
#include "PDFxTMDLib/Common/ConfigWrapper.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/PDFUtils.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/TMD/TZeroExtrapolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/Collinear/CLHAPDFBicubicInterpolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/TMD/TTrilinearInterpolator.h"
#include "PDFxTMDLib/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Implementation/Reader/TMD/TDefaultLHAPDF_TMDReader.h"
#include "PDFxTMDLib/Interface/IExtrapolator.h"
#include "PDFxTMDLib/Interface/IInterpolator.h"
#include "PDFxTMDLib/Interface/IReader.h"
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace PDFxTMD
{
struct TMDPDFTag
{
};
struct CollinearPDFTag
{
};

/**
 * @brief Tag types for PDF implementations
 *
 * These tag types are used to differentiate between different PDF implementations:
 * - TMDPDFTag: Used for Transverse Momentum Dependent PDFs
 * - CollinearPDFTag: Used for standard collinear PDFs
 *
 * To learn more about the different PDF implementations, please refer to the factory classes in the
 * Factory.h file.
 *
 * @tparam Reader The reader class responsible for loading PDF data from the relevant grid file
 * @tparam Interpolator The interpolator class that provides interpolation between grid points
 * @tparam Extrapolator The extrapolator class that handles values outside the grid boundaries
 */

// Type trait to get default implementations based on tag
template <typename Tag> struct DefaultPDFImplementations;

// Specialization for TMDPDFTag
template <> struct DefaultPDFImplementations<TMDPDFTag>
{
    using Reader = TDefaultLHAPDF_TMDReader;
    using Interpolator = TTrilinearInterpolator;
    using Extrapolator = TZeroExtrapolator;
};

// Specialization for CollinearPDFTag
template <> struct DefaultPDFImplementations<CollinearPDFTag>
{
    using Reader = CDefaultLHAPDFFileReader;
    using Interpolator = CLHAPDFBicubicInterpolator;
    using Extrapolator = CContinuationExtrapolator<CLHAPDFBicubicInterpolator>;
};

template <typename Tag, typename Reader = typename DefaultPDFImplementations<Tag>::Reader,
          typename Interpolator = typename DefaultPDFImplementations<Tag>::Interpolator,
          typename Extrapolator = typename DefaultPDFImplementations<Tag>::Extrapolator>
class GenericPDF
{
  public:
    GenericPDF(const std::string &pdfName, int setNumber)
        : m_pdfName(pdfName), m_setNumber(setNumber)
    {
        loadStandardInfo();
        loadData();
    }
    ~GenericPDF()
    {
    }
    /**
     * @brief Retrieves the collinear PDF value for a specific parton flavor
     *
     * @param flavor The parton flavor
     * @param x Bjorken x variable (momentum fraction)
     * @param mu2 Factorization scale squared
     */
    double pdf(PartonFlavor flavor, double x, double mu2) const
    {
        if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
        {
            if (isInRange(m_reader, x, mu2))
                return m_interpolator.interpolate(flavor, x, mu2);
            return m_extrapolator.extrapolate(flavor, x, mu2);
        }
        else
        {
            throw std::logic_error(
                "pdf(PartonFlavor, double, double) is not supported for this tag.");
        }
    }
    /**
     * @brief Evaluate the array of Collinear PDF values for {tbar, bbar, cbar, sbar, ubar, dbar,
     * g, d, u, s, c, b, t}
     *
     * @param x Bjorken x variable (momentum fraction)
     * @param mu2 Factorization scale squared
     * @param output The array to store the Collinear PDF values for all flavors.
     *
     * @return std::array<double, 13> The vector of Collinear PDF values
     *
     * @throws std::logic_error If called on a PDF type that doesn't support Collinear PDF
     */
    void pdf(double x, double mu2, std::array<double, DEFAULT_TOTAL_PDFS> &output) const
    {
        if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
        {
            if (isInRange(m_reader, x, mu2))
                return m_interpolator.interpolate(x, mu2, output);
            return m_extrapolator.extrapolate(x, mu2, output);
        }
        else
        {
            throw std::logic_error("pdf(double, double, std::array<double, DEFAULT_TOTAL_PDFS>&) "
                                   "is not supported for this tag.");
        }
    }
    GenericPDF(GenericPDF &&other) noexcept
        : m_pdfName(std::move(other.m_pdfName)), m_setNumber(other.m_setNumber),
          m_reader(std::move(other.m_reader)), m_interpolator(std::move(other.m_interpolator)),
          m_extrapolator(std::move(other.m_extrapolator)), m_stdInfo(std::move(other.m_stdInfo))
    {
        m_interpolator.initialize(&m_reader);
        if constexpr (std::is_base_of_v<IcAdvancedPDFExtrapolator<Extrapolator>, Extrapolator>)
        {
            m_extrapolator.setInterpolator(&m_interpolator);
        }
    }
    GenericPDF(const GenericPDF &other)
        : m_pdfName(other.m_pdfName), m_setNumber(other.m_setNumber), m_reader(other.m_reader),
          m_interpolator(other.m_interpolator), m_extrapolator(other.m_extrapolator),
          m_stdInfo(other.m_stdInfo)
    {
        m_interpolator.initialize(&m_reader);
        if constexpr (std::is_base_of_v<IcAdvancedPDFExtrapolator<Extrapolator>, Extrapolator>)
        {
            m_extrapolator.setInterpolator(&m_interpolator);
        }
    }
    /**
     * @brief Evaluates the TMD PDF value for a specific parton flavor
     *
     * @param flavor The parton flavor
     * @param x Bjorken x variable (momentum fraction)
     * @param kt2 Transverse momentum squared
     */
    double tmd(PartonFlavor flavor, double x, double kt2, double mu2)
    {
        if constexpr (std::is_same_v<Tag, TMDPDFTag>)
        {
            if (isInRange(m_reader, x, kt2, mu2))
                return m_interpolator.interpolate(flavor, x, kt2, mu2);

            return m_extrapolator.extrapolate(flavor, x, kt2, mu2);
        }
        else
        {
            throw std::logic_error(
                "pdf(double, double, std::array<double, 13>&) is not supported for this tag.");
        }
    }
    /**
     * @brief Evaluates the vector of TMD PDF values for {tbar, bbar, cbar, sbar, ubar, dbar, g, d,
     * u, s, c, b, t}
     *
     * @param x Bjorken x variable (momentum fraction)
     * @param kt2 Transverse momentum squared
     * @param mu2 Factorization scale squared
     *
     * @return std::vector<double> The vector of TMD PDF values
     *
     * @throws std::logic_error If called on a PDF type that doesn't support TMD
     */
    void tmd(double x, double kt2, double mu2, std::array<double, DEFAULT_TOTAL_PDFS> &output)
    {
        if constexpr (std::is_same_v<Tag, TMDPDFTag>)
        {
            isInRange(m_reader, x, kt2, mu2) ? m_interpolator.interpolate(x, kt2, mu2, output)
                                             : m_extrapolator.extrapolate(x, kt2, mu2, output);
        }
        else
        {
            throw std::logic_error(
                "pdf(double, double, std::array<double, 13>&) is not supported for this tag.");
        }
    }
    /**
     * @brief Retrieves the standard PDF info
     *
     * @return YamlStandardTMDInfo The standard PDF info
     */
    YamlStandardTMDInfo getStdPDFInfo() const
    {
        return m_stdInfo;
    }

  private:
    void loadStandardInfo()
    {
        auto infoPathPair = StandardInfoFilePath(m_pdfName);
        if (infoPathPair.second != ErrorType::None)
            throw FileLoadException("Unable to find info file of PDF set " + m_pdfName);
        if constexpr (std::is_same_v<Tag, TMDPDFTag>)
        {
            auto pdfStandardInfo = YamlStandardPDFInfoReader(*infoPathPair.first);
            if (pdfStandardInfo.second != ErrorType::None)
                throw InvalidFormatException("Invalid standard info file " + *infoPathPair.first);
            m_stdInfo = *pdfStandardInfo.first;
        }
        else if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
        {
            auto pdfStandardInfo = YamlStandardPDFInfoReader(*infoPathPair.first);
            if (pdfStandardInfo.second != ErrorType::None)
                throw InvalidFormatException("Invalid standard info file " + *infoPathPair.first);
            m_stdInfo = *pdfStandardInfo.first;
        }
        else
        {
            static_assert(!std::is_same_v<Tag, Tag>, "Unsupported Tag");
        }
    }
    void loadData()
    {
        m_reader.read(m_pdfName, m_setNumber);
        m_interpolator.initialize(&m_reader);
        if constexpr (std::is_same_v<Tag, CollinearPDFTag>)
        {
            if constexpr (std::is_base_of_v<IcAdvancedPDFExtrapolator<Extrapolator>, Extrapolator>)
            {
                m_extrapolator.setInterpolator(&m_interpolator);
            }
        }
    }
    std::string m_pdfName;
    int m_setNumber;
    Reader m_reader;
    Interpolator m_interpolator;
    Extrapolator m_extrapolator;
    YamlStandardTMDInfo m_stdInfo;
};

// Convenient type aliases for common use cases
using TMDPDF = GenericPDF<TMDPDFTag>;
using CollinearPDF = GenericPDF<CollinearPDFTag>;

} // namespace PDFxTMD
