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
    const auto &defaultShapes = m_reader->getData();
    for (auto &&defaultShape : defaultShapes)
    {
        auto m_gslCompatibleShapeData = std::move(ConvertDefaultShapeGrids(defaultShape));
        std::pair<DefaultAllFlavorShape, std::unordered_map<PartonFlavor, GSLInterp2D>> gsl2dObj;
        for (const auto &partonFlavor : defaultShape.flavors)
        {
            gsl2dObj.first = m_gslCompatibleShapeData;
            gsl2dObj.second.emplace(partonFlavor,
                                    GSLInterp2D(m_gslCompatibleShapeData.x_vec,
                                                m_gslCompatibleShapeData.mu2_vec,
                                                m_gslCompatibleShapeData.grids.at(partonFlavor),
                                                GSLInterp2D::InterpolationType::Bicubic));
        }
        gsl2dObjVecPair.emplace_back(gsl2dObj);
    }

    m_isInitialized = true;
}
const IReader<CDefaultLHAPDFFileReader> *CGSLBicubicInterpolator::getReader() const
{
    return m_reader;
}
// Main interface method - hot path
double CGSLBicubicInterpolator::interpolate(PartonFlavor flavor, double x, double q) const
{
    if (!m_isInitialized)
    {
        throw std::runtime_error("GSLBilinearInterpolator::interpolate is not "
                                 "initialized");
    }
    for (auto &&gsl2dObj : gsl2dObjVecPair)
    {
        if (q >= gsl2dObj.first.mu2_vec.front() && q <= gsl2dObj.first.mu2_vec.back())
        {
            return gsl2dObj.second.at(flavor).interpolate(x, q);
        }
    }
    throw std::runtime_error("q is out of range. Something is wrong in pdfxtmdlib");
}

} // namespace PDFxTMD