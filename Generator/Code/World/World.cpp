
#include "World.h"
#include <Math/Math.h>

namespace generator {

World::World(I32 seed, Size drawDistance, Size regionSize, Size chunkSize, Size chunkHeight):
    filler(128, seed),
    manager(regionSize, chunkSize, chunkHeight),
    drawDistance((U8)drawDistance),
    pipeline(filler, seed, 1u << chunkSize, 1u << (7 - chunkSize)) {}

void World::update(WorldPosition* positions, Size count) {
    // TODO: Update blocks and stuff.
}

void World::updateView(WorldPosition* positions, Size count, ViewCallback& callback) {
    for(Size i = 0; i < count; i++) {
        fillArea(Tritium::Math::roundInt(positions[i].x), Tritium::Math::roundInt(positions[i].y), callback);
    }
}

void World::fillArea(Int x, Int y, ViewCallback& callback) {
    // TODO: Do this as a background task.
    for(Int column = x - drawDistance; column < x + drawDistance; column++) {
        for(Int row = y - drawDistance; row < y + drawDistance; row++) {
            callback.addChunk(fetchChunk(column, row));
        }
    }
}

Chunk& World::fetchChunk(Int x, Int y) {
    return manager.at(x, y, pipeline);
}

} // namespace generator
