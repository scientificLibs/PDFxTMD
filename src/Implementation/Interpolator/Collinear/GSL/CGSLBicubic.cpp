#include "Implementation/Interpolator/Collinear/GSL/CGSLBicubic.h"
#include <cassert>
#include <cmath>
#include <stdexcept>

#include "Common/Exception.h"
#include "Common/GSL/GSLInterp2D.h"
#include "Common/GSL/common.h"

namespace PDFxTMD
{
void CGSLBicubicInterpolator::initialize(const IReader<CDefaultLHAPDFFileReader> *reader)
{
    m_reader = reader;
    const auto &defaultShape = m_reader->getData();
    auto m_gslCompatibleShapeData = std::move(ConvertDefaultShapeGrids(defaultShape));
    for (const auto &partonFlavor : m_reader->getData().flavors)
    {
        gsl2dObj.emplace(partonFlavor, GSLInterp2D(m_gslCompatibleShapeData.x_vec,
                                                   m_gslCompatibleShapeData.mu2_vec,
                                                   m_gslCompatibleShapeData.grids.at(partonFlavor),
                                                   GSLInterp2D::InterpolationType::Bicubic));
    }

    m_isInitialized = true;
}

// Main interface method - hot path
double CGSLBicubicInterpolator::interpolate(PartonFlavor flavor, double x, double q) const
{
    if (!m_isInitialized)
    {
        throw std::runtime_error("GSLBilinearInterpolator::interpolate is not "
                                 "initialized");
    }
    return gsl2dObj.at(flavor).interpolate(x, q);
}
}; // namespace PDFxTMD