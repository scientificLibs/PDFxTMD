
#include "Factory.h"
#include "GenericPDF.h"
#include "Implementation/Interpolator/Collinear/CBilinearInterpolator.h"
#include "Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h"

int main()
{
    using namespace PDFxTMD;
    GenericCPDFFactory cPDF;
    auto CJ12min = cPDF.mkCPDF("CJ12min", 0);
    std::cout << "test " <<CJ12min.pdf(PartonFlavor::u, 0.0001, 100) << std::endl;
    GenericPDF<CollinearPDFTag, CDefaultLHAPDFFileReader, CBilinearInterpolator, CContinuationExtrapolator<CDefaultLHAPDFFileReader, CBilinearInterpolator>> genCPDF("CJ12min", 0);
    auto info = genCPDF.getStdPDFInfo();
    std::cout << "X_min: " << info.XMin << " X_max: " << info.XMax << std::endl;  
    return 0;
}