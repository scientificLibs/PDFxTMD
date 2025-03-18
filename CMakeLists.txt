cmake_minimum_required(VERSION 3.30)
include(cmake/version.cmake)
include(CheckLanguage)
project(PDFxTMDLib VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} LANGUAGES CXX)

option(ENABLE_FORMATTING "Enable code formatting" OFF)
option(ENABLE_DOCUMENTATION "Enable documentation generation" OFF)
option(ENABLE_BUILDING_WRAPPERS "Enable building wrappers" OFF)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (NOT WIN32)
    check_language(Fortran)
    if (CMAKE_Fortran_COMPILER)
        enable_language(Fortran OPTIONAL)
    endif()
endif()

include(GNUInstallDirs)
if(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "Applying custom optimization flags for Release build.")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -ffast-math -march=native -funroll-loops")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=auto")
        else()
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
        endif()
    else()
        message(WARNING "Custom optimization flags not defined for compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

option(PDFxTMDLib_STATIC "PDFxTMDLib static type library" ON)

set(PDFxTMDLib_Type SHARED)
if (PDFxTMDLib_STATIC)
    set(PDFxTMDLib_Type STATIC)
endif()


set(ALL_SOURCES
    src/Common/YamlInfoReader.cpp 
    src/Common/PartonUtils.cpp 
    src/Implementation/Interpolator/Collinear/CLHAPDFBilinearInterpolator.cpp
    src/Implementation/Interpolator/Collinear/CLHAPDFBicubicInterpolator.cpp
    src/Implementation/Interpolator/TMD/TTrilinearInterpolator.cpp
    src/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.cpp
    src/Implementation/Reader/TMD/TDefaultAllFlavorReader.cpp
    src/Factory.cpp
    src/Implementation/Coupling/Interpolation/YamlCouplingInterp.cpp
    src/FortranFactoryWrapper.cpp
    src/Common/FileUtils.cpp
    src/Common/AllFlavorsShape.cpp
)

add_library(${PROJECT_NAME} ${PDFxTMDLib_Type})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
target_sources(${PROJECT_NAME} PRIVATE  ${ALL_SOURCES})
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)
if(UNIX)
    set(DEFAULT_ENV_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/PDFxTMDLib")
elseif(WIN32)
    set(DEFAULT_ENV_PATH "C:/ProgramData/PDFxTMDLib")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    PROJECT_VERSION="${PROJECT_VERSION}"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE DEFAULT_ENV_PATH="${DEFAULT_ENV_PATH}")
if(ENABLE_BUILDING_WRAPPERS)
    add_subdirectory(wrappers)
endif()
include(cmake/Install.cmake)


if(ENABLE_FORMATTING)
    include(cmake/Format.cmake)
    Format(${PROJECT_NAME} include/)
    Format(${PROJECT_NAME} src/)
endif()
    if(ENABLE_DOCUMENTATION)
        include(cmake/Doxygen.cmake)
        Doxygen(${PROJECT_NAME} include/)
    endif()
add_subdirectory(examples)
