#pragma once
#include "PDFxTMDLib/Interface/IInterpolator.h"
#include "PDFxTMDLib/Interface/IReader.h"
#include <array>

namespace PDFxTMD
{
template <typename Derived, typename... ExtrapolateArgs>
class IExtrapolator : public CRTPBase<Derived>
{
  public:
    double extrapolate(PartonFlavor parton, ExtrapolateArgs... args) const
    {
        return this->derived().extrapolate(parton, args...);
    }
    double extrapolate(ExtrapolateArgs... args, std::array<double, DEFAULT_TOTAL_PDFS>& output) const
    {
        return this->derived().extrapolate(args..., output);
    }
};

template <typename Derived, typename... ExtrapolateArgs>
class IAdvancedExtrapolator : public IExtrapolator<Derived, ExtrapolateArgs...>
{
  protected:
    template <typename InterpolatorType>
    void setInterpolator(const InterpolatorType* interpolator)
    {
        this->derived().setInterpolator(interpolator);
    }

};

template <typename Derived>
class IcPDFExtrapolator : public IExtrapolator<Derived, X_T, MU_T>
{
};

template <typename Derived>
class IcAdvancedPDFExtrapolator
    : public IAdvancedExtrapolator<Derived, X_T, MU_T>
{
};

template <typename Derived>
class ITMDExtrapolator : public IExtrapolator<Derived, X_T, KT_T, MU_T>
{
};

template <typename Derived>
class ITMDAdvancedTMDExtrapolator
    : public IAdvancedExtrapolator<Derived, X_T, KT_T, MU_T>
{
};

} // namespace PDFxTMD