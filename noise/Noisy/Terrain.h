#pragma once
#include "..\Simplex\simplex.h"

class Terrain
{

public:

	static const int PLAINS_OCTAVES = 4;
	static const float PLAINS_PERSISTANCE;
	static const int PLAINS_HM = 0;

	static const int MOUNTAINS_OCTAVES = 6;
	static const float MOUNTAINS_PERSISTANCE;
	static const int MOUNTAINS_HM = 1;

	Terrain(int chunks, int chunkSize, int seed);
	~Terrain();

	float calculate_height(int x, int y, int chunkX, int chunkY);

	void generateHeights();

	void generate2D(float** zvalues);

	void generate3D(bool* allValues, int height);

	void Generate3DCustom(bool* allValues, int height, int octaves, float persistance, int heightMult);

	bool fillVoxel(int baseHeight, int x, int y, int z, int height, int octaves, float persistance, int heightMult);

	void GenerateMountains(bool* allValues, int height);

	void GeneratePlains(bool* allValues, int height);

	void generateMountainsPlains(bool* allValues, int height);

	void generateMountainsPlainsInterpolated(bool* allValues, int height);

	void generate2DTunnels(float** zValues);

	void generate3DTunnels(bool* density, int height);

	void generate3DComposite(bool* density, int height);

	void generate3DCaverns(bool* density, int height);

	void generate3DCliffs(bool * density, int height);

	void generate3DSomething(bool* density, int height);
private:
	NoiseContext nc;
	int chunks;
	int chunkSize;
	int** chunkHeights;
};