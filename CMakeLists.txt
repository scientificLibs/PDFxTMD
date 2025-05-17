cmake_minimum_required(VERSION 3.22)
include(cmake/version.cmake)
include(CheckLanguage)
project(PDFxTMDLib VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} LANGUAGES CXX)

option(ENABLE_FORMATTING "Enable code formatting" OFF)
option(ENABLE_DOCUMENTATION "Enable documentation generation" OFF)
option(ENABLE_BUILDING_WRAPPERS "Enable building wrappers" OFF)
option(ENABLE_BUILDING_EXAMPLES "Enable building examples" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (NOT WIN32)
    check_language(Fortran)
    if (CMAKE_Fortran_COMPILER)
        enable_language(Fortran OPTIONAL)
    endif()
endif()

include(GNUInstallDirs)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _processor)
set(IS_ARM64 FALSE)
if(_processor MATCHES "^(aarch64|arm64)")
    set(IS_ARM64 TRUE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(IS_ARM64)
        add_compile_options(-march=armv8-a+simd)
    else()
        add_compile_options(-mavx2 -mfma)
    endif()
    add_compile_options(
        "$<$<CONFIG:Release>:-O3>"
        "$<$<CONFIG:Debug>:-O0;-g>"
    )
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
        add_compile_options("$<$<CONFIG:Release>:-flto=auto>")
    else()
        add_compile_options("$<$<CONFIG:Release>:-flto>")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(NOT IS_ARM64)
        add_compile_options(/arch:AVX2)
    endif()
else()
    message(WARNING "Compiler ${CMAKE_CXX_COMPILER_ID} not fully optimized. Add SIMD flags if needed.")
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

if(WIN32)
    add_library(${PROJECT_NAME} STATIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
    target_sources(${PROJECT_NAME} PRIVATE ${ALL_SOURCES})
    target_include_directories(${PROJECT_NAME} 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
else()
    add_library(${PROJECT_NAME} SHARED ${ALL_SOURCES})
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
    target_include_directories(${PROJECT_NAME} 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    add_library(${PROJECT_NAME}_static STATIC ${ALL_SOURCES})
    set_target_properties(${PROJECT_NAME}_static PROPERTIES 
        OUTPUT_NAME ${PROJECT_NAME}
        DEBUG_POSTFIX "d"
    )
    target_include_directories(${PROJECT_NAME}_static 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endif()

if(WIN32)
    set(DEFAULT_ENV_PATH "C:/ProgramData/PDFxTMDLib")
else()
    set(DEFAULT_ENV_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/PDFxTMDLib")
endif()

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        PROJECT_VERSION="${PROJECT_VERSION}"
        DEFAULT_ENV_PATH="${DEFAULT_ENV_PATH}"
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        PROJECT_VERSION="${PROJECT_VERSION}"
        DEFAULT_ENV_PATH="${DEFAULT_ENV_PATH}"
    )
    target_compile_definitions(${PROJECT_NAME}_static PRIVATE 
        PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        PROJECT_VERSION="${PROJECT_VERSION}"
        DEFAULT_ENV_PATH="${DEFAULT_ENV_PATH}"
    )
endif()

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

if(ENABLE_BUILDING_EXAMPLES)
    add_subdirectory(examples)
endif()
