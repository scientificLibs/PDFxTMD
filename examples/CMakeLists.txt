cmake_minimum_required(VERSION 3.10)
project(Examples LANGUAGES CXX)

add_executable(tutorial tutorial.cpp)
target_link_libraries(tutorial PRIVATE PDFxTMDLib)
target_include_directories(tutorial PRIVATE "../include")

if (NOT WIN32)
    if (CMAKE_Fortran_COMPILER)
        add_subdirectory(Fortran)
    endif()
else()
    message(STATUS "Fortran not found. Fortran example is not going to be compiled")
endif()
