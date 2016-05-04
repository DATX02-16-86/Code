#include "Pipeline.h"
#include "Voxel.h"
#include "Biome/BiomeStage.h"

namespace generator {

static Segment toSegment(Area area) {
    return Segment {area.x, area.y, (U32)area.worldWidth(), (U32)area.worldHeight(), 1.f, 0};
}

TiledMatrix* Pipeline::Data::get(StreamId stream) {
    if(stream.id >= count) return nullptr;
    if(matrices[stream.id].isEmpty()) return nullptr;
    return matrices + stream.id;
}

TiledMatrix* Pipeline::Data::getOrCreate(StreamId stream, Size detail) {
    if(stream.id >= count) resize(stream.id + 1);
    if(matrices[stream.id].isEmpty()) matrices[stream.id].create(detail, stream.itemBits, tileSize);
    return matrices + stream.id;
}

void Pipeline::Data::resize(Size count) {
    matrices = (TiledMatrix*)realloc(matrices, sizeof(TiledMatrix) * count);
    for(Size i = this->count; i < count; i++) {
        new (matrices + i) TiledMatrix;
    }
    this->count = (U32)count;
}

struct RelevantVertex {landmass::Chunk* chunk; U32 index; landmass::Vertex position;};
struct RelevantEdge {landmass::Chunk* chunk; U32 index; landmass::Vertex a; landmass::Vertex b;};

F32 pointDistanceToLine(landmass::Vertex l1, landmass::Vertex l2, landmass::Vertex p) {
    auto d = Tritium::Math::absolute((l2.y - l1.y) * p.x - (l2.x - l1.x) * p.y + l2.x * l1.y - l1.x * l2.y);
    return d / (l2 - l1).length();
}

void Pipeline::fillChunk(Chunk& chunk) {
    auto& land = landmass.generate(chunk.area.x, chunk.area.y, seed);

    // Get the closest vertex and its direct neighbours, then calculate biome strengths for each voxel pillar.
	auto closestVertex = land.getClosestVertex(chunk.area.x, chunk.area.y);

    static const U32 kMaxNeighbours = 6;
    static const U32 kMaxEdges = 5;
    ArrayF<RelevantVertex, kMaxNeighbours> relevantVertices;
    ArrayF<RelevantEdge, kMaxEdges> relevantEdges;

    auto edgeStart = land.vertices[closestVertex];
    relevantVertices.push(RelevantVertex {&land, closestVertex, edgeStart});
    for(auto e: land.vertexEdges[closestVertex]) {
        auto edgeChunk = land.neighbour(landmass.matrix, e.chunkIndex);
        auto edge = edgeChunk.edges[e.index];
        landmass::Vertex edgeEnd;

        if(edge.a.chunkIndex || edge.a.index != closestVertex) {
            auto aChunk = edgeChunk.neighbour(landmass.matrix, edge.a.chunkIndex);
            edgeEnd = aChunk.vertices[edge.a.index];
            relevantVertices.push(RelevantVertex {&aChunk, edge.a.index, edgeEnd});
        } else {
            auto bChunk = edgeChunk.neighbour(landmass.matrix, edge.b.chunkIndex);
            edgeEnd = bChunk.vertices[edge.b.index];
            relevantVertices.push(RelevantVertex {&bChunk, edge.b.index, edgeEnd});
        }

        relevantEdges.push(RelevantEdge {&edgeChunk, e.index, edgeStart, edgeEnd});
    }

    // Generate the terrain for this chunk.
    if(auto biomes = data.get(Biomes)) {
        auto id = (BiomeId)biomes->get(chunk.area.x, chunk.area.y, 0);
        auto biome = findBiome(id);
        biome(chunk, *this);
    }
}

} // namespace generator