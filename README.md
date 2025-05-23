# PDFxTMDLib

<!-- AppVeyor Build Status Badges -->
[![Build status: Windows](https://ci.appveyor.com/api/projects/status/github/Raminkord92/PDFxTMD?branch=main&svg=true&job=Image:Visual%20Studio%202019)](https://ci.appveyor.com/project/Raminkord92/PDFxTMD)
[![Build status: Linux](https://ci.appveyor.com/api/projects/status/github/Raminkord92/PDFxTMD?branch=main&svg=true&job=Image:Ubuntu2204)](https://ci.appveyor.com/project/Raminkord92/PDFxTMD)
[![Build status: macOS](https://ci.appveyor.com/api/projects/status/github/Raminkord92/PDFxTMD?branch=main&svg=true&job=Image:macOS)](https://ci.appveyor.com/project/Raminkord92/PDFxTMD)

PDFxTMDLib is a high-performance C++ library for parton distribution functions (PDFs), supporting both collinear PDFs (cPDFs) and transverse momentum-dependent PDFs (TMDs). It is designed with modern C++17 principles for performance and extensibility, and provides interfaces for reading standard PDF grid files (LHAPDF, TMDLib) as well as custom formats.

For a comprehensive understanding of PDFxTMDLib, please refer to the full paper available on [https://arxiv.org/abs/2412.16680](https://arxiv.org/abs/2412.16680).

---

## Features

- Unified interface for collinear PDFs and TMDs
- Type-erased interfaces for easy extension
- Factory design pattern for simple instantiation
- Cross-platform support (Linux, Windows, macOS)
- Thread-safety
- Optional Fortran and Python wrappers

---

## Installation

### Requirements

- C++17 compatible compiler
- CMake 3.30 or higher

### Option 1: Package Manager (Ubuntu/Debian)

1. Download the `.deb` file from the [Releases](https://github.com/Raminkord92/PDFxTMD/releases) section of GitHub.
2. Install with:
   ```bash
   sudo dpkg -i PDFxTMDLib-0.3.0-Linux.deb
   ```

### Option 2: Building from Source

```bash
git clone https://github.com/Raminkord92/PDFxTMD.git
cd PDFxTMD
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```

---

## Quick Start (C++)

### 1. Write Your Program

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

### 2. Compile

```bash
g++ main.cpp -lPDFxTMDLib -o test.out
```

### 3. Run

```bash
./test.out
```

---

## Basic Usage (C++)

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


### C++ Usage

PDFxTMDLib can be used in various ways within C++ projects. Here are some common usage patterns:

- **Creating and Using a CPDF Object**

  ```cpp
  #include <PDFxTMDLib/Factory.h>

  using namespace PDFxTMD;

  // Create a factory for collinear PDFs
  GenericCPDFFactory cPDF;

  // Make a specific CPDF object (e.g., "CJ12min")
  auto CJ12min = cPDF.mkCPDF("CJ12min", 0);

  // Define variables for x and mu2
  double x = 0.01;
  double mu2 = 100;

  // Access the PDF value for the up quark
  double upQuarkPDF = CJ12min.pdf(PartonFlavor::u, x, mu2);
  ```

- **Creating and Using a TMD Object**

  ```cpp
  #include <PDFxTMDLib/Factory.h>

  using namespace PDFxTMD;

  // Create a factory for TMDs
  GenericTMDFactory PBTMDObj;

  // Make a specific TMD object (e.g., "PB-NLO-HERAI+II-2023-set2-qs=0.74")
  auto PBTMDObj_ = PBTMDObj.mkTMD("PB-NLO-HERAI+II-2023-set2-qs=0.74", 0);

  // Define variables for x, kt2, and mu2
  double x = 0.0001;
  double kt2 = 10;
  double mu2 = 100;

  // Access the TMD value for the gluon
  double gluonTMD = PBTMDObj_.tmd(PartonFlavor::g, x, kt2, mu2);
  ```

- **Batch Processing with All Flavors**

  To retrieve PDFs or TMDs for all parton flavors simultaneously, you can use an array or vector:

  ```cpp
  #include <array>
  #include <PDFxTMDLib/Factory.h>

  using namespace PDFxTMD;

  // Assuming CJ12min is already created as in previous examples
  std::array<double, DEFAULT_TOTAL_PDFS> allFlavors;

  // Get PDFs for all flavors
  CJ12min.pdf(x, mu2, allFlavors);

  // Access individual flavor results
  for (size_t i = 0; i < allFlavors.size(); ++i) {
      std::cout << "Flavor " << i << ": " << allFlavors[i] << std::endl;
  }
  ```

- **Custom PDF or TMD Implementations**

  For advanced users, PDFxTMDLib allows the creation of PDFs or TMDs with custom components for reading, interpolating, and extrapolating:

  ```cpp
  #include <PDFxTMDLib/Factory.h>

  using namespace PDFxTMD;

  // Define custom components
  using PDFTag = CollinearPDFTag;
  using ReaderType = CDefaultLHAPDFFileReader;
  using InterpolatorType = CLHAPDFBicubicInterpolator;
  using ExtrapolatorType = CErrExtrapolator;

  // Create a custom PDF
  GenericPDF<PDFTag, ReaderType, InterpolatorType, ExtrapolatorType> customPDF("MMHT2014lo68cl", 0);
  ```

- **QCD Coupling**

  Accessing the QCD coupling constant is straightforward:

  ```cpp
  #include <PDFxTMDLib/CouplingFactory.h>

  using namespace PDFxTMD;

  CouplingFactory couplingFactory;
  auto coupling = couplingFactory.mkCoupling("MMHT2014lo68cl");

  // Get Alpha_s value at a specific scale
  double alphaS = coupling.AlphaQCDMu2(100);
  ```

---

### Python Wrapper

PDFxTMDLib also provides a native Python interface using `pybind11`, exposing all major features of the C++ library.

#### Installation

Install from PyPI:

```bash
pip install pdfxtmd
```

#### Quick Python Example

```python
import pdfxtmd

# Create a collinear PDF object
cpdf_factory = pdfxtmd.GenericCPDFFactory()
cpdf = cpdf_factory.mkCPDF("CT18NLO", 0)
x = 0.01
mu2 = 100
up_pdf = cpdf.pdf(pdfxtmd.PartonFlavor.u, x, mu2)
print("Up quark PDF:", up_pdf)

# Create a TMD PDF object
tmd_factory = pdfxtmd.GenericTMDFactory()
tmd = tmd_factory.mkTMD("PB-NLO-HERAI+II-2023-set2-qs=0.74", 0)
kt2 = 10
gluon_tmd = tmd.tmd(pdfxtmd.PartonFlavor.g, x, kt2, mu2)
print("Gluon TMD:", gluon_tmd)

# Compute all flavors at once
all_flavors = [0.0] * 13
cpdf.pdf(x, mu2, all_flavors)
print("All flavors:", all_flavors)
```

#### QCD Coupling Example

```python
coupling_factory = pdfxtmd.CouplingFactory()
coupling = coupling_factory.mkCoupling("CT18NLO")
for scale in [10, 100, 1000, 10000]:
    print(f"Alpha_s at mu2={scale}:", coupling.AlphaQCDMu2(scale))
```

#### Error Handling

The Python API raises exceptions for invalid input:

```python
try:
    cpdf.pdf(pdfxtmd.PartonFlavor.u, -0.1, mu2)  # Invalid x
except Exception as e:
    print("Expected error for invalid x in CPDF:", e)

try:
    tmd.tmd(pdfxtmd.PartonFlavor.g, x, -5, mu2)  # Invalid kt2
except Exception as e:
    print("Expected error for invalid kt2 in TMD:", e)

try:
    coupling.AlphaQCDMu2(-1)  # Invalid mu2
except Exception as e:
    print("Expected error for invalid mu2 in Coupling:", e)
```

#### Enumerating Parton Flavors

```python
print("All PartonFlavor enum values:")
for name, flavor in pdfxtmd.PartonFlavor.__members__.items():
    print(f"  {name}: {flavor.value}")
```

> **Full Python documentation:**  
> See [`examples/python/readme-pyversion.md`](examples/python/readme-pyversion.md) for a complete guide and advanced usage.
#### Tutorials

- **Jupyter Notebook Tutorial (run online):**  
  [Open in Google Colab](https://colab.research.google.com/drive/1C_h9oGJJzt5h3m-h6o3lAJ5dl-AVv0j3#scrollTo=l1iGviJu1eUc)

- **Local Example:**  
  See [`examples/python/python_tutorial.py`](examples/python/python_tutorial.py)
---

## Advanced Usage (C++)

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

For more comprehensive examples, see [examples/tutorial.cpp](examples/tutorial.cpp).

---

## Configuration

PDFxTMDLib uses YAML configuration files to locate PDF sets:

- **Windows**: `C:\ProgramData\PDFxTMDLib\config.yaml`
- **Linux**: `~/.PDFxTMDLib/config.yaml`

Example configuration:
```yaml
paths: "/home/user/pdfs|/usr/share/PDFxTMDLib/pdfs"
```

Multiple paths can be specified with the `|` separator. By default, the current directory and system-wide locations are included.

## Performance

PDFxTMDLib is optimized for high performance. The library includes benchmarking tools to compare PDFxTMDLib with other libraries like LHAPDF and TMDLib in the `performance_vs_LHAPDF_TMDLIB` directory. Results are shown in the article [https://arxiv.org/abs/2412.16680](https://arxiv.org/abs/2412.16680).

---

## Visualization Tools

For visualization of PDFs and TMDs, you can use the [QtPDFxTMDPlotter](https://github.com/Raminkord92/QtPDFxTMDPlotter), a Qt-based graphical tool built on top of PDFxTMDLib.

---

## Contributing

Contributions are welcome! If you are interested in contributing, please contact [raminkord92@gmail.com](mailto:raminkord92@gmail.com).

---

## License

This project is licensed under the Creative Commons Attribution 4.0 International License. For more details, see the [LICENSE](LICENSE) file.

---

## Contact

For any inquiries, please contact [raminkord92@gmail.com](mailto:raminkord92@gmail.com).
