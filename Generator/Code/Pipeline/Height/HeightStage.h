
#ifndef GENERATOR_HEIGHTSTAGE_H
#define GENERATOR_HEIGHTSTAGE_H

#include "../Generator.h"

namespace generator {

GeneratorStream(BaseHeight, 16);

/**
 * Helper base class for height generators.
 * This will automatically send the height map to the overloaded generate().
 */
struct HeightGenerator: Generator {
    using Generator::Generator;

    void generate(const Segment& segment, IdMatrix** auxiliaries, Pipeline& pipeline) override;

    /**
     * Generates height data for a specific sample segment.
     * The default implementation uses no base height.
     */
    virtual void generate(const Segment& segment, IdMatrix& heightMap, IdMatrix** auxiliaries, Pipeline& pipeline) {
        for(Size x = 0; x < segment.width; x++) {
            for(Size y = 0; y < segment.height; y++) {
                heightMap.set(x, y, segment.detail, 0);
            }
        }
    }
};

} // namespace generator

#endif //GENERATOR_HEIGHTSTAGE_H
