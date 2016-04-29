#pragma once
#include "..\Simplex\simplex.h"

struct BiomeRepresentation {
	
	BiomeRepresentation(float mountainRatio, float plainRatio, float ridgeRatio, float weirdRatio, float pillarRatio, float caveRatio) 
		: mountainness(mountainRatio), plainness(plainRatio), ridgyness(ridgeRatio), weirdness(weirdRatio), pillarness(pillarRatio), caviness(caveRatio) {};
	BiomeRepresentation() {};
	float ridgyness;
	float mountainness;
	float plainness;
	float weirdness;
	float pillarness;
	float caviness;
};

static const BiomeRepresentation mountainBiome = BiomeRepresentation(1.f, 0.f, 0.f, 0.f, 0.f,1.f);
static const BiomeRepresentation plainBiome = BiomeRepresentation(0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
static const BiomeRepresentation ridgyPlainsBiome = BiomeRepresentation(0.f, 1.f, 1.f, 0.f, 0.f, 1.f);
static const BiomeRepresentation weirdBiome = BiomeRepresentation(0.f, 0.f, 0.f, 1.f,0.f, 1.f);
static const BiomeRepresentation pillarBiome = BiomeRepresentation(0.f, 0.f, 0.f, 0.f, 1.f, 1.f);

class Terrain
{

private:

	int interpolate(float aP, int a, int b);
	float interpolate(float aP, float a, float b);

// Universal biome
	bool worldBiomeFunction(int x, int y, int z, float baseHeight, BiomeRepresentation biome);
	float plainHeightOffset(int x, int y);
	float mountainHeightOffset(int x, int y);
	float ridgeHeightOffset(int x, int y);
	float weirdDensity(int x, int y, int z, float baseHeight, float weirdnessHeight);
	float pillarDensity(int x, int y, int z, float baseHeight, float pillarParameter);
	float caveDensity(int x, int y, int z, float baseHeight, float caveParameter);
	// End of universal biome
public:

	static const int PLAINS_OCTAVES = 4;
	static const float PLAINS_PERSISTANCE;
	static const int PLAINS_HM = 0;
	static const int PLAINS_BM = 0;

	static const int MOUNTAINS_OCTAVES = 6;
	static const float MOUNTAINS_PERSISTANCE;
	static const int MOUNTAINS_HM = 5;
	static const int MOUNTAINS_BM = 0;

	Terrain(int chunks, int chunkSize, int seed);
	Terrain(int chunks, int chunkSize, int chunkSizeZ, int seed);
	~Terrain();

	float calculateHeight(int x, int y, int chunkX, int chunkY);

	BiomeRepresentation calculateBiome(int x, int y, int chunkX, int chunkY);

	void generateHeights();

	void generateBiomes();
	
	void generateFromBiomes(bool* allValues, bool interpolate);

	void generate2D(float** zvalues);

	void generate3D(bool* allValues, int height);

	void Generate3DCustom(bool* allValues, int height, int octaves, float persistance, int heightMult);

	bool fillVoxel(int baseHeight, int x, int y, int z, int height, int octaves, float persistance, int heightMult, int baseMult);

	float getVoxelDensity(int baseHeight, int x, int y, int z, int height, int octaves, float persistance, int heightMult, int baseMult);

	void GenerateMountains(bool* allValues, int height);

	void GeneratePlains(bool* allValues, int height);

	void generateMountainsPlains(bool* allValues, int height);

	void generateMountainsPlainsInterpolated(bool* allValues, int height);

	void generateMountainsPlainsInterpolatedD(bool* allValues, int height);

	void generate2DTunnels(float** zValues);

	void generate3DTunnels(bool* density, int height);

	void generate3DComposite(bool* density, int height);

	void generate3DCaverns(bool* density, int height);

	void generate3DCliffs(bool * density, int height);

	void generate3DSomething(bool* density, int height);

	void removeFloating(bool* density);

private:
	NoiseContext nc;
	int chunks;
	int chunkSize;
	int chunkSizeZ;
	int** chunkHeights;
	BiomeRepresentation** biomes;
};