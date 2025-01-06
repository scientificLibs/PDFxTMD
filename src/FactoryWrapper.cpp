// FactoryWrapper.cpp
#include "FactoryWrapper.h"
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
    double cpdf_pdf(void *cpdf_obj, int flavor, double x, double mu2)
    {
        auto *cpdf = static_cast<te::poly<PDFxTMD::ICPDF> *>(cpdf_obj);
        return (*cpdf).pdf(static_cast<PDFxTMD::PartonFlavor>(flavor), x, mu2);
    }
}