#include "PDFxTMDLib/Implementation/Coupling/Analytic/AnalyticQCDCoupling.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/PartonUtils.h"

namespace PDFxTMD
{
void AnalyticQCDCoupling::initialize(const YamlCouplingInfo &couplingInfo)
{
    m_couplingInfo = couplingInfo;
    if (m_couplingInfo.alphasLambda3 != -1)
    {
        setLambda(3, m_couplingInfo.alphasLambda3);
    }
    if (m_couplingInfo.alphasLambda4 != -1)
    {
        setLambda(4, m_couplingInfo.alphasLambda4);
    }
    if (m_couplingInfo.alphasLambda5 != -1)
    {
        setLambda(5, m_couplingInfo.alphasLambda5);
    }
}
// Recalculate nfmax and nfmin after a new lambda has been set
void AnalyticQCDCoupling::setFlavors()
{
    for (int it = 0; it <= 6; ++it)
    {
        std::map<int, double>::iterator element = m_lambdas.find(it);
        if (element == m_lambdas.end())
            continue;
        m_nfmin = it;
        break;
    }
    for (int it = 6; it >= 0; --it)
    {
        std::map<int, double>::iterator element = m_lambdas.find(it);
        if (element == m_lambdas.end())
            continue;
        m_nfmax = it;
        break;
    }
}

// Set lambda_i && recalculate nfmax and nfmin
void AnalyticQCDCoupling::setLambda(unsigned int i, double lambda)
{
    m_lambdas[i] = lambda;
    setFlavors();
}
// Return the correct lambda for a given number of active flavours
// Uses recursion to find the closest defined-but-lower lambda for the given
// number of active flavours
// If a fixed flavor scheme is used, require the correct lambda to be set
double AnalyticQCDCoupling::lambdaQCD(int nf) const
{
    if (m_couplingInfo.flavorScheme == AlphasFlavorScheme::fixed)
    {
        auto lambda = m_lambdas.find(m_couplingInfo.AlphaS_NumFlavors);
        if (lambda == m_lambdas.end())
            throw AlphaQCDError("Set lambda(" + std::to_string(m_couplingInfo.AlphaS_NumFlavors) +
                                ") when using a fixed " +
                                std::to_string(m_couplingInfo.AlphaS_NumFlavors) +
                                " flavor scheme.");
        return lambda->second;
    }
    else
    {
        if (nf < 0)
            throw AlphaQCDError("Requested lambdaQCD for " + std::to_string(nf) +
                                " number of flavours.");
        std::map<int, double>::const_iterator lambda = m_lambdas.find(nf);
        if (lambda == m_lambdas.end())
            return lambdaQCD(nf - 1);
        return lambda->second;
    }
}
// Calculate alpha_s(Q2) by an analytic approximation
double AnalyticQCDCoupling::AlphaQCDMu2(double q2)
{
    /// Get the number of active flavours and corresponding LambdaQCD
    /// Should support any number of active flavors as long as the
    /// corresponding lambas are set
    if (m_lambdas.empty())
        throw AlphaQCDError(
            "You need to set at least one lambda value to calculate alpha_s by analytic means!");
    const int nf = numFlavorsQ2(q2, m_couplingInfo, m_nfmin, m_nfmax);
    const double lambdaQCD_ = lambdaQCD(nf);

    if (q2 <= lambdaQCD_ * lambdaQCD_)
        return std::numeric_limits<double>::max();

    // Get beta coeffs for the number of active (above threshold) quark flavours at energy Q
    const std::vector<double> beta = betas(nf);
    const double beta02 = SQR(beta[0]);
    const double beta12 = SQR(beta[1]);

    // Pre-calculate ln(Q2/lambdaQCD) and expansion term y = 1/ln(Q2/lambdaQCD)
    const double x = q2 / (lambdaQCD_ * lambdaQCD_);
    const double lnx = log(x);
    const double lnlnx = log(lnx);
    const double lnlnx2 = lnlnx * lnlnx;
    const double lnlnx3 = lnlnx * lnlnx * lnlnx;
    const double y = 1 / lnx;

    // Calculate terms up to qcdorder = 4
    // A bit messy because the actual expressions are
    // quite messy...
    /// @todo Is it okay to use _alphas_mz as the constant value?
    if (m_couplingInfo.alphasOrder == OrderQCD::LO)
        return m_couplingInfo.AlphaS_MZ;
    const double A = 1 / beta[0];
    const double a_0 = 1;
    double tmp = a_0;
    if ((int)m_couplingInfo.alphasOrder > 1)
    {
        const double a_1 = beta[1] * lnlnx / beta02;
        tmp -= a_1 * y;
    }
    if ((int)m_couplingInfo.alphasOrder > 2)
    {
        const double B = beta12 / (beta02 * beta02);
        const double a_20 = lnlnx2 - lnlnx;
        const double a_21 = beta[2] * beta[0] / beta12;
        const double a_22 = 1;
        tmp += B * y * y * (a_20 + a_21 - a_22);
    }
    if ((int)m_couplingInfo.alphasOrder > 3)
    {
        const double C = 1. / (beta02 * beta02 * beta02);
        const double a_30 = (beta12 * beta[1]) * (lnlnx3 - (5 / 2.) * lnlnx2 - 2 * lnlnx + 0.5);
        const double a_31 = 3 * beta[0] * beta[1] * beta[2] * lnlnx;
        const double a_32 = 0.5 * beta02 * beta[3];
        tmp -= C * y * y * y * (a_30 + a_31 - a_32);
    }
    const double alphaS = A * y * tmp;
    return alphaS;
}

} // namespace PDFxTMD