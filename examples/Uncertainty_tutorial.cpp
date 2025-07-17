#include <PDFxTMDLib/PDFSet.h>
#include <iostream>
int main()
{
    // Instantiate a PDFSet for collinear distributions
    PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> cpdfSet("MSHT20nlo_as120");
    double x = 0.1;     // Longitudinal momentum fraction
    double mu2 = 100;   // 100 GeV^2
    // Calculate PDF uncertainty at default confidence level
    PDFxTMD ::PDFUncertainty cPDFUncertainty = cpdfSet.Uncertainty(PDFxTMD::PartonFlavor::g, x, mu2);
    std::cout << "xg (MSHT20nlo_as120) = " << cPDFUncertainty.central << " + " << cPDFUncertainty.errplus << " - " << cPDFUncertainty.errminus << std ::endl;
    // Calculate uncertainty at 90% confidence level
    PDFxTMD ::PDFUncertainty cPDFuncertainty_90 = cpdfSet.Uncertainty(PDFxTMD::PartonFlavor::g, x, mu2, 90.0);
    // Calculate correlation between gluon and up quark distributions
    double cpfCorrelation = cpdfSet.Correlation(PDFxTMD::PartonFlavor ::g, x, mu2, PDFxTMD::PartonFlavor::u, x, mu2);
    std::cout << "cPDF correlation (MSHT20nlo_as120) between g and u: " << cpfCorrelation << std::endl;
    // TMD
        // Instantiate a PDFSet for TMD distributions
    PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> tmdSet("PB-LO-HERAI+II-2020-set2");
    double kt2 = 10;    // 10 GeV^2
    // Calculate PDF uncertainty at default confidence level
    PDFxTMD ::PDFUncertainty tmdUncertainty = tmdSet.Uncertainty(PDFxTMD::PartonFlavor::g, x, kt2, mu2);
    std::cout << "xg (PB-LO-HERAI+II-2020-set2) = " << tmdUncertainty.central << " + " << tmdUncertainty.errplus << " - " << tmdUncertainty.errminus << std ::endl;
    // Calculate uncertainty at 90% confidence level
    PDFxTMD ::PDFUncertainty tmdUncertainty_90 = tmdSet.Uncertainty(PDFxTMD::PartonFlavor::g, x, mu2, 90.0);
    // Calculate correlation between gluon and up quark distributions
    double tmdCorrelation = tmdSet.Correlation(PDFxTMD::PartonFlavor ::g, x, kt2, mu2, PDFxTMD::PartonFlavor::u, x, kt2, mu2);
    std::cout << "TMD Correlation for (PB-LO-HERAI+II-2020-set2) between g and u: " << tmdCorrelation << std::endl;
    return 0;
}