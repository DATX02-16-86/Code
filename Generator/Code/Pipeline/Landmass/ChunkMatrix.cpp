
#include "ChunkMatrix.h"
#include "Chunk.h"

namespace generator {
namespace landmass {

ChunkMatrix::~ChunkMatrix() {
    for(U32 i = 0; i < (U32)width * (U32)height; i++) {
        delete tiles[i];
    }
    free(tiles);
    tiles = nullptr;
}

void ChunkMatrix::create(U32 detail, U32 tileSize) {
    this->tileSize = tileSize;
    this->baseDetail = (U8)detail;
}

void ChunkMatrix::resize(I32 x, I32 y) {
    auto left = Tritium::Math::min(x, this->x);
    auto right = Tritium::Math::max(x + 1, this->x + width);
    auto bottom = Tritium::Math::min(y, this->y);
    auto top = Tritium::Math::max(y + 1, this->y + height);

    auto w = right - left;
    auto h = top - bottom;
    auto dx = (U32)(this->x - left);
    auto dy = (U32)(this->y - bottom);

    auto newTiles = (Chunk**)malloc(sizeof(Chunk*) * w * h);

    // Initialize new rows below the existing data.
    for(U32 row = 0; row < dy; row++) {
        for(U32 column = 0; column < w; column++) {
            *(newTiles + row * w + column) = nullptr;
        }
    }

    // Initialize the area with existing rows.
    for(U32 row = 0; row < height; row++) {
        // Initialize new values before the row.
        for(U32 column = 0; column < dx; column++) {
            *(newTiles + (dy + row) * w + column) = nullptr;
        }

        // Copy the existing row.
        memcpy(newTiles + (row + dy) * w + dx, tiles + row * width, sizeof(Chunk*) * width);

        // Initialize new values at the end of the row.
        for(U32 column = dx + width; column < w; column++) {
            *(newTiles + row * w + column) = nullptr;
        }
    }

    // Initialize new rows above the existing data.
    for(U32 row = height + dy; row < h; row++) {
        for(U32 column = 0; column < w; column++) {
            *(newTiles + row * w + column) = nullptr;
        }
    }

    // Update the matrix state.
    free(tiles);
    tiles = newTiles;
    this->x = left;
    this->y = bottom;
    width = (U16)w;
    height = (U16)h;
}

Chunk& ChunkMatrix::getChunk(I32 x, I32 y) {
    auto tileX = tileIndex(x) - this->x;
    auto tileY = tileIndex(y) - this->y;
    if(tileX < 0 || tileY < 0 || width <= tileX || height <= tileY) {
        resize(tileIndex(x), tileIndex(y));
        tileX = tileIndex(x) - this->x;
        tileY = tileIndex(y) - this->y;
    }

    auto& tile = tiles[width * tileY + tileX];
    if(!tile) {
        tile = new Chunk {x, y, tileSize};
    }
    return *tile;
}

}} // namespace generator::landmass