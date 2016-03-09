#include <memory>
#include "Matrix.h"
#include <Math/Math.h>

namespace generator {

void IdMatrix::create(Size w, Size h, Size detail, Size itemBits) {
    free(items);

    this->itemBits = (U8)itemBits;
    this->detail = (U8)detail;
    itemsPerWord = (U8)1 << Tritium::Math::findLastBit(sizeof(Size) * 8 / itemBits);
    itemShift = Tritium::Math::findLastBit(sizeof(Size) * 8 / itemBits);

    wordsPerRow = (U32)(h >> itemShift);
    items = (Size*)malloc(sizeof(Size) * wordsPerRow * h);
}

Size IdMatrix::get(Size x, Size y, Size detail) const {
    auto row = y >> ((Int)detail - (Int)this->detail);
    auto offset = x >> ((Int)detail - (Int)this->detail);
    auto index = row * wordsPerRow + (offset >> itemShift);
    auto mask = (Size(1) << itemBits) - 1;
    auto maskOffset = offset & (itemsPerWord - 1);

    return items[index] >> (maskOffset * itemBits) & mask;
}

void IdMatrix::set(Size x, Size y, Size detail, Size value) {
    auto row = y >> ((Int)detail - (Int)this->detail);
    auto offset = x >> ((Int)detail - (Int)this->detail);
    auto index = row * wordsPerRow + (offset >> itemShift);
    auto maskOffset = offset & (itemsPerWord - 1);
    auto totalShift = maskOffset * itemBits;
    auto mask = ((Size(1) << itemBits) - 1) << totalShift;

    auto c = items[index];
    c &= ~mask;
    c |= (value << totalShift);
    items[index] = c;
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
    auto left = Tritium::Math::min((I32)x, this->x);
    auto right = Tritium::Math::max((I32)x + 1, this->x + width);
    auto bottom = Tritium::Math::min((I32)y, this->y);
    auto top = Tritium::Math::max((I32)y + 1, this->y + height);

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

IdMatrix& TiledMatrix::getTile(Int x, Int y) {
    auto tileX = tileIndex(x) - this->x;
    auto tileY = tileIndex(y) - this->y;
    if(tileX < 0 || tileY < 0 || width <= tileX || height <= tileY) {
        resize(tileX, tileY);
		tileX = tileIndex(x) - this->x;
		tileY = tileIndex(y) - this->y;
    }

    auto& tile = tiles[width * tileY + tileX];
    if(tile.isEmpty()) {
        auto size = Size(1) << tileSize;
        tile.create(size, size, baseDetail, itemBits);
    }
    return tile;
}

Size TiledMatrix::get(Int x, Int y, Size detail) const {
    auto tileX = tileIndex(x) - this->x;
    auto tileY = tileIndex(y) - this->y;
    if(tileX < 0 || tileY < 0 || width <= tileX || height <= tileY) return 0;

    return tiles[width * tileY + tileX].get(indexInTile(x), indexInTile(y), detail);
}

Float TiledMatrix::getBilinear(Int x, Int y, Size detail) const {
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

void TiledMatrix::set(Int x, Int y, Size detail, Size value) {
    getTile(x, y).set(indexInTile(x), indexInTile(y), detail, value);
}

} // namespace generator