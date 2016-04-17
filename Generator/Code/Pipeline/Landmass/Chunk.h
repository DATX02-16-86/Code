
#pragma once

#include "Voronoi.h"
#include "Filler.h"
#include "ChunkMatrix.h"
#include "Attribute.h"

namespace generator {
namespace landmass {

struct ChunkMatrix;

extern const Int2 neighbourOffsets[8];

inline U8 packRelative(Int2 relativePos) {
    return (U8)((relativePos.x & 0b11) | (relativePos.y & 0b11 << 2));
}

inline Int2 unpackRelative(U8 relativePos) {
    I32 x = (relativePos & 0b11) << 30 >> 30;
    I32 y = ((relativePos >> 2) & 0b11) << 30 >> 30;
    return Int2 {x, y};
}

inline VertexIndex nextVertexIndex(VertexIndex current, Edge next) {
    if(current == next.b) {
        return next.a;
    }
    return next.b;
}

struct Chunk {
    /// Each chunk is built in multiple stages to prevent recursive dependencies between them.
    /// This defines the stages a chunk can be in.
    enum Stage: U8 {
        None,
        Points,
        Vertices,
        Edges,
        Connections
    };

    Chunk(I32 x, I32 y, U32 size, U32 gridSize, U32 gridSpread):
        x(x), y(y), size((U16)size), gridSize((U16)gridSize), gridSpread((U8)gridSpread) {}
    ~Chunk() {Tritium::hFree(gridVertices);}

    /// Builds the cell centers of this chunk.
    /// The stage is set to Points.
    /// @param filler The location generator to use.
    void buildCenters(Filler& filler);

    /// Builds the base voronoi shape of this chunk.
    /// This creates a diagram with empty metadata.
    /// @param fill The method use use for constructing the diagram.
    void buildVertices(ChunkMatrix& matrix, Filler& filler);

    /// Builds the voronoi edges of this chunk.
    /// This connects the vertices produced by the previous stage.
    void buildEdges(ChunkMatrix& matrix, Filler& filler);

    /// Connects the border edges in this chunk to its neighbours.
    /// This is the last stage before attribute generation.
    void connectEdges(ChunkMatrix& matrix, Filler& filler);

    /// Fully builds this chunk, then creates the attribute structures.
    void build(ChunkMatrix& matrix, Filler& filler, AttributeId* attributes, Size attributeCount);

    template<class F>
    void mapNeighbours(ChunkMatrix& matrix, F&& f) {
        auto pivot = Int2 {x, y};
        for(auto offset: neighbourOffsets) {
            auto position = pivot + offset;
            f(matrix.getChunk(position.x, position.y));
        }
    }

    /// Checks if the provided point is close to the border of this chunk.
    bool isRelevantBorderCell(F32 x, F32 y) {
        auto size = (F32)this->size;
        return
            x >= ((F32)this->x - 0.1f) * size &&
            x < ((F32)this->x + 1.1f) * size &&
            y >= ((F32)this->y - 0.1f) * size &&
            y < ((F32)this->y + 1.1f) * size;
    }

    /// Checks if the provided vertex is inside this chunk.
    bool isVertexInChunk(const Chunk& chunk, Vertex p) {
        return
            p.x >= chunk.x * chunk.size &&
            p.x < (chunk.x + 1) * chunk.size &&
            p.y >= chunk.y * chunk.size &&
            p.y < (chunk.y + 1) * chunk.size;
    }

    /// Calculates the position of the provided chunk relative to this one.
    Int2 relativeChunkPosition(const Chunk& chunk) {
        return {chunk.x - x, chunk.y - y};
    }

    /// Calculates the position of the provided point relative to this one.
    Int2 relativeChunkPosition(Vertex p) {
        return {(I32)Tritium::Math::floor(p.x / size) - x, (I32)Tritium::Math::floor(p.y / size) - y};
    }

    /// Returns the absolute position of the neighbour at this offset.
    Int2 neighbourPosition(U8);

    /// Returns the neighbour at the provided offset.
    Chunk& neighbour(ChunkMatrix& matrix, U8 offset) {
        auto p = neighbourPosition(offset);
        return matrix.getChunk(p.x, p.y);
    }

    U32 findEdgeIndexCand(ChunkMatrix& matrix, Vertex a, Vertex b);
    U32 findEdgeIndex(ChunkMatrix& matrix, Vertex a, Vertex b);

    std::vector<Point> cellCenters;
    Array<ArrayF<EdgeIndex, kMaxCellEdges>> cellEdges;
    Array<Vertex> vertices;
    Array<ArrayF<EdgeIndex, kMaxVertexEdges>> vertexEdges;
    Array<Edge> edges;
    std::vector<U32> edgeConnectCandidates;
    U16* gridVertices = nullptr;

    // These are used in the Vertices and Edges stages; after that they are destroyed.
    std::vector<Point> cellCentersWithBorder;
    Uninitialized<Diagram> diagram;

    // These are used in the Connections stage; after that they are destroyed.
    Array<ArrayF<UnconnectedEdge, kMaxCellEdges>> unconnectedCellEdges;
    Array<ArrayF<UnconnectedEdge, kMaxVertexEdges>> unconnectedVertexEdges;

    AttributeMap attributes;

    I32 x;
    I32 y;
    U16 size;
    U16 gridSize;
    U8 generatorStage = 0;
    U8 gridSpread;
    Stage stage = None;
};

}}