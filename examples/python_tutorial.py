"""
PDFxTMDLib Python Tutorial

This tutorial demonstrates how to use all exposed methods of the PDFxTMD Python bindings,
including collinear PDFs (CPDF), TMDs, and QCD coupling.

Installation:
    pip install pdfxtmd

API Reference: https://github.com/Raminkord92/PDFxTMD

Exposed classes and methods:
- PartonFlavor (enum)
- GenericCPDFFactory.mkCPDF(pdfSetName: str, setMember: int) -> ICPDF
- ICPDF.pdf(flavor: PartonFlavor, x: float, mu2: float) -> float
- ICPDF.pdf(x: float, mu2: float, output: list) -> None
- GenericTMDFactory.mkTMD(pdfSetName: str, setMember: int) -> ITMD
- ITMD.tmd(flavor: PartonFlavor, x: float, kt2: float, mu2: float) -> float
- ITMD.tmd(x: float, kt2: float, mu2: float, output: list) -> None
- CouplingFactory.mkCoupling(pdfSetName: str) -> IQCDCoupling
- IQCDCoupling.AlphaQCDMu2(mu2: float) -> float
"""

import pdfxtmd

def main():
    # --- Collinear PDFs (CPDF) ---

    # 1. Create a CPDF factory and CPDF object
    cpdf_factory = pdfxtmd.GenericCPDFFactory()
    cpdf = cpdf_factory.mkCPDF("CT18NLO", 0)

    # 2. Evaluate a single flavor
    x = 0.01
    mu2 = 100
    print("Up quark PDF:", cpdf.pdf(pdfxtmd.PartonFlavor.u, x, mu2))
    print("Gluon PDF:", cpdf.pdf(pdfxtmd.PartonFlavor.g, x, mu2))

    # 3. Evaluate all flavors at once (output is modified in-place)
    all_flavors = [0.0] * 13
    cpdf.pdf(x, mu2, all_flavors)
    print("All flavors (CPDF):", all_flavors)

    # --- TMD PDFs ---

    # 4. Create a TMD factory and TMD object
    tmd_factory = pdfxtmd.GenericTMDFactory()
    tmd = tmd_factory.mkTMD("PB-NLO-HERAI+II-2023-set2-qs=0.74", 0)

    # 5. Evaluate a single flavor TMD
    kt2 = 10
    print("Gluon TMD:", tmd.tmd(pdfxtmd.PartonFlavor.g, x, kt2, mu2))
    print("Up quark TMD:", tmd.tmd(pdfxtmd.PartonFlavor.u, x, kt2, mu2))

    # 6. Evaluate all TMD flavors at once (output is modified in-place)
    all_tmd_flavors = [0.0] * 13
    tmd.tmd(x, kt2, mu2, all_tmd_flavors)
    print("All flavors (TMD):", all_tmd_flavors)

    # --- QCD Coupling ---

    # 7. Create a coupling factory and coupling object
    coupling_factory = pdfxtmd.CouplingFactory()
    coupling = coupling_factory.mkCoupling("CT18NLO")

    # 8. Evaluate alpha_s at different scales
    for scale in [10, 100, 1000, 10000]:
        print(f"Alpha_s at mu2={scale}:", coupling.AlphaQCDMu2(scale))

    # --- Error Handling Examples ---

    try:
        # Invalid x (should raise an error)
        cpdf.pdf(pdfxtmd.PartonFlavor.u, -0.1, mu2)
    except Exception as e:
        print("Expected error for invalid x in CPDF:", e)

    try:
        # Invalid kt2 (should raise an error)
        tmd.tmd(pdfxtmd.PartonFlavor.g, x, -5, mu2)
    except Exception as e:
        print("Expected error for invalid kt2 in TMD:", e)

    try:
        # Invalid mu2 (should raise an error)
        coupling.AlphaQCDMu2(-1)
    except Exception as e:
        print("Expected error for invalid mu2 in Coupling:", e)

    # --- Enumerate all PartonFlavor values ---
    print("All PartonFlavor enum values:")
    for flavor in pdfxtmd.PartonFlavor:
        print(f"  {flavor.name}: {flavor.value}")

if __name__ == "__main__":
    main()