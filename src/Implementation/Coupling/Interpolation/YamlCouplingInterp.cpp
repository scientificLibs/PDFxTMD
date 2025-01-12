#include "include/Implementation/Coupling/Interpolation/YamlCouplingInterp.h"
#include <algorithm>
#include <cmath>
namespace PDFxTMD
{

void YamlCouplingInterp::initialize()
{
    std::vector<double> logq2_vec;
    logq2_vec.resize(m_mu2_vec.size());
    std::transform(m_mu2_vec.begin(), m_mu2_vec.end(), logq2_vec.begin(),
                   [](double q2) { return std::log(q2); });
    m_gslInterp1D = GSLInterp1D(std::move(logq2_vec), std::move(m_alsphasVec_vec),
                                GSLInterp1D::InterpolationType::Cubic);
    m_mu2Min = m_mu2_vec.front();
    m_mu2Max = m_mu2_vec.back();
}
YamlCouplingInterp::YamlCouplingInterp(std::vector<double> mu2_vec, std::vector<double> alphasVec)
    : m_mu2_vec(std::move(mu2_vec)), m_alsphasVec_vec(std::move(alphasVec))
{
    initialize();
}
double YamlCouplingInterp::AlphaQCDMu2(double mu2)
{
    if (mu2 >= m_mu2Min && mu2 <= m_mu2Max)
        return m_gslInterp1D.interpolate(std::log(mu2));
    return 0;
}
} // namespace PDFxTMD
