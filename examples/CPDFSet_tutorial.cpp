#include <PDFxTMDLib/PDFSet.h>
#include <iostream>
int main()
{
    // Instantiate a PDFSet for collinear distributions
    PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> cpdfSet("MSHT20nlo_as120");
    // Access the central member ( index 0)
    auto central_pdf = cpdfSet[0];
    // Define kinematics
    double x = 0.1;     // Longitudinal momentum fraction
    double mu2 = 10000; // Factorization scale squared
    // Evaluate PDF for gluon
    double gluon_pdf = central_pdf->pdf(PDFxTMD::PartonFlavor ::g, x, mu2);
    std ::cout << "Gluon PDF at x=" << x << ", mu2 =" << mu2 << " GeV2 : " << gluon_pdf
               << std ::endl;
    return 0;
}