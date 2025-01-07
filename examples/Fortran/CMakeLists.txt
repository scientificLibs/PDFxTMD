cmake_minimum_required(VERSION 3.10)
project(Examples LANGUAGES Fortran)
# Create the Fortran example executable
add_executable(test_factory ../../wrappers/fortran/FactoryInterface.f90
../../wrappers/fortran/FactoryInterface_mod.f90  # Add the module file first
example.f90)

# Link the Fortran executable with the PDFxTMDLib library
target_link_libraries(test_factory PRIVATE PDFxTMDLib)

# Include directories for the Fortran example
target_include_directories(test_factory PRIVATE 
    ${CMAKE_SOURCE_DIR}/include    # Adjust this path to where your library headers are
)