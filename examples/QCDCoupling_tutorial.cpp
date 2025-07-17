#include <PDFxTMDLib/PDFSet.h>
#include <PDFxTMDLib/Interface/IQCDCoupling.h>
#include <iostream>

int main() 
{
	using namespace PDFxTMD;
    // Calculate coupling with PDFSet class
    // Method 1: From a PDFSet
    //cPDF
    PDFSet<CollinearPDFTag> cpdfSet("MSHT20nlo_as120");
    double mu2 = 10000; 
    double alpha_sCPDF = cpdfSet.alphasQ2(mu2);
    std::cout << "PDFSet (MSHT cPDF) alphas(mu2=" << mu2 << ") = " << alpha_sCPDF << std::endl;

    // TMD
    PDFSet<TMDPDFTag> tmdSet("PB-LO-HERAI+II-2020-set2");
    double alpha_sTMD = tmdSet.alphasQ2(mu2);
    std::cout << "PDFSet (PB TMD) alphas(mu2=" << mu2 << ") = " << alpha_sTMD << std::endl;

    // Method 2: From a factory method
    auto couplingFactory = CouplingFactory();
	// Using cPDFset for calculating coupling
    auto MMHT2014LO68cl_coupling = couplingFactory.mkCoupling("MMHT2014lo68cl");
    std::cout << "Coupling factory (MMHT cPDF) " << "alphas(mu2=" << mu2 << ") = " << MMHT2014LO68cl_coupling.AlphaQCDMu2(mu2) << std::endl;
    // Using TMD for calculating coupling
    auto PB_coupling = couplingFactory.mkCoupling("PB-LO-HERAI+II-2020-set2");
    std::cout << "Coupling factory (PB TMD) " << "alphas(mu2=" << mu2 << ") = " << PB_coupling.AlphaQCDMu2(mu2) << std::endl;
    return 0;
}