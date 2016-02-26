
#include "HeightStage.h"
#include "../Pipeline.h"

namespace generator {

DefineStream(BaseHeight, 16);
	
void HeightGenerator::generate(const Segment& segment, TiledMatrix** auxiliaries, Pipeline& pipeline) {
    auto heightTiles = pipeline.data.getOrCreate(BaseHeight, segment.detail);
    generate(segment, *heightTiles, auxiliaries, pipeline);
}

} // namespace generator
