#include <PDFxTMDLib/PDFSet.h>
#include <iostream>

int main() 
{
	using namespace PDFxTMD;
    // Create a cPDF with factory
    auto cpdf_factory = GenericCPDFFactory();

    // Create a single cPDF member (set="MMHT2014lo68cl", member=0)
    auto cpdf = cpdf_factory.mkCPDF("MMHT2014lo68cl", 0);

    // Evaluate cPDF directly
    double x = 0.001, mu2 = 100;
    double gluon_pdf = cpdf.pdf(PartonFlavor::g, x, mu2);
    std::cout << "Gluon cPDF: " << gluon_pdf << std::endl;

    // Create a TMD with factory
    auto tmd_factory = GenericTMDFactory();

    // Create a single tmd member (set="PB-LO-HERAI+II-2020-set2", member=0)
    auto tmd = tmd_factory.mkTMD("PB-LO-HERAI+II-2020-set2", 0);

    // Evaluate TMD directly
    double kt2 = 10;
    double gluon_tmd = tmd.tmd(PartonFlavor::g, x, kt2, mu2);
    std::cout << "Gluon TMD: " << gluon_tmd << std::endl;
    return 0;
}