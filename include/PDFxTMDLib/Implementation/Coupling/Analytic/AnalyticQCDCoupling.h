// taken from lhapdf
#pragma once
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlCouplingInfo.h"
#include <map>

namespace PDFxTMD
{
class AnalyticQCDCoupling
{
  public:
    double AlphaQCDMu2(double mu2);
    void initialize(const YamlCouplingInfo &couplingInfo);

  private:
    double lambdaQCD(int nf) const;
    void setLambda(unsigned int i, double lambda);
    void setFlavors();

  private:
    std::map<int, double> m_lambdas;
    YamlCouplingInfo m_couplingInfo;
    int m_nfmin;
    int m_nfmax;
};
} // namespace PDFxTMD