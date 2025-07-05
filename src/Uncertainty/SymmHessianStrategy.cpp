#include <PDFxTMDLib/Common/PartonUtils.h>
#include <PDFxTMDLib/Uncertainty/SymmHessianStrategy.h>
#include <cmath>

namespace PDFxTMD
{
void SymmHessianStrategy::Uncertainty(const std::vector<double> &values, const int numCoreErrMember,
                                      const double cl, PDFUncertainty &uncertainty)
{
    double errsymm = 0;
    for (size_t ieigen = 1; ieigen <= numCoreErrMember; ieigen++)
    {
        errsymm += SQR(values[ieigen] - values[0]);
    }
    errsymm = std::sqrt(errsymm);
    uncertainty.errplus = uncertainty.errminus = uncertainty.errsymm = errsymm;
    uncertainty.central = values[0];
}
double SymmHessianStrategy::Correlation(const std::vector<double> &valuesA,
                                        const std::vector<double> &valuesB,
                                        const int numCoreErrMember)
{
    double cor = 0.;
    PDFUncertainty errA;
    Uncertainty(valuesA, numCoreErrMember, -1, errA);
    PDFUncertainty errB;
    Uncertainty(valuesB, numCoreErrMember, -1, errB);
    // Calculate the correlation over symmetrised Hessian vectors
    for (size_t ieigen = 1; ieigen < numCoreErrMember + 1; ieigen++)
    {
        cor += (valuesA[ieigen] - errA.central) * (valuesB[ieigen] - errB.central);
    }
    cor /= errA.errsymm * errB.errsymm;
    return cor;
}
} // namespace PDFxTMD