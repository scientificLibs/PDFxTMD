#include "PDFxTMDLib/Implementation/Interpolator/Collinear/CLHAPDFBicubicInterpolator.h"
#include <cassert>
#include <vector>

namespace PDFxTMD
{
namespace
{ // Unnamed namespace
struct shared_data
{
    // Pre-calculate parameters
    double logx, logq2, dlogx_1, dlogq_0, dlogq_1, dlogq_2, tlogq;
    double tlogx;
    // bools to find out if at grid edges
    bool q2_lower, q2_upper;
};

shared_data fill(const DefaultAllFlavorShape &grid, double x, double q2, size_t ix, size_t iq2)
{
    shared_data shared;

    // Compute logarithms of inputs
    shared.logx = std::log(x);
    shared.logq2 = std::log(q2);

    // Determine if we're at the grid edges for q2
    shared.q2_lower = (iq2 == 0) || (grid.log_mu2_vec[iq2] == grid.log_mu2_vec[iq2 - 1]);
    shared.q2_upper = (iq2 + 1 == grid.log_mu2_vec.size() - 1) || 
                      (grid.log_mu2_vec[iq2 + 1] == grid.log_mu2_vec[iq2 + 2]);

    // Assign precomputed differences
    shared.dlogx_1 = grid.dlogx[ix];    // Difference at ix in x-direction
    shared.dlogq_1 = grid.dlogq[iq2];   // Difference at iq2 in q2-direction

    // Compute inverse differences for q2, only if not at edges
    if (!shared.q2_lower) {
        shared.dlogq_0 = 1.0 / grid.dlogq[iq2 - 1];  // 1 / (log_mu2_vec[iq2] - log_mu2_vec[iq2 - 1])
    }
    if (!shared.q2_upper) {
        shared.dlogq_2 = 1.0 / grid.dlogq[iq2 + 1];  // 1 / (log_mu2_vec[iq2 + 2] - log_mu2_vec[iq2 + 1])
    }

    // Compute fractional positions using precomputed differences
    shared.tlogx = (shared.logx - grid.log_x_vec[ix]) / shared.dlogx_1;
    shared.tlogq = (shared.logq2 - grid.log_mu2_vec[iq2]) / shared.dlogq_1;

    return shared;
}


/// One-dimensional linear interpolation for y(x)
/// @todo Expose for re-use
inline double _interpolateLinear(double x, double xl, double xh, double yl, double yh)
{
    assert(x >= xl);
    assert(xh >= x);
    return yl + (x - xl) / (xh - xl) * (yh - yl);
}

/// One-dimensional cubic interpolation
///
/// @arg t is the fractional distance of the evaluation x into the dx
/// interval.  @arg vl and @arg vh are the function values at the low and
/// high edges of the interval. @arg vdl and @arg vdh are linearly
/// extrapolated value changes from the product of dx and the discrete low-
/// and high-edge derivative estimates.
///
/// @note See Numerical Recipes 3.6:
/// http://www.it.uom.gr/teaching/linearalgebra/NumericalRecipiesInC/c3-6.pdf
///
/// @todo Expose for re-use
inline double _interpolateCubic(double t, double vl, double vdl, double vh, double vdh)
{
    // Pre-calculate powers of t
    const double t2 = t * t;
    const double t3 = t * t2;

    // Calculate polynomial (grouped by input param rather than powers of t for efficiency)
    const double p0 = (2 * t3 - 3 * t2 + 1) * vl;
    const double m0 = (t3 - 2 * t2 + t) * vdl;
    const double p1 = (-2 * t3 + 3 * t2) * vh;
    const double m1 = (t3 - t2) * vdh;

    return p0 + m0 + p1 + m1;
}

/// Cubic interpolation using a passed array of coefficients
///
/// @todo Expose for re-use

inline double _interpolateCubic(double t, double t2, double t3, const double *coeffs) {
    return coeffs[0] * t3 + coeffs[1] * t2 + coeffs[2] * t + coeffs[3];
}
inline double _interpolateCubic(double t, const double *coeffs)
{
    const double x = t;
    const double x2 = x * x;
    const double x3 = x2 * x;
    return coeffs[0] * x3 + coeffs[1] * x2 + coeffs[2] * x + coeffs[3];
}
double _interpolate(const DefaultAllFlavorShape &grid, size_t ix, size_t iq2, PartonFlavor pid,
                    shared_data &_share)
{
    // Precompute t powers
    double t_x = _share.tlogx;
    double t_x2 = t_x * t_x;
    double t_x3 = t_x2 * t_x;

    // X-interpolation for q2 points
    double vl = _interpolateCubic(t_x, t_x2, t_x3, &grid.coeff(ix, iq2, pid, 0));
    double vh = _interpolateCubic(t_x, t_x2, t_x3, &grid.coeff(ix, iq2 + 1, pid, 0));

    // Derivatives (optimized)
    double vdiff = vh - vl;
    double vdl, vdh;
    if (_share.q2_lower)
    {
        vdl = vdiff;
        double vhh = _interpolateCubic(t_x, t_x2, t_x3, &grid.coeff(ix, iq2 + 2, pid, 0));
        vdh = (vdiff + (vhh - vh) * _share.dlogq_1 * _share.dlogq_2) * 0.5;
    }
    else if (_share.q2_upper)
    {
        vdh = vdiff;
        double vll = _interpolateCubic(t_x, t_x2, t_x3, &grid.coeff(ix, iq2 - 1, pid, 0));
        vdl = (vdiff + (vl - vll) * _share.dlogq_1 * _share.dlogq_0) * 0.5;
    }
    else
    {
        double vll = _interpolateCubic(t_x, t_x2, t_x3, &grid.coeff(ix, iq2 - 1, pid, 0));
        double vhh = _interpolateCubic(t_x, t_x2, t_x3, &grid.coeff(ix, iq2 + 2, pid, 0));
        vdl = (vdiff + (vl - vll) * _share.dlogq_1 * _share.dlogq_0) * 0.5;
        vdh = (vdiff + (vhh - vh) * _share.dlogq_1 * _share.dlogq_2) * 0.5;
    }

    return _interpolateCubic(_share.tlogq, vl, vdl, vh, vdh);
}

double _interpolateFallback(const DefaultAllFlavorShape &grid, size_t ix, size_t iq2,
                            PartonFlavor flavor, shared_data &_share)
{
    // First interpolate in x
    const double logx0 = grid.log_x_vec[ix];
    const double logx1 = grid.log_x_vec[ix + 1];
    const double f_ql = _interpolateLinear(_share.logx, logx0, logx1,
                                           DefaultAllFlavorShape::xf(grid, ix, iq2, flavor),
                                           DefaultAllFlavorShape::xf(grid, ix + 1, iq2, flavor));
    const double f_qh = _interpolateLinear(
        _share.logx, logx0, logx1, DefaultAllFlavorShape::xf(grid, ix, iq2 + 1, flavor),
        DefaultAllFlavorShape::xf(grid, ix + 1, iq2 + 1, flavor));
    // Then interpolate in Q2, using the x-ipol results as anchor points
    return _interpolateLinear(_share.logq2, grid.log_mu2_vec[iq2], grid.log_mu2_vec[iq2 + 1],
                              f_ql, f_qh);
}

void _checkGridSize(const DefaultAllFlavorShape &grid, const size_t ix, const size_t iq2)
{
    // Raise an error if there are too few knots even for a linear fall-back
    const size_t nxknots = grid.logXSize;
    const size_t nq2knots = grid.logMu2Size;

    /// @todo MK: do you really need different number of knots in the directions?
    ///   Probably should be <2 for both methods, and fall back to linear in both cases.
    if (nxknots < 4)
        throw std::runtime_error("PDF subgrids are required to have at least 4 x-knots for use "
                                 "with CLHAPDFBicubicInterpolator");
    if (nq2knots < 2)
        throw std::runtime_error("PDF subgrids are required to have at least 2 Q-knots for use "
                                 "with CLHAPDFBicubicInterpolator");

    // Check x and q index ranges -- we always need i and i+1 indices to be valid
    const size_t ixmax = nxknots - 1;
    const size_t iq2max = nq2knots - 1;
    if (ix + 1 > ixmax) // also true if ix is off the end
        throw std::runtime_error("Attempting to access an x-knot index past the end of the array, "
                                 "in linear fallback mode");
    if (iq2 + 1 > iq2max) // also true if iq2 is off the end
        throw std::runtime_error("Attempting to access an Q-knot index past the end of the array, "
                                 "in linear fallback mode");
}

} // namespace
void CLHAPDFBicubicInterpolator::initialize(const IReader<CDefaultLHAPDFFileReader> *reader)
{
    m_reader = reader;
    m_Shape = reader->getData();
    for (auto &shape : m_Shape)
    {
        if (shape.flavors.size() == 0)
            break;
        shape.initializeBicubicCoeficient();
    }
    m_isInitialized = true;
}
const IReader<CDefaultLHAPDFFileReader> *CLHAPDFBicubicInterpolator::getReader() const
{
    return m_reader;
}

double CLHAPDFBicubicInterpolator::interpolate(PartonFlavor flavor, double x, double mu2) const
{
    if (!m_isInitialized)
    {
        throw std::runtime_error("CBilinearInterpolator::interpolate is not "
                                 "initialized");
    }
    int selectedIndex = -1;

    const size_t ix = indexbelow(x, m_Shape[0].x_vec);
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

    const size_t imu2 = indexbelow(mu2, m_Shape[selectedIndex].mu2_vec);
    shared_data shared = fill(m_Shape[selectedIndex], x, mu2, ix, imu2);
    return _interpolate(m_Shape[selectedIndex], ix, imu2, flavor, shared);
}

} // namespace PDFxTMD