#include <PDFxTMDLib/PDFSet.h>
#include <iostream>
int main()
{
    // Instantiate a PDFSet for TMD distributions
    PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> tmdSet("PB-LO-HERAI+II-2020-set2");
    // Access the central member ( index 0)
    auto central_tmd = tmdSet[0];
    // Define kinematics
    double x = 0.01;  // Longitudinal momentum fraction
    double kt2 = 10;  // Transverse momentum squared
    double mu2 = 100; // Factorization scale squared
    // Evaluate TMD for up quark
    double up_tmd = central_tmd->tmd(PDFxTMD::PartonFlavor::u, x, kt2, mu2);
    std::cout << "Up - quark TMD at x=" << x << ", kT2=" << kt2 << ", mu2=" << mu2
               << " GeV2 : " << up_tmd << std::endl;
    return 0;
}