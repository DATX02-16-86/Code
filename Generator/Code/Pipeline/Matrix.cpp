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

void TiledMatrix::resize(Int x, Int y) {
    auto left = Tritium::Math::min(x, this->x);
    auto right = Tritium::Math::max(x, this->x + width);
    auto bottom = Tritium::Math::min(y, this->y);
    auto top = Tritium::Math::max(y, this->y + height);

    auto w = right - left;
    auto h = top - bottom;
    auto dx = (Size)(this->x - left);
    auto dy = (Size)(this->y - bottom);

    auto newTiles = (IdMatrix*)malloc(sizeof(IdMatrix) * w * h);

    // Initialize new rows below the existing data.
    for(Size row = 0; row < dy; row++) {
        for(Size column = 0; column < w; column++) {
            new (newTiles + row * w + column) IdMatrix;
        }
    }

    // Initialize the area with existing rows.
    for(Size row = 0; row < height; row++) {
        // Initialize new values before the row.
        for(Size column = 0; column < dx; column++) {
            new (newTiles + (dy + row) * w + column) IdMatrix;
        }

        // Copy the existing row.
        memcpy(newTiles + (row + dy) * w + dx, tiles + row * width, sizeof(IdMatrix) * width);

        // Initialize new values at the end of the row.
        for(Size column = dx + width; column < w; column++) {
            new (newTiles + row * w + column) IdMatrix;
        }
    }

    // Initialize new rows above the existing data.
    for(Size row = height + dy; row < h; row++) {
        for(Size column = 0; column < w; column++) {
            new (newTiles + row * w + column) IdMatrix;
        }
    }

    // Update the matrix state.
    free(tiles);
    tiles = newTiles;
    this->x = (I32)left;
    this->y = (I32)bottom;
    width = (U16)w;
    height = (U16)h;
}

IdMatrix& TiledMatrix::getTile(Size x, Size y) {
    auto tileX = tileIndex(x);
    auto tileY = tileIndex(y);
    if(tileX < this->x || tileY < this->y || this->x + width <= tileX || this->y + height <= tileY) {
        resize(tileX, tileY);
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
    if(tileX < this->x || tileY < this->y || this->x + width <= tileX || this->y + height <= tileY) return 0;

    return tiles[width * tileY + tileX].get(indexInTile(x), indexInTile(y), detail);
}

Float TiledMatrix::getBilinear(Size x, Size y, Size detail) const {
    // 1/(x2-x1)(y2-y1) * (q11(x2-x)(y2-y)+q21(x-x1)(y2-y)+q12(x2-x)(y-y1)+q22(x-x1)(y-y1))
    auto offset = Size(1) << baseDetail;
    auto baseX = x >> baseDetail << baseDetail;
    auto baseY = y >> baseDetail << baseDetail;
    auto bl = get(baseX, baseY, detail);
    auto tl = get(baseX, baseY + offset, detail);
    auto br = get(baseX + offset, baseY, detail);
    auto tr = get(baseX + offset, baseY + offset, detail);
    auto blFactor = bl * (baseX + offset - x) * (baseY + offset - y);
    auto brFactor = br * (x - baseX) * (baseY + offset - y);
    auto tlFactor = tl * (baseX + offset - x) * (y - baseY);
    auto trFactor = tr * (x - baseX) * (y - baseY);
    return (Float)(blFactor + brFactor + tlFactor + trFactor) / (Float)(offset * offset);
}

void TiledMatrix::set(Size x, Size y, Size detail, Size value) {
    getTile(x, y).set(indexInTile(x), indexInTile(y), detail, value);
}

} // namespace generator