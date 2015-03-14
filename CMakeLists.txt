project(ultrasound-simulation)
 
cmake_minimum_required(VERSION 2.8)
 
include_directories ("${PROJECT_SOURCE_DIR}/common")
include_directories ("${PROJECT_SOURCE_DIR}/rfData")

add_executable(createPhantom   common/phantom.cpp create/createphantom.cpp)
add_executable(compressPhantom common/phantom.cpp compress/compressphantom.cpp)
add_executable(rfDataProgram   common/phantom.cpp rfData/rf_data.cpp rfData/pressureField.cpp rfData/util.cpp)

SET(CMAKE_CXX_FLAGS "-O3")
