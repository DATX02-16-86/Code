#include <iostream>
#include <fstream>
#include "..\..\Simplex\simplex.h"
#include <math.h>

const int chunk_width = 10;
const int chunks_y = 100;
const int chunks_x = 100;

float point_z_values[chunk_width*chunks_y][chunk_width*chunks_x];

int heights[chunks_x][chunks_y] = { { 1, 2, 1, 5 },{ 3, 6, 2, 4 },{ 2, 6, 3, 6 },{ 3, 0, 1, 2 } };
//int heights[chunks_y][chunks_x] = { { 1, 2, 1 },{ 3, 6, 2 },{ 2, 6, 3 } };

// heights in order: (0,0), (1,0), (1,1), (0,1)
static float bilinearInterpolation(float x, float y, float* interpolationHeights)
{
	return interpolationHeights[0] * (1 - x) * (1 - y) + interpolationHeights[3] * x*(1 - y) + interpolationHeights[1] * (1 - x)*y + interpolationHeights[2] * x*y;
}

float calculate_height(int x, int y, int chunkX, int chunkY)
{
	float interpX = x + 0.5f; // shift x value to middle of voxel
	float interpY = y + 0.5f; // shift y value to middle of voxel
	float halfChunk = chunk_width / 2.0;

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

	return bilinearInterpolation(interpX / chunk_width, interpY / chunk_width, interpolationHeights);
}

int main() {

	for (int x = 0; x < chunks_x; x++)
	{
		for (int y = 0; y < chunks_y; y++)
		{
			heights[x][y] = 1;
		}
	}

	for (int chY = 1; chY < chunks_y - 1; chY++) {
		for (int chX = 1; chX < chunks_x - 1; chX++) {
			int current_height = heights[chY][chX];

			for (int y = 0; y < chunk_width; ++y) {
				for (int x = 0; x < chunk_width; ++x) {
					int true_x = x + (chunk_width * chX);
					int true_y = y + (chunk_width * chY);
					float z = Simplex::octave_noise(3, 0.005f, 0.05f, true_x, true_y) + calculate_height(x, y, chX, chY);
					point_z_values[true_y][true_x] = z;
				}
			}
		}
	}

	// basic file operations
	std::ofstream myfile;
	myfile.open("points.txt");

	for (int i = chunk_width; i < (chunks_x - 1)*chunk_width; ++i) {
		for (int j = chunk_width; j < (chunks_y - 1)*chunk_width; ++j) {
			myfile << point_z_values[i][j];
			myfile << ",";
		}

		myfile << std::endl;
	}

	myfile.close();

	return 0;
}