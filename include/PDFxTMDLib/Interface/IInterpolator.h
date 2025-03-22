#pragma once
#include "PDFxTMDLib/Common/InterfaceUtils.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Interface/IReader.h"

namespace PDFxTMD
{
template <typename Derived, typename Reader, typename... InterpolateArgs>
class IInterpolator : public CRTPBase<Derived>
{
  public:
    void initialize(const IReader<Reader> *reader)
    {
        this->derived().initialize(reader);
    }

    double interpolate(PartonFlavor flavor, InterpolateArgs... args) const
    {
        return this->derived().interpolate(flavor, args...);
    }
    void interpolate(InterpolateArgs... args, std::vector<double>& output) const
    {
        return this->derived().interpolate(args..., output);
    }
    const IReader<Reader> *getReader() const
    {
        return this->derived().getReader();
    }
};
template <typename Reader, typename Derived>
class IcPDFInterpolator : public IInterpolator<Derived, Reader, X_T, MU_T>
{
};

template <typename Reader, typename Derived>
class ITMDInterpolator : public IInterpolator<Derived, Reader, X_T, KT_T, MU_T>
{
};
} // namespace PDFxTMD