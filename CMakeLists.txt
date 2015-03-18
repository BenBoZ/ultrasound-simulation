project(ultrasound-simulation)
 
cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
 
include_directories ("${PROJECT_SOURCE_DIR}/common")
include_directories ("${PROJECT_SOURCE_DIR}/rfData")

add_executable(createPhantom   common/phantom.cpp create/createphantom.cpp)
add_executable(compressPhantom common/phantom.cpp compress/compressphantom.cpp)
add_executable(rfDataProgram   common/phantom.cpp rfData/rf_data.cpp rfData/pressureField.cpp rfData/util.cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(COPY
     ${CMAKE_CURRENT_SOURCE_DIR}/matlab_scripts/writeBscFile.m
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

file(COPY
     ${CMAKE_CURRENT_SOURCE_DIR}/matlab_scripts/binary2matrix.m
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

file(GLOB example_files "${CMAKE_CURRENT_SOURCE_DIR}/example/*")

foreach(file ${example_files})

    # Copy each file to bld dir
    file(COPY
        "${file}"
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

endforeach()

SET(CMAKE_CXX_FLAGS "-O3")
