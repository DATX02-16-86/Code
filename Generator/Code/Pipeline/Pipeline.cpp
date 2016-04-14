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
    // Create a segment larger than the chunk size, which allows biomes to loop up metadata outside of the chunk area.
    auto segment = toSegment(chunk.area);
    segment.x -= chunk.area.width;
    segment.y -= chunk.area.height;
    segment.width += chunk.area.width * 2;
    segment.height += chunk.area.height * 2;

    // Generate base data around the chunk location.
    heightStage.generate(segment, *this);
    biomeStage.generate(segment, *this);

    // Generate the terrain for this chunk.
    if(auto biomes = data.get(Biomes)) {
        auto id = (BiomeId)biomes->get(chunk.area.x, chunk.area.y, 0);
        auto biome = findBiome(id);
        biome(chunk, *this);
    }
}

} // namespace generator