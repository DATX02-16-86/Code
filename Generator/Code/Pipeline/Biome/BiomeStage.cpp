#include "BiomeStage.h"
#include <vector>
#include "../Pipeline.h"
#include "../Height/HeightStage.h"

namespace generator {

DefineStream(Biomes, 8);
	
void BiomeGenerator::generate(const Segment& segment, TiledMatrix** auxiliaries, Pipeline& pipeline) {
    auto map = pipeline.data.getOrCreate(Biomes, segment.detail);
    generate(segment, *map, auxiliaries, pipeline);
}


static std::vector<GenerateChunk> registeredBiomes;

BiomeId registerBiome(GenerateChunk generateChunk) {
    registeredBiomes.push_back(generateChunk);
    return (BiomeId)registeredBiomes.size() - 1;
}

GenerateChunk findBiome(BiomeId id) {
    return registeredBiomes[id];
}

const BiomeId DefaultBiome::id = registerBiome(DefaultBiome::fillChunk);

void DefaultBiome::fillChunk(Chunk& chunk, Pipeline& pipeline) {
    auto height = pipeline.data.get(BaseHeight);
    chunk.build([=](Voxel& current, Int x, Int y, Int z) -> Voxel {
        Size baseHeight = 0;
        if(height) baseHeight = height->get(x, y, z);

        U16 blockType = 0;
        if(z <= baseHeight) blockType = 1;

        return Voxel {blockType};
    });
}

}