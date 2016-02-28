#include "WorldManager.h"
#include <Math/Math.h>

namespace generator {

WorldManager::WorldManager(Size regionSize, Size chunkSize, Size chunkHeight):
        regionSize((U8)regionSize), chunkSize((U8)chunkSize), chunkHeight((U8)chunkHeight) {}

Region& WorldManager::regionAt(Int x, Int y) {
    auto regionX = regionIndex(x) - this->x;
    auto regionY = regionIndex(y) - this->y;
    if(regionX < 0 || regionY < 0 || width <= regionX || height <= regionY) {
        resize(regionX, regionY);
        regionX = regionIndex(x) - this->x;
        regionY = regionIndex(y) - this->y;
    }

    auto& region = regions[width * regionY + regionX];
    if(region.chunks == nullptr) {
        auto size = Size(1) << regionSize;
        region.chunks = (Chunk**)calloc(size * size, sizeof(Chunk*));
    }
    return region;
}

Chunk& WorldManager::at(Int x, Int y, Pipeline& pipeline) {
    auto& region = regionAt(x, y);
    auto index = Size(1) << (regionSize * indexInRegion(y) + indexInRegion(x));
    if(region.chunks[index] == nullptr) {
        auto chunkWidth = U16(1) << chunkSize;
        Area area {(I32)x, (I32)y, 0, (U16)chunkWidth, (U16)chunkWidth, U16(1 << chunkHeight), 0};
        region.chunks[index] = new Chunk(area);
        pipeline.fillChunk(*region.chunks[index]);
    }

    return *region.chunks[index];
}

void WorldManager::resize(Int x, Int y) {
    auto left = Tritium::Math::min((I32)x, this->x);
    auto right = Tritium::Math::max((I32)x + 1, this->x + width);
    auto bottom = Tritium::Math::min((I32)y, this->y);
    auto top = Tritium::Math::max((I32)y + 1, this->y + height);

    auto w = right - left;
    auto h = top - bottom;
    auto dx = (Size)(this->x - left);
    auto dy = (Size)(this->y - bottom);

    auto newRegions = (Region*)malloc(sizeof(Region) * w * h);

    // Initialize new rows below the existing data.
    for(Size row = 0; row < dy; row++) {
        for(Size column = 0; column < w; column++) {
            new (newRegions + row * w + column) Region;
        }
    }

    // Initialize the area with existing rows.
    for(Size row = 0; row < height; row++) {
        // Initialize new values before the row.
        for(Size column = 0; column < dx; column++) {
            new (newRegions + (dy + row) * w + column) Region;
        }

        // Copy the existing row.
        memcpy(newRegions + (row + dy) * w + dx, regions + row * width, sizeof(Region) * width);

        // Initialize new values at the end of the row.
        for(Size column = dx + width; column < w; column++) {
            new (newRegions + row * w + column) Region;
        }
    }

    // Initialize new rows above the existing data.
    for(Size row = height + dy; row < h; row++) {
        for(Size column = 0; column < w; column++) {
            new (newRegions + row * w + column) Region;
        }
    }

    // Update the matrix state.
    free(regions);
    regions = newRegions;
    this->x = (I32)left;
    this->y = (I32)bottom;
    width = (U16)w;
    height = (U16)h;
}

} // namespace generator