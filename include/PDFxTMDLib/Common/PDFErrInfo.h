#pragma once
#include <PDFxTMDLib/Common/StringUtils.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlErrorInfo.h>
#include <string>
#include <utility>
#include <vector>

namespace PDFxTMD
{
/// @brief Structure encoding the structure of the PDF error-set (Taken from LHAPDF library code)
struct PDFErrInfo
{
    using EnvPart = std::pair<std::string, size_t>;
    using EnvParts = std::vector<EnvPart>;
    using QuadParts = std::vector<EnvParts>;

    /// Constructor
    PDFErrInfo(QuadParts parts, double cl, const std::string &errtypestr = "")
        : qparts(parts), conflevel(cl), errtype(errtypestr)
    {
    }

    /// Default constructor (for STL, Cython, etc.)
    PDFErrInfo() = default;

    /// Error-set quadrature parts
    QuadParts qparts;

    /// Default confidence-level
    double conflevel;

    /// Error-type annotation
    std::string errtype;

    /// Calculated name of a quadrature part
    std::string coreType() const
    {
        return qpartName(0);
    }

    /// Calculated name of a quadrature part
    std::string qpartName(size_t iq) const;
    /// Calculated names of all quadrature parts
    std::vector<std::string> qpartNames() const;

    /// Number of core-set members
    size_t nmemCore() const;
    /// Number of par-set members
    size_t nmemPar() const;
    /// Parse extended error type syntax
    static PDFErrInfo CalculateErrorInfo(const YamlErrorInfo &yamlErrInfo);
};
} // namespace PDFxTMD
