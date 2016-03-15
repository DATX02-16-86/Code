
#include "Geometry.h"
#include "../Pipeline/Voxel.h"

namespace generator {

static const U32 kMaxCubeVerts = 24;
static const U32 kMaxCubeInds = 36;

template<class V> struct ChunkBuilder {
    ChunkBuilder(V* verts, IndexType* inds) : verts(verts), inds(inds) {}

    ChunkBuilder& operator ++ (int) {v++; return *this;}
    V* operator -> () {return verts + v;}

    void addi(std::initializer_list<int> list) {
        for(auto ind : list) {
            inds[i] = (IndexType)(v + ind);
            i++;
        }
    }

    V& operator [] (int i) {return verts[(int)v + i - 1];}

    V* verts;
    IndexType* inds;
    U32 v = 0;
    U32 i = 0;
};

static void makeFace(ChunkBuilder<CubeVoxelVertex>& b, U32 x, U32 y, U32 z, VoxelNormal normal, bool dir) {
    if(dir) b.addi({0, 2, 1, 0, 3, 2});
    else b.addi({0, 1, 2, 0, 2, 3});

    b->x = (U16)x; b->y = (U16)y; b->z = (U16)z;
    b->light = 1;
    b->u = 0; b->v = 0;
    b->normal = normal;
    b++;
    b->x = (U16)x; b->y = (U16)y; b->z = (U16)z;
    b->light = 1;
    b->normal = normal;
    b->u = 1; b->v = 0;
    b++;
    b->x = (U16)x; b->y = (U16)y; b->z = (U16)z;
    b->light = 1;
    b->normal = normal;
    b->u = 1; b->v = 1;
    b++;
    b->x = (U16)x; b->y = (U16)y; b->z = (U16)z;
    b->light = 1;
    b->normal = normal;
    b->u = 0; b->v = 1;
    b++;
}

static void makeWestFace(ChunkBuilder<CubeVoxelVertex>& v, U32 x, U32 y, U32 z, char light) {
    makeFace(v, x, y, z, {-128, 0, 0, light}, false);
    ++v[-2].z;
    ++v[-1].z;
    ++v[-1].y;
    ++v[ 0].y;
}

static void makeEastFace(ChunkBuilder<CubeVoxelVertex>& v, U32 x, U32 y, U32 z, char light) {
    x++;
    makeFace(v, x, y, z, {127, 0, 0, light}, true);
    ++v[-2].z;
    ++v[-1].z;
    ++v[-1].y;
    ++v[ 0].y;
}

static void makeNorthFace(ChunkBuilder<CubeVoxelVertex>& v, U32 x, U32 y, U32 z, char light) {
    makeFace(v, x, y, z, {0, -128, 0, light}, true);
    ++v[-2].z;
    ++v[-1].z;
    ++v[-1].x;
    ++v[ 0].x;
}

static void makeSouthFace(ChunkBuilder<CubeVoxelVertex>& v, U32 x, U32 y, U32 z, char light) {
    y++;
    makeFace(v, x, y, z, {0, 127, 0, light}, false);
    ++v[-2].z;
    ++v[-1].z;
    ++v[-1].x;
    ++v[ 0].x;
}

static void makeBottomFace(ChunkBuilder<CubeVoxelVertex>& v, U32 x, U32 y, U32 z, char light) {
    makeFace(v, x, y, z, {0, 0, -128, light}, false);
    ++v[-2].y;
    ++v[-1].y;
    ++v[-1].x;
    ++v[ 0].x;
}

static void makeTopFace(ChunkBuilder<CubeVoxelVertex>& v, U32 x, U32 y, U32 z, char light) {
    z++;
    makeFace(v, x, y, z, {0, 0, 127, light}, true);
    ++v[-2].y;
    ++v[-1].y;
    ++v[-1].x;
    ++v[ 0].x;
}

static void makeCube(const Chunk& c, ChunkBuilder<CubeVoxelVertex>& v, U32 x, U32 y, U32 z, char light) {
    auto w = c.area.width - 1;
    auto h = c.area.height - 1;
    auto d = c.area.depth - 1;

    if(x >= w || !c.at(x+1, y, z).blockType)
        makeEastFace(v, x, y, z, light);

    if(x == 0 || !c.at(x-1, y, z).blockType)
        makeWestFace(v, x, y, z, light);

    if(y == 0 || !c.at(x, y-1, z).blockType)
        makeNorthFace(v, x, y, z, light);

    if(y >= h || !c.at(x, y+1, z).blockType)
        makeSouthFace(v, x, y, z, light);

    // Check if we need to build the top and bottom.
    bool top = false, bottom = false;
    if(z >= d) {
        top = true;
        if(!c.at(x, y, z-1).blockType)
            bottom = true;
    } else if(z == 0) {
        // The bottom of the world can never be visible.
        if(c.area.z != 0)
            bottom = true;
        if(!c.at(x, y, z+1).blockType)
            top = true;
    } else {
        if(!c.at(x, y, z+1).blockType)
            top = true;
        if(!c.at(x, y, z-1).blockType)
            bottom = true;
    }

    if(top) makeTopFace(v, x, y, z, light);
    if(bottom) makeBottomFace(v, x, y, z, light);
}

static void buildChunk(const Chunk& c, ChunkBuilder<CubeVoxelVertex>& builder) {
    for(U32 x = 0; x < c.area.width; x++) {
        for(U32 y = 0; y < c.area.height; y++) {
            for(U32 z = 0; z < c.area.depth; z++) {
                // If the type is not air, we create a cube.
                auto v = c.at(x, y, z);
                if(v.blockType) {
                    makeCube(c, builder, x, y, z, v.baseLight);
                }
            }
        }
    }
}

ChunkGeometry<CubeVoxelVertex> buildCubeGeometry(const Chunk& chunk) {
    // By using glass, it is possible to get a chunk where every cube needs to be fully drawn.
    // Reserve space for the worst case.
    U32 maxCubes = chunk.area.width * chunk.area.height * chunk.area.depth;
    U32 sectionVerts = maxCubes * kMaxCubeVerts;
    U32 sectionInds = maxCubes * kMaxCubeInds;
    U32 maxVerts = 1 * sectionVerts;
    U32 maxInds = 1 * sectionInds;

    // We don't want to risk allocating this much memory on the stack, so create an external buffer.
    auto verts = (CubeVoxelVertex*)malloc(maxVerts * sizeof(CubeVoxelVertex) + maxInds * sizeof(IndexType));
    IndexType* inds = (IndexType*)(verts + maxVerts);

    ChunkBuilder<CubeVoxelVertex> builder(verts, inds);
    buildChunk(chunk, builder);

    U16 indexStride = sizeof(IndexType);
    U32 vertexCount = builder.v;
    U32 indexCount = builder.i;

    // Check if it is possible to use R16 indices for this chunk (only very complex ones need R32).
    // If this is the case, we convert the indices - this saves quite a bit of GPU memory and bandwidth.
    if(vertexCount <= 65535) {
        indexStride = 2;
        auto ip16 = (U16*)inds;

        // Convert indices for the first sector.
        for(Size ix=0; ix<builder.i; ix++)
            ip16[ix] = (U16)(builder.inds[ix]);
    }

    return ChunkGeometry<CubeVoxelVertex>{verts, inds, vertexCount, indexCount, indexStride};
}

} // generator