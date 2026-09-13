"""Minimal Python example for dense or hybrid PDFxTMD DPD sets."""

import numpy as np
import pdfxtmd


DPD_SET = "MSTW2008lo68cl_GSDPDF_PDFxTMD"


def main() -> None:
    if not pdfxtmd.__has_dpd__:
        raise RuntimeError("This pdfxtmd build was compiled without DPD support")

    dpd = pdfxtmd.GenericCDPDFactory().mkCDPD(DPD_SET, 0)

    gg = dpd.dpd(
        pdfxtmd.PartonFlavor.g,
        pdfxtmd.PartonFlavor.g,
        1.0e-2,
        100.0,
        2.0e-2,
        400.0,
    )
    print("g-g DPD:", gg)

    x1 = np.array([1.0e-3, 1.0e-2, 5.0e-2])
    x2 = np.array([2.0e-3, 2.0e-2, 1.0e-1])
    mu1_2 = np.full_like(x1, 100.0)
    mu2_2 = np.full_like(x2, 400.0)

    values = dpd.dpd_batch(
        pdfxtmd.PartonFlavor.g,
        pdfxtmd.PartonFlavor.g,
        x1,
        mu1_2,
        x2,
        mu2_2,
    )
    print("g-g DPD batch:", values)


if __name__ == "__main__":
    main()
