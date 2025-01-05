#pragma once
#include "Common/GSL/GSLInterp1D.h"
#include "Interface/ICoupling.h"
#include <memory>

namespace PDFxTMD
{
class YamlCouplingInterp : public ICoupling<YamlCouplingInterp>
{
  public:
    YamlCouplingInterp(std::vector<double> mu2_vec, std::vector<double> alphasVec);
    double AlphaQCDMu2(double mu2);

  private:
    std::unique_ptr<GSLInterp1D> m_gslInterp1D;
    double m_mu2Min = 0;
    double m_mu2Max = 0;
};
} // namespace PDFxTMD
