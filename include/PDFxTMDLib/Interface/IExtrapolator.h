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

template <typename Derived, typename Interpolator, typename... ExtrapolateArgs>
class IAdvancedExtrapolator : public IExtrapolator<Derived, ExtrapolateArgs...>
{
  public:
    void setInterpolator(const Interpolator *interpolator)
    {
        this->derived().setInterpolator(interpolator);
    }
};

template <typename Derived>
class IcPDFExtrapolator : public IExtrapolator<Derived,  X_T, MU_T>
{
};

template <typename Derived, typename Interpolator>
class IcAdvancedPDFExtrapolator
    : public IAdvancedExtrapolator<Interpolator,  X_T, MU_T>
{
};

template <typename Derived>
class ITMDExtrapolator : public IExtrapolator<Derived, X_T, KT_T, MU_T>
{
};

template <typename Derived, typename Interpolator>
class IcAdvancedTMDExtrapolator
    : public IAdvancedExtrapolator<Derived, Interpolator, X_T, KT_T, MU_T>
{
};

} // namespace PDFxTMD