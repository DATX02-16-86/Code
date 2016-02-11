#include "Matrix.h"
#include <Math/Math.h>
#include <string.h>

namespace generator {

void IdMatrix::create(Size w, Size h, Size detail, Size itemBits) {
    free(items);

    wordsPerRow = (U32)(h >> getItemShift(itemBits));
    itemShift = 0;
    this->itemBits = (U8)itemBits;
    this->detail = (U8)detail;
    items = (Size*)malloc(sizeof(Size) * wordsPerRow * w);
}

Size IdMatrix::get(Size x, Size y, Size detail) const {
    auto row = x >> ((Int)detail - (Int)this->detail);
    auto offset = y >> ((Int)detail - (Int)this->detail);
    auto index = x * row + (y >> itemShift);

    auto mask = 0;
    return items[row * wordsPerRow + offset] & mask;
}

void IdMatrix::set(Size x, Size y, Size detail, Size value) {
    auto row = x >> ((Int)detail - (Int)this->detail);
    auto offset = y >> ((Int)detail - (Int)this->detail);
    auto index = 0;
    auto mask = 0;

    auto c = items[index];
    c &= ~mask;
    c |= value;
    items[index] = c;
}

Size IdMatrix::getItemShift(Size itemBits) {
    return Tritium::Math::findLastBit(sizeof(Size) * 8 / itemBits) - 1;
}

Size IdMatrix::getItemMask(Size itemBits, Size itemShift) {
    return ~Size(0) >> (sizeof(Size) * 8 - itemShift);
}

TiledMatrix::~TiledMatrix() {
    for(Size i = 0; i < (Size)width * (Size)height; i++) {
        tiles[i].~IdMatrix();
    }
    free(tiles);
    tiles = nullptr;
}

void TiledMatrix::create(Size detail, Size itemBits, U8 tileSize) {
    this->tileSize = tileSize;
    this->itemBits = (U8)itemBits;
    this->baseDetail = (U8)detail;

    for(Size i = 0; i < (Size)width * (Size)height; i++) {
        tiles[i].create(tileSize, tileSize, detail, itemBits);
    }
}

void TiledMatrix::resize(Size minWidth, Size minHeight) {
    auto w = Tritium::Math::max(width, minWidth);
    auto h = Tritium::Math::max(height, minHeight);
    auto newTiles = (IdMatrix*)malloc(sizeof(IdMatrix) * w * h);

    for(Size row = 0; row < height; row++) {
        // Copy the existing row.
        memcpy(newTiles + row * h, tiles + row * height, sizeof(IdMatrix) * width);

        // Initialize new values at the end of the row.
        for(Size column = width; column < w; column++) {
            new (newTiles + row * h + column) IdMatrix;
        }
    }

    // Initialize new rows.
    for(Size row = height; row < h; row++) {
        for(Size column = 0; column < w; column++) {
            new (newTiles + row * h + column) IdMatrix;
        }
    }

    // Update the matrix state.
    free(tiles);
    tiles = newTiles;
    width = (U16)w;
    height = (U16)h;
}

IdMatrix& TiledMatrix::getTile(Size x, Size y) {
    auto tileX = tileIndex(x);
    auto tileY = tileIndex(y);
    if(width <= tileX || height <= tileY) {
        resize(tileX + 1, tileY + 1);
    }

    auto& tile = tiles[width * tileY + tileX];
    if(tile.isEmpty()) {
        auto size = Size(1) << tileSize;
        tile.create(size, size, baseDetail, itemBits);
    }
    return tile;
}

Size TiledMatrix::get(Size x, Size y, Size detail) const {
    auto tileX = tileIndex(x);
    auto tileY = tileIndex(y);
    if(tileX >= width || tileY >= height) return 0;

    return tiles[width * tileY + tileX].get(indexInTile(x), indexInTile(y), detail);
}

void TiledMatrix::set(Size x, Size y, Size detail, Size value) {
    getTile(x, y).set(indexInTile(x), indexInTile(y), detail, value);
}

} // namespace generator