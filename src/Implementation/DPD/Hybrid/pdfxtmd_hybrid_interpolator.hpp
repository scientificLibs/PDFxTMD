#pragma once

#include "hybrid_artifact.hpp"

#include <array>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace PDFxTMD::DPDHybrid {

constexpr double kPDFxTMDEps6 = 1.0e-6;
constexpr double kPDFxTMDEps8 = 1.0e-8;

struct InterpolationStencil {
    std::array<NodeIndex, 16> nodes{};
    std::size_t node_count{};
    int n1{};
    int n2{};
    int m1{};
    int m2{};
    int corner_lower{-10};
    bool corner{};
    bool first_is_corner{};
    double x1{};
    double x2{};
    double t1{};
    double t2{};
    double scale_fraction1{};
    double scale_fraction2{};
    double x_fraction1{};
    double x_fraction2{};
    bool logarithmic_x1{};
    bool logarithmic_x2{};
};

using StencilNodeRows = std::array<const double*, 16>;
using ChannelValues = std::array<double, HybridArtifact::kPairCount>;

int locate_bracket(const std::vector<double>& axis, double value);

std::vector<NodeIndex> required_nodes(
    const HybridArtifact& artifact,
    double x1,
    double x2,
    double t1,
    double t2
);

InterpolationStencil make_interpolation_stencil(
    const HybridArtifact& artifact,
    double x1,
    double x2,
    double t1,
    double t2
);

void interpolate_core_channels(
    const HybridArtifact& artifact,
    const InterpolationStencil& stencil,
    const StencilNodeRows& node_rows,
    const std::vector<std::size_t>& channels,
    ChannelValues& output
);

double interpolate_core(
    const HybridArtifact& artifact,
    const std::function<double(const NodeIndex&)>& at,
    double x1,
    double x2,
    double t1,
    double t2
);


}  // namespace PDFxTMD::DPDHybrid
