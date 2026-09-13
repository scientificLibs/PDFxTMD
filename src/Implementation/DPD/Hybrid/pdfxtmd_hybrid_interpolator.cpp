#include "pdfxtmd_hybrid_interpolator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>

// This is a callback-based adaptation of PDFxTMD's authoritative DPDs branch:
// https://github.com/Raminkord92/PDFxTMD/blob/DPDs/include/PDFxTMDLib/Implementation/Interpolator/Collinear/DPD/CPDFxTMDDPDInterpolator.tpp
// Operation order and branch conditions are kept aligned with that source.
// The support_fraction helper also implements
// the continuous zero-boundary limit used by the v1 implicit-grid runtime when a
// rounded complementary axis collapses a triangular denominator to 0/0.

namespace PDFxTMD::DPDHybrid {
namespace {

double linint(double fraction, double low, double high) {
    return high * fraction + (1.0 - fraction) * low;
}

double support_fraction(double numerator, double denominator) {
    if (denominator == 0.0) {
        if (numerator == 0.0) {
            return 0.0;
        }
        throw std::runtime_error(
            "nonzero numerator on collapsed PDFxTMD support edge"
        );
    }
    return numerator / denominator;
}

int find_corner_triangle(
    const HybridArtifact& artifact,
    double x,
    double y
) {
    if (artifact.nmcor < 2) {
        return -10;
    }
    double ycomp = 0.0;
    for (int index = 1; index < artifact.nmcor; ++index) {
        ycomp = (1.0 - x) / (1.0 - artifact.xmax)
            * artifact.xs.at(static_cast<std::size_t>(index));
        if (y < ycomp) {
            return index - 1;
        }
    }
    if (y > ycomp) {
        return -10;
    }
    return artifact.nmcor - 2;
}

double corner_plane_values(
    const HybridArtifact& artifact,
    double x,
    double y,
    int lower,
    double lower_value,
    double upper_value
) {
    const std::array<double, 3> xv = {artifact.xmax, artifact.xmax, 1.0};
    const std::array<double, 3> yv = {
        artifact.xs.at(static_cast<std::size_t>(lower)),
        artifact.xs.at(static_cast<std::size_t>(lower + 1)),
        0.0
    };
    const std::array<double, 3> zv = {
        lower_value,
        upper_value,
        0.0
    };
    const double a = yv[0] * (zv[1] - zv[2])
        + yv[1] * (zv[2] - zv[0]) + yv[2] * (zv[0] - zv[1]);
    const double b = zv[0] * (xv[1] - xv[2])
        + zv[1] * (xv[2] - xv[0]) + zv[2] * (xv[0] - xv[1]);
    const double c = xv[0] * (yv[1] - yv[2])
        + xv[1] * (yv[2] - yv[0]) + xv[2] * (yv[0] - yv[1]);
    const double d = -xv[0] * (yv[1] * zv[2] - yv[2] * zv[1])
        - xv[1] * (yv[2] * zv[0] - yv[0] * zv[2])
        - xv[2] * (yv[0] * zv[1] - yv[1] * zv[0]);
    return (-d - a * x - b * y) / c;
}

template <typename Sampler>
double interpolate_stencil_channel(
    const HybridArtifact& artifact,
    const InterpolationStencil& stencil,
    Sampler&& sample
) {
    if (stencil.node_count == 0) {
        return 0.0;
    }
    const double fraction1 = stencil.scale_fraction1;
    const double fraction2 = stencil.scale_fraction2;

    if (stencil.corner) {
        const double corner_x = stencil.first_is_corner
            ? stencil.x1 : stencil.x2;
        const double corner_y = stencil.first_is_corner
            ? stencil.x2 : stencil.x1;
        double values[2][2]{};
        for (int scale1 = 0; scale1 < 2; ++scale1) {
            for (int scale2 = 0; scale2 < 2; ++scale2) {
                const std::size_t slot = static_cast<std::size_t>(
                    (scale1 * 2 + scale2) * 2
                );
                values[scale1][scale2] = corner_plane_values(
                    artifact,
                    corner_x,
                    corner_y,
                    stencil.corner_lower,
                    sample(slot),
                    sample(slot + 1)
                );
            }
        }
        const double scale1_values[2] = {
            linint(fraction2, values[0][0], values[0][1]),
            linint(fraction2, values[1][0], values[1][1])
        };
        return linint(fraction1, scale1_values[0], scale1_values[1]);
    }

    double local[2][2][2][2]{};
    for (int first = 0; first < 2; ++first) {
        for (int second = 0; second < 2; ++second) {
            for (int scale1 = 0; scale1 < 2; ++scale1) {
                for (int scale2 = 0; scale2 < 2; ++scale2) {
                    const std::size_t slot = static_cast<std::size_t>(
                        ((first * 2 + second) * 2 + scale1) * 2 + scale2
                    );
                    local[first][second][scale1][scale2] = sample(slot);
                }
            }
        }
    }
    double after_scales[2][2]{};
    for (int first = 0; first < 2; ++first) {
        for (int second = 0; second < 2; ++second) {
            const double scale2_values[2] = {
                linint(fraction1, local[first][second][0][0], local[first][second][1][0]),
                linint(fraction1, local[first][second][0][1], local[first][second][1][1])
            };
            after_scales[first][second] = linint(
                fraction2, scale2_values[0], scale2_values[1]
            );
        }
    }

    const double x1low = artifact.xs.at(stencil.n1);
    const double x1high = artifact.xs.at(stencil.n1 + 1);
    const double x2low = artifact.xs.at(stencil.n2);
    const double x2high = artifact.xs.at(stencil.n2 + 1);
    const double x1 = stencil.x1;
    const double x2 = stencil.x2;
    if (x1high + x2high > 1.0 + kPDFxTMDEps8) {
        if (x1high + x2low <= 1.0 + kPDFxTMDEps8) {
            const double low_fraction = (x1 - x1low) / (x1high - x1low);
            const double low_value = linint(
                low_fraction, after_scales[0][0], after_scales[1][0]
            );
            if (x1 > 1.0 - x2high) {
                const double fraction = support_fraction(
                    x2 - x2low, 1.0 - x1 - x2low
                );
                return linint(fraction, low_value, 0.0);
            }
            const double high_fraction = support_fraction(
                x1 - x1low, 1.0 - x2high - x1low
            );
            const double high_value = linint(
                high_fraction, after_scales[0][1], 0.0
            );
            const double fraction = (x2 - x2low) / (x2high - x2low);
            return linint(fraction, low_value, high_value);
        }
        if (x2high + x1low <= 1.0 + kPDFxTMDEps8) {
            const double low_fraction = (x2 - x2low) / (x2high - x2low);
            const double low_value = linint(
                low_fraction, after_scales[0][0], after_scales[0][1]
            );
            if (x2 > 1.0 - x1high) {
                const double fraction = support_fraction(
                    x1 - x1low, 1.0 - x2 - x1low
                );
                return linint(fraction, low_value, 0.0);
            }
            const double high_fraction = support_fraction(
                x2 - x2low, 1.0 - x1high - x2low
            );
            const double high_value = linint(
                high_fraction, after_scales[1][0], 0.0
            );
            const double fraction = (x1 - x1low) / (x1high - x1low);
            return linint(fraction, low_value, high_value);
        }
        const double span = 1.0 - x2low - x1low;
        const double support1 = support_fraction(x1 - x1low, span);
        const double support2 = support_fraction(x2 - x2low, span);
        return after_scales[0][0] * (1.0 - support1 - support2);
    }

    double after_x1[2]{};
    if (stencil.logarithmic_x1) {
        for (int second = 0; second < 2; ++second) {
            if (after_scales[0][second] > 0.0 && after_scales[1][second] > 0.0) {
                after_x1[second] = std::exp(linint(
                    stencil.x_fraction1,
                    std::log(after_scales[0][second]),
                    std::log(after_scales[1][second])
                ));
            } else {
                after_x1[second] = linint(
                    stencil.x_fraction1,
                    after_scales[0][second],
                    after_scales[1][second]
                );
            }
        }
    } else {
        for (int second = 0; second < 2; ++second) {
            after_x1[second] = linint(
                stencil.x_fraction1,
                after_scales[0][second],
                after_scales[1][second]
            );
        }
    }

    if (stencil.logarithmic_x2) {
        if (after_x1[0] > 0.0 && after_x1[1] > 0.0) {
            return std::exp(linint(
                stencil.x_fraction2,
                std::log(after_x1[0]),
                std::log(after_x1[1])
            ));
        }
        return linint(stencil.x_fraction2, after_x1[0], after_x1[1]);
    }
    return linint(stencil.x_fraction2, after_x1[0], after_x1[1]);
}

int pdg_to_internal(int pid) {
    switch (pid) {
    case 2: return 0;
    case 1: return 1;
    case 3: return 2;
    case 4: return 3;
    case 5: return 4;
    case -2: return 5;
    case -1: return 6;
    case 0:
    case 21: return 7;
    default: return -1;
    }
}

}  // namespace

int locate_bracket(const std::vector<double>& axis, double value) {
    const int size = static_cast<int>(axis.size());
    if (size < 2) {
        throw std::invalid_argument("locate_bracket needs at least two nodes");
    }
    if (value == axis.front()) {
        return 0;
    }
    if (value >= axis.back()) {
        return size - 2;
    }
    int upper = size;
    int lower = -1;
    while (upper - lower > 1) {
        const int middle = (upper + lower) / 2;
        if (value >= axis.at(static_cast<std::size_t>(middle))) {
            lower = middle;
        } else {
            upper = middle;
        }
    }
    return lower;
}

std::vector<NodeIndex> required_nodes(
    const HybridArtifact& artifact,
    double x1,
    double x2,
    double t1,
    double t2
) {
    const InterpolationStencil stencil = make_interpolation_stencil(
        artifact, x1, x2, t1, t2
    );
    std::vector<NodeIndex> nodes(
        stencil.nodes.begin(), stencil.nodes.begin() + stencil.node_count
    );
    std::sort(nodes.begin(), nodes.end());
    nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());
    return nodes;
}

InterpolationStencil make_interpolation_stencil(
    const HybridArtifact& artifact,
    double x1,
    double x2,
    double t1,
    double t2
) {
    InterpolationStencil stencil;
    stencil.x1 = x1;
    stencil.x2 = x2;
    stencil.t1 = t1;
    stencil.t2 = t2;
    stencil.m1 = std::clamp(locate_bracket(artifact.mus, t1), 0,
                            static_cast<int>(artifact.nmu()) - 2);
    stencil.m2 = std::clamp(locate_bracket(artifact.mus, t2), 0,
                            static_cast<int>(artifact.nmu()) - 2);
    stencil.scale_fraction1 = (t1 - artifact.mus.at(stencil.m1))
        / (artifact.mus.at(stencil.m1 + 1) - artifact.mus.at(stencil.m1));
    stencil.scale_fraction2 = (t2 - artifact.mus.at(stencil.m2))
        / (artifact.mus.at(stencil.m2 + 1) - artifact.mus.at(stencil.m2));

    if (x1 >= artifact.xmax || x2 >= artifact.xmax) {
        stencil.corner = true;
        stencil.first_is_corner = x1 >= artifact.xmax;
        const double corner_x = stencil.first_is_corner ? x1 : x2;
        const double corner_y = stencil.first_is_corner ? x2 : x1;
        stencil.corner_lower = find_corner_triangle(artifact, corner_x, corner_y);
        if (stencil.corner_lower == -10) {
            return stencil;
        }
        for (int scale1 = 0; scale1 < 2; ++scale1) {
            for (int scale2 = 0; scale2 < 2; ++scale2) {
                for (int offset = 0; offset < 2; ++offset) {
                    const int lower = stencil.corner_lower + offset;
                    stencil.nodes[stencil.node_count++] = stencil.first_is_corner
                        ? NodeIndex{
                            static_cast<std::uint16_t>(artifact.nx() - 1),
                            static_cast<std::uint16_t>(lower),
                            static_cast<std::uint16_t>(stencil.m1 + scale1),
                            static_cast<std::uint16_t>(stencil.m2 + scale2)}
                        : NodeIndex{
                            static_cast<std::uint16_t>(lower),
                            static_cast<std::uint16_t>(artifact.nx() - 1),
                            static_cast<std::uint16_t>(stencil.m1 + scale1),
                            static_cast<std::uint16_t>(stencil.m2 + scale2)};
                }
            }
        }
        return stencil;
    }

    stencil.n1 = std::clamp(locate_bracket(artifact.xs, x1), 0,
                            static_cast<int>(artifact.nx()) - 2);
    stencil.n2 = std::clamp(locate_bracket(artifact.xs, x2), 0,
                            static_cast<int>(artifact.nx()) - 2);
    const double x1low = artifact.xs.at(stencil.n1);
    const double x1high = artifact.xs.at(stencil.n1 + 1);
    const double x2low = artifact.xs.at(stencil.n2);
    const double x2high = artifact.xs.at(stencil.n2 + 1);
    stencil.logarithmic_x1 = x1high < 0.1;
    stencil.logarithmic_x2 = x2high < 0.1;
    if (x1high + x2high <= 1.0 + kPDFxTMDEps8) {
        stencil.x_fraction1 = stencil.logarithmic_x1
            ? (std::log(x1) - std::log(x1low))
                / (std::log(x1high) - std::log(x1low))
            : (x1 - x1low) / (x1high - x1low);
        stencil.x_fraction2 = stencil.logarithmic_x2
            ? (std::log(x2) - std::log(x2low))
                / (std::log(x2high) - std::log(x2low))
            : (x2 - x2low) / (x2high - x2low);
    }
    for (int first = 0; first < 2; ++first) {
        for (int second = 0; second < 2; ++second) {
            for (int scale1 = 0; scale1 < 2; ++scale1) {
                for (int scale2 = 0; scale2 < 2; ++scale2) {
                    stencil.nodes[stencil.node_count++] = NodeIndex{
                        static_cast<std::uint16_t>(stencil.n1 + first),
                        static_cast<std::uint16_t>(stencil.n2 + second),
                        static_cast<std::uint16_t>(stencil.m1 + scale1),
                        static_cast<std::uint16_t>(stencil.m2 + scale2)
                    };
                }
            }
        }
    }
    return stencil;
}

void interpolate_core_channels(
    const HybridArtifact& artifact,
    const InterpolationStencil& stencil,
    const StencilNodeRows& node_rows,
    const std::vector<std::size_t>& channels,
    ChannelValues& output
) {
    for (std::size_t slot = 0; slot < stencil.node_count; ++slot) {
        if (node_rows[slot] == nullptr) {
            throw std::invalid_argument("interpolation stencil has a null node row");
        }
    }
    for (std::size_t channel : channels) {
        if (channel >= HybridArtifact::kPairCount) {
            throw std::out_of_range("interpolation channel is out of range");
        }
        output[channel] = interpolate_stencil_channel(
            artifact,
            stencil,
            [&](std::size_t slot) {
                return node_rows[slot][channel];
            }
        );
    }
}

double interpolate_core(
    const HybridArtifact& artifact,
    const std::function<double(const NodeIndex&)>& at,
    double x1,
    double x2,
    double t1,
    double t2
) {
    const InterpolationStencil stencil = make_interpolation_stencil(
        artifact, x1, x2, t1, t2
    );
    return interpolate_stencil_channel(
        artifact,
        stencil,
        [&](std::size_t slot) {
            return at(stencil.nodes.at(slot));
        }
    );
}

}  // namespace PDFxTMD::DPDHybrid
