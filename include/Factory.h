#pragma once
#include "GenericPDF.h"
#include <lib3/boost/te.hpp>

namespace te = boost::te;
namespace PDFxTMD
{

struct ITMD
{
    ITMD() = default; // Default constructor
    double tmd(PartonFlavor flavor, double x, double kt2, double mu2)
    {
        return te::call<double>([](auto &self, auto flavor, auto x, double kt2,
                                   auto mu2) { self->tmd(flavor, x, kt2, mu2); },
                                *this, flavor, x, kt2, mu2);
    }
};

struct ICPDF
{
    ICPDF() = default; // Default constructor
    double pdf(PartonFlavor flavor, double x, double mu2)
    {
        return te::call<double>(
            [](auto &self, auto flavor, auto x, auto mu2) { self->pdf(flavor, x, mu2); }, *this,
            flavor, x, mu2);
    }
};

class GenericTMDFactory
{
  public:
    GenericTMDFactory() = default;
    te::poly<ITMD> mkTMD(const std::string &pdfSetName, int setMember);

  private:
    std::string m_pdfSetName;
    int m_setMember;
};

class GenericCPDFFactory
{
  public:
    GenericCPDFFactory() = default;
    te::poly<ICPDF> mkCPDF(const std::string &pdfSetName, int setMember);
};
} // namespace PDFxTMD