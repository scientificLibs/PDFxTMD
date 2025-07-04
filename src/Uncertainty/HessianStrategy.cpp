#include <PDFxTMDLib/Common/PartonUtils.h>
#include <PDFxTMDLib/Uncertainty/HessianStrategy.h>
#include <cmath>

namespace PDFxTMD
{
void HessianStrategy::Uncertainty(const std::vector<double> &values, const int numCoreErrMember,
                                  const double cl, PDFUncertainty &uncertainty)
{
    // Calculate the asymmetric and symmetric Hessian uncertainties
    // using Eqs. (2.1), (2.2) and (2.6) of arXiv:1106.5788v2.
    double errplus = 0, errminus = 0, errsymm = 0;
    for (size_t ieigen = 1; ieigen <= numCoreErrMember / 2; ieigen++)
    {
        errplus += SQR(std::max(
            std::max(values[2 * ieigen - 1] - values[0], values[2 * ieigen] - values[0]), 0.0));
        errminus += SQR(std::max(
            std::max(values[0] - values[2 * ieigen - 1], values[0] - values[2 * ieigen]), 0.0));
        errsymm += SQR(values[2 * ieigen - 1] - values[2 * ieigen]);
    }
    uncertainty.errsymm = 0.5 * std::sqrt(errsymm);
    uncertainty.errplus = std::sqrt(errplus);
    uncertainty.errminus = std::sqrt(errminus);
}

double HessianStrategy::Correlation(const std::vector<double> &valuesA,
                                    const std::vector<double> &valuesB, const int numCoreErrMember)
{
    PDFUncertainty errA;
    Uncertainty(valuesA, numCoreErrMember, -1, errA);
    PDFUncertainty errB;
    Uncertainty(valuesB, numCoreErrMember, -1, errB);
    double cor = 0.0;
    // Calculate the correlation over asymm Hessian vectors using Eq. (2.5) of arXiv:1106.5788
    for (size_t ieigen = 1; ieigen < numCoreErrMember / 2 + 1; ieigen++)
    {
        cor += (valuesA[2 * ieigen - 1] - valuesA[2 * ieigen]) *
               (valuesB[2 * ieigen - 1] - valuesB[2 * ieigen]);
    }
    cor /= 4.0 * errA.errsymm * errB.errsymm;
    return cor;
}
} // namespace PDFxTMD