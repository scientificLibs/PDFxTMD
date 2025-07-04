#pragma once
#include "PDFxTMDLib/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Interface/IInterpolator.h"
#include <vector>

// taken from lhapdf library!
namespace PDFxTMD
{
class CLHAPDFBicubicInterpolator
    : public IcPDFInterpolator<CLHAPDFBicubicInterpolator, CDefaultLHAPDFFileReader>
{
  public:
    CLHAPDFBicubicInterpolator() = default;
    ~CLHAPDFBicubicInterpolator() = default;

    double interpolate(PartonFlavor flavor, double x, double q2) const;
    void interpolate(double x, double q2, std::array<double, DEFAULT_TOTAL_PDFS> &output) const;
    void initialize(const IReader<CDefaultLHAPDFFileReader> *reader);
    const IReader<CDefaultLHAPDFFileReader> *getReader() const;

  private:
    const IReader<CDefaultLHAPDFFileReader> *m_reader;
    mutable DefaultAllFlavorShape m_Shape;
};
} // namespace PDFxTMD
