
#ifndef GENERATOR_MATRIX_H
#define GENERATOR_MATRIX_H

#include "Base.h"
#include <stdlib.h>

namespace generator {

struct IdMatrix {
    IdMatrix() = default;
    IdMatrix(const IdMatrix&) = delete;

    IdMatrix(Size w, Size h, Size detail, Size itemBits) {
        create(w, h, detail, itemBits);
    }

    ~IdMatrix() {
        free(items);
    }

    void create(Size w, Size h, Size detail, Size itemBits);
    Size get(Size x, Size y, Size detail) const;
    void set(Size x, Size y, Size detail, Size value);
    bool isEmpty() const {return !items;}

private:
    Size* items = nullptr;
    U32 wordsPerRow = 0;
    U8 itemShift;
    U8 itemBits;
    U8 detail;
    U8 itemsPerWord;
};

struct TiledMatrix {
    TiledMatrix() = default;
    TiledMatrix(const TiledMatrix&) = delete;
    TiledMatrix(Size detail, Size itemBits, U8 tileSize) { create(detail, itemBits, tileSize); }
    ~TiledMatrix();

    void create(Size detail, Size itemBits, U8 tileSize);

    /// Returns the value at the provided global index.
    Size get(Int x, Int y, Size detail) const;

    /// Returns a value at the provided global index.
    /// The value is interpolated bilinearly if the matrix has a higher LOD index.
    Float getBilinear(Int x, Int y, Size detail) const;

    /// Sets the value at the provided global index.
    void set(Int x, Int y, Size detail, Size value);

    /// Returns the tile that contains the provided global position.
    /// The tile may be created if it doesn't exist.
    IdMatrix& getTile(Int x, Int y);

    bool isEmpty() const {return itemBits == 0;}
private:
    /// Resizes the tileset to include the provided position.
    void resize(Int x, Int y);

    Int tileIndex(Int position) const {
        return position >> tileSize;
    }

    Size indexInTile(Int position) const {
        return position & ((Int(1) << tileSize) - 1);
    }

    IdMatrix* tiles = nullptr;
    I32 x = 0;
    I32 y = 0;
    U16 width = 0;
    U16 height = 0;
    U8 tileSize = 0;
    U8 itemBits = 0;
    U8 baseDetail = 0;
};

} // namespace generator

#endif //GENERATOR_MATRIX_H
