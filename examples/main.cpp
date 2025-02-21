
#include "Factory.h"
#include "GenericPDF.h"
#include "Implementation/Interpolator/Collinear/CBilinearInterpolator.h"
#include "Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h"
#include "Implementation/Interpolator/Collinear/GSL/CGSLBilinear.h"
#include "Common/YamlInfoReader.h"
#include "Common/PartonUtils.h"
#include <string>
#include <utility>
#include <optional>
#include <iostream>


int main()
{

    using namespace PDFxTMD;
    GenericCPDFFactory cPDF;
    auto CJ12min = cPDF.mkCPDF("MSHT20lo_as130", 0);
    std::cout << "Calculating PDF: " << std::endl;
    double x = 0.0001;
    for (double mu2 = 2; mu2 < 100; mu2++)
        std::cout << mu2 << ": " << CJ12min.pdf(PartonFlavor::u, x, mu2) << std::endl;

    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBilinearInterpolator, CContinuationExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>> genCPDF("CJ12min", 0);
    auto info = genCPDF.getStdPDFInfo();
    std::cout << "X_min: " << info.XMin << " X_max: " << info.XMax << std::endl; 
    std::cout << "----------------" << std::endl;
    for (double mu2 = 10; mu2 < 100; mu2++)
    {
        std::cout << genCPDF.pdf(PartonFlavor::g, x, mu2) << std::endl;
    }
    std::cout << "----------------" << std::endl;
    GenericTMDFactory tmdFactory;
    auto PBNLOHera2023 = tmdFactory.mkTMD("PB-NLO-HERAI+II-2023-set2-qs=1.04", 0);
    double xTMD = 0.0001;
    double mu2TMD = 100;
    for (double kt2 = 2; kt2 < 100; kt2++)
        std::cout << kt2 << ": " << PBNLOHera2023.tmd(PartonFlavor::u, xTMD, kt2, mu2TMD) << std::endl;
    CouplingFactory qcdCouplingFactory;
    auto cj12QCDCoupling = qcdCouplingFactory.mkCoupling("CJ12min");

    for (double mu2 = 2; mu2 < 10; mu2++)
    {
        std::cout << mu2 << " " << cj12QCDCoupling.AlphaQCDMu2(mu2) << std::endl;
    }
    return 0;
}
