from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext
import os
import sys

if sys.platform == "win32":
    if sys.maxsize > 2**32:
        install_dir = "installed64" 
    else:
        install_dir = "installed32"
else:
    install_dir = "installed"

include_dirs = [os.path.join(install_dir, "include")]

def find_linux_lib(basename):
    """
    Look for lib{basename}.a under installed/lib64 or installed/lib.
    Returns the full path to the .a or raises if not found.
    """
    for sub in ("lib64", "lib"):
        candidate = os.path.join(install_dir, sub, f"lib{basename}.a")
        if os.path.isfile(candidate):
            return candidate
    raise FileNotFoundError(
        f"static library lib{basename}.a not found under {install_dir}/lib64 or {install_dir}/lib"
    )

def find_windows_lib(basename):
    """
    Look for {basename}.lib under installed32/lib or installed64/lib based on architecture.
    """
    lib_dir = os.path.join(install_dir, "lib")
    candidate = os.path.join(lib_dir, f"{basename}.lib")
    if os.path.isfile(candidate):
        return candidate
    raise FileNotFoundError(
        f"Windows import library {basename}.lib not found under {lib_dir}"
    )

if sys.platform == "win32":
    lib_path = find_windows_lib("PDFxTMDLib")
    extra_objects = [lib_path]
    libraries = []
    extra_link_args = []
else:
    lib_path = find_linux_lib("PDFxTMDLib")
    extra_objects = [lib_path]
    libraries = []
    extra_link_args = []

ext_modules = [
    Pybind11Extension(
        "pdfxtmd",
        sources=["wrappers/python/pyBindings.cpp"],
        include_dirs=include_dirs,
        libraries=libraries,
        extra_objects=extra_objects,
        extra_link_args=extra_link_args,
        language="c++",
    ),
]

setup(
    name="pdfxtmd",
    version="0.3.9",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)