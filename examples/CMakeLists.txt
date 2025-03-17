cmake_minimum_required(VERSION 3.10)
project(Examples LANGUAGES CXX)

add_executable(pdf_example main.cpp)

target_link_libraries(pdf_example PRIVATE PDFxTMDLib)
target_include_directories(pdf_example PRIVATE "../include")

if (NOT WIN32)
    if (CMAKE_Fortran_COMPILER)
        add_subdirectory(Fortran)
    endif()
else()
    message(STATUS "Fortran not found. Fortran example is not going to be compiled")
endif()
