#include "Common/GSL/common.h"

PDFxTMD::DefaultAllFlavorShape ConvertDefaultShapeGrids(
    const PDFxTMD::DefaultAllFlavorShape &defaultShape)
{
    PDFxTMD::DefaultAllFlavorShape output;
    const auto &defaultGrid = defaultShape.grids;
    size_t xSize = defaultShape.log_x_vec.size();
    size_t q2Size = defaultShape.log_mu2_vec.size();
    for (const auto &pairFlavorGrid : defaultGrid)
    {
        size_t perPartonGridSize = pairFlavorGrid.second.size();
        output.grids[pairFlavorGrid.first].resize(perPartonGridSize);
        for (size_t ix = 0; ix < xSize; ++ix)
        {
            for (size_t iq2 = 0; iq2 < q2Size; ++iq2)
            {
                output.grids[pairFlavorGrid.first][iq2 * xSize + ix] =
                    defaultGrid.at(pairFlavorGrid.first)[ix * q2Size + iq2];
            }
        }
    }
    output.flavors = std::move(defaultShape.flavors);
    output.log_mu2_vec = std::move(defaultShape.log_mu2_vec);
    output.log_x_vec = std::move(defaultShape.log_x_vec);
    output.mu2_vec = std::move(defaultShape.mu2_vec);
    output.x_vec = std::move(defaultShape.x_vec);
    return output;
}