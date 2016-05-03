#include <iostream>
#include <fstream>
#include "..\..\Simplex\simplex.h"
#include <math.h>
#include "..\Terrain.h"

void writePointsToFile2D(int chunkSize, int chunks, float **point_z_values)
{
	std::ofstream myfile;
	myfile.open("points.txt");
	myfile.clear();
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
}

void writePointsToFile3D(int chunkSize, int chunks, int height, bool *point_z_values)
{

	// basic file operations
	std::ofstream myfile;
	myfile.open("points3D.txt");

	myfile.clear();

	// Write all points to file
	for (int k = 0; k < height; ++k) {
		for (int i = chunkSize; i < (chunks - 1)*chunkSize; ++i) {
			for (int j = chunkSize; j < (chunks - 1)*chunkSize; ++j) {
				bool d = point_z_values[i * chunkSize * chunks * height + j * height + k];

				if (d)
				{
					myfile << 1;
				}
				else
				{
					myfile << 0;
				}

				if (j + 1 < (chunks - 1) * chunkSize)
					myfile << ",";
			}

			myfile << std::endl;
		}
		myfile << std::endl;
	}
	myfile.close();

	// Free memory for points
	free(point_z_values);
}

void generateAndWrite2D(int seed, int chunkSize, int chunks)
{

	// Allocate memory for points
	float **point_z_values = (float**)std::malloc(chunks * chunkSize * sizeof(float *));
	for (int i = 0; i < chunkSize * chunks; i++)
		point_z_values[i] = (float*)std::malloc(chunks * chunkSize * sizeof(float *));

	// Create perm table from seed (not used??)
	// NoiseContext nc = NoiseContext(seed);

	// Init and generate terrain
	Terrain t = Terrain(chunks, chunkSize, seed);
	t.generateHeights();
	t.generate2D(point_z_values);

	writePointsToFile2D(chunkSize, chunks, point_z_values);
}

void generateAndWrite2DTunnels(int seed, int chunkSize, int chunks)
{

	// Allocate memory for points
	float **point_z_values = (float**)std::malloc(chunks * chunkSize * sizeof(float *));
	for (int i = 0; i < chunkSize * chunks; i++)
		point_z_values[i] = (float*)std::malloc(chunks * chunkSize * sizeof(float *));

	// Create perm table from seed (not used??)
	// NoiseContext nc = NoiseContext(seed);

	// Init and generate terrain
	Terrain t = Terrain(chunks, chunkSize, seed);
	t.generateHeights();
	t.generate2DTunnels(point_z_values);

	// basic file operations
	std::ofstream myfile;
	myfile.open("points.txt");
	myfile.clear();
	// Write all points to file
	for (int i = chunkSize; i < (chunks - 1)*chunkSize; ++i) {
		for (int j = chunkSize; j < (chunks - 1)*chunkSize; ++j) {
			myfile << point_z_values[i][j];
			if( j + 1 < (chunks - 1)*chunkSize)
				myfile << ",";
		}

		myfile << std::endl;
	}
	myfile.close();

	// Free memory for points
	for (int i = 0; i < chunkSize * chunks; i++)
		free(point_z_values[i]);
	free(point_z_values);
}

void generateAndWrite3D(int seed, int chunkSize, int chunks, int height)
{
	// Allocate memory for points
	bool *point_z_values = (bool *)std::malloc(chunks * chunkSize * chunks * chunkSize * height * sizeof(bool *));

	// Create perm table from seed (not used??)
	// NoiseContext nc = NoiseContext(seed);

	// Init and generate terrain
	Terrain t = Terrain(chunks, chunkSize, seed);
	t.generateHeights();
	t.generate3D(point_z_values, height);

	// basic file operations
	std::ofstream myfile;
	myfile.open("points3D.txt");

	myfile.clear();

	// Write all points to file
	for (int k = 0; k < height; ++k) {
		for (int i = chunkSize; i < (chunks - 1)*chunkSize; ++i) {
			for (int j = chunkSize; j < (chunks - 1)*chunkSize; ++j) {
				bool d = point_z_values[i * chunkSize * chunks * height + j * height + k];

				if (d)
				{
					myfile << 1;
				}
				else
				{
					myfile << 0;
				}

				if (j + 1 < (chunks - 1) * chunkSize)
					myfile << ",";
			}

			myfile << std::endl;
		}
		myfile << std::endl;
	}
	myfile.close();

	// Free memory for points
	free(point_z_values);
}

void generateAndWrite3DWithBiomes(int seed, int chunkSize, int chunks, int height)
{
	// Allocate memory for points
	bool *point_z_values = (bool *)std::malloc(chunks * chunkSize * chunks * chunkSize * height * sizeof(bool *));

	// Create perm table from seed (not used??)
	// NoiseContext nc = NoiseContext(seed);

	// Init and generate terrain
	Terrain t = Terrain(chunks, chunkSize, height, seed);
	t.generateHeights();
	t.generateBiomes();
	t.generateFromBiomes(point_z_values, false);

	// basic file operations
	std::ofstream myfile;
	myfile.open("points3D.txt");

	myfile.clear();

	// Write all points to file
	for (int k = 0; k < height; ++k) {
		for (int i = chunkSize; i < (chunks - 1)*chunkSize; ++i) {
			for (int j = chunkSize; j < (chunks - 1)*chunkSize; ++j) {
				bool d = point_z_values[i * chunkSize * chunks * height + j * height + k];

				if (d)
				{
					myfile << 1;
				}
				else
				{
					myfile << 0;
				}

				if (j + 1 < (chunks - 1) * chunkSize)
					myfile << ",";
			}

			myfile << std::endl;
		}
		myfile << std::endl;
	}
	myfile.close();

	// Free memory for points
	free(point_z_values);
}

void generateAndWrite3DInterpolation(int seed, int chunkSize, int chunks, int height)
{
	int chunkVoxelAmount = chunks * chunkSize * chunks * chunkSize * height;
	// Allocate memory for points
	bool *point_z_values = (bool *)std::malloc(chunkVoxelAmount * sizeof(bool *));

	// Init and generate terrain
	Terrain t = Terrain(chunks, chunkSize, seed);
	t.generateHeights();
	t.GenerateMountains(point_z_values, height);
	std::vector <std::vector<std::vector<bool>>> d = t.convertTo3DArray(point_z_values, chunks, chunkSize, height);
	t.removeFloating(d, chunkVoxelAmount);

	writePointsToFile3D(chunkSize, chunks, height, point_z_values);
}

int main() 
{

	//const int seed = 23195;
	const int seed = 7;
	const int chunkSize = 32;
	const int chunks = 4;
	const int height = 32;
	generateAndWrite3DInterpolation(seed, chunkSize, chunks, height);

	//std::cin.get();

	return 0;
}

