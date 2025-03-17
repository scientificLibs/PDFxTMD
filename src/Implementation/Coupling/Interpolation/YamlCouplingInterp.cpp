#include "PDFxTMDLib/Implementation/Coupling/Interpolation/YamlCouplingInterp.h"
#include <PDFxTMDLib/external/mlinterp/mlinterp.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace PDFxTMD
{

void YamlCouplingInterp::initialize()
{
    m_logMu2_vec.resize(m_mu2_vec.size());
    std::transform(m_mu2_vec.begin(), m_mu2_vec.end(), m_logMu2_vec.begin(),
                   [](double q2) { return std::log(q2); });
    m_mu2Min = m_mu2_vec.front();
    m_mu2Max = m_mu2_vec.back();
    m_dimensions = {static_cast<int>(m_logMu2_vec.size())};
    m_isInitialized = true;
}
YamlCouplingInterp::YamlCouplingInterp(std::vector<double> mu2_vec, std::vector<double> alphasVec)
    : m_mu2_vec(std::move(mu2_vec)), m_alsphasVec_vec(std::move(alphasVec))
{
    initialize();
}
double YamlCouplingInterp::AlphaQCDMu2(double mu2)
{
    if (!m_isInitialized)
    {
        throw std::runtime_error("YamlCouplingInterp::AlphaQCDMu2 is not "
                                 "initialized");
    }
    if (mu2 >= m_mu2Min && mu2 <= m_mu2Max)
    {
        using namespace mlinterp;
        double output[1];
        double logMu2 = std::log(mu2);
        interp(m_dimensions.data(), 1, m_alsphasVec_vec.data(), output, m_logMu2_vec.data(),
               &logMu2);
        return output[0];
    }
    return 0.;
}
} // namespace PDFxTMD
