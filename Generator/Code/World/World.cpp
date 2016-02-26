
#include "World.h"

namespace generator {

World::World(Size drawDistance, Size regionSize, Size chunkSize, Size chunkHeight):
    manager(regionSize, chunkSize, chunkHeight), drawDistance((U8)drawDistance) {}

void World::update(std::initializer_list<WorldPosition> positions) {
    // TODO: Update blocks and stuff.
}

void World::updateView(std::initializer_list<WorldPosition> positions, ViewCallback callback) {
    for(auto pos: positions) {
        fillArea(Tritium::Math::roundInt(pos.x), Tritium::Math::roundInt(pos.y), callback);
    }
}

void World::fillArea(Int x, Int y, ViewCallback callback) {
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
