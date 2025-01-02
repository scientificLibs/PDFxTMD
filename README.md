# PDFxTMD

PDFxTMD is a library for parton distribution functions (PDFs) that integrates both collinear PDFs (cPDFs) and transverse momentum-dependent PDFs (TMDs). It is designed with modern C++ principles to achieve high performance and extensibility. The library provides interfaces for reading PDF grid files and supports standard formats from LHAPDF and TMDLib, while allowing adaptability to non-standard formats. 

For a comprehensive understanding of PDFxTMD, please refer to the full paper available on https://arxiv.org/abs/2412.16680.

## Installation

To install PDFxTMD, please refer to the [INSTALL](INSTALL) file. Also, Check wiki for more information.

## Usage

Here is a simple example of how to use PDFxTMD:

```cpp
    using namespace PDFxTMD;
    GenericTMDFactory PBTMDObj;
    auto PBTMDObj_ = PBTMDObj.mkTMD("PB-NLO-HERAI+II-2023-set2-qs=0.74", 0);
    double x = 0.0001;
    double kt2 = 10;
    double mu2 = 100;
    double gPBTMD = PBTMDObj_.tmd(PartonFlavor::g, x, kt2, mu2);
    std::cout << "Gluon TMD of PB is: " << gPBTMD << std::endl;
    
    GenericCPDFFactory ct18Obj;
    auto ct18Obj_ = ct18Obj.mkCPDF("CT18NLO", 0);
    double gCt18Obj_ = ct18Obj_.pdf(PartonFlavor::g, x, mu2);
    std::cout << "Gluon PDF of CT18 is: " << gCt18Obj_ << std::endl;
```

## Contributing

Contributions are welcome! If you are interested in contributing, please don't hesitate to contact me at [raminkord92@gmail.com](mailto:raminkord92@gmail.com).

## License

This project is licensed under the Creative Commons Attribution 4.0 International License. For more details, please see the [LICENSE](LICENSE) file.

## Contact

For any inquiries, please contact [raminkord92@gmail.com](mailto:raminkord92@gmail.com).
