#pragma once

class Terrain
{

public:

	Terrain(int chunks, int chunkSize, int seed);
	~Terrain();

	float calculate_height(int x, int y, int chunkX, int chunkY);

	void generateHeights();

	void generate2D(float** zvalues);

private:
	NoiseContext nc;
	int chunks;
	int chunkSize;
	int** heights;
};