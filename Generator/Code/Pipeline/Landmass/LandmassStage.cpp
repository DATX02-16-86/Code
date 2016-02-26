#include "LandmassStage.h"
#include "../Pipeline.h"

namespace generator {

DefineStream(Landmass, 1);
	
void LandmassGenerator::generate(const Segment& segment, TiledMatrix** auxiliaries, Pipeline& pipeline) {
    auto map = pipeline.data.getOrCreate(Landmass, segment.detail);
    generate(segment, *map, auxiliaries, pipeline);
}

} // namespace generator