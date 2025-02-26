#include "Implementation/Interpolator/Collinear/CBilinearInterpolator.h"
#include "mlinterp/mlinterp.hpp"
#include <cassert>
#include <cmath>
#include <stdexcept>

#include "Common/Exception.h"

// taken from lhapdf library!
namespace PDFxTMD
{

namespace
{ // Unnamed namespace
struct shared_data
{
    // manual cache some values
    double logx;
    double logq2;
    double logx0;
    double logx1;
};

shared_data fill(const DefaultAllFlavorShape &grid, double x, double q2, size_t ix)
{
    // manual cache some values
    shared_data share;
    share.logq2 = log(q2);
    share.logx = log(x);
    share.logx0 = grid.log_x_vec[ix];
    share.logx1 = grid.log_x_vec[ix + 1];
    return share;
}

// One-dimensional linear interpolation for y(x)
inline double _interpolateLinear(double x, double xl, double xh, double yl, double yh)
{
    assert(x >= xl);
    assert(xh >= x);
    return yl + (x - xl) / (xh - xl) * (yh - yl);
}

double _interpolate(const DefaultAllFlavorShape &grid, size_t ix, size_t iq2, PartonFlavor flavor,
                    shared_data _share)
{
    const double f_ql =
        _interpolateLinear(_share.logx, _share.logx0, _share.logx1, xf(grid, ix, iq2, flavor),
                           xf(grid, ix + 1, iq2, flavor));
    const double f_qh =
        _interpolateLinear(_share.logx, _share.logx0, _share.logx1, xf(grid, ix, iq2 + 1, flavor),
                           xf(grid, ix + 1, iq2 + 1, flavor));
    // Then interpolate in Q2, using the x-ipol results as anchor points
    return _interpolateLinear(_share.logq2, grid.log_mu2_vec.at(iq2), grid.log_mu2_vec.at(iq2 + 1),
                              f_ql, f_qh);
}
} // namespace
void CBilinearInterpolator::initialize(const IReader<CDefaultLHAPDFFileReader> *reader)
{
    m_reader = reader;
    m_Shape = reader->getData();
    m_isInitialized = true;
}
double CBilinearInterpolator::interpolate(PartonFlavor flavor, double x, double mu2) const
{
    if (!m_isInitialized)
    {
        throw std::runtime_error("CBilinearInterpolator::interpolate is not "
                                 "initialized");
    }
    int selectedIndex = -1;

    const size_t ix = indexbelow(x, m_Shape.at(0).x_vec);
    {
        for (auto &&shape_ : m_Shape)
        {
            selectedIndex++;
            if (mu2 >= shape_.mu2_vec.front() && mu2 <= shape_.mu2_vec.back())
            {
                break;
            }
        }
    }

    const size_t imu2 = indexbelow(mu2, m_Shape.at(selectedIndex).mu2_vec);
    shared_data shared = fill(m_Shape.at(selectedIndex), x, mu2, ix);
    return _interpolate(m_Shape.at(selectedIndex), ix, imu2, flavor, shared);
}
} // namespace PDFxTMD
