#pragma once
#include "Common/InterfaceUtils.h"
#include "Interface/ICoupling.h"
#include <vector>

namespace PDFxTMD
{
template <typename Derived> class ICoupling : public CRTPBase<Derived>
{
  public:
    void initialize(const std::vector<double> &mu2_vec, const std::vector<double> &alphasVec)
    {
        return this->derived().initialize(mu2_vec, alphasVec);
    }
    double AlphaQCDMu2(double mu2)
    {
        return this->derived().AlphaQCD(mu2);
    }
};

} // namespace PDFxTMD