cmake_minimum_required(VERSION 3.3)
project(Demo)

if(WIN32)
set(BIN_DIR "Win32")
elseif(APPLE)
set(BIN_DIR "OSX")
else()
set(BIN_DIR "Linux")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")

include_directories(../Libraries/Tritium/Code/Tritium/Core)
include_directories(../Libraries/Tritium/Code/Tritium)
include_directories(../Libraries)
include_directories(../Generator/Code)

link_directories(../Libraries/Tritium/Bin/Release)

add_executable(Demo Demo.cpp)
target_link_libraries(Demo Generator TritiumCore ${OTHER_LIBS})