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
    void initialize(const IReader<CDefaultLHAPDFFileReader> *reader);
    const IReader<CDefaultLHAPDFFileReader> *getReader() const;

  private:
    const IReader<CDefaultLHAPDFFileReader> *m_reader;
    mutable std::vector<DefaultAllFlavorShape> m_Shape;
    std::vector<double> m_coefficients, m_shape;
    mutable bool m_isInitialized = false;
};
} // namespace PDFxTMD
