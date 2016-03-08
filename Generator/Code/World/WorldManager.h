
#ifndef GENERATOR_WORLDMANAGER_H
#define GENERATOR_WORLDMANAGER_H

#include "../Pipeline/Pipeline.h"
#include "../Pipeline/Voxel.h"

namespace generator {

struct WorldPosition {
    float x, y;
};

struct Region {
    Chunk** chunks = nullptr;
};

struct WorldManager {
    WorldManager(Size regionSize, Size chunkSize, Size chunkHeight);
    Chunk& at(Int x, Int y, Pipeline& pipeline);

private:
    Region& regionAt(Int x, Int y);
    void resize(Int x, Int y);

    Int regionIndex(Int position) const {
        return position >> regionSize;
    }

    Size indexInRegion(Int position) const {
        return position & ((Int(1) << regionSize) - 1);
    }

    /// A rectangle of region pointers.
    Region* regions = nullptr;

    /// The current region area.
    I32 x = 0;
    I32 y = 0;
    U16 width = 0;
    U16 height = 0;

    /// The number of chunks in a region, as a power of 2.
    const U8 regionSize;

    /// The size of each chunk, as a power of 2.
    const U8 chunkSize;

    /// The maximum height of a chunk, as a power of 2.
    const U8 chunkHeight;
};

} // namespace generator

#endif // GENERATOR_WORLDMANAGER_H
