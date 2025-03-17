#pragma once
#include "PDFxTMDLib/Common/PartonUtils.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

// General function to find the knot below a given value
namespace PDFxTMD
{

inline bool isBlockSeparator(const std::string &line)
{
    return line == "---";
}
inline bool isComment(const std::string &line)
{
    return !line.empty() && line[0] == '#';
}

struct DefaultAllFlavorShape
{
    DefaultAllFlavorShape() = default;
    std::vector<double> log_x_vec;
    std::vector<double> log_mu2_vec;
    std::vector<double> x_vec;
    std::vector<double> mu2_vec;
    size_t logXSize = 0;
    size_t logMu2Size = 0;
    std::vector<double> dlogx;  // Differences between consecutive log_x_vec
    std::vector<double> dlogq;  // Differences between consecutive log_mu2_vec
    std::map<PartonFlavor, std::vector<double>> coefficients;
    std::vector<int> _shape;
    const double &coeff(int ix, int iq2, PartonFlavor pid, int in) const;
    void initializeBicubicCoeficient();
    void finalizeXP2();

    std::set<PartonFlavor> flavors;
    std::unordered_map<PartonFlavor, std::vector<double>> grids;
    static double xf(const DefaultAllFlavorShape &shape, int ix, int iq2, PartonFlavor flavor);
  private:
    double _ddxBicubic(size_t ix, size_t iq2, PartonFlavor flavor);
    void _computePolynomialCoefficients();
};

struct DefaultAllFlavorUPDFShape : DefaultAllFlavorShape
{
    DefaultAllFlavorUPDFShape() = default;
    std::vector<double> log_kt2_vec;
    std::vector<double> kt2_vec;
};

} // namespace PDFxTMD
