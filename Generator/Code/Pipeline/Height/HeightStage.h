
#ifndef GENERATOR_HEIGHTSTAGE_H
#define GENERATOR_HEIGHTSTAGE_H

#include "../Generator.h"

namespace generator {

DeclareStream(BaseHeight);

/**
 * Helper base class for height generators.
 * This will automatically send the height map to the overloaded generate().
 */
struct HeightGenerator: Generator {
    using Generator::Generator;
    static const Size kDefaultOceanHeight = 100;

    void generate(const Segment& segment, TiledMatrix** auxiliaries, Pipeline& pipeline) override;

    /**
     * Generates height data for a specific sample segment.
     * The default implementation uses the default ocean height as base.
     */
    virtual void generate(const Segment& segment, TiledMatrix& heightMap, TiledMatrix** auxiliaries, Pipeline& pipeline) {
        segment.map([&](auto x, auto y) {
            heightMap.set(x, y, segment.detail, kDefaultOceanHeight);
        });
    }
};

} // namespace generator

#endif //GENERATOR_HEIGHTSTAGE_H
