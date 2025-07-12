#pragma once
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlCouplingInfo.h"
#include <vector>
#include <PDFxTMDLib/Common/Exception.h>

namespace PDFxTMD
{
/// @brief null uncertainty for pdf set that do not impelement uncertainty.
class NullQCDCoupling
{
  public:
    double AlphaQCDMu2(double mu2)
    {
        throw AlphaQCDError("This is NullQCDCoupling. Use a valid class.");
    }
    void initialize(const YamlCouplingInfo &couplingInfo)
    {
        throw AlphaQCDError("This is NullQCDCoupling. Use a valid class.");
    }
};
} // namespace PDFxTMD