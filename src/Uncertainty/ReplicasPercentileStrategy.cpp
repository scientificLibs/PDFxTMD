#include <PDFxTMDLib/Common/PartonUtils.h>
#include <PDFxTMDLib/Uncertainty/ReplicasPercentileStrategy.h>
#include <algorithm>
#include <cmath>

namespace PDFxTMD
{
void ReplicasPercentileStrategy::Uncertainty(const std::vector<double> &values,
                                             const int numCoreErrMember, const double cl,
                                             PDFUncertainty &uncertainty)
{
    // Compute median and requested CL directly from probability distribution of replicas.
    // Sort "values" into increasing order, ignoring zeroth member (average over replicas).
    // Also ignore possible parameter variations included at the end of the set.
    std::vector<double> sorted(numCoreErrMember);
    std::copy(values.begin() + 1, values.begin() + 1 + numCoreErrMember + 1, sorted.begin());
    std::sort(sorted.begin(), sorted.end());
    // Define central value to be median.
    if (numCoreErrMember % 2)
    { // odd nmem => one middle value
        uncertainty.central = sorted[numCoreErrMember / 2 + 1];
    }
    else
    { // even nmem => average of two middle values
        uncertainty.central =
            0.5 * (sorted[numCoreErrMember / 2] + sorted[numCoreErrMember / 2 + 1]);
    }
    // Define uncertainties via quantiles with a CL given by reqCL.
    const int upper = std::round(0.5 * (1 + cl) * numCoreErrMember);     // round to nearest integer
    const int lower = 1 + std::round(0.5 * (1 - cl) * numCoreErrMember); // round to nearest integer
    uncertainty.errplus = sorted[upper] - uncertainty.central;
    uncertainty.errminus = uncertainty.central - sorted[lower];
    uncertainty.errsymm = (uncertainty.errplus + uncertainty.errminus) / 2.0; // symmetrised
}
double ReplicasPercentileStrategy::Correlation(const std::vector<double> &valuesA,
                                               const std::vector<double> &valuesB,
                                               const int numCoreErrMember)
{
    double cor = -1;
    PDFUncertainty errA;
    Uncertainty(valuesA, numCoreErrMember, -1, errA);
    PDFUncertainty errB;
    Uncertainty(valuesB, numCoreErrMember, -1, errB);
    // Calculate the correlation over replicas using Eq. (2.7) of arXiv:1106.5788
    for (size_t imem = 1; imem < numCoreErrMember + 1; imem++)
    {
        cor += valuesA[imem] * valuesB[imem];
    }
    cor = (cor / numCoreErrMember - errA.central * errB.central) /
          (errA.errsymm * errB.errsymm);
    cor *= numCoreErrMember / (numCoreErrMember - 1.0); //< bias correction
    return cor;
}
} // namespace PDFxTMD