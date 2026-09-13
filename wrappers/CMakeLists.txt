# A Python extension only needs headers and the module-development component;
# requiring Development also demands an embedding library that many
# cibuildwheel CPython installations deliberately do not provide.
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
add_subdirectory(python)
