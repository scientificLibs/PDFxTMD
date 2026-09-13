# Package targets carry Debug/Release paths, DLL definitions, and runtime dependencies.
find_package(dnnl CONFIG QUIET)
if(TARGET DNNL::dnnl)
    set(PDFxTMDDNNL_FOUND TRUE)
    if(NOT TARGET PDFxTMDDNNL::DNNL)
        add_library(PDFxTMDDNNL::DNNL INTERFACE IMPORTED)
        set_target_properties(PDFxTMDDNNL::DNNL PROPERTIES
            INTERFACE_LINK_LIBRARIES DNNL::dnnl)
    endif()
else()
    find_path(PDFXTMD_DNNL_INCLUDE_DIR NAMES dnnl.hpp oneapi/dnnl/dnnl.hpp)
    find_library(PDFXTMD_DNNL_LIBRARY NAMES dnnl)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(PDFxTMDDNNL
        REQUIRED_VARS PDFXTMD_DNNL_INCLUDE_DIR PDFXTMD_DNNL_LIBRARY)
    if(PDFxTMDDNNL_FOUND AND NOT TARGET PDFxTMDDNNL::DNNL)
        add_library(PDFxTMDDNNL::DNNL UNKNOWN IMPORTED)
        set_target_properties(PDFxTMDDNNL::DNNL PROPERTIES
            IMPORTED_LOCATION "${PDFXTMD_DNNL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${PDFXTMD_DNNL_INCLUDE_DIR}")
    endif()
    mark_as_advanced(PDFXTMD_DNNL_INCLUDE_DIR PDFXTMD_DNNL_LIBRARY)
endif()
