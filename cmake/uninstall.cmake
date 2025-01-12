if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest")
endif()

file(READ "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt" files)
string(REPLACE "\n" ";" files "${files}")

foreach(file ${files})
    if(EXISTS "${file}")
        message(STATUS "Removing ${file}")
        file(REMOVE "${file}")
    endif()
endforeach()