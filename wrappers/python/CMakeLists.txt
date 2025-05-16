message(STATUS "Detecting pybind11...")
find_package(pybind11 QUIET)
if(NOT pybind11_FOUND)
  # Install pybind11 on demand
  include(FetchContent)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  message(STATUS "pybind11 not detected, fetching pybind11...")
  FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE)
  FetchContent_MakeAvailable(pybind11)
  set(PYBIND11_INCLUDE_DIRS ${pybind11_SOURCE_DIR})
endif()
pybind11_add_module(pdfxtmd pyBindings.cpp)
target_include_directories(pdfxtmd PUBLIC  ${PROJECT_SOURCE_DIR}/include )
target_link_libraries(pdfxtmd PRIVATE ${PROJECT_NAME})


