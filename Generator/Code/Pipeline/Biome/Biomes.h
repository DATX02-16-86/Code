#include "BiomeStage.h"

namespace generator {

	using NoiseFunc = float(*)(float, float, float, int, int );

	static void fillChunkLayered(std::vector<NoiseFunc> funcArray, std::vector<int> bounds, int interpDepth, Chunk& chunk);

	float NoiseLerp(NoiseFunc funcA, NoiseFunc funcB, float alpha, float x, float y, float z, int baseHeight);

	struct LayeredBiomeTest{
		static const BiomeId id;
		static void fillChunk(Chunk& chunk, Pipeline& pipeline);
	};

	/// This is the global default biome that is used when no specific biome is defined or the requested biome doesn't exist.
	/// It creates a flat world at a low height.
	struct PlainBiome {
		static const BiomeId id;
		static void fillChunk(Chunk& chunk, Pipeline& pipeline);
	};

	/// This is the global default biome that is used when no specific biome is defined or the requested biome doesn't exist.
	/// It creates a flat world at a low height.
	struct WeirdBiome {
		static const BiomeId id;
		static void fillChunk(Chunk& chunk, Pipeline& pipeline);
	};
}

namespace biomeFunctions{

	float air(float x, float y, float z, int lowerBound, int upperBound);

	float bedRock(float x, float y, float z, int lowerBound, int upperBound);

	float plains(float x, float y, float z, int lowerBound, int upperBound);

	float caves(float x, float y, float z, int lowerBound, int upperBound);

	float weirdLand(float x, float y, float z, int lowerBound, int upperBound);

	float floatingIslands(float x, float y, float z, int lowerBound, int upperBound);
}