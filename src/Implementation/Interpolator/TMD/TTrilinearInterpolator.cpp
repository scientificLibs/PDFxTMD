#include "PDFxTMDLib/Implementation/Interpolator/TMD/TTrilinearInterpolator.h"
#include "PDFxTMDLib/Common/AllFlavorsShape.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/YamlInfoReader.h"
#include "PDFxTMDLib/external/mlinterp/mlinterp.hpp"
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
}

void TTrilinearInterpolator::interpolate(double x, double kt2, double mu2, std::array<double, DEFAULT_TOTAL_PDFS>& output) const
{
    using namespace mlinterp;
    double output_[1];
    double logX = std::log(x);
    double logkt2 = std::log(kt2);
    double logMu2 = std::log(mu2);

    for (int i = 0; i < DEFAULT_TOTAL_PDFS; i++)
    {
        double *selectedPdf = &m_updfShape.grids[standardPartonFlavors[i]][0];
        interp(m_dimensions.data(), 1, selectedPdf, output_, m_updfShape.log_kt2_vec.data(), &logkt2,
            m_updfShape.log_x_vec.data(), &logX, m_updfShape.log_mu2_vec.data(), &logMu2);

        output[i]  =  (output_[0] < 0 ? 0 :  output_[0] / kt2);
    }

}

double TTrilinearInterpolator::interpolate(PDFxTMD::PartonFlavor flavor, double x, double kt2,
                                           double mu2) const
{
    using namespace mlinterp;
    double *selectedPdf = &m_updfShape.grids[flavor][0];
    double output[1];
    double logX = std::log(x);
    double logkt2 = std::log(kt2);
    double logMu2 = std::log(mu2);
    interp(m_dimensions.data(), 1, selectedPdf, output, m_updfShape.log_kt2_vec.data(), &logkt2,
           m_updfShape.log_x_vec.data(), &logX, m_updfShape.log_mu2_vec.data(), &logMu2);

    return output[0] < 0 ? 0 :  output[0] / kt2;
}

const IReader<TDefaultAllFlavorReader> *TTrilinearInterpolator::getReader() const
{
    return m_reader;
}

} // namespace PDFxTMD
