#pragma once

#include <string>
#include <vector>

#include "PDFxTMDLib/Common/AllFlavorsShape.h"
#include "PDFxTMDLib/Implementation/Reader/TMD/TDefaultLHAPDF_TMDReader.h"
#include "PDFxTMDLib/Interface/IInterpolator.h"
#include "PDFxTMDLib/Interface/IReader.h"

namespace PDFxTMD
{
class TTrilinearInterpolator
    : public ITMDInterpolator<TTrilinearInterpolator, TDefaultLHAPDF_TMDReader>
{
  public:
    explicit TTrilinearInterpolator() = default;
    void initialize(const IReader<TDefaultLHAPDF_TMDReader> *reader);
    double interpolate(PartonFlavor flavor, double x, double kt2, double mu2) const;
    void interpolate(double x, double kt2, double mu2,
                     std::array<double, DEFAULT_TOTAL_PDFS> &output) const;
    const IReader<TDefaultLHAPDF_TMDReader> *getReader() const;

  private:
    const IReader<TDefaultLHAPDF_TMDReader> *m_reader;
    std::array<int, 3> m_dimensions;
    mutable DefaultAllFlavorTMDShape m_tmdShape;
};

} // namespace PDFxTMD
