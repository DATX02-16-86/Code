#include "Terrain.h"
#include "Tools.h"
#include "..\Simplex\simplex.h"
#include <stdlib.h>


Terrain::Terrain(int chunks, int chunkSize, int seed)
{
	this->nc = NoiseContext(seed);
	this->chunks = chunks;
	this->chunkSize = chunkSize;
}

Terrain::~Terrain()
{
	for (int i = 0; i < chunks; ++i) {
		delete[] heights[i];
	}
	delete[] heights;
}

float Terrain::calculate_height(int x, int y, int chunkX, int chunkY)
{
	float interpX = x + 0.5f; // shift x value to middle of voxel
	float interpY = y + 0.5f; // shift y value to middle of voxel
	float halfChunk = chunkSize / 2.0;

	if (x < halfChunk) //left
	{
		interpX += halfChunk; // increase x coordinate by half a chunk
		chunkX -= 1; // shift chunk coordinate to the left
	}
	else {
		interpX -= halfChunk;
	}
	if (y < halfChunk) // top
	{
		interpY += halfChunk; // increase y coordinate by half a chunk
		chunkY -= 1; // shift chunk coordinate to the top
	}
	else
	{
		interpY -= halfChunk;
	}
	float interpolationHeights[4] = { heights[chunkY][chunkX], heights[chunkY + 1][chunkX], heights[chunkY + 1][chunkX + 1], heights[chunkY][chunkX + 1] };

	return Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationHeights);
}

void Terrain::generateHeights()
{
	heights = new int*[chunks];
	for (int i = 0; i < chunks; i++)
	{
		heights[i] = new int[chunks];
	}
	for (int x = 0; x < chunks; x++)
	{
		for (int y = 0; y < chunks; y++)
		{
			heights[x][y] = (int)(Simplex::octave_noise(4, 0.02f, 0.5, x, y, nc) * 5 + 1);
		}
	}
}

void Terrain::generate2D(float** zValues)
{
	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			int current_height = heights[chY][chX];

			for (int y = 0; y < chunkSize; ++y) {
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					int true_y = y + (chunkSize * chY);
					float z = Simplex::octave_noise(8, 0.0005f, 0.5f, true_x, true_y, nc) * 5 + calculate_height(x, y, chX, chY);
					zValues[true_y][true_x] = z;
				}
			}
		}
	}
}
