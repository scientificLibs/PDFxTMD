#include "Implementation/Interpolator/TMD/TTrilinearInterpolator.h"
#include "Common/AllFlavorsShape.h"
#include "Common/Exception.h"
#include "Common/PartonUtils.h"
#include "Common/YamlInfoReader.h"
#include "mlinterp/mlinterp.hpp"
#include <array>
#include <cmath>
#include <fstream>
#include <set>

namespace PDFxTMD
{
void TTrilinearInterpolator::initialize(const IReader<TDefaultAllFlavorReader> *reader)
{
    m_reader = reader;
    m_updfShape = reader->getData();
    m_dimensions = {static_cast<int>(m_updfShape.x_vec.size()),
                    static_cast<int>(m_updfShape.kt2_vec.size()),
                    static_cast<int>(m_updfShape.mu2_vec.size())};
    m_isInitialized = true;
}

double TTrilinearInterpolator::interpolate(PDFxTMD::PartonFlavor flavor, double x, double kt2,
                                           double mu2) const
{
    if (!m_isInitialized)
    {
        throw std::runtime_error("GSLBilinearInterpolator::interpolate is not "
                                 "initialized");
    }
    using namespace mlinterp;
    double *selectedPdf = &m_updfShape.grids[flavor][0];
    double output[1];
    double logX = std::log(x);
    double logkt2 = std::log(kt2);
    double logMu2 = std::log(mu2);
    interp(m_dimensions.data(), 1, selectedPdf, output, m_updfShape.log_kt2_vec.data(), &logkt2,
           m_updfShape.log_x_vec.data(), &logX, m_updfShape.log_mu2_vec.data(), &logMu2);

    return output[0] / kt2; // Return the interpolated value
}

const IReader<TDefaultAllFlavorReader> *TTrilinearInterpolator::getReader() const
{
    return m_reader;
}


} // namespace PDFxTMD
