#pragma once
#include <vector>
#include <PDFxTMDLib/Interface/IUncertainty.h>

namespace PDFxTMD
{
    /// @brief Strategy interface for calculating replica uncertainties.
    class ReplicasStdDevStrategy : public IUncertainty
    {
        public:
            /// @brief Calculate the uncertainty based on replicas.
            /// @param values A vector of double values representing the replicas.
            /// @return PDFUncertainty object containing the calculated uncertainties.
            void Uncertainty(const std::vector<double>& values, const int numCoreErrMember, const double cl, PDFUncertainty& uncertainty) override;
            double Correlation(const std::vector<double>& valuesA, const std::vector<double>& valuesB, const int numCoreErrMember) override;
    };
} // namespace PDFxTMD
