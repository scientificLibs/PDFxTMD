#include "PDFxTMDLib/Factory.h"
#include "PDFxTMDLib/GenericPDF.h"
#include "PDFxTMDLib/Implementation/Interpolator/Collinear/CLHAPDFBilinearInterpolator.h"
#include "PDFxTMDLib/Implementation/Interpolator/Collinear/CLHAPDFBicubicInterpolator.h"
#include "PDFxTMDLib/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h"
#include "PDFxTMDLib/Implementation/Extrapolator/Collinear/CErrExtrapolator.h"
#include "PDFxTMDLib/Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h"
#include "PDFxTMDLib/Common/YamlInfoReader.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include <string>
#include <utility>
#include <optional>
#include <iostream>
#include "PDFxTMDLib/Common/ConfigWrapper.h"
#include <vector>


int main()
{
    using namespace PDFxTMD;
    GenericPDF<CollinearPDFTag,CDefaultLHAPDFFileReader, CLHAPDFBicubicInterpolator,  CErrExtrapolator> genPDF("MMHT2014lo68cl", 0);
    GenericCPDFFactory cPDF;
    auto CJ12min = cPDF.mkCPDF("CJ12min", 0);
    std::cout << "Calculating PDF: " << std::endl;
    double x = 0.0001;

    for (double mu2 = 2; mu2 < 100; mu2++)
    {
       std::cout << mu2 << ": " << CJ12min.pdf(PartonFlavor::u, 0.0001, mu2) << "---" << genPDF.pdf(PartonFlavor::u, 0.0001, mu2) << std::endl;
    }

    std::cout << "----------------" << std::endl;
    for (double mu2 = 10; mu2 < 100; mu2++)
    {
        std::cout << CJ12min.pdf(PartonFlavor::g, x, mu2) << std::endl;
    }
    // std::cout << "----------------" << std::endl;
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
