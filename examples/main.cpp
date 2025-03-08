
#include "Factory.h"
#include "GenericPDF.h"
#include "Implementation/Interpolator/Collinear/CBilinearInterpolator.h"
#include "Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h"
#include "Implementation/Interpolator/Collinear/GSL/CGSLBilinear.h"
#include "Implementation/Interpolator/Collinear/GSL/CGSLBicubic.h"
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
    auto CJ12min = cPDF.mkCPDF("CJ12min", 0);
    std::cout << "Calculating PDF: " << std::endl;
    double x = 0.0001;
    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CGSLBicubicInterpolator, CContinuationExtrapolator<CGSLBicubicInterpolator>> genCPDF("CJ12min", 0);

    for (double mu2 = 1; mu2 < 100; mu2++)
    {
       std::cout << mu2 << ": " << CJ12min.pdf(PartonFlavor::d, x, mu2) ;
        //std::cout <<": " << genCPDF.pdf(PartonFlavor::d, x, mu2) << std::endl;
    }

    auto info = genCPDF.getStdPDFInfo();
    std::cout << "X_min: " << info.XMin << " X_max: " << info.XMax << std::endl; 
    std::cout << "----------------" << std::endl;
    for (double mu2 = 10; mu2 < 100; mu2++)
    {
        std::cout << CJ12min.pdf(PartonFlavor::g, x, mu2) << std::endl;
    }
    std::cout << "----------------" << std::endl;
    GenericTMDFactory tmdFactory;
    auto PBNLOHera2023 = tmdFactory.mkTMD("PB-LO-HERAI+II-2020-set2", 0);
    double xTMD = 0.08;
    double mu2TMD = 100;
    for (double kt2 = 2; kt2 < 100; kt2++)
        std::cout << "kt2 " <<  kt2 << ": " << PBNLOHera2023.tmd(PartonFlavor::u, xTMD, kt2, mu2TMD)
                  << std::endl;
    CouplingFactory qcdCouplingFactory;
    auto cj12QCDCoupling = qcdCouplingFactory.mkCoupling("CJ12min");

    for (double mu2 = 2; mu2 < 10; mu2++)
    {
        std::cout << mu2 << " " << cj12QCDCoupling.AlphaQCDMu2(mu2) << std::endl;
    }
    return 0;
}
