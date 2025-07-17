# Installation

This document provides instructions for installing PDFxTMDLib and its wrappers.

## Requirements

-   C++17 compatible compiler (e.g., GCC, Clang, MSVC)
-   CMake 3.30 or higher
-   For Fortran wrapper: a Fortran compiler
-   For Python wrapper: Python 3.6+ and pip

---

## Option 1: Using a Package Manager (Linux)

### Ubuntu/Debian

1.  Download the `.deb` package from the [Releases](https://github.com/Raminkord92/PDFxTMD/releases) page.
2.  Install it using `dpkg`:
    ```bash
    sudo dpkg -i PDFxTMDLib-*.deb
    ```

---

## Option 2: Building from Source (All Platforms)

This is the recommended method for getting the latest version.

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/Raminkord92/PDFxTMD.git
    cd PDFxTMD
    ```

2.  **Configure with CMake:**
    Create a build directory and run CMake.
    ```bash
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    ```
    *   The Fortran wrapper will be built automatically if a Fortran compiler is found.
    *   The Python wrapper is handled separately (see below).

3.  **Build the library:**
    ```bash
    make
    ```
    On Windows with Visual Studio, you would build from the IDE or use `cmake --build . --config Release`.

4.  **Install the library:**
    ```bash
    sudo make install
    ```
    On Windows, you may need to run this with administrator privileges. This installs the C++ library and headers to a system-wide location.

---

## Wrapper Installation

### Python Wrapper

The Python wrapper can be installed via PyPI or from the source code.

#### From PyPI

This is the easiest way to get the Python wrapper. It includes pre-compiled binaries for the C++ library.
```bash
pip install pdfxtmd
```

#### From Source

If you have already built and installed the C++ library from source (as described above), you can then install the Python wrapper. From the root of the repository:
```bash
pip install .
```