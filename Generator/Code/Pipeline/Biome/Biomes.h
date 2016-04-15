#include "BiomeStage.h"

namespace generator {

	using NoiseFunc = float(*)(float, float, float, int, int);

	/// Helper function to fill a chunk that using multiple layers with custom bounds
	static void fillChunkLayered(std::vector<NoiseFunc> funcArray, std::vector<int> bounds, int interpDepth, Chunk& chunk);

	/// Linear interpolation between two functions, 0 means only funcA, 1 means only funcB
	float NoiseLerp(NoiseFunc funcA, NoiseFunc funcB, float alpha, float x, float y, float z, int lowerBound, int upperBound);

	/// Biome that is used to test the layered biome implementation
	struct LayeredBiomeTest{
		static const BiomeId id;
		static void fillChunk(Chunk& chunk, Pipeline& pipeline);
	};

	/// Plain biome consists of low amplitude slow moving noise function. Nothing above and caves below.
	struct PlainBiome {
		static const BiomeId id;
		static void fillChunk(Chunk& chunk, Pipeline& pipeline);
	};

	/// Weird biome produces cool but slightly unrealistic terrain by using 3d simplex noise.
	struct WeirdBiome {
		static const BiomeId id;
		static void fillChunk(Chunk& chunk, Pipeline& pipeline);
	};
}

namespace biomeFunctions{

	/// Produces only air (no voxels)
	float air(float x, float y, float z, int lowerBound, int upperBound);

	/// Produces only filled voxels (usually used in the bottom of a chunk)
	float bedRock(float x, float y, float z, int lowerBound, int upperBound);

	/// Calculates a terrain height and then fills everything below it and nothing
	/// above which produces plain-like environments
	float plains(float x, float y, float z, int lowerBound, int upperBound);

	/// Caves (usually placed underground)
	float caves(float x, float y, float z, int lowerBound, int upperBound);

	/// unrealistic 3d terrain (cannot be represented with a heightmap)
	float weirdLand(float x, float y, float z, int lowerBound, int upperBound);

	// floating chunks of rock with air between
	float floatingIslands(float x, float y, float z, int lowerBound, int upperBound);
}