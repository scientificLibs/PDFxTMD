#pragma once
#include "PDFxTMDLib/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Interface/IInterpolator.h"

namespace PDFxTMD
{
// taken from lhapdf library!
class CLHAPDFBilinearInterpolator
    : public IcPDFInterpolator<CLHAPDFBilinearInterpolator, CDefaultLHAPDFFileReader>
{
  public:
    CLHAPDFBilinearInterpolator() = default;
    ~CLHAPDFBilinearInterpolator() = default;

    // Main interface method - hot path
    double interpolate(PartonFlavor flavor, double x, double mu2) const;
    void interpolate(double x, double mu2, std::array<double, DEFAULT_TOTAL_PDFS>& output) const;
    void initialize(const IReader<CDefaultLHAPDFFileReader> *reader);
    const IReader<CDefaultLHAPDFFileReader> *getReader() const;

  private:
    const IReader<CDefaultLHAPDFFileReader> *m_reader;
    mutable DefaultAllFlavorShape m_Shape;
};
} // namespace PDFxTMD
