#pragma once
#include <PDFxTMDLib/Common/Exception.h>
#include <PDFxTMDLib/Common/Uncertainty.h>
#include <vector>
namespace PDFxTMD
{
/// @brief null uncertainty for pdf set that do not impelement uncertainty.
class NullUncertaintyStrategy
{
  public:
    void Uncertainty(const std::vector<double> &values, const int numCoreErrMember, const double cl,
                     PDFUncertainty &uncertainty)
    {
        throw NotImplementedError("This is NullUncertaintyStrategy. Use a valid class.");
    }
    double Correlation(const std::vector<double> &valuesA, const std::vector<double> &valuesB,
                       const int numCoreErrMember)
    {
        throw NotImplementedError("This is NullUncertaintyStrategy. Use a valid class.");
    }
};
} // namespace PDFxTMD