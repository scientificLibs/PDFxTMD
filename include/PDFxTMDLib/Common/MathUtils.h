#pragma once
#include <cmath>
#include <iostream>

namespace PDFxTMD
{
static const double CL1SIGMA = 100 * std::erf(1/std::sqrt(2));

/// gamma functions from Cephes library -- http://www.netlib.org/cephes
/// Copyright 1985, 1987, 2000 by Stephen L. Moshier

static const double kMACHEP = 1.11022302462515654042363166809e-16;
static const double kMAXLOG = 709.782712893383973096206318587;
static const double kBig = 4.503599627370496e15;
static const double kBiginv = 2.22044604925031308085e-16;

double igamc(double a, double x);
double igam(double a, double x);

/// @brief Incomplete gamma function (complement integral)
///
/// \f$ \gamma_c(a,x) = 1 - \gamma(a,x) \f$
/// \f$ \gamma_c(a,x) = 1/\Gamma(a) \int_x^\inf e^-t t^(a-1) dt \f$
///
/// In this implementation both arguments must be positive.
/// The integral is evaluated by either a power series or
/// continued fraction expansion, depending on the relative
/// values of a and x.
double igamc(double a, double x);

/// @brief Left tail of incomplete gamma function
///
/// \f$ \gamma(a,x) = x^a e^-x \sum_k=0^\inf x^k / \Gamma(a+k+1) \f$
double igam(double a, double x);

/// @brief Compute quantiles for standard normal distribution N(0, 1) at probability p
///
/// ALGORITHM AS241  APPL. STATIST. (1988) VOL. 37, NO. 3, 477-484.
double norm_quantile(double p);
/// @brief Compute quantiles of the chi-squared probability distribution function
///
/// Algorithm AS 91   Appl. Statist. (1975) Vol.24, P.35
/// implemented by Anna Kreshuk.
/// Incorporates the suggested changes in AS R85 (vol.40(1), pp.233-5, 1991)
/// Parameters:
///   @arg p   - the probability value, at which the quantile is computed
///   @arg ndf - number of degrees of freedom
double chisquared_quantile(double p, double ndf);
} // namespace PDFxTMD
