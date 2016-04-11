
#pragma once

#include "Voronoi.h"
#include "Filler.h"
#include "ChunkMatrix.h"

namespace generator {
namespace landmass {

struct ChunkMatrix;

struct Chunk {
    /// Each chunk is built in multiple stages to prevent recursive dependencies between them.
    /// This defines the stages a chunk can be in.
    enum Stage {
        None,
        Points,
        Vertices,
        Edges,
        Connections
    };

    Chunk(I32 x, I32 y, U32 size): x(x), y(y), size(size) {}

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

    template<class F>
    void mapNeighbours(ChunkMatrix& matrix, F&& f);

    /// Checks if the provided point is close to the border of this chunk.
    bool isRelevantBorderCell(F32 x, F32 y) {
        auto size = (F32)this->size;
        return
            x >= ((F32)x - 0.15f) * size &&
            x < ((F32)x + 1.15f) * size &&
            y >= ((F32)y - 0.15f) * size &&
            y < ((F32)y + 1.15f) * size;
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
        return {(I32)Tritium::Math::floor(p.x) - x, (I32)Tritium::Math::floor(p.y) - y};
    }

    std::vector<Point> cellCenters;
    Array<ArrayF<EdgeIndex, kMaxCellEdges>> cellEdges;
    Array<ArrayF<UnconnectedEdge, kMaxCellEdges>> unconnectedCellEdges;
    Array<Vertex> vertices;
    Array<ArrayF<EdgeIndex, kMaxVertexEdges>> vertexEdges;
    Array<ArrayF<UnconnectedEdge, kMaxVertexEdges>> unconnectedVertexEdges;
    Array<Edge> edges;
    Array<VertexMeta> vertexMeta;
    Array<CellMeta> cellMeta;
    Array<EdgeMeta> edgeMeta;

    // These are used in the Vertices and Edges stages; after that they are destroyed.
    std::vector<Point> cellCentersWithBorder;
    Uninitialized<Diagram> diagram;

    I32 x;
    I32 y;
    U32 size;
    Stage stage = None;
};

}}