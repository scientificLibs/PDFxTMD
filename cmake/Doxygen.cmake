function(Doxygen project_name)
  set(PDFXTMD_DOXYGEN_HTML_OUTPUT "${CMAKE_BINARY_DIR}/docs" CACHE PATH
      "Directory for generated PDFxTMD Doxygen HTML")

  find_package(Doxygen)
  if (NOT DOXYGEN_FOUND)
    add_custom_target(doxygen COMMAND false
    COMMENT "Doxygen not found")
    return()
  endif()

  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_HTML_OUTPUT "${PDFXTMD_DOXYGEN_HTML_OUTPUT}")
  set(DOXYGEN_PROJECT_NAME ${project_name})
  
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_RECURSIVE YES)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md")
  set(DOXYGEN_FULL_PATH_NAMES NO)
  set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
  set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
  set(DOXYGEN_DISTRIBUTE_GROUP_DOD YES)
  
  set(DOXYGEN_EXAMPLE_PATH "${CMAKE_SOURCE_DIR}/examples")
  set(DOXYGEN_EXAMPLE_RECURSIVE YES)
  set(DOXYGEN_EXAMPLE_PATTERNS "*.cpp *.c *.h *.hpp *.md")
  
  set(DOXYGEN_EXCLUDE "${CMAKE_SOURCE_DIR}/include/PDFxTMDLib/external")
  set(DOXYGEN_EXCLUDE_PATTERNS "*/external/*")

  set(PDFXTMD_PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/PDFxTMDLib/Factory.h
    ${CMAKE_SOURCE_DIR}/include/PDFxTMDLib/FortranFactoryWrapper.h
    ${CMAKE_SOURCE_DIR}/include/PDFxTMDLib/GenericPDF.h
    ${CMAKE_SOURCE_DIR}/include/PDFxTMDLib/PDFSet.h
    ${CMAKE_SOURCE_DIR}/include/PDFxTMDLib/Interface
  )
  
  doxygen_add_docs(doxygen
    ${PDFXTMD_PUBLIC_HEADERS}
    ${CMAKE_SOURCE_DIR}/README.md
    ${CMAKE_SOURCE_DIR}/examples
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generate HTML documentation"
  )
endfunction()
