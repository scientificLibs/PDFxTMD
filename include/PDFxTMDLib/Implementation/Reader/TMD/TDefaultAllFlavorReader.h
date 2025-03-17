
#pragma once

#include "PDFxTMDLib/Common/AllFlavorsShape.h"
#include "PDFxTMDLib/Common/Exception.h"
#include "PDFxTMDLib/Interface/IReader.h"
#include <string>
#include <vector>

namespace PDFxTMD
{
class TDefaultAllFlavorReader : public IReader<TDefaultAllFlavorReader>
{
  public:
    void read(const std::string &pdfName, int setNumber);
    DefaultAllFlavorUPDFShape getData() const;
    std::pair<double, double> getBoundaryValues(PhaseSpaceComponent comp) const;

  private:
    DefaultAllFlavorUPDFShape m_updfShape;
};
} // namespace PDFxTMD
