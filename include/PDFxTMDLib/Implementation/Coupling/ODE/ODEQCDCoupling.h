// taken from lhapdf
#pragma once
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlCouplingInfo.h"
#include "PDFxTMDLib/Implementation/Coupling/Interpolation/InterpolateQCDCoupling.h"
#include <vector>

namespace PDFxTMD
{
class ODEQCDCoupling
{
  public:
    double AlphaQCDMu2(double mu2);
    void initialize(const YamlCouplingInfo &couplingInfo);

  private:
    void _interpolate();
    void _solve(double q2, double &t, double &y, const double &allowed_relative, double h,
                double accuracy) const;
    double _decouple(double y, double t, unsigned int ni, unsigned int nf) const;
    void _rk4(double &t, double &y, double h, const double allowed_change,
              const std::vector<double> &bs) const;
    double _derivative(double t, double y, const std::vector<double> &beta) const;

  private:
    std::vector<double> m_mu2_vec;
    std::vector<double> m_alphas_vec;
    InterpolateQCDCoupling m_couplingInterp;
    YamlCouplingInfo m_couplingInfo;
};
} // namespace PDFxTMD
