find_package(pybind11)
if (pybind11_FOUND)
    pybind11_add_module(pdfxtmd pyBindings.cpp)
    target_link_libraries(pdfxtmd PRIVATE ${PROJECT_NAME})
    set(PYTHON_SITE_PACKAGES 
            "python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages")

    message(STATUS "Applying custom optimization flags for Release build.  ${CMAKE_INSTALL_LIBDIR}/${PYTHON_SITE_PACKAGES}")

    install(TARGETS pdfxtmd
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PYTHON_SITE_PACKAGES})

else ()
    message(STATUS "pybind11 not found, skipping pdfxtmd target build")
endif()
