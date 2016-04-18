#pragma once

#include <Base.h>
#include "Voronoi.h"

namespace generator {
namespace landmass {

struct Chunk;

struct ChunkMatrix {
    ChunkMatrix() = default;
    ChunkMatrix(const ChunkMatrix&) = delete;
    ChunkMatrix(U32 detail, U32 tileSize, U32 gridSize, U32 gridSpread) {
        create(detail, tileSize, gridSize, gridSpread);
    }

    ~ChunkMatrix();

    void create(U32 detail, U32 tileSize, U32 gridSize, U32 gridSpread);

    /// Returns the tile that contains the provided global position.
    /// The tile may be created if it doesn't exist.
    Chunk& getChunk(I32 x, I32 y);

    bool isEmpty() const { return (width | height) == 0; }

private:
    /// Resizes the matrix to include the provided position.
    void resize(I32 x, I32 y);

    Chunk** tiles = nullptr;
    I32 x = 0;
    I32 y = 0;
    U16 width = 0;
    U16 height = 0;
    U16 gridSize;
    U8 gridSpread;
    U8 tileSize;
};

}} // namespace generator::landmass
