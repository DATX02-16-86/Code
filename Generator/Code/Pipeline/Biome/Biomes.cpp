#include "Biomes.h"
#include "../noise/simplex/simplex.h"
#include "../Pipeline.h"
#include "../Height/HeightStage.h"

namespace generator {

	const static int Layer1 = 3;
	const static int Layer2Offset = -10;
	const static int Layer3Offset = 20;

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
		auto baseHeight = pipeline.data.get(BaseHeight);

		std::vector<generator::NoiseFunc> funcs;
		std::vector<int> bounds;

        // TODO: This doesn't work, but I can't compile my stuff otherwise.
        auto height = 10;

		// Z coordinate of end of layer
		bounds.push_back(Layer1); 						// Bedrock from bottom to 3
		bounds.push_back((int)(height + Layer2Offset)); 	// Caves from 3 to third of bheight
		bounds.push_back((int)(height + Layer3Offset));	// Ground from third of bheight to 90% of total height
		bounds.push_back(20);							// Air from 90% of top to the top (top 10%)

		funcs.push_back(biomeFunctions::bedRock);
		funcs.push_back(biomeFunctions::caves);
		funcs.push_back(biomeFunctions::plains);
		funcs.push_back(biomeFunctions::air);

		fillChunkLayered(funcs, bounds, 5, chunk);
	}

    const BiomeId LayeredBiomeTest::id = registerBiome(LayeredBiomeTest::fillChunk);

    void LayeredBiomeTest::fillChunk(generator::Chunk &chunk, generator::Pipeline &pipeline) {

	}

	void fillChunkLayered(
        std::vector<generator::NoiseFunc> funcArray,
        std::vector<int> bounds,
        int interpDepth,
        Chunk& chunk
    ) {
        auto area = chunk.area;
		int layer = 0;
		int layers = bounds.size();

		float density;
		float alpha;

		NoiseFunc currentFunc = funcArray[0];
		NoiseFunc lastFunc = funcArray[0];

		auto step = Size(1) << area.lod;
		auto x = area.x * area.width;
		auto y = area.y * area.height;
		auto z = area.z * area.depth;

		Size height = area.height;
		Size width = area.width;
		Size depth = area.depth;

		for(Size zi = 0; zi < depth; zi++) {
			if(zi >= bounds[layer]){
				layer++;
				currentFunc = funcArray[layer];
				lastFunc = funcArray[layer - 1];
			}

			if(layer > 0 && zi - bounds[layer] < interpDepth){
				alpha = ((float)zi - bounds[layer]) / interpDepth;
			} else {
				alpha = 0;
			}

			for(Size row = 0; row < height; row++) {
				for(Size column = 0; column < width; column++) {
					density  = NoiseLerp(currentFunc, lastFunc, alpha, x + column * step, y + row * step, z + zi * step, bounds[layer - 1], bounds[layer]);
					U16 blockType = (U16) (density > 0.5f);

					Voxel& voxel = chunk.at(column, row, zi);
					voxel = Voxel {blockType};
				}
			}
		}
    }

    float NoiseLerp(NoiseFunc funcA, NoiseFunc funcB, float alpha, float x, float y, float z, int lowerBound, int upperBound)
    {
		if(alpha == 0){
			return funcA(x,y,z,lowerBound,upperBound);
		}
        return funcA(x, y, z, lowerBound, upperBound) * (1-alpha) + funcB(x, y, z, lowerBound, upperBound) * alpha;
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

        float height = lowerBound + 10 + Simplex::octave_noise(8, 0.0005f, 0.5f, x, y) * 5;

        if(z < height)
            return 1;
        else
            return 0;
	}

    float caves(float x, float y, float z, int lowerBound, int upperBound) {
		float d = 0;

		float middle = (upperBound-lowerBound)/2.f;

		float f = 0.009f;
		float dz = -(z - middle) * (z - middle) / middle + middle + 0.1f;

		d = Simplex::octave_noise(5, f, 0.5f, x, y, dz) * 0.7f;
		d += Simplex::octave_noise(5, f, 0.5f, x, y, z) * 0.3f;

		return d +0.3f;
    }

    float weirdLand(float x, float y, float z, int lowerBound, int upperBound) {
        // under a certain height it is solid
        if(z < lowerBound + 5) return 1;
        else {
            //calculate density with simplex noise
            float density = Simplex::octave_noise(8, 0.005f, 0.5f, x, y, z);

            //scale density depending on height
            float scale = 1.f;

            float halfHeight = (upperBound - lowerBound) * 0.5f;

            //decrease density with z over a certain height
            if(z > halfHeight) {
                scale = (z - halfHeight) / halfHeight;
                scale = scale * scale;
            } else {
                //increase density
                scale = (2 * ( halfHeight-z) / halfHeight);
				scale = scale * scale;
            }
            density -= scale;

            return std::max(0.f, std::min(density, 1.f));
        }
    }

    float floatingIslands(float x, float y, float z, int lowerBound, int upperBound) {
        return 0;
    }
}
