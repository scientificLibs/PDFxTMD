cmake_minimum_required(VERSION 3.16)

# Project declaration
include(cmake/version.cmake)
project(PDFInterpLib VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} LANGUAGES CXX)
add_definitions(-DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_definitions(-DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_definitions(-DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH})
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include GNUInstallDirs for standardized install directories
include(GNUInstallDirs)

# Optimization flags based on build type
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

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include_directories(lib3)
add_subdirectory(lib3/yaml-cpp-src)

find_package(GSL QUIET)

if(GSL_FOUND)
    message(STATUS "GSL is available!")
    set(GSL_SOURCES 
        src/Common/GSL/common.cpp
        src/Implementation/Interpolator/Collinear/GSL/CGSLBicubic.cpp
        src/Implementation/Interpolator/Collinear/GSL/CGSLBilinear.cpp
    )
else()
    message(STATUS "GSL is not available! Skipping GSL-dependent sources.")
    set(GSL_SOURCES "")
endif()

add_library(PDFxTMDLib STATIC  
    src/Common/YamlInfoReader.cpp 
    src/Common/PartonUtils.cpp 
    ${GSL_SOURCES}
    src/Implementation/Interpolator/Collinear/CBilinearInterpolator.cpp
    src/Implementation/Interpolator/TMD//TTrilinearInterpolator.cpp
    src/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.cpp
    src/Implementation/Reader/TMD/TDefaultAllFlavorReader.cpp
    src/Factory.cpp
    src/MissingPDFSetHandler/PDFSetDownloadHandler.cpp
    src/MissingPDFSetHandler/PathSelectionCommand.cpp
    src/MissingPDFSetHandler/RepoSelectionCommand.cpp
    src/MissingPDFSetHandler/DownloadUrlCommand.cpp
    src/MissingPDFSetHandler/ArchiveExtractorCommand.cpp
    src/Common/FileUtils.cpp
    src/Common/EnvUtils.cpp
)

# Find the cURL package
find_package(CURL REQUIRED)

# Add this near other find_package commands
find_package(LibArchive REQUIRED)

if(GSL_FOUND)
    target_compile_definitions(PDFxTMDLib PRIVATE USE_GSL)
    target_link_libraries(PDFxTMDLib PRIVATE 
        yaml-cpp::yaml-cpp 
        GSL::gsl 
        GSL::gslcblas 
        ${CURL_LIBRARIES}
        ${LibArchive_LIBRARIES}
    )
else()
    target_link_libraries(PDFxTMDLib PRIVATE 
        yaml-cpp::yaml-cpp 
        ${CURL_LIBRARIES}
        ${LibArchive_LIBRARIES}
    )
endif()



include(Doxygen)
Doxygen(PDFxTMDLib include)

target_include_directories(PDFxTMDLib 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/PDFxTMDLib>
)
include(Install)
include(Format)
Format(PDFxTMDLib include/)
Format(PDFxTMDLib src/)

add_subdirectory(examples)