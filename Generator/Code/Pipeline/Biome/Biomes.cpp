#include "Biomes.h"

namespace generator {

	const BiomeId WeirdBiome::id = registerBiome(WeirdBiome::fillChunk);

	void WeirdBiome::fillChunk(Chunk& chunk, Pipeline& pipeline) {
		auto baseHeight = pipeline.data.get(BaseHeight);
		chunk.build([=](Voxel& current, Int x, Int y, Int z) -> Voxel {
			Size height = 0;
			if (baseHeight) height = baseHeight->get(x, y, z);

			U16 blockType = 0;

			int weirdnessHeight = 20;
			float desnityThreshold;
			// under a certain height it is solid
			if (z < height - weirdnessHeight) blockType = 1;
			else if (z <= height + weirdnessHeight) {
				//calculate density with simplex noise
				float density = Simplex::octave_noise(8, 0.5f, 0.5f, x, y, z);

				//scale with z
				density +=  density - (z - (height + weirdnessHeight)) / (2*weirdnessHeight);

				//determine whether its solid or air
				if (density > 0.6f) blockType = 1;
			}
			
			return Voxel{ blockType };
		});
	}

	const BiomeId PlainBiome::id = registerBiome(PlainBiome::fillChunk);

	void PlainBiome::fillChunk(Chunk& chunk, Pipeline& pipeline) {
		auto baseHeight = pipeline.data.get(BaseHeight);
		chunk.build([=](Voxel& current, Int x, Int y, Int z) -> Voxel {
			Size height = 0;
			if (baseHeight) height = baseHeight->get(x, y, z);

			U16 blockType = 0;

			height += Simplex::octave_noise(8, 0.0005f, 0.5f, x, y) * 5;

			if (z < height) blockType = 1;

			return Voxel{ blockType };
		});
	}
}

