// FactoryWrapper.cpp
#include "FortranFactoryWrapper.h"
#include "stdio.h"

extern "C"
{
    // CouplingFactory
    void *create_coupling_factory()
    {
        return new PDFxTMD::CouplingFactory();
    }

    void delete_coupling_factory(void *factory)
    {
        delete static_cast<PDFxTMD::CouplingFactory *>(factory);
    }

    void *mk_coupling(void *factory, const char *pdfSetName)
    {
        auto *couplingFactory = static_cast<PDFxTMD::CouplingFactory *>(factory);
        return new te::poly<PDFxTMD::IQCDCoupling>(couplingFactory->mkCoupling(pdfSetName));
    }

    double alphaQCDMu2_wrapper(void *couplingObject, double mu2)
    {
        auto *coupling = static_cast<te::poly<PDFxTMD::IQCDCoupling> *>(couplingObject);
        return (*coupling).AlphaQCDMu2(mu2);
    }

    // GenericTMDFactory
    void *create_tmd_factory()
    {
        return new PDFxTMD::GenericTMDFactory();
    }

    void delete_tmd_factory(void *factory)
    {
        delete static_cast<PDFxTMD::GenericTMDFactory *>(factory);
    }

    void *mk_tmd(void *factory, const char *pdfSetName, int setMember)
    {
        auto *tmdFactory = static_cast<PDFxTMD::GenericTMDFactory *>(factory);
        return new te::poly<PDFxTMD::ITMD>(tmdFactory->mkTMD(pdfSetName, setMember));
    }
    double tmd_wrapper(void *tmd_obj, int flavor, double x, double kt2, double mu2)
    {
        auto *tmd = static_cast<te::poly<PDFxTMD::ITMD> *>(tmd_obj);
        return (*tmd).tmd(static_cast<PDFxTMD::PartonFlavor>(flavor), x, kt2, mu2);
    }
    // GenericCPDFFactory
    void *create_cpdf_factory()
    {
        return new PDFxTMD::GenericCPDFFactory();
    }

    void delete_cpdf_factory(void *factory)
    {
        delete static_cast<PDFxTMD::GenericCPDFFactory *>(factory);
    }

    void *mk_cpdf(void *factory, const char *pdfSetName, int setMember)
    {
        auto *cpdfFactory = static_cast<PDFxTMD::GenericCPDFFactory *>(factory);
        printf("pdfSetName %s", pdfSetName);
        return new te::poly<PDFxTMD::ICPDF>(cpdfFactory->mkCPDF(pdfSetName, setMember));
    }
    double cpdf_wrapper(void *cpdf_obj, int flavor, double x, double mu2)
    {
        auto *cpdf = static_cast<te::poly<PDFxTMD::ICPDF> *>(cpdf_obj);
        return (*cpdf).pdf(static_cast<PDFxTMD::PartonFlavor>(flavor), x, mu2);
    }
}