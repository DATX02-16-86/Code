
#include "Chunk.h"
#include "ChunkMatrix.h"

namespace generator {
namespace landmass {

const Int2 neighbourOffsets[] = {{-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}};

inline U8 pack(Int2 relativePos) {
    return (U8)((relativePos.x & 0b11) | (relativePos.y & 0b11 << 2));
}

inline U32 findVertexIndex(const Chunk& chunk, Vertex v) {
    for (U32 i = 0; i < chunk.vertices.size(); ++i) {
        if(almostEqual(chunk.vertices[i], v)) return i;
    }

    assert("Vertex wasn't in the chunk" == 0);
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
    FillContext context {cellCenters, x, y, size};
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
    construct_voronoi(cellCentersWithBorder.begin(), cellCentersWithBorder.end(), &diagram);

    // Add the generated cell vertices that belong to this chunk.
    // We assign each vertex a vertex index, starting at 1 because 0 is the default color.
    diagram.init();
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

    vertexEdges.reserve((U32)vertices.size());
    unconnectedVertexEdges.reserve((U32)vertices.size());
    std::vector<size_t> edgeConnectCandidates;

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
                    vertexEdges[vertexIndex - 1].push({0, (U32)it->color() - 2 });
                    ++position;
                }
            } while (it != incidentEdge);
        }
    }

    edgeMeta.reserve(edges.size());

    auto cellCount = cellCenters.size();
    cellEdges.reserve(cellCount);
    unconnectedCellEdges.reserve(cellCount);

    for(auto& cell : diagram->cells()) {
        auto cellIndex = cell.source_index();

        // Cell is in this chunk
        if(cellIndex < cellCount) {
            const auto* incidentEdge = cell.incident_edge();
            const auto* it = incidentEdge;
            U32 position = 0;
            do {
                it = it->next();

                if(it->color() >= 2) {
                    // Add the edge index, subtract 2 since 0 and 1 were special numbers
                    cellEdges[cellIndex].push({0, (U32)it->color() - 2});
                } else {
                    const auto& vert0 = *(it->vertex0());
                    Vertex point0{ vert0.x(), vert0.y() };
                    const auto& vert1 = *(it->vertex1());
                    Vertex point1{ vert1.x(), vert1.y() };

                    auto decidingVertex = point0 < point1 ? point0 : point1;
                    auto decidingIndex = relativeChunkPosition(decidingVertex);
                    UnconnectedEdge edge {point0, point1, position, pack(decidingIndex)};
                    unconnectedCellEdges[cellIndex].push(edge);
                }

                ++position;
            } while (it != incidentEdge);
        }
    }

    // Release the memory used by the border cell array and voronoi diagram.
    cellCentersWithBorder.clear();
    cellCentersWithBorder.shrink_to_fit();
    diagram->~Diagram();
    diagram.init();

    stage = Edges;
}

}}