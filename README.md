# PDFxTMDLib

PDFxTMDLib is a high-performance C++ library for parton distribution functions (PDFs) that integrates both collinear PDFs (cPDFs) and transverse momentum-dependent PDFs (TMDs). It is designed with modern C++17 principles to achieve excellent performance and extensibility. The library provides interfaces for reading PDF grid files and supports standard formats from LHAPDF and TMDLib, while allowing adaptability to non-standard formats.

For a comprehensive understanding of PDFxTMDLib, please refer to the full paper available on [https://arxiv.org/abs/2412.16680](https://arxiv.org/abs/2412.16680).


## Features

- Unified interface for collinear PDFs and TMDs
- Type-erased interfaces for easy extension
- Factory design pattern for simple instantiation
- Cross-platform support (Linux and Windows)
- Thread-safety
- Optional Fortran and python wrappers

## Installation

### Requirements
1. C++17 compatible compiler
2. CMake 3.30 or higher
3. Install g++:
   ```bash
   sudo apt install g++
   ```
### Option 1: Package Manager (Ubuntu/Debian)

1. Download the `.deb` file from the [Releases](https://github.com/Raminkord92/PDFxTMD/releases) section of GitHub. For example, download the current release: `PDFxTMDLib-0.1.1-Linux.deb`.
2. Install with:
   ```bash
   sudo dpkg -i PDFxTMDLib-0.3.0-Linux.deb
   ```

### Option 2: Building from Source

```bash
# Clone the repository
git clone https://github.com/Raminkord92/PDFxTMD.git
cd PDFxTMD

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build and install
make
sudo make install
```

## Quick Start

### 1. Write Your Program
Create a `main.cpp` file with the following code:

```cpp
#include <iostream>
#include <PDFxTMDLib/Factory.h>

int main()
{
    using namespace PDFxTMD;
    GenericCPDFFactory cPDF;
    auto CJ12min = cPDF.mkCPDF("CJ12min", 0);
    double x = 0.01;
    double mu2 = 100;
    std::cout << "Up quark PDF: " << CJ12min.pdf(PartonFlavor::u, x, mu2) << std::endl;

    return 0;
}
```

### 2. Compile Your Program
Compile the program using the following command:
```bash
g++ main.cpp -lPDFxTMDLib -o test.out
```

### 3. Execute Your Program
Run the compiled program:
```bash
./test.out
```

## Basic Usage

Here is a simple example of how to use PDFxTMDLib:

```cpp
// TMD example
using namespace PDFxTMD;
GenericTMDFactory PBTMDObj;
auto PBTMDObj_ = PBTMDObj.mkTMD("PB-NLO-HERAI+II-2023-set2-qs=0.74", 0);
double x = 0.0001;
double kt2 = 10;
double mu2 = 100;
double gPBTMD = PBTMDObj_.tmd(PartonFlavor::g, x, kt2, mu2);
std::cout << "Gluon TMD of PB is: " << gPBTMD << std::endl;

// Collinear PDF example
GenericCPDFFactory ct18Obj;
auto ct18Obj_ = ct18Obj.mkCPDF("CT18NLO", 0);
double gCt18Obj_ = ct18Obj_.pdf(PartonFlavor::g, x, mu2);
std::cout << "Gluon PDF of CT18 is: " << gCt18Obj_ << std::endl;
```

## Advanced Usage

### Working with All Parton Flavors

Retrieve PDFs for all flavors simultaneously:

```cpp
std::array<double, DEFAULT_TOTAL_PDFS> allFlavors;
CJ12min.pdf(x, mu2, allFlavors);

// Access individual flavors from the array
for (int i = 0; i < DEFAULT_TOTAL_PDFS; i++) {
    std::cout << "Flavor " << i << ": " << allFlavors[i] << std::endl;
}
```

### Custom PDF Implementation

Create PDFs with specific reader, interpolator, and extrapolator components:

```cpp
using PDFTag = PDFxTMD::CollinearPDFTag;
using ReaderType = PDFxTMD::CDefaultLHAPDFFileReader;
using InterpolatorType = PDFxTMD::CLHAPDFBicubicInterpolator;
using ExtrapolatorType = PDFxTMD::CErrExtrapolator;

// Create custom PDF with specific components
PDFxTMD::GenericPDF<PDFTag, ReaderType, InterpolatorType, ExtrapolatorType> 
    customPDF("MMHT2014lo68cl", 0);
```

### QCD Coupling

```cpp
PDFxTMD::CouplingFactory couplingFactory;
auto coupling = couplingFactory.mkCoupling("MMHT2014lo68cl");
std::cout << "Alpha_s at mu2=100: " << coupling.AlphaQCDMu2(100) << std::endl;
```

For more comprehensive examples, please refer to the [tutorial.cpp](examples/tutorial.cpp) file in the examples directory, which demonstrates various approaches to using the library features.

## Configuration

PDFxTMDLib uses YAML configuration files to locate PDF sets:

- **Windows**: `C:\ProgramData\PDFxTMDLib\config.yaml`
- **Linux**: `~/.PDFxTMDLib/config.yaml`

Example configuration:
```yaml
paths: "/home/user/pdfs|/usr/share/PDFxTMDLib/pdfs"
```

Multiple paths can be specified with the `|` separator. By default, the current directory and standard installation paths are already included.

## Performance

PDFxTMDLib is optimized for high performance. The library includes benchmarking tools to compare PDFxTMDLib with other libraries like LHAPDF and TMDLib in the `performance_vs_LHAPDF_TMDLIB` directory, where the results are shown in the article [https://arxiv.org/abs/2412.16680](https://arxiv.org/abs/2412.16680), 

## Visualization Tools

For visualization of PDFs and TMDs, you can use the [QtPDFxTMDPlotter](https://github.com/Raminkord92/QtPDFxTMDPlotter), a Qt-based graphical tool built on top of PDFxTMDLib. This tool provides an intuitive interface for plotting and comparing different PDF and TMD sets, making it easier to visualize and analyze the distributions.

## Contributing

Contributions are welcome! If you are interested in contributing, please don't hesitate to contact me at [raminkord92@gmail.com](mailto:raminkord92@gmail.com).

## License

This project is licensed under the Creative Commons Attribution 4.0 International License. For more details, please see the [LICENSE](LICENSE) file.

## Contact

For any inquiries, please contact [raminkord92@gmail.com](mailto:raminkord92@gmail.com).
