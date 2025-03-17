#pragma once
#include "PDFxTMDLib/Common/InterfaceUtils.h"
#include "PDFxTMDLib/Interface/ICoupling.h"
#include <vector>

namespace PDFxTMD
{
template <typename Derived> class ICoupling : public CRTPBase<Derived>
{
  public:
    double AlphaQCDMu2(double mu2)
    {
        return this->derived().AlphaQCD(mu2);
    }
};

} // namespace PDFxTMD