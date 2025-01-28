cmake_minimum_required(VERSION 3.10)
project(Examples LANGUAGES Fortran)
add_executable(test_factory 
../../wrappers/fortran/FactoryInterface_mod.f90  example.f90)
target_link_libraries(test_factory PRIVATE PDFxTMDLib)
target_include_directories(test_factory PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
    )
