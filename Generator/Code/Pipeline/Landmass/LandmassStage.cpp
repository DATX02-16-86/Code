#include "LandmassStage.h"
#include "../Pipeline.h"

namespace generator {

void LandmassGenerator::generate(const Segment& segment, IdMatrix** auxiliaries, Pipeline& pipeline) {
    auto map = pipeline.data.getOrCreate(Landmass, segment.detail);
    generate(segment, map->getTile(segment.x, segment.y), auxiliaries, pipeline);
}

} // namespace generator