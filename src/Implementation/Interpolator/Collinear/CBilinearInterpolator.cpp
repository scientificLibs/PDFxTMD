#include "Implementation/Interpolator/Collinear/CBilinearInterpolator.h"
#include <cassert>
#include <cmath>
#include <mlinterp/mlinterp.hpp>
#include <stdexcept>

#include "Common/Exception.h"

namespace PDFxTMD
{

void CBilinearInterpolator::initialize(const IReader<CDefaultLHAPDFFileReader> *reader)
{
    m_reader = reader;
    m_Shape = reader->getData();
    m_dimensions = {static_cast<int>(m_Shape.x_vec.size()),
                    static_cast<int>(m_Shape.mu2_vec.size())};
    m_isInitialized = true;
}
double CBilinearInterpolator::interpolate(PartonFlavor flavor, double x, double mu2) const
{
    if (!m_isInitialized)
    {
        throw std::runtime_error("CBilinearInterpolator::interpolate is not "
                                 "initialized");
    }
    using namespace mlinterp;
    double *selectedPdf = &m_Shape.grids[flavor][0];
    if (!selectedPdf)
    {
        throw std::runtime_error("Invalid parton flavor specified.");
    }
    double output[1];

    interp(m_dimensions.data(), 1, selectedPdf, output, m_Shape.x_vec.data(), &x,
           m_Shape.mu2_vec.data(), &mu2);
    return output[0];
}
} // namespace PDFxTMD
