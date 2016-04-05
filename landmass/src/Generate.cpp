
#include "Voronoi.h"
#include "../../noise/Simplex/simplex.h"
#include <stack>

void generateVertexMeta(ChunkMap& map, ChunkWithIndexes& chunk) {
    if (chunk.state >= ChunkState::VERTEXMETA) {
        return;
    }
    connectEdges(map, chunk);

    NoiseContext a(120);
    std::vector<bool> checkForLake;
    checkForLake.reserve(chunk.vertices.size());

    // Calculate height of vertex
    for (size_t i = 0; i < chunk.vertices.size(); ++i)
    {
        const auto& it = chunk.vertices[i];
        float groupA = (Simplex::octave_noise(1, 0.0003f, 0.5f, float(it.x), float(it.y), 0, a) + 1.0f) * 2;
        float groupB = (Simplex::octave_noise(1, 0.0003f, 0.5f, float(it.x), float(it.y), 1000, a) + 1.0f) * 0;
        float groupC = (Simplex::octave_noise(1, 0.0003f, 0.5f, float(it.x), float(it.y), 2000, a) + 1.0f) * 0;
        float sum = groupA + groupB + groupC;
        float multiplier = 1 / (sum > 0.2f ? sum : 0.2f);

        float persistanceB = (Simplex::octave_noise(1, 0.0003f, 0.5f, float(it.x), float(it.y), 2000, a) + 1.0f) * 0.15f + 0.5f;

        float heightA = (Simplex::octave_noise(5, 0.003f, 0.5f, float(it.x), float(it.y), a) + 0.5f);
        float heightB = Simplex::octave_noise(3, 0.0025f, persistanceB, float(it.x), float(it.y), a) + 0.25f;
        float heightC = Simplex::octave_noise(3, 0.00002f, 0.5f, float(it.x), float(it.y), a) + 0.5f;
        float height = (heightA * groupA + heightB * groupB + heightC * groupC) * multiplier;
        WaterType wt = WaterType::land;
        bool isWater = height < 0.3;
        double moisture = 0;
        if (isWater) {
            wt = WaterType::sea;
            moisture = 1;
        }
        checkForLake.push_back(isWater);
        chunk.vertexmetas.push_back({ height, wt, moisture });
    }

    //Has to be small for now since chunks aren't handled well
    size_t LAKE_SIZE = 200;

    // Convert small seas to lakes
    for (size_t i = 0; i < checkForLake.size(); ++i) {
        if (checkForLake[i]) {
            std::stack<int> stack;
            std::vector<int> visitedVerts;
            stack.push(i);
            // Whole stack has to be emptied so we don't start work on connected water twice
            while (!stack.empty()) {
                int it = stack.top();
                VertexIndex vi{ CURRENT_CHUNK_INDEX, (U32)it };
                stack.pop();
                for (EdgeIndex j : chunk.verticeEdges[it]) {
                    auto& edgeChunk = findChunkWithChunkIndex(map, chunk, j.chunkIndex);
                    auto ni = nextVertexIndex(vi, edgeChunk.edges[j.index]);
                    if (ni.chunkIndex == CURRENT_CHUNK_INDEX && checkForLake[ni.index]) {
                        checkForLake[ni.index] = false;
                        stack.push(ni.index);
                        visitedVerts.push_back(ni.index);
                    }
                }
            }

            if (visitedVerts.size() <= LAKE_SIZE) {
                for (int j : visitedVerts) {
                    chunk.vertexmetas[j].wt = WaterType::lake;
                }
            }

        }
    }

    chunk.state = ChunkState::VERTEXMETA;
}

// Sets isRiver recursively and greedily for the edge that has the most slope.
// v is the index of the vertex that is the water source.
void runRiver(ChunkMap& hexChunks, ChunkWithIndexes& chunk, U32 v) {
    const auto& vertex = chunk.vertices[v];
    vertexmeta& bestMeta = chunk.vertexmetas[v];
    VertexIndex bestVertex;
    EdgeIndex bestEdge;
    bool anyBest = false;

    for (EdgeIndex i : chunk.verticeEdges[v]) {
        auto& edgeChunk = findChunkWithChunkIndex(hexChunks, chunk, i.chunkIndex);
        auto j = neighbourVertexIndex(hexChunks, edgeChunk, edgeChunk.edges[i.index], vertex);
        auto& meta = getVertexMeta(hexChunks, edgeChunk, j, vertexMeta);
        if (meta.height < bestMeta.height) {
            anyBest = true;
            bestVertex = j;
            bestMeta = meta;
            bestEdge = i;
        }
    }

    if (anyBest) {
        auto& edgeChunk = findChunkWithChunkIndex(hexChunks, chunk, bestEdge.chunkIndex);
        auto& edgeMeta = edgeChunk.edgemetas[bestEdge.index];
        auto& aMeta = getVertexMeta(hexChunks, edgeChunk, edgeChunk.edges[bestEdge.index].a, vertexMeta);
        auto& bMeta = getVertexMeta(hexChunks, edgeChunk, edgeChunk.edges[bestEdge.index].b, vertexMeta);
        bool aIsLand = aMeta.wt == WaterType::land;
        bool bIsLand = bMeta.wt == WaterType::land;
        bool bestIsLand = bestMeta.wt == WaterType::land;

        if (aIsLand) {
            aMeta.wt = WaterType::river;
            aMeta.moisture = 1;
        }
        if (bIsLand) {
            bMeta.wt = WaterType::river;
            bMeta.moisture = 1;
        }

        if (edgeMeta.isRiver) {
            // Early return if it catches an edge already in a river
            return;
        }
        edgeMeta.isRiver = true;
        if (!bestIsLand) {
            // Don't run rivers into water
            return;
        }
        runRiver(hexChunks, findChunkWithChunkIndex(hexChunks, edgeChunk, bestVertex.chunkIndex), bestVertex.index);
    }
}


void addRivers(ChunkMap& map, ChunkWithIndexes& chunk) {
    if(chunk.state >= ChunkState::RIVERS) {
        return;
    }

    generateVertexMeta(map, chunk);
    chunk.state = ChunkState::RIVERS;

    // Add rivers
    std::vector<int> sourceCandidates;
    for(U32 i = 0; i < chunk.vertices.size(); ++i) {
        auto height = chunk.vertexmetas[i].height;
        if(height > 0.8 && height < 0.9) {
            sourceCandidates.push_back(i);
        }
    }

    std::vector<int> sources;
    for(int i : sourceCandidates) {
        const auto& vertex = chunk.vertices[i];
        if(!std::any_of(sources.begin(), sources.end(), [&chunk, &vertex](int j) {
            return chunk.vertices[j].x <= vertex.x + 100
              && chunk.vertices[j].x >= vertex.x - 100
              && chunk.vertices[j].y <= vertex.y + 100
              && chunk.vertices[j].y >= vertex.y - 100;
        })) {
            sources.push_back(i);
        }
    }

    for(auto i : sources) {
        runRiver(map, chunk, i);
    }

    chunk.state = ChunkState::RIVERS;
}

void addMoisture(ChunkMap& map, ChunkWithIndexes& chunk) {
    if(chunk.state >= ChunkState::MOISTURE) {
        return;
    }
    addRivers(map, chunk);

    std::stack<std::pair<ChunkWithIndexes*, U32>> stack;
    for(size_t i = 0; i < chunk.vertices.size(); ++i) {
        auto wt = chunk.vertexmetas[i].wt;
        if(wt == WaterType::lake || wt == WaterType::river) {
            stack.push({ &chunk, i });
        }
    }

    while(!stack.empty()) {
        auto pair = stack.top();
        auto* pairChunk = pair.first;
        VertexIndex vi{ CURRENT_CHUNK_INDEX, pair.second };
        stack.pop();
        double vertMoist = pairChunk->vertexmetas[pair.second].moisture;

        for(const auto& i : pairChunk->verticeEdges[pair.second]) {
            auto& edgeChunk = findChunkWithChunkIndex(map, *pairChunk, i.chunkIndex);
            const auto ni = nextVertexIndex(vi, edgeChunk.edges[i.index]);
            auto& niChunk = findChunkWithChunkIndex(map, edgeChunk, ni.chunkIndex);
            addRivers(map, niChunk);
            auto& meta = niChunk.vertexmetas[ni.index];
            double newMoist = vertMoist * 0.9;
            if (meta.moisture < newMoist && newMoist > 0.05) {
                meta.moisture = newMoist;
                stack.push({ &niChunk, ni.index });
            }
        }
    }
    chunk.state = ChunkState::MOISTURE;
}

void addMoistureNeighbours(ChunkMap& map, ChunkWithIndexes& chunk) {
    if(chunk.state >= ChunkState::MOISTURE_NEIGHBOURS) {
        return;
    }

    const auto& neighbours = neighbourChunks(map, chunk.x, chunk.y);

    for(auto* nei : neighbours) {
        addMoisture(map, *nei);
    }

    chunk.state = ChunkState::MOISTURE_NEIGHBOURS;
}

void addBiomes(ChunkMap& map, ChunkWithIndexes& chunk) {
    if(chunk.state >= ChunkState::BIOMES) {
        return;
    }
    addMoistureNeighbours(map, chunk);

    // Calculate avarage height etc. for cell
    for(size_t i = 0; i < chunk.cellEdges.size(); ++i) {
        double totalHeight = 0;
        double totalMoisture = 0;
        int count = 0;
        int land = 0;
        int sea = 0;
        int lake = 0;
        for (const auto& ei : chunk.cellEdges[i]) {
            auto& edgeChunk = findChunkWithChunkIndex(map, chunk, ei.chunkIndex);
            const auto& edge = edgeChunk.edges[ei.index];
            auto& vertexChunk = findChunkWithChunkIndex(map, edgeChunk, edge.a.chunkIndex);
            const auto& vertex = getVertexMeta(map, edgeChunk, edge.a, addMoisture);
            totalHeight += vertex.height;
            totalMoisture += vertex.moisture;

            //Count WaterTypes
            switch (vertex.wt) {
                case WaterType::land:
                    land++;
                    break;
                case WaterType::sea:
                    sea++;
                    break;
                case WaterType::lake:
                    lake++;
                    break;
                case WaterType::river:
                    land++;
                    break;
            }
            ++count;
        }

        double avarageHeight = count > 0 ? totalHeight / count : 0;
        double avarageMoisture = count > 0 ? totalMoisture / count : 0;
        Biome biome;
        if(sea == count) {
            biome = Biome::sea;
        } else if(lake == count) {
            biome = Biome::lake;
        } else {
            if(sea > 1) {
                biome = Biome::beach;
            } else {
                biome = Biome::land;
            }
        }

        chunk.cellmetas.push_back({ avarageHeight, biome, avarageMoisture });
    }

    chunk.state = ChunkState::BIOMES;
}
