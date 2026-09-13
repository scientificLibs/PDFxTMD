#pragma once
#include "PDFxTMDLib/Common/DPD/UnequalScaleGrid.h"
#include "PDFxTMDLib/Interface/DPD/IcDPDInterpolator.h"

namespace PDFxTMD
{
template <class Reader>
class CPDFxTMDDPDInterpolator : public IcDPDInterpolator<CPDFxTMDDPDInterpolator<Reader>, Reader>
{
  public:
    void initialize(const IReader<Reader> *reader);
    double interpolate(PartonFlavor flavor1, PartonFlavor flavor2, X_T x1, MU_T mu1_2, X_T x2,
                       MU_T mu2_2) const;
    const IReader<Reader> *getReader() const;

  private:
    const IReader<Reader> *m_reader;
    mutable UnEqualScaleGridData m_Shape;
};
} // namespace PDFxTMD
#include "./CPDFxTMDDPDInterpolator.tpp"
