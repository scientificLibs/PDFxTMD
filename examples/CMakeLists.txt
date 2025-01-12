cmake_minimum_required(VERSION 3.10)

# Create the example executable
add_executable(pdf_example main.cpp)

# Link against your PDFxTMDLib library
target_link_libraries(pdf_example PRIVATE PDFxTMDLibStatic  ${CURL_LIBRARIES})

# Include directories if needed
target_include_directories(pdf_example PRIVATE 
    ${CMAKE_SOURCE_DIR}/include    # Adjust this path to where your library headers are
) 