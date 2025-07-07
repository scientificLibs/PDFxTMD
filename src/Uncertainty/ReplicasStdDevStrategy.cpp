#include <PDFxTMDLib/Common/PartonUtils.h>
#include <PDFxTMDLib/Uncertainty/ReplicasStdDevStrategy.h>
#include <cmath>

namespace PDFxTMD
{
void ReplicasStdDevStrategy::Uncertainty(const std::vector<double> &values,
                                         const int numCoreErrMember, const double cl,
                                         PDFUncertainty &uncertainty)
{
    // Calculate the average and standard deviation using Eqs. (2.3) and (2.4) of arXiv:1106.5788v2
    double av = 0.0, sd = 0.0;
    for (size_t imem = 1; imem <= numCoreErrMember; imem++)
    {
        av += values[imem];
        sd += SQR(values[imem]);
    }
    av /= numCoreErrMember;
    sd /= numCoreErrMember;
    sd = numCoreErrMember / (numCoreErrMember - 1.0) * (sd - SQR(av));
    sd = (sd > 0.0 && numCoreErrMember > 1) ? std::sqrt(sd) : 0.0;
    uncertainty.central = av;
    uncertainty.errplus = uncertainty.errminus = uncertainty.errsymm = sd;
}
double ReplicasStdDevStrategy::Correlation(const std::vector<double> &valuesA,
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
    cor = (cor / numCoreErrMember - errA.central * errB.central) / (errA.errsymm * errB.errsymm);
    cor *= numCoreErrMember / (numCoreErrMember - 1.0); //< bias correction
    return cor;
}
} // namespace PDFxTMD