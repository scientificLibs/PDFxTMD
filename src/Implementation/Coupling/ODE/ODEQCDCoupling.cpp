// taken from lhapdf library
#include "PDFxTMDLib/Implementation/Coupling/ODE/ODEQCDCoupling.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/MathUtils.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include <algorithm>

namespace PDFxTMD
{
double ODEQCDCoupling::AlphaQCDMu2(double mu2)
{
    return m_couplingInterp.AlphaQCDMu2(mu2);
}

void ODEQCDCoupling::initialize(const YamlCouplingInfo &couplingInfo)
{
    // obtain mu2_vec and alphasVec by solving differential equation to use interpolation.
    m_couplingInfo = couplingInfo;
    m_alphas_vec = m_couplingInfo.alphas_vec;
    m_mu2_vec.resize(m_couplingInfo.mu_vec.size());
    std::transform(m_couplingInfo.mu_vec.begin(), m_couplingInfo.mu_vec.end(), m_mu2_vec.begin(),
                   [](double mu) { return mu * mu; });
}
// Solve the differential equation in alphaS using an implementation of RK4
void ODEQCDCoupling::_interpolate()
{
    // Initial step size
    double h = 2.0;
    /// This the the relative error allowed for the adaptive step size.
    const double allowed_relative = 0.01;

    /// Accuracy of Q2 (error in Q2 within this / 2)
    double accuracy = 0.001;

    // Run in Q2 using RK4 algorithm until we are within our defined accuracy
    double t;
    double y;

    if (m_couplingInfo.AlphaS_MassReference != -1)
    {
        t = SQR(m_couplingInfo.AlphaS_MassReference); // starting point
        y = m_couplingInfo.AlphaS_Reference;          // starting value
    }
    else
    {
        t = SQR(m_couplingInfo.MZ);   // starting point
        y = m_couplingInfo.AlphaS_MZ; // starting value
    }

    // If a vector of knots in q2 has been given, solve for those.
    // This creates a default grid which should be overkill for most
    // purposes
    if (m_mu2_vec.empty())
    {
        for (int q = 1; (q / 10.) < 1; ++q)
        {
            m_mu2_vec.emplace_back(SQR(q / 10.));
        }
        for (int q = 4; (q / 4.) < m_couplingInfo.MZ; ++q)
        {
            m_mu2_vec.emplace_back(SQR(q / 4.));
        }
        for (int q = ceil(m_couplingInfo.MZ / 4); (4 * q) < 1000; ++q)
        {
            m_mu2_vec.push_back(SQR(4 * q));
        }
        for (int q = (1000 / 50); (50 * q) < 2000; ++q)
        {
            m_mu2_vec.push_back(SQR(50 * q));
        }

        if (m_couplingInfo.quarkThreshhold.empty())
        {
            for (int it = 4; it <= 6; ++it)
            {
                auto element = m_couplingInfo.quarkMasses.find(static_cast<PartonFlavor>(it));
                if (element == m_couplingInfo.quarkMasses.end())
                    continue;
                m_mu2_vec.push_back(SQR(element->second));
                m_mu2_vec.push_back(SQR(element->second));
            }
        }
        else
        {
            for (int it = 4; it <= 6; ++it)
            {
                auto element = m_couplingInfo.quarkThreshhold.find(static_cast<PartonFlavor>(it));
                if (element == m_couplingInfo.quarkThreshhold.end())
                    continue;
                m_mu2_vec.push_back(SQR(element->second));
                m_mu2_vec.push_back(SQR(element->second));
            }
        }
        std::sort(m_mu2_vec.begin(), m_mu2_vec.end());
    }
    // If for some reason the highest mu2 knot is below m_{Z},
    // force a knot there anyway (since we know it, might as well
    // use it)
    if (m_mu2_vec[m_mu2_vec.size() - 1] < SQR(m_couplingInfo.MZ))
        m_mu2_vec.push_back(SQR(m_couplingInfo.MZ));

    // Find the index of the knot right below m_{Z}
    unsigned int index_of_mz_lower = 0;

    while (m_mu2_vec[index_of_mz_lower + 1] < SQR(m_couplingInfo.MZ))
    {
        if (index_of_mz_lower == m_mu2_vec.size() - 1)
            break;
        index_of_mz_lower++;
    }

    std::vector<std::pair<int, double>> grid; // for storing in correct order
    grid.reserve(m_mu2_vec.size());
    double low_lim = 0;
    double last_val = -1;
    bool threshold = false;

    // We do this by starting from m_{Z}, going down to the lowest q2,
    // and then jumping back up to m_{Z} to avoid calculating things twice
    for (int ind = index_of_mz_lower; ind >= 0; --ind)
    {
        const double q2 = m_mu2_vec[ind];
        // Deal with cases with two identical adjacent points (thresholds) by decreasing step size,
        // allowed errors, and accuracy.
        if (ind != 0 && ind != 1)
        {
            if (q2 == m_mu2_vec[ind - 1])
            {
                last_val = q2;
                threshold = true;
                _solve(q2, t, y, allowed_relative / 5, h / 5, accuracy / 5);
                grid.push_back(std::make_pair(ind, y));
                y = y * _decouple(y, t, numFlavorsQ2(m_mu2_vec[ind + 1], m_couplingInfo),
                                  numFlavorsQ2(m_mu2_vec[ind - 2], m_couplingInfo));
                // Define divergence after y > 2. -- we have no accuracy after that any way
                if (y > 2.)
                {
                    low_lim = q2;
                }
                continue;
            }
        }
        // If q2 is lower than a value that already diverged, it will also diverge
        if (q2 < low_lim)
        {
            grid.push_back(std::make_pair(ind, std::numeric_limits<double>::max()));
            continue;
            // If last point was the same we don't need to recalculate
        }
        else if (q2 == last_val)
        {
            grid.push_back(std::make_pair(ind, y));
            continue;
            // Else calculate
        }
        else
        {
            last_val = q2;
            if (threshold)
            {
                _solve(q2, t, y, allowed_relative / 5, h / 5, accuracy / 5);
                threshold = false;
            }
            else
            {
                _solve(q2, t, y, allowed_relative, h, accuracy);
            }
            grid.push_back(std::make_pair(ind, y));
            // Define divergence after y > 2. -- we have no accuracy after that any way
            if (y > 2.)
            {
                low_lim = q2;
            }
        }
    }

    if (m_couplingInfo.AlphaS_MassReference != -1)
    {
        t = SQR(m_couplingInfo.AlphaS_MassReference); // starting point
        y = m_couplingInfo.AlphaS_Reference;          // starting value
    }
    else
    {
        t = SQR(m_couplingInfo.MZ);   // starting point
        y = m_couplingInfo.AlphaS_MZ; // starting value
    }

    for (size_t ind = index_of_mz_lower + 1; ind < m_mu2_vec.size(); ++ind)
    {
        double q2 = m_mu2_vec[ind];
        // Deal with cases with two identical adjacent points (thresholds) by decreasing step size,
        // allowed errors, and accuracy.
        if (ind != m_mu2_vec.size() - 1 && ind != m_mu2_vec.size() - 2)
        {
            if (q2 == m_mu2_vec[ind + 1])
            {
                last_val = q2;
                _solve(q2, t, y, allowed_relative / 5, h / 5, accuracy / 5);
                grid.push_back(std::make_pair(ind, y));
                y = y * _decouple(y, t, numFlavorsQ2(m_mu2_vec[ind - 1], m_couplingInfo),
                                  numFlavorsQ2(m_mu2_vec[ind + 2], m_couplingInfo));
                // Define divergence after y > 2. -- we have no accuracy after that any way
                if (y > 2.)
                {
                    low_lim = q2;
                }
                continue;
            }
        }
        // If q2 is lower than a value that already diverged, it will also diverge
        if (q2 < low_lim)
        {
            grid.push_back(std::make_pair(ind, std::numeric_limits<double>::max()));
            continue;
            // If last point was the same we don't need to recalculate
        }
        else if (q2 == last_val)
        {
            grid.push_back(std::make_pair(ind, y));
            continue;
            // Else calculate
        }
        else
        {
            last_val = q2;
            _solve(q2, t, y, allowed_relative, h, accuracy);
            grid.push_back(std::make_pair(ind, y));
            // Define divergence after y > 2. -- we have no accuracy after that any way
            if (y > 2.)
            {
                low_lim = q2;
            }
        }
    }

    std::sort(grid.begin(), grid.end(),
              [](const std::pair<int, double> &a, const std::pair<int, double> &b) {
                  return a.first < b.first;
              });
    /// @todo auto lambda args need C++14: std::sort(grid.begin(), grid.end(), [](const auto& a,
    /// const auto& b) { return a.first < b.first; });

    std::vector<double> alphas;
    alphas.reserve(m_mu2_vec.size());

    for (size_t x = 0; x < grid.size(); ++x)
    {
        // cout << sqrt(_q2s.at(x)) << "       " << grid.at(x).second << endl;
        alphas.push_back(grid.at(x).second);
    }
    m_couplingInfo.alphas_vec = std::move(alphas);
    m_couplingInfo.mu_vec.reserve(m_mu2_vec.size());
    std::transform(m_mu2_vec.begin(), m_mu2_vec.end(), m_couplingInfo.mu_vec.begin(),
                   [](double mu2) { return std::sqrt(mu2); });
    m_couplingInterp.initialize(m_couplingInfo);
}

// Calculate decoupling for transition from num. flavour = ni -> nf
double ODEQCDCoupling::_decouple(double y, double t, unsigned int ni, unsigned int nf) const
{
    if (ni == nf || m_couplingInfo.alphasOrder == OrderQCD::LO)
        return 1.;
    double as = y / M_PI;
    unsigned int heavyQuark = nf > ni ? nf : ni;
    auto quark = m_couplingInfo.quarkMasses.find(static_cast<PartonFlavor>(heavyQuark));
    if (quark == m_couplingInfo.quarkMasses.end())
        throw AlphaQCDError("Quark masses are not set, required for using the ODE solver with a "
                            "variable flavor scheme.");
    const double qmass = quark->second;
    double lnmm = log(t / SQR(qmass));
    double as1 = 0, as2 = 0, as3 = 0, as4 = 0;
    if (ni > nf)
    {
        as1 = -0.166666 * lnmm * as;
        as2 = (0.152778 - 0.458333 * lnmm + 0.0277778 * lnmm * lnmm) * as * as;
        as3 = (0.972057 - 0.0846515 * nf + (-1.65799 + 0.116319 * nf) * lnmm +
               (0.0920139 - 0.0277778 * nf) * lnmm * lnmm - 0.00462963 * lnmm * lnmm * lnmm) *
              as * as * as;
        as4 = (5.17035 - 1.00993 * nf - 0.0219784 * nf * nf +
               (-8.42914 + 1.30983 * nf + 0.0367852 * nf * nf) * lnmm +
               (0.629919 - 0.143036 * nf + 0.00371335 * nf * nf) * lnmm * lnmm +
               (-0.181617 - 0.0244985 * nf + 0.00308642 * nf * nf) * lnmm * lnmm * lnmm +
               0.000771605 * lnmm * lnmm * lnmm * lnmm) *
              as * as * as * as;
    }
    else
    {
        as1 = 0.166667 * lnmm * as;
        as2 = (-0.152778 + 0.458333 * lnmm + 0.0277778 * lnmm * lnmm) * as * as;
        as3 = (-0.972057 + 0.0846515 * ni + (1.53067 - 0.116319 * ni) * lnmm +
               (0.289931 + 0.0277778 * ni) * lnmm * lnmm + 0.00462963 * lnmm * lnmm * lnmm) *
              as * as * as;
        as4 = (-5.10032 + 1.00993 * ni + 0.0219784 * ni * ni +
               (7.03696 - 1.22518 * ni - 0.0367852 * ni * ni) * lnmm +
               (1.59462 + 0.0267168 * ni + 0.00371335 * ni * ni) * lnmm * lnmm +
               (0.280575 + 0.0522762 * ni - 0.00308642 * ni * ni) * lnmm * lnmm * lnmm +
               0.000771605 * lnmm * lnmm * lnmm * lnmm) *
              as * as * as * as;
    }
    double decoupling = 1.;
    decoupling += as1;
    if (m_couplingInfo.alphasOrder == OrderQCD::NLO)
        return decoupling;
    decoupling += as2;
    if (m_couplingInfo.alphasOrder == OrderQCD::N2LO)
        return decoupling;
    decoupling += as3;
    if (m_couplingInfo.alphasOrder == OrderQCD::N3LO)
        return decoupling;
    decoupling += as4;
    return decoupling;
}
// Calculate first order derivative, dy/dt, as it appears in the differential equation
double ODEQCDCoupling::_derivative(double t, double y, const std::vector<double> &beta) const
{
    if (m_couplingInfo.alphasOrder == OrderQCD::LO)
        return 0;
    const double b0 = beta[0];
    double d = (b0 * y * y);
    if (m_couplingInfo.alphasOrder == OrderQCD::NLO)
        return -d / t;
    const double b1 = beta[1];
    d += (b1 * y * y * y);
    if (m_couplingInfo.alphasOrder == OrderQCD::N2LO)
        return -d / t;
    const double b2 = beta[2];
    d += (b2 * y * y * y * y);
    if (m_couplingInfo.alphasOrder == OrderQCD::N3LO)
        return -d / t;
    const double b3 = beta[3];
    d += (b3 * y * y * y * y * y);
    if (m_couplingInfo.alphasOrder == OrderQCD::N4LO)
        return -d / t;
    const double b4 = beta[4];
    d += (b4 * y * y * y * y * y * y);
    return -d / t;
}

// Calculate the next step, using recursion to achieve
// adaptive step size. Passing by reference explained
// below.
void ODEQCDCoupling::_rk4(double &t, double &y, double h, const double allowed_change,
                          const std::vector<double> &bs) const
{

    // Determine increments in y based on the slopes of the function at the
    // beginning, midpoint, and end of the interval
    const double k1 = h * _derivative(t, y, bs);
    const double k2 = h * _derivative(t + h / 2.0, y + k1 / 2.0, bs);
    const double k3 = h * _derivative(t + h / 2.0, y + k2 / 2.0, bs);
    const double k4 = h * _derivative(t + h, y + k3, bs);
    const double change = (k1 + 2 * k2 + 2 * k3 + k4) / 6.0;

    // Only call recursively if Q2 > 1 GeV (constant step
    // size after this)
    if (t > 1. && fabs(change) > allowed_change)
    {
        _rk4(t, y, h / 2., allowed_change, bs);
    }
    else
    {
        y += change;
        t += h;
    }
}

// Solve for alpha_s(q2) using alpha_s = y and Q2 = t as starting points
// Pass y and t by reference and change them to avoid having to
// return them in a container -- bit confusing but should be more
// efficient
void ODEQCDCoupling::_solve(double q2, double &t, double &y, const double &allowed_relative,
                            double h, double accuracy) const
{
    if (q2 == t)
        return;
    while (fabs(q2 - t) > accuracy)
    {
        /// Can make the allowed change smaller as the q2 scale gets greater, does not seem
        /// necessary
        const double allowed_change = allowed_relative;

        /// Mechanism to shrink the steps if accuracy < stepsize and we are close to Q2
        if (fabs(h) > accuracy && fabs(q2 - t) / h < 10 && t > 1.)
            h = accuracy / 2.1;
        /// Take constant steps for Q2 < 1 GeV
        if (fabs(h) > 0.01 && t < 1.)
        {
            accuracy = 0.0051;
            h = 0.01;
        }
        // Check if we are going to run forward or backwards in energy scale towards target Q2.
        /// @todo C++11's std::copysign would be perfect here
        if ((q2 < t && sgn(h) > 0) || (q2 > t && sgn(h) < 0))
            h *= -1;

        // Get beta coefficients
        const std::vector<double> bs = betas(numFlavorsQ2(t, m_couplingInfo));

        // Calculate next step
        _rk4(t, y, h, allowed_change, bs);

        if (y > 2.)
        {
            y = std::numeric_limits<double>::max();
            break;
        }
    }
}

} // namespace PDFxTMD