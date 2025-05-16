from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext
import os
import sys

# Define paths
install_dir = "installed"
include_dirs = [os.path.join(install_dir, "include")]
library_dirs = [os.path.join(install_dir, "lib")]
library_path = os.path.join(library_dirs[0], "PDFxTMDLib.lib")

# Configure the extension module
ext_modules = [
    Pybind11Extension(
        "pdfxtmd",
        sources=["wrappers/python/pyBindings.cpp"],
        include_dirs=include_dirs,
        extra_objects=[library_path],
        extra_link_args=[],
    ),
]

# Setup configuration
setup(
    name="pdfxtmd",
    version="0.3.0",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)