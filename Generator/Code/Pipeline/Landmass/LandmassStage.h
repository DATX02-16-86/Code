
#ifndef GENERATOR_LANDMASSSTAGE_H
#define GENERATOR_LANDMASSSTAGE_H

#include "../Generator.h"
#include "../Pipeline.h"

namespace generator {

GeneratorStream(Landmass, 1);

/**
 * Helper base class for landmass generators.
 * This will automatically send the landmass map to the overloaded generate().
 */
struct LandmassGenerator: Generator {
    using Generator::Generator;

    void generate(const Segment& segment, IdMatrix** auxiliaries, Pipeline& pipeline) override;

    /**
     * Generates landmass data for a specific sample segment.
     * The default implementation just creates an endless continent without any other data.
     */
    virtual void generate(const Segment& segment, IdMatrix& map, IdMatrix** auxiliaries, Pipeline& pipeline) {
        segment.map([&](auto x, auto y) {
            map.set(x, y, segment.detail, 1);
        });
    }
};

} // namespace generator

#endif //GENERATOR_LANDMASSSTAGE_H
