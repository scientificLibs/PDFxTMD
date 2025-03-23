#to take advantage of the perofrmance, first install the library, and then build this performance tests. 
#to install library, go to the root directory of the project: mkdir build; cd build; cmake .. -DCMAKE_BUILD_TYPE=Release; make install
#then go to the performance_vs_LHAPDF_TMDLIB directory: mkdir build; cd build; cmake .. -DCMAKE_BUILD_TYPE=Release; make;
#then run the performance tests: ./TMDLib_PDFxTMDmkTMD; ./PDFxTMDTMDPDF; ./LHAPDF_PDFxTMDmkCPDF; ./LHAPDF_PDFxTMDmkCPDFSingle; ./PDFxTMDCollinearPDFSingle; ./PDFxTMDCollinearPDF
cmake_minimum_required(VERSION 3.16)
project(test)
find_package(PDFxTMDLib REQUIRED)

###################TMD#######################################
add_executable(TMDLib_PDFxTMDmkTMD TMDLib_PDFxTMDmkTMD.cpp)
target_include_directories(TMDLib_PDFxTMDmkTMD PUBLIC /usr/local/include/tmdlib)
target_link_directories(TMDLib_PDFxTMDmkTMD PUBLIC /usr/local/lib)
target_link_libraries(TMDLib_PDFxTMDmkTMD PRIVATE -lTMDlib -lLHAPDF -lgsl -lgslcblas -lm -lapfelxx PDFxTMD::PDFxTMDLib) 

add_executable(PDFxTMDTMDPDF PDFxTMDTMDPDF.cpp)
target_include_directories(PDFxTMDTMDPDF PUBLIC /usr/local/include/tmdlib)
target_link_directories(PDFxTMDTMDPDF PUBLIC /usr/local/lib)
target_link_libraries(PDFxTMDTMDPDF PRIVATE -lTMDlib -lLHAPDF -lgsl -lgslcblas -lm -lapfelxx PDFxTMD::PDFxTMDLib) 
######################COLLINEAR######################################
add_executable(LHAPDF_PDFxTMDmkCPDF LHAPDF_PDFxTMDmkCPDF.cpp)
target_include_directories(LHAPDF_PDFxTMDmkCPDF PUBLIC /usr/local/include/LHAPDF)
target_link_directories(LHAPDF_PDFxTMDmkCPDF PUBLIC /usr/local/lib)
target_link_libraries(LHAPDF_PDFxTMDmkCPDF PRIVATE -lLHAPDF PDFxTMD::PDFxTMDLib)

add_executable(LHAPDF_PDFxTMDmkCPDFSingle LHAPDF_PDFxTMDmkCPDFSingle.cpp)
target_include_directories(LHAPDF_PDFxTMDmkCPDFSingle PUBLIC /usr/local/include/LHAPDF)
target_link_directories(LHAPDF_PDFxTMDmkCPDFSingle PUBLIC /usr/local/lib)
target_link_libraries(LHAPDF_PDFxTMDmkCPDFSingle PRIVATE -lLHAPDF PDFxTMD::PDFxTMDLib)


add_executable(PDFxTMDCollinearPDFSingle PDFxTMDCollinearPDFSingle.cpp)
target_include_directories(PDFxTMDCollinearPDFSingle PUBLIC /usr/local/include/LHAPDF)
target_link_directories(PDFxTMDCollinearPDFSingle PUBLIC /usr/local/lib)
target_link_libraries(PDFxTMDCollinearPDFSingle PRIVATE -lLHAPDF PDFxTMD::PDFxTMDLib)


add_executable(PDFxTMDCollinearPDF PDFxTMDCollinearPDF.cpp)
target_include_directories(PDFxTMDCollinearPDF PUBLIC /usr/local/include/LHAPDF)
target_link_directories(PDFxTMDCollinearPDF PUBLIC /usr/local/lib)
target_link_libraries(PDFxTMDCollinearPDF PRIVATE -lLHAPDF PDFxTMD::PDFxTMDLib)
