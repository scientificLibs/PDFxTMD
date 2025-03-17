# Install headers
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PDFxTMDLib
        FILES_MATCHING PATTERN "*.h*")

# Install the shared library
install(TARGETS PDFxTMDLib
        EXPORT PDFxTMDLibTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install the static library
install(TARGETS PDFxTMDLibStatic
        EXPORT PDFxTMDLibTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Create and install CMake config files
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PDFxTMDLibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PDFxTMDLibConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PDFxTMDLib
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/PDFxTMDLibConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install the config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/PDFxTMDLibConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PDFxTMDLibConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PDFxTMDLib
)

# Install the export targets
install(EXPORT PDFxTMDLibTargets
    FILE PDFxTMDLibTargets.cmake
    NAMESPACE PDFxTMDLib::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PDFxTMDLib
) 