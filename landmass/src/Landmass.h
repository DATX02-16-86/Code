
#ifndef GENERATOR_LANDMASS_H
#define GENERATOR_LANDMASS_H

#define CHUNK_SIZE 400

#include <Base.h>

enum class WaterType { land, sea, lake, river};
enum class Biome { sea, lake, beach, land};

struct vertexmeta {
    double height;
    WaterType wt;
    double moisture;
};

struct cellmeta {
    double avarageheight;
    Biome biome;
    double avarageMoisture;
};

struct edgemeta {
    bool isRiver;
};

struct Vertex {
    double x;
    double y;
};

struct VertexIndex { U32 chunkIndex : 4; U32 index : 28; };
struct EdgeIndex { U32 chunkIndex : 4; U32 index : 28; };

struct Edge {
    VertexIndex a;
    VertexIndex b;
};

struct UnconnectedEdge {
    Vertex a;
    Vertex b;
    U8 connectToChunk : 4;
    size_t position;
};

enum ChunkState {
    NOTHING, POINTS_ADDED, VERTICES, EDGES, CONNECTED_EDGES, VERTEXMETA, RIVERS, MOISTURE, MOISTURE_NEIGHBOURS, BIOMES
};


template <class T>
struct Optional {
    bool just;
    T value;
};

template <class T>
inline bool closeEnough(T answer, T value, T epsilon) {
    return value >= answer - epsilon && value <= answer + epsilon;
}

inline bool operator==(const Vertex& lhs, const Vertex& rhs) {
    return closeEnough(lhs.x, rhs.x, 0.0000000001) && closeEnough(lhs.y, rhs.y, 0.0000000001);
}

inline bool operator<(const Vertex& lhs, const Vertex& rhs) {
    if (lhs.x < rhs.x) {
        return true;
    } else if (lhs.x > rhs.x) {
        return false;
    } else {
        return lhs.y < rhs.y;
    }
}

inline bool operator==(const VertexIndex& lhs, const VertexIndex& rhs) {
    return lhs.chunkIndex == rhs.chunkIndex && lhs.index == rhs.index;
}

inline bool operator==(const Edge& lhs, const Edge& rhs) {
    return (lhs.a == rhs.a && lhs.b == rhs.b) || (lhs.a == rhs.b && lhs.b == rhs.a);
}

#endif // GENERATOR_LANDMASS_H
