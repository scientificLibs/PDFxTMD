// FactoryWrapper.h
#pragma once
#include "PDFxTMDLib/Factory.h"

extern "C"
{
    /** @brief Create a QCD-coupling factory for C or Fortran callers. */
    void *create_coupling_factory();
    /** @brief Destroy a coupling factory created by create_coupling_factory. */
    void delete_coupling_factory(void *factory);
    /** @brief Create a QCD-coupling object from a factory and set name. */
    void *mk_coupling(void *factory, const char *pdfSetName);

    /** @brief Create a TMD-PDF factory for C or Fortran callers. */
    void *create_tmd_factory();
    /** @brief Destroy a TMD-PDF factory created by create_tmd_factory. */
    void delete_tmd_factory(void *factory);
    /** @brief Create a TMD-PDF member from a factory, set name, and member index. */
    void *mk_tmd(void *factory, const char *pdfSetName, int setMember);

    /** @brief Create a collinear-PDF factory for C or Fortran callers. */
    void *create_cpdf_factory();
    /** @brief Destroy a collinear-PDF factory created by create_cpdf_factory. */
    void delete_cpdf_factory(void *factory);
    /** @brief Create a collinear-PDF member from a factory, set name, and member index. */
    void *mk_cpdf(void *factory, const char *pdfSetName, int setMember);
}
