#pragma once
#include "PDFxTMDLib/Interface/IReader.h"

namespace PDFxTMD
{
template <typename Reader> bool isInRangeX(const Reader &reader, double x)
{
    auto [xmin, xmax] = reader.getBoundaryValues(PhaseSpaceComponent::X);
    return x >= xmin && x <= xmax;
}
template <typename Reader> bool isInRangeQ2(const Reader &reader, double q2)
{
    auto [q2min, q2max] = reader.getBoundaryValues(PhaseSpaceComponent::Q2);
    return q2 >= q2min && q2 <= q2max;
}
template <typename Reader> bool isInRangeKt2(const Reader &reader, double kt2)
{
    auto [kt2min, kt2max] = reader.getBoundaryValues(PhaseSpaceComponent::Kt2);
    return kt2 >= kt2min && kt2 <= kt2max;
}
// Check if the point (x, mu) is within the range of the PDF
// set
 template <typename Reader> inline bool isInRange(const Reader &reader, double x, double mu2)
{
    const auto [xmin, xmax] = std::move(reader.getBoundaryValues(PhaseSpaceComponent::X));
    const auto [q2min, q2max] = std::move(reader.getBoundaryValues(PhaseSpaceComponent::Q2));
    return x >= xmin && x <= xmax && mu2 >= q2min && mu2 <= q2max;
}
template <typename Reader> bool isInRange(const Reader &reader, double x, double kt2, double mu2)
{
    return isInRange<Reader>(reader, x, mu2) && isInRangeKt2<Reader>(reader, kt2);
}
} // namespace PDFxTMD