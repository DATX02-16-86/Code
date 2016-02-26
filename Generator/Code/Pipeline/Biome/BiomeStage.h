
#ifndef GENERATOR_CLIMATESTAGE_H
#define GENERATOR_CLIMATESTAGE_H

#include "../Generator.h"
#include "../Voxel.h"

namespace generator {

struct Pipeline;

DeclareStream(Biomes);

/**
 * Helper base class for biome id generators.
 * This will automatically send the biome map to the overloaded generate().
 */
struct BiomeGenerator: Generator {
    using Generator::Generator;

    void generate(const Segment& segment, TiledMatrix** auxiliaries, Pipeline& pipeline) override;

    /**
     * Generates biome id data for a specific sample segment.
     * The default implementation just sets the default biome everywhere.
     */
    virtual void generate(const Segment& segment, TiledMatrix& map, TiledMatrix** auxiliaries, Pipeline& pipeline) {
        segment.map([&](auto x, auto y) {
            map.set(x, y, segment.detail, 0);
        });
    }
};

/// Currently, biome ids are represented as bytes. We may want to change this to 16-bit values
/// in the future if it turns out that more than 256 biomes are used in one world.
using BiomeId = U8;

/// Chunk generators are called through this interface.
using GenerateChunk = void(*)(Chunk&, Pipeline&);

/**
 * Registers a new Biome and returns its biome id.
 * This must be done for all biomes that are used through the pipeline.
 * If the returned biome id is stored in the biome stage,
 * the pipeline will automatically call the corresponding biome in the terrain stage.
 * @param fillChunk This is called to generate voxel data for this biome.
 * The function should fill the provided chunk using the pipeline state.
 */
BiomeId registerBiome(GenerateChunk generateChunk);

/**
 * Returns the biome generator at the provided id.
 */
GenerateChunk findBiome(BiomeId id);

/// This is the global default biome that is used when no specific biome is defined or the requested biome doesn't exist.
/// It creates a flat world at a low height.
struct DefaultBiome {
    static const BiomeId id;
    static void fillChunk(Chunk& chunk, Pipeline& pipeline);
};

} // namespace generator

#endif //GENERATOR_CLIMATESTAGE_H
