
#ifndef GENERATOR_VORONOI_H
#define GENERATOR_VORONOI_H

#define BOOST_POLYGON_NO_DEPS
#define GRID_DIVISIONS 10

#include <boost/polygon/voronoi.hpp>
#include "Landmass.h"

typedef double coordinate_type;
typedef boost::polygon::point_data<coordinate_type> Point;
typedef boost::polygon::voronoi_diagram<double> VD;

static const U8 CURRENT_CHUNK_INDEX = 4;

struct Chunk {
    Chunk(int x, int y) : x(x), y(y), voronoi(std::make_shared<VD>()) {}

    int x;
    int y;
    std::vector<Point> points;
    std::vector<Point> neighbourPoints;
    std::shared_ptr<VD> voronoi;
    std::vector<vertexmeta> vertexmetas;
    std::vector<cellmeta> cellmetas;
    std::vector<edgemeta> edgemetas;
};

struct ChunkWithIndexes {
    int x;
    int y;
    ChunkState state;
    std::vector<Point> cellPoints;
    std::vector<std::vector<EdgeIndex>> cellEdges;
    std::vector<std::vector<UnconnectedEdge>> unconnectedCellEdges;
    std::vector<Vertex> vertices;
    std::vector<std::vector<EdgeIndex>> verticeEdges;
    std::vector<std::vector<UnconnectedEdge>> unconnectedVerticeEdges;
    std::vector<size_t> edgesConnectCandidates;
    std::vector<Edge> edges;
    std::vector<vertexmeta> vertexmetas;
    std::vector<cellmeta> cellmetas;
    std::vector<edgemeta> edgemetas;
};

typedef std::map<std::pair<int, int>, ChunkWithIndexes> ChunkMap;

ChunkWithIndexes& findChunkWithChunkIndex(ChunkMap& hexChunks, ChunkWithIndexes& chunk, U8 chunkIndex);
const ChunkWithIndexes& findChunkWithChunkIndex(const ChunkMap& hexChunks, const ChunkWithIndexes& chunk, U8 chunkIndex);

vertexmeta& getVertexMeta(ChunkMap& map, ChunkWithIndexes& c, VertexIndex i, void (*prepare) (ChunkMap&, ChunkWithIndexes&));

std::pair<Optional<vertexmeta>, Optional<vertexmeta>> getEdgeMetas(const boost::polygon::voronoi_edge<double>& edge, const std::vector<vertexmeta>& metadata);

VertexIndex neighbourVertexIndex(const ChunkMap& map, const ChunkWithIndexes& c, const Edge& e, const Vertex& v);

std::vector<Chunk*> neighbourChunks(int x, int y, std::vector<Chunk>& in);
std::vector<ChunkWithIndexes*> neighbourChunks(ChunkMap& map, int x, int y);

VertexIndex sharedVertexIndex(Edge first, Edge second, U8 secondChunkIndex);

U8 chunkIndex(const ChunkWithIndexes& chunk, const ChunkWithIndexes& chunk2);

std::pair<Point, Point> getEdgePoints(const boost::polygon::voronoi_edge<double>& edge, std::vector<Point>& points);

void connectEdges(ChunkMap& map, ChunkWithIndexes& chunk);
void vertexMeta(ChunkMap& map, ChunkWithIndexes& chunk);

inline VertexIndex nextVertexIndex(VertexIndex current, Edge next) {
    if(current == next.b) {
        return next.a;
    }
    return next.b;
}

inline int chunkSeed(int chunkX, int chunkY, int seed) {
    return (chunkX * 31 + chunkY * CHUNK_SIZE) * 31 * seed;
}

inline Vertex getVertex(ChunkMap& map, const ChunkWithIndexes& chunk, const VertexIndex& index) {
    return findChunkWithChunkIndex(map, chunk, index.chunkIndex).vertices[index.index];
}

#endif // GENERATOR_VORONOI_H
