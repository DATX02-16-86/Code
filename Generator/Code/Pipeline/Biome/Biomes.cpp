#include <d2d1helper.h>
#include "Biomes.h"
#include "../noise/simplex/simplex.h"
#include "../Pipeline.h"
#include "../Height/HeightStage.h"

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

    const BiomeId LayeredBiomeTest::id = registerBiome(LayeredBiomeTest::fillChunk);


    void LayeredBiomeTest::fillChunk(generator::Chunk &chunk, generator::Pipeline &pipeline)
    {

	}

	void fillChunkLayered(std::vector<generator::NoiseFunc> funcArray, std::vector<int> bounds,
									   int interpDepth, generator::Chunk &chunk)
    {

    }

    float NoiseLerp(NoiseFunc funcA, NoiseFunc funcB, float alpha, float x, float y, float z, int lowerBound, int upperBound)
    {
        return funcA(x, y, z, lowerBound, upperBound) * alpha + funcB(x, y, z, lowerBound, upperBound) * (1 - alpha);
    }
}

namespace biomeFunctions{

    float air(float x, float y, float z, int lowerBound, int upperBound) {
        return 0;
    }

    float bedRock(float x, float y, float z, int lowerBound, int upperBound) {
        return 1;
    }

    float plains(float x, float y, float z, int lowerBound, int upperBound) {

        float height = lowerBound + Simplex::octave_noise(8, 0.0005f, 0.5f, x, y) * 5;

        if (z < height)
            return 1;
        else
            return 0;
}

    float caves(float x, float y, float z, int lowerBound, int upperBound) {
        return 1;
    }

    float weirdLand(float x, float y, float z, int lowerBound, int upperBound) {

        // under a certain height it is solid
        if (z < lowerBound + 5) return 1;
        else {
            //calculate density with simplex noise
            float density = Simplex::octave_noise(8, 0.005f, 0.5f, x, y, z);

            //scale density depending on height
            float scale = 1.f;

            float halfHeight = (upperBound - lowerBound) * 0.5f;

            //decrease density with z over a certain height
            if (z > halfHeight) {
                scale = (z - halfHeight) / halfHeight;
                scale = scale * scale;
            }
            else{
                //increase density
                scale = (2 * ( halfHeight-z) / halfHeight);
				scale = scale * scale;
            }
            density -= scale;

            return std::max(0, std::min(density, 1));
        }
    }

    float floatingIslands(float x, float y, float z, int lowerBound, int upperBound) {
        return 0;
    }
}
