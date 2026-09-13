# Installation

PDFxTMDLib provides a C++ library with Python and Fortran interfaces. Choose the
CMake workflow for the C++ library or the Python instructions for the Python package.

**Windows recommendation:** use vcpkg to install the required Zstandard and
oneDNN dependencies. Follow the [Windows vcpkg workflow](#windows-hybrid-builds-with-onednn-and-openmp)
for the C++ library.

## Requirements

- A C++17-compatible compiler and its build tools (for example, GCC, Clang, or MSVC).
- CMake 3.15 or newer for the commands below.
- Git if cloning the repository.
- For DPD builds (`ENABLE_DPD=ON`): Zstandard development headers (`zstd.h` and `zstd_errors.h`) and library (`zstd`).
  The compiler must be able to find the headers and CMake must find the library.
- For the Fortran example: a Fortran compiler on Linux or macOS. The current CMake
  configuration does not enable Fortran on Windows.

DPD support defaults to enabled and requires both Zstandard and oneDNN.
CMake configuration fails if either dependency is missing with `ENABLE_DPD=ON`. OpenMP is optional.
The basic commands below explicitly disable all DPD support; enable it when its
dependencies are available.

## Linux dependencies without vcpkg

### Debian and Ubuntu

On Debian or Ubuntu, install the C++ build tools and native dependencies using APT:

```sh
sudo apt update
sudo apt install build-essential cmake git libzstd-dev libdnnl-dev
```

`libzstd-dev` provides Zstandard headers and libraries; `libdnnl-dev` provides
oneDNN for hybrid DPD builds. See the distribution package listings for
[Zstandard](https://packages.ubuntu.com/noble/libzstd-dev) and
[oneDNN](https://packages.ubuntu.com/noble/libdnnl-dev). On Ubuntu, oneDNN is in the
Universe repository; enable that repository if APT cannot locate the package.
GCC provides OpenMP support, which CMake detects automatically.

For Python source builds, also install:

```sh
sudo apt install python3-dev python3-pip python3-venv
```

For the Fortran example, also install:

```sh
sudo apt install gfortran
```

### Fedora

```sh
sudo dnf install gcc-c++ cmake make git libzstd-devel onednn-devel
```

Fedora provides oneDNN headers and libraries in
[`onednn-devel`](https://packages.fedoraproject.org/pkgs/onednn/onednn-devel/).
For Python source builds, also install `python3-devel` and `python3-pip`; for the
Fortran example, install `gcc-gfortran`:

```sh
sudo dnf install python3-devel python3-pip
sudo dnf install gcc-gfortran
```

These commands are for Fedora. RHEL, Rocky Linux, and AlmaLinux may have different
package availability; do not assume their enabled repositories contain oneDNN.

### Arch Linux

```sh
sudo pacman -Syu --needed base-devel cmake git zstd onednn
```

Arch's [`onednn`](https://archlinux.org/packages/extra/x86_64/onednn/files/)
package includes development headers. For Python source builds, also install
`python` and `python-pip`; for the Fortran example, install `gcc-fortran`:

```sh
sudo pacman -S --needed python python-pip
sudo pacman -S --needed gcc-fortran
```

The initial `-Syu` command also upgrades the system, keeping Arch's installed
packages synchronized. Python's `venv` module is included in its Python package.

### Build after installing dependencies

Then follow the C++ build steps below, using `-DENABLE_DPD=ON` in the
configure command to include the oneDNN runtime. Do not pass a vcpkg toolchain
file. A build without DPD support can omit both the Zstandard and oneDNN packages and use
`-DENABLE_DPD=OFF`. Python source installation still has the packaging
limitations described in the Python section.

On other Linux distributions, install the equivalent C++ compiler, CMake, Git,
Zstandard development package, and oneDNN development package with the distribution's
package manager. Package names and availability vary by release and architecture.
If your repositories do not provide oneDNN, install it following the
[upstream build instructions](https://github.com/oneapi-src/oneDNN/blob/main/doc/build/build.md)
or use the vcpkg workflow below.

## Build and install the C++ library

The commands use CMake's portable configure, build, and install interfaces and can
be run in a POSIX shell, PowerShell, or Command Prompt. CMake still needs the build
tool for the selected generator, such as Ninja or Visual Studio/MSBuild.

1. Clone the repository and enter its root directory:

   ```text
   git clone https://github.com/Raminkord92/PDFxTMD.git
   cd PDFxTMD
   ```

2. Configure from the repository root:

   ```text
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=installed -DENABLE_BUILDING_WRAPPERS=OFF -DENABLE_BUILDING_EXAMPLES=OFF -DENABLE_DPD=OFF
   ```

   CMake creates the build directory automatically. These commands install into
   `installed` inside the repository, avoiding a system-wide installation. To use
   another location, replace `installed` with a quoted absolute path at configure
   time. Choose a writable directory unless a system installation is intended. If you need system installation, remove -DCMAKE_INSTALL_PREFIX=installed option.

3. Build:

   ```text
   cmake --build build --config Release --parallel 2
   ```

   Adjust `2` to control parallel build jobs. `CMAKE_BUILD_TYPE` selects Release
   for single-configuration generators such as Ninja and Unix Makefiles;
   `--config Release` selects it for multi-configuration generators such as Visual
   Studio and Xcode. Keeping both makes the workflow usable with either kind.

4. Install the completed build:

   ```text
   cmake --install build --config Release
   ```

   This installs libraries, headers, and CMake package files under the configured
   prefix. Linux and macOS builds produce shared and static libraries; Windows
   builds produce a static library.

Run all subsequent commands from the repository root as well.

### Build options

Add these options to the configure command as needed, then build and install again:

| Option | Default | Effect |
| --- | --- | --- |
| `ENABLE_BUILDING_WRAPPERS` | `ON` | Builds the Python extension if Python interpreter and development files are found. |
| `ENABLE_BUILDING_EXAMPLES` | `ON` | Builds C++ examples and, on non-Windows systems with a Fortran compiler, the Fortran example. |
| `ENABLE_DPD` | `ON` | Builds dense and hybrid DPD support; requires Zstandard and oneDNN. |
| `ENABLE_FORMATTING` | `OFF` | Enables the project's formatting integration. |
| `ENABLE_DOCUMENTATION` | `OFF` | Enables the project's documentation integration. |

For dependencies installed outside standard search locations, supply
`-DCMAKE_PREFIX_PATH="/absolute/path/to/dependencies"` or a toolchain file using
`-DCMAKE_TOOLCHAIN_FILE="/absolute/path/to/toolchain.cmake"`. Zstandard's include
path and library are discovered together. CMake prefers Zstandard's package targets
and falls back to a header/library search for system installations.

## Install dependencies with vcpkg

The repository includes a `vcpkg.json` manifest with a `dpd` feature containing
Zstandard and oneDNN. CMake automatically selects this feature before toolchain
initialization when `ENABLE_DPD=ON` (the default). With `ENABLE_DPD=OFF`, these
dependencies are not required.

Install and bootstrap vcpkg using its
[setup instructions](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started).
Then configure PDFxTMD with the vcpkg CMake toolchain. Replace
`/absolute/path/to/vcpkg` with your vcpkg checkout path.

For Linux, macOS, or a portable vcpkg build:

```text
cmake -S . -B build-vcpkg -DCMAKE_TOOLCHAIN_FILE="/absolute/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=installed -DENABLE_BUILDING_WRAPPERS=OFF -DENABLE_BUILDING_EXAMPLES=OFF -DENABLE_DPD=ON
cmake --build build-vcpkg --config Release --parallel 2
cmake --install build-vcpkg --config Release
```

On 64-bit Windows with Visual Studio:

```text
cmake -S . -B build-vcpkg -A x64 -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows -DENABLE_DPD=ON -DENABLE_BUILDING_WRAPPERS=OFF -DENABLE_BUILDING_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=installed64
cmake --build build-vcpkg --config Release --parallel 2
cmake --install build-vcpkg --config Release
```

## Use as a CMake dependency with FetchContent

As an alternative to installing the C++ library separately, use
[CMake FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)
to download PDFxTMD and build it as part of your application. Git and the native
requirements above are still needed; this does not fetch Zstandard or oneDNN when DPD support is enabled.

Add the following to your application's `CMakeLists.txt`, replacing
`<commit-hash>` with the full Git commit hash you want to use. Pin a revision that
includes the subproject support described below.

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApplication LANGUAGES CXX)

include(FetchContent)

# Configure PDFxTMD before adding it to this build.
set(ENABLE_DPD OFF CACHE BOOL "Enable DPD support")

FetchContent_Declare(
    pdfxtmd
    GIT_REPOSITORY https://github.com/Raminkord92/PDFxTMD.git
    GIT_TAG <commit-hash>
)
FetchContent_MakeAvailable(pdfxtmd)

add_executable(my_app main.cpp)
target_compile_features(my_app PRIVATE cxx_std_17)
target_link_libraries(my_app PRIVATE PDFxTMDLib)
```

Create `main.cpp` for your application alongside that file, then run from your
application's root:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target my_app --parallel 2
```

## Python wrapper

Use `python` below for your chosen interpreter; substitute `python3` or `py` if
that is how you invoke it. Prefer a virtual environment and use that environment's
interpreter consistently for pip and verification.

### Native dependencies

Python DPD support uses the same native implementation as the C++ library. With
DPDs enabled (the default), install the Zstandard and oneDNN development packages
before building from source. OpenMP is optional. The platform-specific dependency
commands are described above in this guide.

For a vcpkg build, pass the vcpkg toolchain through `CMAKE_ARGS`, for example:

```text
CMAKE_ARGS='-DCMAKE_TOOLCHAIN_FILE=/absolute/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake' python -m pip install .
```

On Windows PowerShell, the equivalent is:

```powershell
$env:CMAKE_ARGS='-DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows'
python -m pip install .
```

### Install from PyPI

```text
python -m pip install pdfxtmd
python -c "import pdfxtmd; print(pdfxtmd.__file__); print('DPD:', pdfxtmd.__has_dpd__)"
```

A compatible wheel avoids compiling locally. Wheel availability depends on the
Python version, operating system, and architecture. If pip selects a source
archive, the native dependencies above are required.

### Install from source

The Python build is driven by the same CMake targets as the C++ library. This is
important for DPD support: the extension no longer links an installed
`libPDFxTMDLib.a` by hand. CMake carries the Zstandard, oneDNN, and optional OpenMP
link requirements into the final Python extension.

With DPD support enabled (default):

```text
python -m pip install .
python -c "import pdfxtmd; print(pdfxtmd.__has_dpd__)"
```

To build a Python package without DPD support, set `PDFXTMD_ENABLE_DPD=OFF`:

```text
PDFXTMD_ENABLE_DPD=OFF python -m pip install .
```

On Windows PowerShell:

```powershell
$env:PDFXTMD_ENABLE_DPD='OFF'
python -m pip install .
```

When DPD support is disabled, `pdfxtmd.__has_dpd__` is `False`. The concrete
`CollinearDPD` binding is omitted and `GenericCDPDFactory.mkCDPD()` reports that
DPD support is disabled.

### DPD usage from Python

The recommended entry point is `GenericCDPDFactory`. It supports both dense
`PDFxTMD-DPDB1` sets and hybrid `PDFxTMD-DPDH1` sets through the same `ICDPD`
interface:

```python
import numpy as np
import pdfxtmd

factory = pdfxtmd.GenericCDPDFactory()
dpd = factory.mkCDPD("MSTW2008lo68cl_GSDPDF_PDFxTMD", 0)

value = dpd.dpd(
    pdfxtmd.PartonFlavor.g,
    pdfxtmd.PartonFlavor.g,
    1.0e-2, 100.0,
    2.0e-2, 400.0,
)
print(value)

x1 = np.array([1.0e-3, 1.0e-2, 5.0e-2])
x2 = np.array([2.0e-3, 2.0e-2, 1.0e-1])
mu1_2 = np.full_like(x1, 100.0)
mu2_2 = np.full_like(x2, 400.0)

values = dpd.dpd_batch(
    pdfxtmd.PartonFlavor.g,
    pdfxtmd.PartonFlavor.g,
    x1, mu1_2, x2, mu2_2,
)
print(values)
```

The Python interface handles the physical DPD support condition automatically:
for `x1 + x2 >= 1`, scalar and batch evaluations return zero without calling the
underlying DPD implementation.

### Build the Python extension directly with CMake

For development, the extension can still be built directly with CMake:

```text
cmake -S . -B build-python -DCMAKE_BUILD_TYPE=Release -DENABLE_BUILDING_WRAPPERS=ON -DENABLE_BUILDING_EXAMPLES=OFF -DENABLE_DPD=ON
cmake --build build-python --config Release --target pdfxtmd --parallel 2
```

The pip workflow is preferred for installation because setuptools places the
built extension into the active Python environment. Direct CMake builds leave the
module in the CMake build tree.

## Fortran interface

On Linux and macOS, CMake detects an available Fortran compiler. With
`-DENABLE_BUILDING_EXAMPLES=ON`, the build compiles
`wrappers/fortran/FactoryInterface_mod.f90` together with the Fortran example into
`test_factory`. This is controlled by the examples option, not the wrappers option.

There is no separate installation rule for the Fortran module or its source.
For an external Fortran project, compile the interface source with your own Fortran
compiler and link against PDFxTMDLib and its native dependencies. See the
[Fortran example](examples/Fortran/example.f90). Windows Fortran builds are not
configured by this project.

For the portable command syntax, see the official
[CMake command-line reference](https://cmake.org/cmake/help/latest/manual/cmake.1.html).
