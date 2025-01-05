#include "include/Implementation/Coupling/Interpolation/YamlCouplingInterp.h"
#include <algorithm>
#include <cmath>
namespace PDFxTMD
{
YamlCouplingInterp::YamlCouplingInterp(std::vector<double> mu2_vec, std::vector<double> alphasVec)
{
    std::vector<double> logq2_vec;
    logq2_vec.resize(mu2_vec.size());
    std::transform(mu2_vec.begin(), mu2_vec.end(), logq2_vec.begin(),
                   [](double q2) { return std::log(q2); });
    m_gslInterp1D = std::move(std::make_unique<GSLInterp1D>(GSLInterp1D(
        std::move(logq2_vec), std::move(alphasVec), GSLInterp1D::InterpolationType::Cubic)));
    m_mu2Min = mu2_vec.front();
    m_mu2Max = mu2_vec.back();
}
double YamlCouplingInterp::AlphaQCDMu2(double mu2)
{
    if (mu2 >= m_mu2Min && mu2 <= m_mu2Max)
        return m_gslInterp1D->interpolate(std::log(mu2));
    return 0;
}
} // namespace PDFxTMD
