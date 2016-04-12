cmake_minimum_required(VERSION 3.3)
project(Generator)

if(WIN32)
set(BIN_DIR "Win32")
elseif(APPLE)
set(BIN_DIR "OSX")
else()
set(BIN_DIR "Linux")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -msse4.1")

include_directories(../../Libraries/Tritium/Code/Tritium/Core)
include_directories(../../Libraries/Tritium/Code/Tritium)
include_directories(../../Libraries)

link_directories(../../Libraries/Tritium/Bin/Release)

add_library(Generator
    Geometry/Geometry.cpp
    Geometry/Geometry.h

    Pipeline/Block.cpp
    Pipeline/Block.h
    Pipeline/Generator.cpp
    Pipeline/Generator.h
    Pipeline/Matrix.cpp
    Pipeline/Matrix.h
    Pipeline/Pipeline.cpp
    Pipeline/Pipeline.h
    Pipeline/Voxel.cpp
    Pipeline/Voxel.h

    Pipeline/Biome/BiomeStage.h
    Pipeline/Biome/BiomeStage.cpp
    Pipeline/Biome/Biomes.h
    Pipeline/Biome/Biomes.cpp

    Pipeline/Height/HeightStage.h
    Pipeline/Height/HeightStage.cpp

    Pipeline/Landmass/LandmassStage.cpp
    Pipeline/Landmass/LandmassStage.h
    Pipeline/Landmass/Voronoi.h
    Pipeline/Landmass/ChunkMatrix.h
    Pipeline/Landmass/ChunkMatrix.cpp
    Pipeline/Landmass/Filler.cpp
    Pipeline/Landmass/Filler.h
    Pipeline/Landmass/Chunk.cpp
    Pipeline/Landmass/Chunk.h

    World/World.h
    World/World.cpp
    World/WorldManager.h
    World/WorldManager.cpp)

add_executable(GeneratorTest Tests/Pipeline.cpp Tests/Matrix.cpp)
target_link_libraries(GeneratorTest Generator TritiumCore ${OTHER_LIBS})