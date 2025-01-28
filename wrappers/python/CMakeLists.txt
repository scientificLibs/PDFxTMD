find_package(pybind11 REQUIRED)
pybind11_add_module(pdfxtmd pyBindings.cpp)
target_link_libraries(pdfxtmd PRIVATE PDFxTMDLibStatic)
set(PYTHON_SITE_PACKAGES 
        "python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages")


message(STATUS "Applying custom optimization flags for Release build.  ${CMAKE_INSTALL_LIBDIR}/${PYTHON_SITE_PACKAGES}")

install(TARGETS pdfxtmd
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PYTHON_SITE_PACKAGES})

