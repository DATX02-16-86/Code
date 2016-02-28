
#ifndef GENERATOR_WORLD_H
#define GENERATOR_WORLD_H

#include "WorldManager.h"

namespace generator {

struct ViewCallback {
    virtual void addChunk(Chunk& chunk) = 0;
    virtual void removeChunk(Chunk& chunk) = 0;
};

struct World {
    World(Size drawDistance = 6, Size regionSize = 7, Size chunkSize = 5, Size chunkHeight = 7);

    /// The pipeline used to generate this world.
    Pipeline pipeline;

    /// Updates the generated world.
    /// @param positions A list of world locations that are currently active.
    void update(WorldPosition* positions, Size count);

    /// Updates the rendering of the world for the provided viewpoints.
    /// Any missing chunks are generated around each position.
    void updateView(WorldPosition* positions, Size count, ViewCallback& callback);

private:
    void fillArea(Int x, Int y, ViewCallback& callback);
    Chunk& fetchChunk(Int x, Int y);

    /// Stores regions and their chunks.
    WorldManager manager;

    /// The draw distance from each viewport, as a number of chunks.
    const U8 drawDistance;
};

} // namespace generator

#endif //GENERATOR_WORLD_H
