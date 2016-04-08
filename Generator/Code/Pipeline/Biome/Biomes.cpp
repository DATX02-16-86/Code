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

		Size chunkHeight = chunk.area.depth;
		Size baseHeight = pipeline.data.get(BaseHeight);

		std::vector<generator::NoiseFunc> funcs;
		std::vector<int> bounds;

		// Z coordinate of end of layer
		bounds.add(3); 					// Bedrock
		bounds.add(baseHeight * 3/4); 	// Caves
		bounds.add(height * 3/4);		// Ground, Rest air

		funcs.add(biomeFunctions::bedRock);
		funcs.add(biomeFunctions::caves);
		funcs.add(biomeFunctions::plains());
		funcs.add(biomeFunctions::air);

		fillChunkLayered(funcs, bounds, 5, chunk);
	}

    const BiomeId LayeredBiomeTest::id = registerBiome(LayeredBiomeTest::fillChunk);


    void LayeredBiomeTest::fillChunk(generator::Chunk &chunk, generator::Pipeline &pipeline)
    {

	}

	void fillChunkLayered(std::vector<generator::NoiseFunc> funcArray, std::vector<int> bounds,
									   int interpDepth, generator::Chunk &chunk)
    {

        height = pipeline.get(height);

        int height = chunk.area.depth;
        int baseHeight = pipeline.data.get(BaseHeight);

        // Use noise up to coordinate
        // Underground
        bounds.add(3); // Bedrock
        bounds.add(baseHeight * 3 / 4); // Caves
        //Above ground
        bounds.add(height * 3 / 4); // Plains
        // Rest Air to height

        //funcarray....
        arr.add (Bedrock)
        arr.add (BigCaves)
        arr.add (Plains)
        arr.add (Air)

        InterpMastery(arr, bounds, chunk);

    }

    float NoiseLerp(NoiseFunc funcA, NoiseFunc funcB, float alpha, float x, float y, float z, int baseHeight)
    {
        return funcA(x, y, z, baseHeight) * alpha + funcB(x, y, z, baseHeight) * (1 - alpha);
    }

}

namespace biomeFunctions{
    float bedRock(float x, float y, float z, int lowerBound, int upperBound) {
        return 0;
    }

    float plains(float x, float y, float z, int lowerBound, int upperBound) {
        return 0;
    }

    float caves(float x, float y, float z, int lowerBound, int upperBound) {
        return 0;
    }

    float weirdLand(float x, float y, float z, int lowerBound, int upperBound) {
        return 0;
    }

    float floatingIslands(float x, float y, float z, int lowerBound, int upperBound) {
        return 0;
    }
}
