#pragma once
#include "..\Simplex\simplex.h"

class Terrain
{

public:

	Terrain(int chunks, int chunkSize, int seed);
	~Terrain();

	float calculate_height(int x, int y, int chunkX, int chunkY);

	void generateHeights();

	void generate2D(float** zvalues);

	void generate3D(bool* allValues, int height);

private:
	NoiseContext nc;
	int chunks;
	int chunkSize;
	int** chunkHeights;
};