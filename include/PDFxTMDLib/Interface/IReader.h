#pragma once
#include <string>
#include <vector>

#include "PDFxTMDLib/Common/InterfaceUtils.h"
#include "PDFxTMDLib/Common/PartonUtils.h"

namespace PDFxTMD
{
template <typename Derived> class IReader : public CRTPBase<Derived>
{
  public:
    /** @brief Load one member of a named PDF set. */
    void read(const std::string &pdfName, int setNumber)
    {
        this->derived().read(pdfName, setNumber);
    }

    /** @brief Return the reader's loaded data object. */
    auto getData() const
    {
        return this->derived().getData();
    }

    /** @brief Return the grid values for one phase-space component. */
    std::vector<double> getValues(PhaseSpaceComponent comp) const
    {
        return this->derived().getValues(comp);
    }

    /** @brief Return the lower and upper bounds of one grid component. */
    std::pair<double, double> getBoundaryValues(PhaseSpaceComponent comp) const
    {
        return this->derived().getBoundaryValues(comp);
    }
};
} // namespace PDFxTMD
