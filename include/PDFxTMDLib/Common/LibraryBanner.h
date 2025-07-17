#pragma once
#include <iostream>
#include <string>

namespace PDFxTMD
{
class LibraryBanner
{
  public:
    static LibraryBanner &Instance()
    {
        static LibraryBanner instance;
        return instance;
    }

    LibraryBanner(const LibraryBanner &) = delete;
    LibraryBanner &operator=(const LibraryBanner &) = delete;

  private:
    LibraryBanner()
    {
        printBanner();
    }

    void printBanner() const
    {
        std::string banner = R"(
============================================================
          Welcome to the PDFxTMDLib Library
============================================================
Please cite the following paper when using this library:

Title: PDFxTMDLib: A High-Performance C++ Library for Collinear and Transverse Momentum Dependent Parton Distribution Functions
Authors: R. Kord Valeshabadi, S. Rezaie
Source: https://arxiv.org/abs/2412.16680

Thank you for using our library!
============================================================
        )";
        std::cout << banner << std::endl;
    }
};
} // namespace PDFxTMD