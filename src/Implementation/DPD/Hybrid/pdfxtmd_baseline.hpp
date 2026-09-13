#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include <PDFxTMDLib/Factory.h>

namespace PDFxTMD::DPDHybrid {

class PDFxTMDBaseline {
public:
    static constexpr std::size_t kPidCount = 11;
    static constexpr std::size_t kPairCount = kPidCount * kPidCount;

    explicit PDFxTMDBaseline(
        const std::string& set_name,
        int member = 0,
        double charm_mass = 1.40,
        double bottom_mass = 4.75
    );

    using LegValues = std::array<double, kPidCount>;

    // Evaluate one single-PDF leg. Hybrid deployment uses this during
    // initialization to precompute every (x, mu^2, flavour) grid value once.
    LegValues evaluate_leg(double x, double mu2);

    // Form the flavour-independent phase-space baseline used by the asinh
    // model. Output uses x1*x2*D, exactly as in the Python implementation.
    static void phase_space_pairs(const LegValues& leg1, const LegValues& leg2,
                                  double x1, double x2, double* output);

    std::vector<double> evaluate(
        const std::vector<double>& x1,
        const std::vector<double>& x2,
        const std::vector<double>& mu1_2,
        const std::vector<double>& mu2_2
    );

private:
    static constexpr std::array<int, kPidCount> kPids = {
        -5, -4, -3, -2, -1, 21, 1, 2, 3, 4, 5
    };

    static bool active(int pid, double mu2, double charm2, double bottom2);

    PDFxTMD::ICPDF pdf_;
    double charm2_;
    double bottom2_;
};

}  // namespace PDFxTMD::DPDHybrid
