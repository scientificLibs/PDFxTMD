#pragma once
#include "Common/InterfaceUtils.h"
#include "Interface/ICoupling.h"

namespace PDFxTMD
{
template <typename Derived> class ICoupling : public CRTPBase<Derived>
{
  public:
    double AlphaQCD(double q2)
    {
        return this->derived().AlphaQCD(q2);
    }
};

} // namespace PDFxTMD