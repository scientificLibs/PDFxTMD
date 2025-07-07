// taken from lhapdf
#pragma once
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlCouplingInfo.h"
#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace PDFxTMD
{
class InterpolateQCDCoupling
{
  public:
    void initialize(const YamlCouplingInfo &couplingInfo);
    double AlphaQCDMu2(double mu2);

  private:
    void setup_grids();
    double interpolateCubic(double T, double VL, double VDL, double VH, double VDH) const;
    class AlphaSArray
    {
      public:
        AlphaSArray()
        {
        }
        AlphaSArray(const std::vector<double> &q2knots, const std::vector<double> &as)
            : _q2s(q2knots), _as(as)
        {
            _syncq2s();
        }
        const std::vector<double> &q2s() const
        {
            return _q2s;
        }

        const std::vector<double> &logq2s() const
        {
            return _logq2s;
        }

        /// Get the index of the closest Q2 knot row <= q2
        ///
        /// If the value is >= q2_max, return i_max-1 (for polynomial spine construction)
        size_t iq2below(double q2) const
        {
            // Test that Q2 is in the grid range
            if (q2 < q2s().front())
                throw AlphaQCDError("Q2 value " + std::to_string(q2) +
                                    " is lower than lowest-Q2 grid point at " +
                                    std::to_string(q2s().front()));
            if (q2 > q2s().back())
                throw AlphaQCDError("Q2 value " + std::to_string(q2) +
                                    " is higher than highest-Q2 grid point at " +
                                    std::to_string(q2s().back()));
            /// Find the closest knot below the requested value
            size_t i = upper_bound(q2s().begin(), q2s().end(), q2) - q2s().begin();
            if (i == q2s().size())
                i -= 1; // can't return the last knot index
            i -= 1;     // have to step back to get the knot <= q2 behaviour
            return i;
        }

        /// Get the index of the closest logQ2 knot row <= logq2
        ///
        /// If the value is >= q2_max, return i_max-1 (for polynomial spine construction)
        size_t ilogq2below(double logq2) const
        {
            // Test that log(Q2) is in the grid range
            if (logq2 < logq2s().front())
                throw AlphaQCDError("logQ2 value " + std::to_string(logq2) +
                                    " is lower than lowest-logQ2 grid point at " +
                                    std::to_string(logq2s().front()));
            if (logq2 > logq2s().back())
                throw AlphaQCDError("logQ2 value " + std::to_string(logq2) +
                                    " is higher than highest-logQ2 grid point at " +
                                    std::to_string(logq2s().back()));
            /// Find the closest knot below the requested value
            size_t i = upper_bound(logq2s().begin(), logq2s().end(), logq2) - logq2s().begin();
            if (i == logq2s().size())
                i -= 1; // can't return the last knot index
            i -= 1;     // have to step back to get the knot <= q2 behaviour
            return i;
        }
        const std::vector<double> &alphas() const
        {
            return _as;
        }

        /// Forward derivative w.r.t. logQ2
        double ddlogq_forward(size_t i) const
        {
            return (alphas()[i + 1] - alphas()[i]) / (logq2s()[i + 1] - logq2s()[i]);
        }

        /// Backward derivative w.r.t. logQ2
        double ddlogq_backward(size_t i) const
        {
            return (alphas()[i] - alphas()[i - 1]) / (logq2s()[i] - logq2s()[i - 1]);
        }

        /// Central (avg of forward and backward) derivative w.r.t. logQ2
        double ddlogq_central(size_t i) const
        {
            return 0.5 * (ddlogq_forward(i) + ddlogq_backward(i));
        }

      private:
        /// Synchronise the log(Q2) array from the Q2 one
        void _syncq2s()
        {
            _logq2s.resize(_q2s.size());
            for (size_t i = 0; i < _q2s.size(); ++i)
                _logq2s[i] = log(_q2s[i]);
        }

        /// List of Q2 knots
        std::vector<double> _q2s;
        /// List of log(Q2) knots
        std::vector<double> _logq2s;
        /// List of alpha_s values across the knot array
        std::vector<double> _as;
    };

  private:
    double m_mu2Min = 0;
    double m_mu2Max = 0;
    std::vector<double> m_mu2_vec;
    std::vector<double> m_alsphasVec_vec;
    std::vector<double> m_logMu2_vec;
    std::array<int, 1> m_dimensions;
    YamlCouplingInfo m_couplingInfo;
    std::map<double, AlphaSArray> _knotarrays;
};
} // namespace PDFxTMD
