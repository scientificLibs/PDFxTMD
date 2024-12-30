
#include "Factory.h"

int main()
{
    using namespace PDFxTMD;
    GenericCPDFFactory cPDF;
    auto CJ12min = cPDF.mkCPDF("CJ12min", 0);
    std::cout << "test " <<CJ12min.pdf(PartonFlavor::u, 0.0001, 100) << std::endl;
    return 0;
}