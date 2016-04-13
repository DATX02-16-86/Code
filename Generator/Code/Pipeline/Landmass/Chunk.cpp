
#include "Chunk.h"
#include "ChunkMatrix.h"

namespace generator {
namespace landmass {

const Int2 neighbourOffsets[] = {{-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}};

inline U8 pack(Int2 relativePos) {
    return (U8)((relativePos.x & 0b11) | (relativePos.y & 0b11 << 2));
}

inline Int2 unpack(U8 relativePos) {
    return Int2 {relativePos & 0b11, (relativePos >> 2) & 0b11};
}

inline U32 findVertexIndex(const Chunk& chunk, Vertex v) {
    for (U32 i = 0; i < chunk.vertices.size(); ++i) {
        if(almostEqual(chunk.vertices[i], v)) return i;
    }

    debugError("Vertex wasn't in the chunk");
    return 0;
}

template<class F>
void Chunk::mapNeighbours(ChunkMatrix& matrix, F&& f) {
    auto pivot = Int2 {x, y};
    for(auto offset: neighbourOffsets) {
        auto position = pivot + offset;
        f(matrix.getChunk(position.x, position.y));
    }
}

void Chunk::buildCenters(Filler& filler) {
    if(stage >= Points) return;

    // Generate the source points we will construct the diagram form.
    FillContext context {cellCenters, x, y, (I32)size};
    filler.fill(context);

    stage = Points;
}

void Chunk::buildVertices(ChunkMatrix& matrix, Filler& filler) {
    if(stage >= Vertices) return;
    buildCenters(filler);

    /*
     * Create a voronoi diagram from the cell centers we created.
     * This is done by generating cell centers for each neighbour and
     * adding any border regions to this voronoi diagram.
     * By doing so, we ensure that the border cells in each chunk are generated the same way,
     * allowing us to merge them without any problems.
     */

    cellCentersWithBorder = cellCenters;
    mapNeighbours(matrix, [&](Chunk& chunk) {
        // Make sure that the neighbour exists and has generated its cell centers.
        chunk.buildCenters(filler);
        for(auto p : chunk.cellCenters) {
            if(isRelevantBorderCell(p.x(), p.y())) {
                cellCentersWithBorder.push_back(p);
            }
        }
    });

    // Construct the voronoi diagram. This is saved until we have built the edges as well.
    diagram.init();
    construct_voronoi(cellCentersWithBorder.begin(), cellCentersWithBorder.end(), &diagram);

    // Add the generated cell vertices that belong to this chunk.
    // We assign each vertex a vertex index, starting at 1 because 0 is the default color.
    U32 vertexIndex = 1;
    for(const auto& vertex : diagram->vertices()) {
        Vertex point{vertex.x(), vertex.y()};
        if(isVertexInChunk(*this, point)) {
            vertices.push(point);
            vertex.color(vertexIndex++);
        }
    }

    stage = Vertices;
}

void Chunk::buildEdges(ChunkMatrix& matrix, Filler& filler) {
    if(stage >= Edges) return;
    buildVertices(matrix, filler);

    mapNeighbours(matrix, [&](Chunk& chunk) {
        // Make sure that the neighbour exists and has generated its vertices.
        chunk.buildVertices(matrix, filler);
    });

    vertexEdges.resize((U32)vertices.size());
    unconnectedVertexEdges.resize((U32)vertices.size());

    auto pivot = Int2 {x, y};
    for(auto& vertex : diagram->vertices()) {
        auto vertexIndex = vertex.color();
        if(vertexIndex) {
            auto* incidentEdge = vertex.incident_edge();
            auto* it = incidentEdge;
            U32 position = 0;
            do {
                it = it->rot_next();

                // Can process edge and it's not yet processed via its twin
                if(it->is_primary() && it->is_finite() && it->color() == 0) {
                    auto& vert0 = *it->vertex0();
                    Vertex point0 {vert0.x(), vert0.y()};
                    auto& vert1 = *it->vertex1();
                    Vertex point1 {vert1.x(), vert1.y()};

                    auto chunkIndex0 = relativeChunkPosition(point0);
                    auto chunkIndex1 = relativeChunkPosition(point1);
                    auto vertIndex0 = (U32)vert0.color();
                    auto vertIndex1 = (U32)vert1.color();

                    // Is one of the vertices outside this chunk?
                    if(!(chunkIndex0.isZero() && chunkIndex1.isZero())) {
                        // Should this edge be in this chunk?
                        auto decidingVertex = point0 < point1 ? point0 : point1;
                        auto decidingIndex = relativeChunkPosition(decidingVertex);
                        if(decidingIndex.x | decidingIndex.y) {
                            it->color(1);
                            it->twin()->color(1);
                            UnconnectedEdge edge {point0, point1, position++, pack(decidingIndex)};
                            unconnectedVertexEdges[vertexIndex - 1].push(edge);
                            continue;
                        }

                        edgeConnectCandidates.push_back(edges.size());
                        if(!chunkIndex0.isZero()) {
                            auto p = pivot + chunkIndex0;
                            vertIndex0 = findVertexIndex(matrix.getChunk(p.x, p.y), point0) + 1;
                        }
                        if(!chunkIndex1.isZero()) {
                            auto p = pivot + chunkIndex1;
                            vertIndex1 = findVertexIndex(matrix.getChunk(p.x, p.y), point1) + 1;
                        }
                    }

                    // 0 and 1 are reserved numbers
                    it->color(edges.size() + 2);
                    it->twin()->color(edges.size() + 2);

                    edges.push(Edge {{pack(chunkIndex0), vertIndex0 - 1}, {pack(chunkIndex1), vertIndex1 - 1}});
                }

                if(it->color() >= 2) {
                    // Add the edge index, subtract 1 since 1 was added to avoid 0
                    vertexEdges[vertexIndex - 1].push(EdgeIndex {0, (U32)it->color() - 2 });
                    ++position;
                }
            } while (it != incidentEdge);
        }
    }

    auto cellCount = cellCenters.size();
    cellEdges.resize(cellCount);
    unconnectedCellEdges.resize(cellCount);

    for(const DiagramCell& cell : diagram->cells()) {
        auto cellIndex = cell.source_index();

        // Cell is in this chunk
        if(cellIndex < cellCount) {
            const DiagramEdge* incidentEdge = cell.incident_edge();
            const DiagramEdge* it = incidentEdge;
            U32 position = 0;
            do {
                it = it->next();

                if(it->color() >= 2) {
                    // Add the edge index, subtract 2 since 0 and 1 were special numbers
                    cellEdges[cellIndex].push(EdgeIndex {0, (U32)it->color() - 2});
                } else {
                    const auto& vert0 = *(it->vertex0());
                    Vertex point0 {vert0.x(), vert0.y()};
                    const auto& vert1 = *(it->vertex1());
                    Vertex point1 {vert1.x(), vert1.y()};

                    auto decidingVertex = point0 < point1 ? point0 : point1;
                    auto decidingIndex = relativeChunkPosition(decidingVertex);
                    UnconnectedEdge edge {point0, point1, position, pack(decidingIndex)};
                    unconnectedCellEdges[cellIndex].push(edge);
                }

                ++position;
            } while(it != incidentEdge);
        }
    }

    // Release the memory used by the border cell array and voronoi diagram.
    cellCentersWithBorder.clear();
    cellCentersWithBorder.shrink_to_fit();
    diagram->~Diagram();
    diagram.init();

    stage = Edges;
}

void Chunk::connectEdges(ChunkMatrix& matrix, Filler& filler) {
    if(stage >= Connections) return;
    buildEdges(matrix, filler);

    mapNeighbours(matrix, [&](Chunk& chunk) {
        // Make sure that the neighbour exists and has generated its edges.
        chunk.buildEdges(matrix, filler);
    });

    auto pivot = Int2 {x, y};
    for(int i = 0; i < vertexEdges.size(); ++i) {
        for(auto e : unconnectedVertexEdges[i]) {
            auto position = pivot + unpack(e.connectToChunk);
            auto& chunk = matrix.getChunk(position.x, position.y);
            EdgeIndex index {e.connectToChunk, (U32)chunk.findEdgeIndexCand(matrix, e.a, e.b)};
            vertexEdges[i].insert(e.position, index);
        }
    }

    for(int i = 0; i < cellEdges.size(); ++i) {
        for(auto e : unconnectedCellEdges[i]) {
            auto position = pivot + unpack(e.connectToChunk);
            auto& chunk = matrix.getChunk(position.x, position.y);
            EdgeIndex index {e.connectToChunk, (U32)chunk.findEdgeIndex(matrix, e.a, e.b)};
            cellEdges[i].insert(e.position, index);
        }
    }

    unconnectedCellEdges.destroy();
    unconnectedVertexEdges.destroy();
    stage = Connections;
}

void Chunk::build(ChunkMatrix& matrix, Filler& filler) {
    
}

Int2 Chunk::neighbourPosition(U8 offset) {
    auto position = unpack(offset);
    return Int2 {x, y} + position;
}

U32 Chunk::findEdgeIndexCand(ChunkMatrix& matrix, Vertex a, Vertex b) {
    for(auto i : edgeConnectCandidates) {
        auto e = edges[i];
        auto ea = neighbour(matrix, e.a.chunkIndex).vertices[e.a.index];

        if(ea == a) {
            auto eb = neighbour(matrix, e.b.chunkIndex).vertices[e.b.index];
            if(eb == b) return i;
        } else if(ea == b) {
            auto eb = neighbour(matrix, e.b.chunkIndex).vertices[e.b.index];
            if(eb == a) return i;
        }
    }

    debugError("Edge wasn't in the chunk");
    return 0;
}

U32 Chunk::findEdgeIndex(ChunkMatrix& matrix, Vertex a, Vertex b) {
    for(U32 i = 0; i < edges.size(); ++i) {
        auto e = edges[i];
        auto ea = neighbour(matrix, e.a.chunkIndex).vertices[e.a.index];

        if(ea == a) {
            auto eb = neighbour(matrix, e.b.chunkIndex).vertices[e.b.index];
            if(eb == b) return i;
        } else if(ea == b) {
            auto eb = neighbour(matrix, e.b.chunkIndex).vertices[e.b.index];
            if(eb == a) return i;
        }
    }

    debugError("Edge wasn't in the chunk");
    return 0;
}

}}