cmake_minimum_required(VERSION 3.3)
project(Landmass)

if(WIN32)

elseif(APPLE)
set(BIN_DIR "OSX")
endif()

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -msse4.1")

include_directories(../../boost/)

set(SOURCE_FILES main.cpp)
add_executable(Landmass ${SOURCE_FILES})