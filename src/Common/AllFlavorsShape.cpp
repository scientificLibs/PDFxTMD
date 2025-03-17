#include "PDFxTMDLib/Common/AllFlavorsShape.h"

namespace PDFxTMD
{
void DefaultAllFlavorShape::finalizeXP2()
{
    for (double mu2 : mu2_vec)
    {
        log_mu2_vec.emplace_back(std::log(mu2));
    }
    for (double x : x_vec)
    {
        log_x_vec.emplace_back(std::log(x));
    }
    logXSize = log_x_vec.size();
    logMu2Size = log_mu2_vec.size();
}

double DefaultAllFlavorShape::xf(const DefaultAllFlavorShape &shape, int ix, int iq2,
                                 PartonFlavor flavor)
{
    return shape.grids.at(flavor)[ix * shape.logMu2Size + iq2];
}

double DefaultAllFlavorShape::_ddxBicubic(size_t ix, size_t iq2, PartonFlavor flavor)
{
    const size_t nxknots = logXSize;
    double del1, del2;
    del1 = (ix == 0) ? 0 : log_x_vec[ix] - log_x_vec[ix - 1];
    del2 = (ix == nxknots - 1) ? 0 : log_x_vec[ix + 1] - log_x_vec[ix];

    if (ix != 0 && ix != nxknots - 1)
    { //< If central, use the central difference
        const double lddx = (xf(*this, ix, iq2, flavor) - xf(*this, ix - 1, iq2, flavor)) / del1;
        const double rddx = (xf(*this, ix + 1, iq2, flavor) - xf(*this, ix, iq2, flavor)) / del2;
        return (lddx + rddx) / 2.0;
    }
    else if (ix == 0)
    { //< If at leftmost edge, use forward difference
        return (xf(*this, ix + 1, iq2, flavor) - xf(*this, ix, iq2, flavor)) / del2;
    }
    else if (ix == nxknots - 1)
    { //< If at rightmost edge, use backward difference
        return (xf(*this, ix, iq2, flavor) - xf(*this, ix - 1, iq2, flavor)) / del1;
    }
    else
    {
        throw std::runtime_error("We shouldn't be able to get here!");
    }
}

void DefaultAllFlavorShape::_computePolynomialCoefficients()
{
    const size_t nxknots = logXSize;
    if (nxknots < 2)
    {
        throw std::invalid_argument("log_x_vec must have at least 2 elements");
    }

    std::vector<size_t> shape{nxknots - 1, logMu2Size, 4};
    for (auto flavor : flavors)
    {
        coefficients[flavor].resize(shape[0] * shape[1] * shape[2]);
    }

    for (size_t ix = 0; ix < nxknots - 1; ++ix)
    {
        for (size_t iq2 = 0; iq2 < logMu2Size; ++iq2)
        {
            for (auto flavor : flavors)
            {
                double dlogx = log_x_vec[ix + 1] - log_x_vec[ix];
                double VL = xf(*this, ix, iq2, flavor);
                double VH = xf(*this, ix + 1, iq2, flavor);
                double VDL = _ddxBicubic(ix, iq2, flavor) * dlogx;
                double VDH = _ddxBicubic(ix + 1, iq2, flavor) * dlogx;

                // Calculate polynomial coefficients
                double a = VDH + VDL - 2 * VH + 2 * VL;
                double b = 3 * VH - 3 * VL - 2 * VDL - VDH;
                double c = VDL;
                double d = VL;

                // Correct index calculation
                size_t base_index = ix * shape[1] * shape[2] + iq2 * shape[2];
                coefficients[flavor][base_index + 0] = a;
                coefficients[flavor][base_index + 1] = b;
                coefficients[flavor][base_index + 2] = c;
                coefficients[flavor][base_index + 3] = d;
            }
        }
    }
    return;
}

void DefaultAllFlavorShape::initializeBicubicCoeficient()
{
    _shape = {(int)logXSize, (int)logMu2Size, (int)flavors.size()};
    _computePolynomialCoefficients();
    dlogx.resize(logXSize - 1);

    for (size_t i = 0; i < logXSize - 1; ++i)
    {
        dlogx[i] = log_x_vec[i + 1] - log_x_vec[i];
    }
    dlogq.resize(logMu2Size - 1);
    for (size_t i = 0; i < logMu2Size - 1; ++i)
    {
        dlogq[i] = log_mu2_vec[i + 1] - log_mu2_vec[i];
    }
}

const double &DefaultAllFlavorShape::coeff(int ix, int iq2, PartonFlavor pid, int in) const
{
    return coefficients.at(pid)[ix * (_shape[1]) * 4 + iq2 * 4 + in];
}
} // namespace PDFxTMD
