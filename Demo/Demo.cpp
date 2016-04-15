#include <stack>
#include "../Generator/Code/World/World.h"
#include "../noise/Simplex/simplex.h"

using namespace generator;
using namespace landmass;

enum class WaterType { land, sea, lake, river};
enum class Biome { sea, lake, beach, land};

Attribute height {32, AttributeType::Vertex};
Attribute moisture {32, AttributeType::Vertex};
Attribute waterType {2, AttributeType::Vertex};
Attribute biome {8, AttributeType::Cell};
Attribute cellHeight {32, AttributeType::Cell};

struct HeightGenerator: landmass::Generator {
    enum {Height, Moisture, Water};

    static constexpr float groupFrequency = 0.0003f;
    static constexpr float groupPersistence = 0.5f;

    // The maximum lake size we generate.
    static const U32 lakeSize = 200;

    HeightGenerator(): Generator({&height, &moisture, &waterType}) {}

    virtual void generate(landmass::Chunk& chunk, ChunkMatrix& matrix, I32 seed) override {
        NoiseContext noise(120);
        std::vector<bool> checkForLake(chunk.vertices.size());

        auto heightAttribute = attribute(Height);
        auto moistureAttribute = attribute(Moisture);
        auto waterAttribute = attribute(Water);

        // Calculate the height of each vertex.
        for(U32 i = 0; i < chunk.vertices.size(); i++) {
            auto v = chunk.vertices[i];

            float groupA = (Simplex::octave_noise(1, groupFrequency, groupPersistence, v.x, v.y, 0, noise) + 1.0f) * 2;
            float groupB = (Simplex::octave_noise(1, groupFrequency, groupPersistence, v.x, v.y, 1000, noise) + 1.0f) * 0;
            float groupC = (Simplex::octave_noise(1, groupFrequency, groupPersistence, v.x, v.y, 2000, noise) + 1.0f) * 0;
            float sum = groupA + groupB + groupC;
            float multiplier = 1 / (sum > 0.2f ? sum : 0.2f);

            float persistenceB = (Simplex::octave_noise(1, groupFrequency, groupPersistence, v.x, v.y, 2000, noise) + 1.0f) * 0.15f + 0.5f;

            float heightA = (Simplex::octave_noise(5, 0.003f, 0.5f, v.x, v.y, noise) + 0.5f);
            float heightB = Simplex::octave_noise(3, 0.0025f, persistenceB, v.x, v.y, noise) + 0.25f;
            float heightC = Simplex::octave_noise(3, 0.00002f, 0.5f, v.x, v.y, noise) + 0.5f;
            float height = (heightA * groupA + heightB * groupB + heightC * groupC) * multiplier;

            WaterType water = WaterType::land;
            float moisture = 0.f;
            bool isWater = height < 0.3;
            if(isWater) {
                water = WaterType::sea;
                moisture = 1.f;
            }
            checkForLake.push_back(isWater);

            chunk.attributes.set(heightAttribute, i, height);
            chunk.attributes.set(moistureAttribute, i, moisture);
            chunk.attributes.set(waterAttribute, i, water);
        }

        // Convert small seas to lakes
        for (size_t i = 0; i < checkForLake.size(); ++i) {
            if (checkForLake[i]) {
                std::stack<U32> stack;
                std::vector<U32> visitedVerts;
                stack.push(i);

                // Whole stack has to be emptied so we don't start work on connected water twice
                while(!stack.empty()) {
                    auto it = stack.top();
                    VertexIndex vi{0, it};
                    stack.pop();

                    for(EdgeIndex j : chunk.vertexEdges[it]) {
                        auto& edgeChunk = chunk.neighbour(matrix, j.chunkIndex);
                        auto ni = nextVertexIndex(vi, edgeChunk.edges[j.index]);
                        if(!ni.chunkIndex && checkForLake[ni.index]) {
                            checkForLake[ni.index] = false;
                            stack.push(ni.index);
                            visitedVerts.push_back(ni.index);
                        }
                    }
                }

                if(visitedVerts.size() <= lakeSize) {
                    for(U32 j : visitedVerts) {
                        chunk.attributes.set(waterAttribute, j, WaterType::lake);
                    }
                }
            }
        }
    }
};

struct BiomeGenerator: landmass::Generator {
    enum {BiomeType, Height, VertexHeight, VertexWater};

    BiomeGenerator(): Generator({&biome, &cellHeight, &height, &waterType}) {}

    virtual void generate(landmass::Chunk& chunk, ChunkMatrix& matrix, I32 seed) override {
        auto biomeAttribute = attribute(BiomeType);
        auto heightAttribute = attribute(Height);
        auto vertexHeightAttribute = attribute(VertexHeight);
        auto vertexWaterAttribute = attribute(VertexWater);

        // Calculate average height etc. for cell
        for(U32 i = 0; i < chunk.cellEdges.size(); ++i) {
            float totalHeight = 0;
            int count = 0;
            int land = 0;
            int sea = 0;
            int lake = 0;

            for(const auto& ei : chunk.cellEdges[i]) {
                auto& edgeChunk = chunk.neighbour(matrix, ei.chunkIndex);
                const auto& edge = edgeChunk.edges[ei.index];
                auto& vertexChunk = edgeChunk.neighbour(matrix, edge.a.chunkIndex);
                totalHeight += edgeChunk.attributes.get<float>(vertexHeightAttribute, i);

                // Count WaterTypes
                switch(edgeChunk.attributes.get<WaterType>(vertexWaterAttribute, i)) {
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

            chunk.attributes.set(biomeAttribute, i, biome);
            chunk.attributes.set(heightAttribute, i, count > 0 ? totalHeight / count : 0);
        }
    }
};

int main() {
    I32 seed = 1;
    World world(seed);

    world.pipeline.landmass += std::make_unique<HeightGenerator>();
    world.pipeline.landmass += std::make_unique<BiomeGenerator>();

    for(int x = 0; x < 20; x++) {
        for(int y = 0; y < 20; y++) {
            world.pipeline.landmass.generate(x, y, seed);
        }
    }

    exit(0);
    return 0;
}