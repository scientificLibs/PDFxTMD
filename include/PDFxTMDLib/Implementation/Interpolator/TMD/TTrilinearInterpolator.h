#pragma once

#include <string>
#include <vector>

#include "PDFxTMDLib/Common/AllFlavorsShape.h"
#include "PDFxTMDLib/Implementation/Reader/TMD/TDefaultAllFlavorReader.h"
#include "PDFxTMDLib/Interface/IInterpolator.h"
#include "PDFxTMDLib/Interface/IReader.h"

namespace PDFxTMD
{
class TTrilinearInterpolator
    : public ITMDInterpolator<TTrilinearInterpolator, TDefaultAllFlavorReader>
{
  public:
    explicit TTrilinearInterpolator() = default;
    void initialize(const IReader<TDefaultAllFlavorReader> *reader);
    double interpolate(PartonFlavor flavor, double x, double kt2, double mu2) const;
    void interpolate(double x, double kt2, double mu2, std::array<double, DEFAULT_TOTAL_PDFS>& output) const;
    const IReader<TDefaultAllFlavorReader> *getReader() const;

  private:
    const IReader<TDefaultAllFlavorReader> *m_reader;
    std::array<int, 3> m_dimensions;
    mutable DefaultAllFlavorUPDFShape m_updfShape;
};

} // namespace PDFxTMD
