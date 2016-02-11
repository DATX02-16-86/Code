
#ifndef GENERATOR_LANDMASSSTAGE_H
#define GENERATOR_LANDMASSSTAGE_H

#include "../Generator.h"
#include "../Pipeline.h"

namespace generator {

GeneratorStream(Landmass, 1);

struct LandmassGenerator: Generator {
    using Generator::Generator;

    void generate(const Segment& segment, IdMatrix** auxiliaries, Pipeline& pipeline) override;

    /**
     * Generates landmass data for a specific sample segment.
     * The default implementation just creates an endless continent without any other data.
     */
    virtual void generate(const Segment& segment, IdMatrix& map, IdMatrix** auxiliaries, Pipeline& pipeline) {
        for(Size x = 0; x < segment.width; x++) {
            for(Size y = 0; y < segment.height; y++) {
                map.set(x, y, segment.detail, 1);
            }
        }
    }
};

} // namespace generator

#endif //GENERATOR_LANDMASSSTAGE_H
