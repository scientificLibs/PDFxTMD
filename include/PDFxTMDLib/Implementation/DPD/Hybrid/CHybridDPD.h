#pragma once

#include "PDFxTMDLib/Common/PartonUtils.h"

#include <memory>
#include <string>

namespace PDFxTMD
{

/**
 * @brief Collinear DPD backed by a neural implicit grid and sparse exact overrides.
 *
 * A PDFxTMD-DPDH1 member consists of two sibling files:
 *
 *   <set>_<member>.dat     DPD-HYBRID-B1 axes, metadata and overrides
 *   <set>_<member>.native  DPDNAT1 neural node provider
 *
 * Construction validates the model/artifact checkpoint identity. Copies share the
 * immutable deployment, its optional node cache, and its most recent 121-channel
 * result. Calls are serialized because caches and inference workspaces are reused.
 */
class CHybridDPD
{
  public:
    static constexpr const char *FormatName = "PDFxTMD-DPDH1";

    CHybridDPD(const std::string &pdfSetName, int setMember);
    CHybridDPD(const CHybridDPD &) noexcept = default;
    CHybridDPD &operator=(const CHybridDPD &) noexcept = default;
    CHybridDPD(CHybridDPD &&) noexcept = default;
    CHybridDPD &operator=(CHybridDPD &&) noexcept = default;
    ~CHybridDPD();

    double dpd(PartonFlavor flavor1, PartonFlavor flavor2, double x1, double mu1_2, double x2,
               double mu2_2);

  private:
    struct State;
    std::shared_ptr<State> state_;
};

} // namespace PDFxTMD
