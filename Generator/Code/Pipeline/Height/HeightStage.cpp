
#include "HeightStage.h"
#include "../Pipeline.h"

namespace generator {

void HeightGenerator::generate(const Segment& segment, IdMatrix** auxiliaries, Pipeline& pipeline) {
    auto heightTiles = pipeline.data.getOrCreate(BaseHeight, segment.detail);
    generate(segment, heightTiles->getTile(segment.x, segment.y), auxiliaries, pipeline);
}

} // namespace generator
