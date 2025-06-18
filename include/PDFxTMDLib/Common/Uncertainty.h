#pragma once
#include <vector>
#include <cmath>
namespace PDFxTMD
{
/// @brief Structure for storage of uncertainty info calculated over a PDF error set
///
/// Used by the PDFSet::uncertainty functions.
struct PDFUncertainty
{
    using ErrPairs = std::vector<std::pair<double, double>>;

    /// Constructor
    PDFUncertainty(double cent = 0, double eplus = 0, double eminus = 0, double esymm = 0,
                   double scalefactor = 1, double eplus_pdf = 0, double eminus_pdf = 0,
                   double esymm_pdf = 0, double eplus_par = 0, double eminus_par = 0,
                   double esymm_par = 0)
        : central(cent), errplus(eplus), errminus(eminus), errsymm(esymm), scale(scalefactor),
          errplus_pdf(eplus_pdf), errminus_pdf(eminus_pdf), errsymm_pdf(esymm_pdf),
          errplus_par(eplus_par), errminus_par(eminus_par), errsymm_par(esymm_par)
    {
    }

    /// Variables for the central value, +ve, -ve & symmetrised errors, and a CL scalefactor
    double central, errplus, errminus, errsymm, scale;

    /// Variables for separate PDF and parameter variation errors with combined sets
    double errplus_pdf, errminus_pdf, errsymm_pdf;
    double errplus_par, errminus_par, errsymm_par;
    /// Full error-breakdown of all quadrature uncertainty components, as (+,-) pairs
    ErrPairs errparts;
};
const PDFUncertainty NULL_PDF_UNCERTAINTY = []() {
    PDFUncertainty temp(
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN() 
    );
    temp.errparts = {};
    return temp;
}();
} // namespace PDFxTMD
