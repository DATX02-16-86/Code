
#include "Voronoi.h"
#include <random>
#include <iostream>

bool vertexIsInChunk(int chunkX, int chunkY, Vertex p) {
    return p.x >= chunkX * CHUNK_SIZE &&
           p.x < (chunkX + 1) * CHUNK_SIZE &&
           p.y >= chunkY * CHUNK_SIZE &&
           p.y < (chunkY + 1) * CHUNK_SIZE;
}

bool pointIsInChunk(int chunkX, int chunkY, Point p) {
    return vertexIsInChunk(chunkX, chunkY, { p.x(), p.y() });
}

const int chunkIndexRelativeX[9] = { -1,  0,  1, -1, 0, 1, -1, 0, 1 };
int chunkIndexRelativeY[9] = { -1, -1, -1,  0, 0, 0,  1, 1, 1 };

// Gives the relative index of the chunk where p is in
// 0 1 2
// 3 4 5
// 6 7 8
U8 chunkIndex(int chunkX, int chunkY, Vertex p) {
    for (int i = 0; i < 9; ++i) {
        if (vertexIsInChunk(chunkX + chunkIndexRelativeX[i], chunkY + chunkIndexRelativeY[i], p)) {
            return i;
        }
    }

    throw std::range_error("Vertex wasn't in any adjacent chunks");
}

U8 oppositeChunkIndex(U8 chunkIndex) {
    return 8 - chunkIndex;
}

U8 chunkIndex(const ChunkWithIndexes& chunk, const ChunkWithIndexes& chunk2) {
    if (chunk.x == chunk2.x && chunk.y == chunk2.y) {
        return CURRENT_CHUNK_INDEX;
    }
    for (int i = 0; i < 9; ++i) {
        if (chunk.x + chunkIndexRelativeX[i] == chunk2.x, chunk.y + chunkIndexRelativeY[i] == chunk2.y) {
            return i;
        }
    }

    throw std::range_error("Chunks not close enough");
}

ChunkWithIndexes& findChunkWithChunkIndex(ChunkMap& hexChunks, ChunkWithIndexes& chunk, U8 chunkIndex) {
    if (chunkIndex == CURRENT_CHUNK_INDEX) {
        return chunk;
    }
    else {
        std::pair<int, int> pos{ chunk.x + chunkIndexRelativeX[chunkIndex], chunk.y + chunkIndexRelativeY[chunkIndex] };
        return hexChunks[pos];
    }
}

const ChunkWithIndexes& findChunkWithChunkIndex(const ChunkMap& hexChunks, const ChunkWithIndexes& chunk, U8 chunkIndex) {
    if (chunkIndex == CURRENT_CHUNK_INDEX) {
        return chunk;
    }
    else {
        std::pair<int, int> pos{ chunk.x + chunkIndexRelativeX[chunkIndex], chunk.y + chunkIndexRelativeY[chunkIndex] };
        return hexChunks.at(pos);
    }
}

vertexmeta& getVertexMeta(ChunkMap& map, ChunkWithIndexes& c, VertexIndex i, void (*prepare) (ChunkMap&, ChunkWithIndexes&)) {
    auto& chunk = findChunkWithChunkIndex(map, c, i.chunkIndex);
    prepare(map, chunk);
    return chunk.vertexmetas[i.index];
}

std::pair<Optional<vertexmeta>, Optional<vertexmeta>> getEdgeMetas(const boost::polygon::voronoi_edge<double>& edge, const std::vector<vertexmeta>& metadata) {
    if (edge.is_primary())
    {
        if (edge.is_finite())
        {
            auto index0 = edge.vertex0()->color();
            auto index1 = edge.vertex1()->color();
            return {{true, metadata[index0]},{true, metadata[index1]} };
        }
        else
        {
            return{ {false}, {false} };
        }
    }
    return{ { false },{ false } };
}

VertexIndex sharedVertexIndex(Edge first, Edge second, U8 secondChunkIndex) {
    // Normalize chunk indexes
    Edge normalized = first;
    if (normalized.a.chunkIndex == CURRENT_CHUNK_INDEX) {
        normalized.a.chunkIndex = oppositeChunkIndex(secondChunkIndex);
    } else if (normalized.a.chunkIndex == secondChunkIndex) {
        second.a.chunkIndex = CURRENT_CHUNK_INDEX;
    }

    if (normalized.a == second.a || normalized.a == second.b) {
        return first.a;
    }
    return first.b;
}

void insertRandomPoints(int chunkX, int chunkY, int count, int seed, std::vector<Point>& points) {
    std::random_device rd;
    std::mt19937 gen(chunkSeed(chunkX, chunkY, seed));
    std::uniform_int_distribution<> dis(0, CHUNK_SIZE - 1);
    for (int i = 0; i < count; i++) {
        int x = dis(gen);
        int y = dis(gen);
        points.push_back(Point(CHUNK_SIZE * chunkX + x, CHUNK_SIZE * chunkY + y));
    }
}


void relaxPoints(int chunkX, int chunkY, std::vector<Point>& points, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        VD diagram;
        construct_voronoi(points.begin(), points.end(), &diagram);
        for (auto it : diagram.cells()) {
            auto edge = it.incident_edge();
            double x = 0;
            double y = 0;
            int count = 0;
            do {
                edge = edge->next();
                auto edgepPoints = getEdgePoints(*edge, points);
                x += edgepPoints.first.x() - chunkX * CHUNK_SIZE;
                y += edgepPoints.first.y() - chunkY * CHUNK_SIZE;
                ++count;
            } while (edge != it.incident_edge());
            points[it.source_index()].x(x / count + chunkX * CHUNK_SIZE);
            points[it.source_index()].y( y / count + chunkY * CHUNK_SIZE);
        }
    }
}

void filterPointsOutsideChunks(int chunkX, int chunkY, std::vector<Point>& points) {
    points.erase(std::remove_if(points.begin(), points.end(), [&](Point p) {return !pointIsInChunk(chunkX, chunkY, p); }), points.end());
}

void insertGridPoints(int chunkX, int chunkY, int pointDistance, std::vector<Point>& points) {

    for (int y = 0; y <= CHUNK_SIZE; y += pointDistance) {
        for (int x = 0; x <= CHUNK_SIZE; x += pointDistance) {
            points.push_back(Point(CHUNK_SIZE * chunkX + x, CHUNK_SIZE * chunkY + y));
        }
    }
}

void insertHexPoints(int chunkX, int chunkY, int pointDistance, std::vector<Point>& points) {
    int stepsNeeded = CHUNK_SIZE / pointDistance;
    for (int i = 0; i < stepsNeeded; ++i) {
        for (int j = 0; j < stepsNeeded; ++j) {
            int x = j * pointDistance;
            int y = i * pointDistance;
            auto offset = i & 1 ? 0 : (pointDistance / 2);
            points.push_back(Point(CHUNK_SIZE * chunkX + x + offset, CHUNK_SIZE * chunkY + y));
        }
    }
}

void insertHexPointsWithRandomness(int chunkX, int chunkY, int pointDistance, int seed, std::vector<Point>& points) {
    int stepsNeeded = CHUNK_SIZE / pointDistance;

    std::random_device rd;
    std::mt19937 gen(chunkSeed(chunkX, chunkY, seed));
    double maxDistance = pointDistance / 4.0;
    std::uniform_int_distribution<> dis(0, int (maxDistance / 2.0));
    std::bernoulli_distribution bdis(0.5);

    for (int i = 0; i < stepsNeeded; ++i) {
        for (int j = 0; j < stepsNeeded; ++j) {
            int x = j * pointDistance;
            int y = i * pointDistance;
            auto hexOffset = i & 1 ? 0 : (pointDistance / 2);
            int xOffset = int(bdis(gen) ? maxDistance / 2.0 + dis(gen) : - maxDistance / 2.0 - dis(gen));
            int yOffset = int(bdis(gen) ? maxDistance / 2.0 + dis(gen) : -maxDistance / 2.0 - dis(gen));
            points.push_back(Point(CHUNK_SIZE * chunkX + x + hexOffset + xOffset, CHUNK_SIZE * chunkY + y + yOffset));
        }
    }
}

//Neighoburs to a chunk, inclouding the chunk itself
std::vector<Chunk*> neighbourChunks(int x, int y, std::vector<Chunk>& in) {
    std::vector<Chunk*> out;
    for (auto& chunk: in) {
        if (chunk.x >= x - 1 && chunk.x <= x + 1 && chunk.y >= y - 1 && chunk.y <= y + 1 && !(chunk.y == y && chunk.x == x)) {
            out.push_back(&chunk);
        }
    }
    return out;
}

//Neighoburs to a chunk, not inclouding the chunk itself
std::vector<ChunkWithIndexes*> neighbourChunks(int x, int y, std::vector<ChunkWithIndexes>& in) {
    std::vector<ChunkWithIndexes*> out;
    for (auto& chunk : in) {
        if (chunk.x >= x - 1 && chunk.x <= x + 1 && chunk.y >= y - 1 && chunk.y <= y + 1 && !(chunk.y == y && chunk.x == x)) {
            out.push_back(&chunk);
        }
    }
    return out;
}

//Neighoburs to a chunk, not inclouding the chunk itself
std::vector<ChunkWithIndexes*> neighbourChunks(ChunkMap& map, int x, int y) {
    std::vector<ChunkWithIndexes*> out;
    for (int i = 0; i < 9; ++i) {
        int chunkX = x + chunkIndexRelativeX[i];
        int chunkY = y + chunkIndexRelativeY[i];
        try {
            std::pair<int, int> pos{ chunkX, chunkY };
            out.push_back(&map.at(pos));
        } catch (...) {
        }
    }
    return out;
}

VertexIndex neighbourVertexIndex(const ChunkMap& map, const ChunkWithIndexes& c, const Edge& e, const Vertex& v) {
    if (findChunkWithChunkIndex(map, c, e.a.chunkIndex).vertices[e.a.index] == v) {
        return e.a;
    }
    return e.b;
}



bool calculationWorthy(double x, double y, const Chunk &chunk) {
    return x >= (chunk.x - 0.3) * CHUNK_SIZE &&
           x < (chunk.x + 1.3) * CHUNK_SIZE &&
           y >= (chunk.y - 0.3) * CHUNK_SIZE &&
           y < (chunk.y + 1.3) * CHUNK_SIZE;
}

bool calculationWorthy(double x, double y, const ChunkWithIndexes &chunk) {
    return x >= (chunk.x - 0.3) * CHUNK_SIZE &&
           x < (chunk.x + 1.3) * CHUNK_SIZE &&
           y >= (chunk.y - 0.3) * CHUNK_SIZE &&
           y < (chunk.y + 1.3) * CHUNK_SIZE;
}

std::pair<Point, Point> getEdgePoints(const boost::polygon::voronoi_edge<double>& edge, std::vector<Point>& points) {
    if (edge.is_primary())
    {
        if (edge.is_finite())
        {
            double x0 = edge.vertex0()->x();
            double y0 = edge.vertex0()->y();

            double x1 = edge.vertex1()->x();
            double y1 = edge.vertex1()->y();
            return{ {x0, y0}, {x1, y1} };
        }
        else
        {
            Point p1 = points[edge.cell()->source_index()];
            Point p2 = points[edge.twin()->cell()->source_index()];
            Point origin;
            Point direction;
            coordinate_type koef = 1.0;

            origin.x((p1.x() + p2.x()) * 0.5);
            origin.y((p1.y() + p2.y()) * 0.5);
            direction.x(p1.y() - p2.y());
            direction.y(p2.x() - p1.x());

            double x0;
            double y0;
            double x1;
            double y1;

            if (edge.vertex0() == NULL) {
                x0 = origin.x() - direction.x() * koef;
                y0 = origin.y() - direction.y() * koef;
            }
            else {
                x0 = edge.vertex0()->x();
                y0 = edge.vertex0()->y();
            }

            if (edge.vertex1() == NULL) {
                x1 = origin.x() + direction.x() * koef;
                y1 = origin.y() + direction.y() * koef;
            }
            else {
                x1 = edge.vertex1()->x();
                y1 = edge.vertex1()->y();
            }

            return{ { x0, y0 },{ x1, y1 } };
        }
    }
    return{ {0, 0}, {0, 0} };
}

void addPoints(ChunkWithIndexes& chunk) {
    if (chunk.state >= ChunkState::POINTS_ADDED) {
        return;
    }

    insertHexPointsWithRandomness(chunk.x, chunk.y, 20, 0, chunk.cellPoints);
    chunk.state = ChunkState::POINTS_ADDED;
}

void addVertices(ChunkMap& map, ChunkWithIndexes& chunk) {
    if (chunk.state >= ChunkState::VERTICES) {
        return;
    }

    addPoints(chunk);

    auto neighbours = neighbourChunks(map, chunk.x, chunk.y);
    int size = chunk.cellPoints.size();

    for (auto* nei : neighbours) {
        addPoints(*nei);
        size += nei->cellPoints.size();
    }

    std::vector<Point> cells;

    cells.reserve(size);
    for (Point point : chunk.cellPoints) {
        cells.push_back(point);
    }

    for (auto& neighbour : neighbours) {
        for (Point point : neighbour->cellPoints) {
            if (calculationWorthy(point.x(), point.y(), chunk)) {
                cells.push_back(point);
            }
        }
    }

    VD voronoi;
    construct_voronoi(cells.begin(), cells.end(), &voronoi);

    // Add vertices to chunk and remember their index
    for (const auto& vertex : voronoi.vertices()) {
        Vertex point{ vertex.x(), vertex.y() };
        if (vertexIsInChunk(chunk.x, chunk.y, point)) {
            chunk.vertices.push_back(point);
        }
    }

    chunk.state = ChunkState::VERTICES;
}

// TODO: speed up
int findVertexIndex(const ChunkWithIndexes& chunk, Vertex v) {
    for (size_t i = 0; i < chunk.vertices.size(); ++i) {
        if (chunk.vertices[i] == v) {
            return i;
        }
    }

    std::cout << "Vertex wasn't in the chunk";
    throw std::range_error("Vertex wasn't in the chunk");
}

int findEdgeIndexCand(ChunkMap& map, const ChunkWithIndexes& chunk, Vertex a, Vertex b) {
    for (auto i : chunk.edgesConnectCandidates) {
        auto e = chunk.edges[i];
        auto ea = findChunkWithChunkIndex(map, chunk, e.a.chunkIndex).vertices[e.a.index];

        if (ea == a) {
            auto eb = findChunkWithChunkIndex(map, chunk, e.b.chunkIndex).vertices[e.b.index];
            if (eb == b) {
                return i;
            }
        }
        else if (ea == b) {
            auto eb = findChunkWithChunkIndex(map, chunk, e.b.chunkIndex).vertices[e.b.index];
            if (eb == a) {
                return i;
            }
        }
    }

    std::cout << "Edge wasn't in the chunk";
    throw std::range_error("Edge wasn't in the chunk");
}

int findEdgeIndex(ChunkMap& map, const ChunkWithIndexes& chunk, Vertex a, Vertex b) {
    for (int i = 0; i < chunk.edges.size(); ++i) {
        auto e = chunk.edges[i];
        auto ea = findChunkWithChunkIndex(map, chunk, e.a.chunkIndex).vertices[e.a.index];

        if (ea == a) {
            auto eb = findChunkWithChunkIndex(map, chunk, e.b.chunkIndex).vertices[e.b.index];
            if (eb == b) {
                return i;
            }
        }
        else if (ea == b) {
            auto eb = findChunkWithChunkIndex(map, chunk, e.b.chunkIndex).vertices[e.b.index];
            if (eb == a) {
                return i;
            }
        }
    }

    std::cout << "Edge wasn't in the chunk";
    throw std::range_error("Edge wasn't in the chunk");
}

void addVoronoi(ChunkMap& map, ChunkWithIndexes& chunk) {
    if (chunk.state >= ChunkState::EDGES) {
        return;
    }
    addVertices(map, chunk);

    auto neighbours = neighbourChunks(map, chunk.x, chunk.y);
    int size = chunk.cellPoints.size();

    for (auto* nei : neighbours) {
        addVertices(map, *nei);
        size += nei->cellPoints.size();
    }

    std::vector<Point> cells;

    cells.reserve(size);
    for (Point point : chunk.cellPoints) {
        cells.push_back(point);
    }

    for (const auto& neighbour : neighbours) {
        for (Point point : neighbour->cellPoints) {
            if (calculationWorthy(point.x(), point.y(), chunk)) {
                cells.push_back(point);
            }
        }
    }

    VD voronoi;
    construct_voronoi(cells.begin(), cells.end(), &voronoi);

    int i = 1;
    for (const auto& vertex : voronoi.vertices()) {
        Vertex point{ vertex.x(), vertex.y() };
        if (vertexIsInChunk(chunk.x, chunk.y, point)) {
            vertex.color(i++);
        }
    }

    chunk.verticeEdges.resize(i - 1);
    chunk.unconnectedVerticeEdges.resize(i - 1);

    for (auto& vertex : voronoi.vertices()) {
        auto vi = vertex.color();

        if (vi != 0) {
            auto* incident_edge = vertex.incident_edge();
            auto* it = incident_edge;
            size_t position = 0;
            do {
                it = it->rot_next();
                auto& edge = *it;
                // Can process edge and it's not yet processed via its twin
                if (edge.is_primary() && edge.is_finite() && edge.color() == 0)
                {
                    auto& vert0 = *edge.vertex0();
                    Vertex point0{ vert0.x(), vert0.y() };
                    auto& vert1 = *edge.vertex1();
                    Vertex point1{ vert1.x(), vert1.y() };

                    U8 chunkIndex0 = chunkIndex(chunk.x, chunk.y, point0);
                    U8 chunkIndex1 = chunkIndex(chunk.x, chunk.y, point1);
                    U32 vertIndex0 = vert0.color();
                    U32 vertIndex1 = vert1.color();

                    // Is one of the vertices outside this chunk?
                    if (chunkIndex0 != CURRENT_CHUNK_INDEX || chunkIndex1 != CURRENT_CHUNK_INDEX) {
                        // Should this edge be in this chunk?
                        auto decidingVertex = std::min(point0, point1);
                        U8 decidingIndex = chunkIndex(chunk.x, chunk.y, decidingVertex);
                        if (decidingIndex != CURRENT_CHUNK_INDEX) {
                            edge.color(1);
                            edge.twin()->color(1);
                            UnconnectedEdge ue{ point0, point1, decidingIndex, position++ };
                            chunk.unconnectedVerticeEdges[vi - 1].push_back(std::move(ue));
                            continue;
                        }
                        chunk.edgesConnectCandidates.push_back(chunk.edges.size());
                        if (chunkIndex0 != CURRENT_CHUNK_INDEX) {
                            vertIndex0 = findVertexIndex(findChunkWithChunkIndex(map, chunk, chunkIndex0), point0) + 1;
                        }
                        if (chunkIndex1 != CURRENT_CHUNK_INDEX) {
                            vertIndex1 = findVertexIndex(findChunkWithChunkIndex(map, chunk, chunkIndex1), point1) + 1;
                        }
                    }

                    // 0 and 1 are reserved numbers
                    edge.color(chunk.edges.size() + 2);
                    edge.twin()->color(chunk.edges.size() + 2);

                    chunk.edges.push_back({ { chunkIndex0, vertIndex0 - 1 },{ chunkIndex1, vertIndex1 - 1} });
                }
                if (edge.color() >= 2) {
                    // Add the edge index, subtract 1 since 1 was added to avoid 0
                    chunk.verticeEdges[vi - 1].push_back({ CURRENT_CHUNK_INDEX, (U32)edge.color() - 2 });
                    ++position;
                }
            } while (it != incident_edge);

        }
    }

    chunk.edgemetas.resize(chunk.edges.size());

    auto cellCount = chunk.cellPoints.size();
    chunk.cellEdges.resize(cellCount);
    chunk.unconnectedCellEdges.resize(cellCount);


    for (auto& cell : voronoi.cells()) {
        auto cellIndex = cell.source_index();

        // Cell is in this chunk
        if (cellIndex < cellCount) {
            auto* incident_edge = cell.incident_edge();
            auto* it = incident_edge;
            size_t position = 0;
            do {
                it = it->next();

                if (it->color() >= 2) {
                    // Add the edge index, subtract 2 since 0 and 1 were special numbers
                    chunk.cellEdges[cellIndex].push_back({ CURRENT_CHUNK_INDEX, (U32)it->color() - 2 });
                }
                else {
                    const auto& vert0 = *(it->vertex0());
                    Vertex point0{ vert0.x(), vert0.y() };
                    const auto& vert1 = *(it->vertex1());
                    Vertex point1{ vert1.x(), vert1.y() };

                    U8 chunkIndex0 = chunkIndex(chunk.x, chunk.y, point0);
                    U8 chunkIndex1 = chunkIndex(chunk.x, chunk.y, point1);
                    U32 vertIndex0 = vert0.color();
                    U32 vertIndex1 = vert1.color();

                    auto decidingVertex = std::min(point0, point1);
                    U8 decidingIndex = chunkIndex(chunk.x, chunk.y, decidingVertex);
                    UnconnectedEdge ue{ point0, point1, decidingIndex, position };
                    chunk.unconnectedCellEdges[cellIndex].push_back(std::move(ue));
                }

                ++position;
            } while (it != incident_edge);
        }
    }

    chunk.state = ChunkState::EDGES;
}

void connectEdges(ChunkMap& map, ChunkWithIndexes& chunk) {
    if (chunk.state >= ChunkState::CONNECTED_EDGES) {
        return;
    }
    addVoronoi(map, chunk);

    auto neighbours = neighbourChunks(map, chunk.x, chunk.y);

    for (auto* nei : neighbours) {
        addVoronoi(map, *nei);
    }

    for (int i = 0; i < chunk.verticeEdges.size(); ++i) {
        for (auto e : chunk.unconnectedVerticeEdges[i]) {
            const auto& echunk = findChunkWithChunkIndex(map, chunk, e.connectToChunk);
            EdgeIndex ei{ e.connectToChunk, (U32)findEdgeIndexCand(map, echunk, e.a, e.b) };
            chunk.verticeEdges[i].insert(chunk.verticeEdges[i].begin() + e.position, ei);
        }
    }

    for (int i = 0; i < chunk.cellEdges.size(); ++i) {
        for (auto e : chunk.unconnectedCellEdges[i]) {
            const auto& echunk = findChunkWithChunkIndex(map, chunk, e.connectToChunk);
            EdgeIndex ei{ e.connectToChunk, (U32)findEdgeIndex(map, echunk, e.a, e.b) };
            chunk.cellEdges[i].insert(chunk.cellEdges[i].begin() + e.position, ei);
        }
    }

    chunk.state = CONNECTED_EDGES;
}
