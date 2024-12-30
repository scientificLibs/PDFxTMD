
# Install Targets for yaml-cpp
install(TARGETS yaml-cpp EXPORT PDFxTMDLibTargets)

# Install PDFLib library
install(TARGETS PDFxTMDLib 
        EXPORT PDFxTMDLibTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install header files
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PDFxTMDLib)

# Export targets for package config
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
include(CPack)

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})