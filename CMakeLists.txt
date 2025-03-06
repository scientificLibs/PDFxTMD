cmake_minimum_required(VERSION 3.22)
include(cmake/version.cmake)
include(CheckLanguage)
project(PDFxTMDLib VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} LANGUAGES CXX)
add_definitions(-DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_definitions(-DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_definitions(-DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH})
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

check_language(Fortran)
if (NOT WIN32)  
if(CMAKE_Fortran_COMPILER)
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


set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include_directories(./include)
include_directories(./lib3)
find_path(SYSTEM_INCLUDE_DIR gsl/gsl_interp.h)
find_path(SYSTEM_INCLUDE_DIR gsl/gsl_interp2d.h)
include_directories(${SYSTEM_INCLUDE_DIR})
find_package(GSL REQUIRED)
find_package(CURL REQUIRED)
find_package(LibArchive REQUIRED)
set(GSL_SOURCES 
        src/Common/GSL/common.cpp
        src/Implementation/Interpolator/Collinear/GSL/CGSLBicubic.cpp
        src/Implementation/Interpolator/Collinear/GSL/CGSLBilinear.cpp
        src/Implementation/Coupling/Interpolation/YamlCouplingInterp.cpp
    )


set(ALL_SOURCES
src/Common/YamlInfoReader.cpp 
src/Common/PartonUtils.cpp 
${GSL_SOURCES}
src/Implementation/Interpolator/Collinear/CBilinearInterpolator.cpp
src/Implementation/Interpolator/TMD/TTrilinearInterpolator.cpp
src/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.cpp
src/Implementation/Reader/TMD/TDefaultAllFlavorReader.cpp
src/Factory.cpp
src/MissingPDFSetHandler/PDFSetDownloadHandler.cpp
src/MissingPDFSetHandler/PathSelectionCommand.cpp
src/MissingPDFSetHandler/RepoSelectionCommand.cpp
src/MissingPDFSetHandler/DownloadUrlCommand.cpp
src/MissingPDFSetHandler/ArchiveExtractorCommand.cpp
src/Implementation/Coupling/Interpolation/YamlCouplingInterp.cpp
src/FortranFactoryWrapper.cpp
src/Common/FileUtils.cpp
src/Common/EnvUtils.cpp
)
add_library(PDFxTMDLib SHARED  ${ALL_SOURCES})

# Add static library
add_library(PDFxTMDLibStatic STATIC  ${ALL_SOURCES})

if(UNIX)
    set(DEFAULT_ENV_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/PDFxTMDLib")
elseif(WIN32)
    set(DEFAULT_ENV_PATH "C:/ProgramData/PDFxTMDLib")
endif()
target_compile_definitions(PDFxTMDLib PRIVATE DEFAULT_ENV_PATH="${DEFAULT_ENV_PATH}")
target_compile_definitions(PDFxTMDLibStatic PRIVATE DEFAULT_ENV_PATH="${DEFAULT_ENV_PATH}")
target_link_libraries(PDFxTMDLib PRIVATE 
        GSL::gsl 
        GSL::gslcblas 
        ${CURL_LIBRARIES}
        ${LibArchive_LIBRARIES}
    )
target_link_libraries(PDFxTMDLibStatic PRIVATE 
        GSL::gsl 
        GSL::gslcblas 
        ${CURL_LIBRARIES}
        ${LibArchive_LIBRARIES}
    )

target_include_directories(PDFxTMDLib 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/PDFxTMDLib>
)
add_subdirectory(wrappers)
include(Install)
if(UNIX)
include(Format)
Format(PDFxTMDLib include/)
Format(PDFxTMDLib src/)
include(Doxygen)
Doxygen(PDFxTMDLib include/)
endif()
add_subdirectory(examples)