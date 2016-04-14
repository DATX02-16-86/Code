cmake_minimum_required(VERSION 3.3)
project(Generator C CXX ASM)

if(WIN32)
set(BIN_DIR "Win32")
elseif(APPLE)
set(BIN_DIR "OSX")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -msse4.1")

# Tritium include files.
include_directories(../../Libraries/Tritium/Code/Core)
include_directories(../../Libraries/Tritium/Code)

# Libraries required to link Tritium-Core.
if(APPLE)
    find_library(COCOA_LIB Cocoa)
    set(OTHER_LIBS ${COCOA_LIB})
elseif(WIN32)
    set(OTHER_LIBS TritiumWin32)
endif()

add_subdirectory(Generator/Code)
#add_subdirectory(landmass/src)
add_subdirectory(Libraries/Tritium/Code/Tritium)
add_subdirectory(Demo)