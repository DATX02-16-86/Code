cmake_minimum_required(VERSION 3.3)
project(Generator)

if(WIN32)
set(BIN_DIR "Win32")
elseif(APPLE)
set(BIN_DIR "OSX")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -msse4.1")

include_directories(../../Tritium/Code/Core)
include_directories(../../Tritium/Code)

set(SOURCE_FILES Pipeline/Pipeline.h Pipeline/Biome/BiomeStage.h Pipeline/Feature/FeatureStage.h Pipeline/Height/HeightStage.h Pipeline/Landmass/LandmassStage.h Pipeline/Structure/StructureStage.h Pipeline/Landmass/LandmassStage.cpp Pipeline/Voxel.h Pipeline/Matrix.h Pipeline/Generator.h Pipeline/Height/HeightStage.cpp Pipeline/Biome/BiomeStage.cpp Pipeline/Generator.cpp Pipeline/Voxel.cpp Pipeline/Pipeline.cpp Pipeline/Matrix.cpp World/World.h)
add_library(Generator ${SOURCE_FILES})