# Install PDFLib library using default CMake paths
install(TARGETS PDFxTMDLib 
        EXPORT PDFxTMDLibTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install PDFLib static library using default CMake paths
install(TARGETS PDFxTMDLibStatic
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Install header files using default CMake paths
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PDFxTMDLib)

# Install lib3 folder using default CMake paths
install(DIRECTORY lib3/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PDFxTMDLib/lib3)

# Create the directory during installation using default CMake paths
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/PDFxTMDLib)

# Create an empty file called .PDFxTMD inside the directory
file(WRITE "${CMAKE_BINARY_DIR}/.PDFxTMD" "")  # Create an empty file in the build directory
install(FILES "${CMAKE_BINARY_DIR}/.PDFxTMD" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/PDFxTMDLib)

# Install Python wrapper
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Determine Python package directory
set(PYTHON_SITE_PACKAGES 
    "lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages")

install(TARGETS pdfxtmd
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${PYTHON_SITE_PACKAGES})

# Export targets for package config using default CMake paths
install(EXPORT PDFxTMDLibTargets
        FILE PDFxTMDLibTargets.cmake
        NAMESPACE PDFxTMDLib::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PDFxTMDLib)

# Configure package config files
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/PDFxTMDLibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PDFxTMDLibConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PDFxTMDLib
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PDFxTMDLibConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PDFxTMDLib)

# Add uninstall target
configure_file(${CMAKE_SOURCE_DIR}/cmake/uninstall.cmake 
               ${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake 
               COPYONLY)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake
    COMMENT "Uninstalling the project...")

# CPack configuration
set(CPACK_PACKAGE_VENDOR "Ramin Kord")
set(CPACK_PACKAGE_CONTACT "raminkord92@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION "PDF library for Collinear PDFs and TMDs")

# Add GitHub repository link
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Raminkord92/PDFxTMD")

include(CPack)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")  # Ensure this points to a valid file
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_WIX_UPGRADE_GUID "{9674B1B2-ABC1-4B0D-9F28-510F30F8E0E2}")  # Set explicitly
