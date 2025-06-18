#pragma once
#include <vector>
#include <PDFxTMDLib/Interface/IUncertainty.h>

namespace PDFxTMD
{
    /// @brief Strategy interface for calculating Hessian uncertainties.
    class HessianStrategy : public IUncertainty
    {
        public:
            /// @brief Calculate the uncertainty based on hessian.
            /// @param values A vector of double values representing the errors where the 0 element is central element.
            /// @param cl Confidence level for the uncertainty calculation (if applicable).
            /// @return PDFUncertainty object containing the calculated uncertainties.
            void Uncertainty(const std::vector<double>& values, const int numCoreErrMember, const double cl, PDFUncertainty& uncertainty) override;
            double Correlation(const std::vector<double>& valuesA, const std::vector<double>& valuesB, const int numCoreErrMember) override;
    };
} // namespace PDFxTMD