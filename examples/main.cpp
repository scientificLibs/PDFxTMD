
#include "Factory.h"
#include "GenericPDF.h"
#include "Implementation/Interpolator/Collinear/CBilinearInterpolator.h"
#include "Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h"
#include "Common/YamlInfoReader.h"
#include "Common/PartonUtils.h"
#include <string>
#include <utility>
#include <optional>
#include <iostream>
#include "Implementation/Coupling/Interpolation/YamlCouplingInterp.h"

int main()
{
    using namespace PDFxTMD;
    GenericCPDFFactory cPDF;
    auto CJ12min = cPDF.mkCPDF("CJ12min", 0);
    std::cout << "test " <<CJ12min.pdf(PartonFlavor::u, 0.0001, 100) << std::endl;
    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CBilinearInterpolator, CContinuationExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>> genCPDF("CJ12min", 0);
    auto info = genCPDF.getStdPDFInfo();
    std::cout << "X_min: " << info.XMin << " X_max: " << info.XMax << std::endl;  
    std::pair<std::optional<std::string>, ErrorType> CJ12minInfoPath = StandardInfoFilePath("CJ12min");
    YamlCouplingInfo couplingInfo_;
    if (CJ12minInfoPath.second == ErrorType::None)
    {
        std::cout << "Info path of CJ12min is: " << *CJ12minInfoPath.first << std::endl;
        std::pair<std::optional<YamlCouplingInfo>, ErrorType> couplingInfo = YamlCouplingInfoReader(*CJ12minInfoPath.first);
        if (couplingInfo.second == ErrorType::None)
        {
            couplingInfo_ = *couplingInfo.first;
            std::cout << "Info path of CJ12min is: " << (int)couplingInfo_.alphsOrder << std::endl;
        }
    }

    CouplingFactory qcdCouplingFactory;
    auto cj12QCDCoupling = qcdCouplingFactory.mkCoupling("CJ12min");

    for (double mu2 = 2; mu2 < 1000; mu2++)
    {
        std::cout << mu2 << " " << cj12QCDCoupling.AlphaQCDMu2(mu2) << std::endl;
    }
    return 0;
}