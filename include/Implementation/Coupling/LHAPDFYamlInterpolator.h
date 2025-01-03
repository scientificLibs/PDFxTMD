#pragma once
#include "Interface/ICoupling.h"

namespace PDFxTMD
{
class LHAPDFYamlInterpolator : public ICoupling<LHAPDFYamlInterpolator>
{
  public:
    double AlphaQCD(double q2);
};
} // namespace PDFxTMD
