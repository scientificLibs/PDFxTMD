#include "pdfxtmd_baseline.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <unordered_map>

namespace PDFxTMD::DPDHybrid {

PDFxTMDBaseline::PDFxTMDBaseline(
    const std::string& set_name,
    int member,
    double charm_mass,
    double bottom_mass
)
    : pdf_(PDFxTMD::GenericCPDFFactory().mkCPDF(set_name, member)),
      charm2_(charm_mass * charm_mass),
      bottom2_(bottom_mass * bottom_mass) {
    if (member < 0) {
        throw std::invalid_argument("PDF member must be non-negative");
    }
    if (!(charm_mass > 0.0) || !(bottom_mass > 0.0)) {
        throw std::invalid_argument("Heavy-flavour masses must be positive");
    }
}

bool PDFxTMDBaseline::active(
    int pid,
    double mu2,
    double charm2,
    double bottom2
) {
    const int absolute_pid = std::abs(pid);
    return (absolute_pid != 4 || mu2 >= charm2)
        && (absolute_pid != 5 || mu2 >= bottom2);
}


PDFxTMDBaseline::LegValues PDFxTMDBaseline::evaluate_leg(double x, double mu2) {
    if (!(x > 0.0) || !(mu2 > 0.0)) {
        throw std::invalid_argument("SPDF baseline leg requires positive x and mu^2");
    }
    LegValues values{};
    for (std::size_t flavor = 0; flavor < kPidCount; ++flavor) {
        const int pid = kPids[flavor];
        values[flavor] = active(pid, mu2, charm2_, bottom2_)
            ? pdf_.pdf(static_cast<PDFxTMD::PartonFlavor>(pid), x, mu2)
            : 0.0;
    }
    return values;
}

void PDFxTMDBaseline::phase_space_pairs(
    const LegValues& leg1, const LegValues& leg2, double x1, double x2, double* output
) {
    if (!(x1 > 0.0 && x2 > 0.0 && x1 + x2 < 1.0)) {
        std::fill(output, output + kPairCount, 0.0);
        return;
    }
    const double u = 1.0 - x1, v = 1.0 - x2;
    const double rem = 1.0 - x1 - x2;
    const double rho = std::pow(rem / (u * v), 2);
    for (std::size_t i = 0; i < kPidCount; ++i) {
        for (std::size_t j = 0; j < kPidCount; ++j) {
            output[i * kPidCount + j] = leg1[i] * leg2[j] * rho;
        }
    }
}

std::vector<double> PDFxTMDBaseline::evaluate(
    const std::vector<double>& x1,
    const std::vector<double>& x2,
    const std::vector<double>& mu1_2,
    const std::vector<double>& mu2_2
) {
    const std::size_t batch = x1.size();
    if (x2.size() != batch || mu1_2.size() != batch || mu2_2.size() != batch) {
        throw std::invalid_argument("All coordinate arrays must have equal length");
    }

    std::vector<double> baseline(batch * kPairCount, 0.0);
    if (batch == 0) {
        return baseline;
    }

    struct LegKey {
        double x;
        double mu2;

        bool operator==(const LegKey& other) const noexcept {
            return x == other.x && mu2 == other.mu2;
        }
    };

    struct LegKeyHash {
        std::size_t operator()(const LegKey& key) const noexcept {
            const std::size_t x_hash = std::hash<double>{}(key.x);
            const std::size_t mu2_hash = std::hash<double>{}(key.mu2);
            return x_hash ^ (mu2_hash + static_cast<std::size_t>(0x9e3779b9)
                + (x_hash << 6) + (x_hash >> 2));
        }
    };

    std::unordered_map<LegKey, LegValues, LegKeyHash> leg_cache;
    leg_cache.reserve(batch * 2);

    const auto get_leg = [&](double x, double mu2) -> const LegValues& {
        const LegKey key{x, mu2};
        const auto found = leg_cache.find(key);
        if (found != leg_cache.end()) {
            return found->second;
        }

        return leg_cache.emplace(key, evaluate_leg(x, mu2)).first->second;
    };

    for (std::size_t point = 0; point < batch; ++point) {
        if (!(x1[point] > 0.0) || !(x2[point] > 0.0)
            || x1[point] + x2[point] >= 1.0) {
            continue;
        }
        const LegValues& leg1 = get_leg(x1[point], mu1_2[point]);
        const LegValues& leg2 = get_leg(x2[point], mu2_2[point]);
        phase_space_pairs(
            leg1, leg2, x1[point], x2[point], baseline.data() + point * kPairCount
        );
    }
    return baseline;
}

}  // namespace PDFxTMD::DPDHybrid
