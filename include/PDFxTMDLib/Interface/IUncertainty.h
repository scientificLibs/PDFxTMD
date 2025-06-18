#pragma once
#include <vector>
#include <PDFxTMDLib/Common/Uncertainty.h>
#include <PDFxTMDLib/Common/YamlMetaInfo/YamlErrorInfo.h>

namespace PDFxTMD
{
  
class IUncertainty
{
  public:
    /// @brief Calculate the uncertainty based on the provided values.
    /// @param values A vector of double values representing the errors where the 0 element is
    /// central element.
    /// @param cl Confidence level for the uncertainty calculation (if applicable).
    /// @return A boolean indicating success or failure of the calculation.
    virtual void Uncertainty(const std::vector<double> &values, const int numCoreErrMember, const double cl, PDFUncertainty& uncertainty) = 0;
    virtual double Correlation(const std::vector<double>& valuesA, const std::vector<double>& valuesB, const int numCoreErrMember) = 0;
    /// @brief Virtual destructor for proper cleanup of derived classes.
    virtual ~IUncertainty() = default;
};
} // namespace PDFxTMD
