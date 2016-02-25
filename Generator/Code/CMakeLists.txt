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

set(SOURCE_FILES
    Pipeline/Block.cpp
    Pipeline/Block.h
    Pipeline/Generator.cpp
    Pipeline/Generator.h
    Pipeline/Geometry.cpp
    Pipeline/Geometry.h
    Pipeline/Matrix.cpp
    Pipeline/Matrix.h
    Pipeline/Pipeline.cpp
    Pipeline/Pipeline.h
    Pipeline/Voxel.cpp
    Pipeline/Voxel.h
)

set(SOURCE_FILES ${SOURCE_FILES} 
    Pipeline/Biome/BiomeStage.h
    Pipeline/Biome/BiomeStage.cpp
)

set(SOURCE_FILES ${SOURCE_FILES}
    Pipeline/Height/HeightStage.h
    Pipeline/Height/HeightStage.cpp
)

set(SOURCE_FILES ${SOURCE_FILES} 
    Pipeline/Landmass/LandmassStage.cpp
    Pipeline/Landmass/LandmassStage.h
)

set(SOURCE_FILES ${SOURCE_FILES} 
    Pipeline/Structure/StructureStage.h
    Pipeline/Feature/FeatureStage.h
)

add_library(Generator ${SOURCE_FILES})