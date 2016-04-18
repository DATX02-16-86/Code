#include "Pipeline.h"
#include "Voxel.h"
#include "Biome/BiomeStage.h"

namespace generator {

static Segment toSegment(Area area) {
    return Segment {area.x, area.y, (U32)area.worldWidth(), (U32)area.worldHeight(), 1.f, 0};
}

TiledMatrix* Pipeline::Data::get(StreamId stream) {
    if(stream.id >= count) return nullptr;
    if(matrices[stream.id].isEmpty()) return nullptr;
    return matrices + stream.id;
}

TiledMatrix* Pipeline::Data::getOrCreate(StreamId stream, Size detail) {
    if(stream.id >= count) resize(stream.id + 1);
    if(matrices[stream.id].isEmpty()) matrices[stream.id].create(detail, stream.itemBits, tileSize);
    return matrices + stream.id;
}

void Pipeline::Data::resize(Size count) {
    matrices = (TiledMatrix*)realloc(matrices, sizeof(TiledMatrix) * count);
    for(Size i = this->count; i < count; i++) {
        new (matrices + i) TiledMatrix;
    }
    this->count = (U32)count;
}

void Pipeline::fillChunk(Chunk& chunk) {
    auto& land = landmass.generate(chunk.area.x, chunk.area.y, seed);

    // Get the closest vertex and its direct neighbours, then calculate biome strengths for each voxel pillar.


    // Generate the terrain for this chunk.
    if(auto biomes = data.get(Biomes)) {
        auto id = (BiomeId)biomes->get(chunk.area.x, chunk.area.y, 0);
        auto biome = findBiome(id);
        biome(chunk, *this);
    }
}

} // namespace generator