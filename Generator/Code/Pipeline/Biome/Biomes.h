#include "BiomeStage.h"

namespace generator {

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