#include "Biomes.h"
#include "..\noise\simplex\simplex.h"

namespace generator {

	const BiomeId WeirdBiome::id = registerBiome(WeirdBiome::fillChunk);

	void WeirdBiome::fillChunk(Chunk& chunk, Pipeline& pipeline) {
		auto baseHeight = pipeline.data.get(BaseHeight);
		int weirdnessHeight = 20;

		chunk.build([=](Voxel& current, Int x, Int y, Int z) -> Voxel {
			Size height = 0;
			if (baseHeight) height = baseHeight->get(x, y, z);

			U16 blockType = 0;

			// under a certain height it is solid
			if (z < height - weirdnessHeight) blockType = 1;
			else {
				//calculate density with simplex noise
				float density = Simplex::octave_noise(8, 0.005f, 0.5f, x, y, z);

				//scale density depending on height
				float scale = 1.f;

				//decrease density with z over a certain height
				if (z > height + weirdnessHeight) {
					scale = (z - height) / ((float)height);
					scale = scale * scale;
				}
				else if (z < height) {
					//increase density 
					scale = (2 * ((float)z - height / 2) / height);
				}
				density -= scale;

				//determine whether its solid or air
				if (density > 0.3f) blockType = 1;
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

