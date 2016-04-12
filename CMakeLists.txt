cmake_minimum_required(VERSION 3.3)
project(Generator C CXX ASM)

if(WIN32)
set(BIN_DIR "Win32")
elseif(APPLE)
set(BIN_DIR "OSX")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -msse4.1")

include_directories(../../Tritium/Code/Core)
include_directories(../../Tritium/Code)

add_subdirectory(Generator/Code)
add_subdirectory(landmass/src)
add_subdirectory(Tritium/Code/Tritium)