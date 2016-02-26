
#define CATCH_CONFIG_MAIN
#include "../Pipeline/Pipeline.h"
#include "../Pipeline/Voxel.h"
#include "../Pipeline/Height/HeightStage.h"
#include "../Pipeline/Landmass/LandmassStage.h"
#include "../Pipeline/Biome/BiomeStage.h"
#include "../Pipeline/Block.h"
#include <catch.hpp>

using namespace generator;

TEST_CASE("Chunk generation") {
    Pipeline pipeline;
    pipeline.landmassStage += std::make_unique<LandmassGenerator>();
    pipeline.heightStage += std::make_unique<HeightGenerator>();
    pipeline.biomeStage += std::make_unique<BiomeGenerator>();

    Chunk chunk(Area {0, 0, 0, 16, 16, 128, 0});
    pipeline.fillChunk(chunk);
	
	REQUIRE(chunk.at(0, 0, 0).blockType == block::solid.id);
	REQUIRE(chunk.at(0, 0, 127).blockType == block::air.id);
}