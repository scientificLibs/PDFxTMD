#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "Common/PartonUtils.h"

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

inline size_t indexbelow(double value, const std::vector<double> &knots)
{
    auto it = std::upper_bound(knots.begin(), knots.end(), value);
    return (it == knots.begin()) ? 0 : (it - knots.begin() - 1);
}
struct DefaultAllFlavorShape
{
    DefaultAllFlavorShape() = default;
    std::vector<double> log_x_vec;
    std::vector<double> log_mu2_vec;
    std::vector<double> x_vec;
    std::vector<double> mu2_vec;
    void initializeXP2()
    {
        if (x_vec.size() == 0)
        {
            x_vec.resize(log_x_vec.size());
            std::transform(log_x_vec.begin(), log_x_vec.end(), x_vec.begin(),
                           [](const double &logX) { return std::exp(logX); });
        }
        if (mu2_vec.size() == 0)
        {
            mu2_vec.resize(log_mu2_vec.size());
            // Transform log_p2 to p2_s
            std::transform(log_mu2_vec.begin(), log_mu2_vec.end(), mu2_vec.begin(),
                           [](const double &logQ2) { return std::exp(logQ2); });
        }
    }

    std::vector<PartonFlavor> flavors;
    std::unordered_map<PartonFlavor, std::vector<double>> grids;
};

struct DefaultAllFlavorUPDFShape : DefaultAllFlavorShape
{
    DefaultAllFlavorUPDFShape() = default;
    std::vector<double> log_kt2_vec;
    std::vector<double> kt2_vec;
    void initializeXKt2P2()
    {
        if (kt2_vec.size() == 0)
        {
            // Resize output vectors to match the size of input
            // vectors
            initializeXP2();
            kt2_vec.resize(log_kt2_vec.size());
            // Transform log_x to x_s
            std::transform(log_kt2_vec.begin(), log_kt2_vec.end(), kt2_vec.begin(),
                           [](const double &logKt2) { return std::exp(logKt2); });
        }
    }
};
struct FastDefaultAllFlavorShape : DefaultAllFlavorShape
{
    FastDefaultAllFlavorShape() = default;
    std::vector<PartonFlavor> flavors;
    std::vector<double> grids;
    std::vector<size_t> shapes;

    void initializeShape()
    {
        shapes.resize(3);
        shapes[0] = log_x_vec.size();
        shapes[1] = log_mu2_vec.size();
        shapes[2] = flavors.size();
    }
    double xf(int ix, int iq2, int ipid) const
    {
        return grids[ix * shapes[2] * shapes[1] + iq2 * shapes[2] + ipid];
    }
};
} // namespace PDFxTMD
