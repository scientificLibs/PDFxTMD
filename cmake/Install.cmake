if(WIN32)
    install(
        TARGETS ${PROJECT_NAME}
        EXPORT pdfxtmdlib-config
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
else()
    install(
        TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_static
        EXPORT pdfxtmdlib-config
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# Export targets separately so the package configuration can load dependencies
# before it defines PDFxTMD's imported targets.
install(
    EXPORT pdfxtmdlib-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    NAMESPACE PDFxTMD::
    FILE pdfxtmdlib-targets.cmake
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pdfxtmdlib-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pdfxtmdlib-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/pdfxtmdlib-config.cmake
          ${PROJECT_SOURCE_DIR}/cmake/FindPDFxTMDZstd.cmake
          ${PROJECT_SOURCE_DIR}/cmake/FindPDFxTMDDNNL.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Install header files using default CMake paths
message(STATUS "Items location to copy to install dir: ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}")
message(STATUS "Install dir is: ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Create PDFxTMD directory in shared folder
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})

# Packaging is only configured for standalone builds.
if(NOT CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    return()
endif()

# CPack configuration
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "Ramin Kord <raminkord92@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ramin Kord")

include(CPack)
set(CPACK_PACKAGE_NAME "PDFxTMDLib")
set(CPACK_PACKAGE_VENDOR "Ramin Kord")
set(CPACK_PACKAGE_CONTACT "raminkord92@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION "PDF library for Collinear PDFs and TMDs")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Raminkord92/PDFxTMD")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")  # Ensure this points to a valid file
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_DEBIAN_PACKAGE_NAME "PDFxTMD")
set(CPACK_WIX_UPGRADE_GUID "{9674B1B2-ABC1-4B0D-9F28-510F30F8E0E2}")  # Set explicitly
