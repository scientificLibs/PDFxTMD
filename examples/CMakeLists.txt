cmake_minimum_required(VERSION 3.10)
project(Examples LANGUAGES CXX)

add_executable(pdf_example main.cpp)

target_link_libraries(pdf_example PRIVATE PDFxTMDLibStatic  ${CURL_LIBRARIES})

target_include_directories(pdf_example PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)
if (CMAKE_Fortran_COMPILER)
    add_subdirectory(Fortran)
else()
    message(WARNING "Fortran not found. Fortran example is not going to be compiled")
endif()