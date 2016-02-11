#include <stdlib.h>
#include "Voxel.h"

namespace generator {

Chunk::Chunk(Area area): area(area) {
    auto elements = area.width * area.height;
    voxels = (Voxel*)malloc(sizeof(Voxel) * elements * area.depth + sizeof(U16) * elements);
    heightMap = (U16*)(voxels + elements * area.depth);
}

Chunk::~Chunk() {
    free(voxels);
    voxels = nullptr;
    heightMap = nullptr;
}

Voxel Chunk::at(Size x, Size y, Size z) const {
    return voxels[z * area.width * area.height + y * area.width + x];
}

Voxel& Chunk::at(Size x, Size y, Size z) {
    return voxels[z * area.width * area.height + y * area.width + x];
}

void Chunk::set(Size x, Size y, Size z, Voxel voxel) {
    at(x, y, z) = voxel;
}

void Chunk::updateHeightMap() {
    for(Size x = 0; x < area.width; x++) {
        for(Size y = 0; y < area.height; y++) {
            U16 height = 0;
            for(Int z = area.depth - 1; z >= 0; z--) {
                if(at(x, y, (Size)z).blockType) {
                    height = (U16)z;
                    break;
                }
            }
            heightMap[area.width * y + x] = height;
        }
    }
}

} // namespace generator