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
    /** @brief Initialize the concrete interpolator with its data reader. */
    void initialize(const IReader<Reader> *reader)
    {
        this->derived().initialize(reader);
    }
    /** @brief Forward an interpolation request to the concrete interpolator. */
    template <typename... Flavors>
    double interpolate(Flavors &&...flavors, InterpolateArgs &&...args) const
    {
        return this->derived().interpolate(std::forward<Flavors>(flavors)...,
                                           std::forward<Flavors>(args)...);
    }
    /** @brief Return the reader used by the concrete interpolator. */
    const IReader<Reader> *getReader() const
    {
        return this->derived().getReader();
    }
};

} // namespace PDFxTMD
