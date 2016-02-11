
#ifndef GENERATOR_MATRIX_H
#define GENERATOR_MATRIX_H

#include "Base.h"
#include <Math/Math.h>
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
    static Size getItemShift(Size itemBits);
    static Size getItemMask(Size itemBits, Size itemShift);

    Size* items = nullptr;
    U32 wordsPerRow;
    U16 itemShift;
    U8 itemBits;
    U8 detail;
};

struct TiledMatrix {
    TiledMatrix() = default;
    TiledMatrix(const TiledMatrix&) = delete;
    TiledMatrix(Size detail, Size itemBits, U8 tileSize) { create(detail, itemBits, tileSize); }
    ~TiledMatrix();

    void create(Size detail, Size itemBits, U8 tileSize);

    /// Returns the value at the provided global index.
    Size get(Size x, Size y, Size detail) const;

    /// Sets the value at the provided global index.
    void set(Size x, Size y, Size detail, Size value);

    /// Returns the tile that contains the provided global position.
    /// The tile may be created if it doesn't exist.
    IdMatrix& getTile(Size x, Size y);

private:
    void resize(Size minWidth, Size minHeight);
    Size tileIndex(Size position) const {return position >> baseDetail >> tileSize;}
    Size indexInTile(Size position) const {return position >> tileSize;}

    IdMatrix* tiles = nullptr;
    U16 width = 0;
    U16 height = 0;
    U8 tileSize = 0;
    U8 itemBits = 0;
    U8 baseDetail = 0;
};

} // namespace generator

#endif //GENERATOR_MATRIX_H
