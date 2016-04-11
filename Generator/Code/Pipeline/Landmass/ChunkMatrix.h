#pragma once

#include <Base.h>
#include "Voronoi.h"

namespace generator {
namespace landmass {

struct Chunk;

struct ChunkMatrix {
    ChunkMatrix() = default;
    ChunkMatrix(const ChunkMatrix&) = delete;
    ChunkMatrix(U32 detail, U8 tileSize) { create(detail, tileSize); }

    ~ChunkMatrix();

    void create(U32 detail, U8 tileSize);

    /// Returns the tile that contains the provided global position.
    /// The tile may be created if it doesn't exist.
    Chunk& getChunk(I32 x, I32 y);

    bool isEmpty() const { return (width | height) == 0; }

private:
    /// Resizes the matrix to include the provided position.
    void resize(I32 x, I32 y);

    I32 tileIndex(I32 position) const {
        return position >> tileSize;
    }

    U32 indexInTile(I32 position) const {
        return position & ((I32(1) << tileSize) - 1);
    }

    Chunk** tiles = nullptr;
    I32 x = 0;
    I32 y = 0;
    U16 width = 0;
    U16 height = 0;
    U8 tileSize = 0;
    U8 baseDetail = 0;
};

}} // namespace generator::landmass
