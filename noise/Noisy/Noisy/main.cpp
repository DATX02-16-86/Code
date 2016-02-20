#include <iostream>
#include <fstream>
#include "..\..\Simplex\simplex.h"
#include <math.h>

const int chunk_width = 10;
const int chunks_y = 10;
const int chunks_x = 10;

float point_z_values[chunk_width*chunks_y][chunk_width*chunks_x];



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

	NoiseContext nc = NoiseContext(23190);



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