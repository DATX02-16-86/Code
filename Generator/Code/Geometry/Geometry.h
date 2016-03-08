
#ifndef GENERATOR_GEOMETRY_H
#define GENERATOR_GEOMETRY_H

#include <Base.h>
#include <Math/Half.h>
#include <stdlib.h>

namespace generator {

struct Chunk;

using Tritium::Math::F16;

typedef U32 IndexType;

struct VoxelNormal {
    I8 nx, ny, nz, nw;
};

/** A single vertex of a cubic voxel. */
struct CubeVoxelVertex {
    // Position as half floats.
    F16 x, y, z, light;

    // Texture coordinates as half floats.
    F16 u, v;

    // Normal vector with normalized components.
    VoxelNormal normal;
};

template<class V> struct ChunkGeometry {
    const V* vertices;
    const void* indices;
    U32 vertexCount;
    U32 indexCount;
    U16 indexSize;

    ChunkGeometry(const V* vertices, const void* indices, U32 vertexCount, U32 indexCount, U16 indexSize):
        vertices(vertices), indices(indices), vertexCount(vertexCount), indexCount(indexCount), indexSize(indexSize) {}

    void reference() const {refCount++;}
    void release() const {
        refCount--;
        if(refCount == 0) {
            free((void*)vertices);
        }
    }

private:
    mutable U16 refCount = 1;
};

ChunkGeometry<CubeVoxelVertex> buildCubeGeometry(const Chunk& chunk);

} // namespace generator

#endif //GENERATOR_GEOMETRY_H
