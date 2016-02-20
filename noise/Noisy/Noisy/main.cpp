#include <iostream>
#include <fstream>
#include "..\..\Simplex\simplex.h"
#include <math.h>
#include "..\Terrain.h"

int main() {

	const int seed = 23195;
	const int chunkSize = 10;
	const int chunks = 10;

	// Allocate memory for points
	float **point_z_values = (float**) std::malloc(chunks * chunkSize * sizeof(float * ));
	for (int i = 0; i < chunkSize * chunks; i++)
		point_z_values[i] = (float*) std::malloc(chunks * chunkSize * sizeof(float *));

	// Create perm table from seed
	NoiseContext nc = NoiseContext(seed);

	// Init and generate terrain
	Terrain t = Terrain(chunks, chunkSize, seed);
	t.generateHeights();
	t.generate2D(point_z_values);

	// basic file operations
	std::ofstream myfile;
	myfile.open("points.txt");

	// Write all points to file
	for (int i = chunkSize; i < (chunks - 1)*chunkSize; ++i) {
		for (int j = chunkSize; j < (chunks - 1)*chunkSize; ++j) {
			myfile << point_z_values[i][j];
			myfile << ",";
		}

		myfile << std::endl;
	}
	myfile.close();

	// Free memory for points
	for (int i = 0; i < chunkSize * chunks; i++)
		free(point_z_values[i]);
	free(point_z_values);

	return 0;
}