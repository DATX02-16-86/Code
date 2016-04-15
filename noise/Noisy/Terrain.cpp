#include "Terrain.h"
#include "Tools.h"
#include "..\Simplex\simplex.h"
#include <stdlib.h>
#include <iostream>
#include <algorithm>

//Init constants
const float Terrain::PLAINS_PERSISTANCE = 0.3f;
const float Terrain::MOUNTAINS_PERSISTANCE = 0.55f;

Terrain::Terrain(int chunks, int chunkSize, int seed)
{
	this->nc = NoiseContext(seed);
	this->chunks = chunks;
	this->chunkSize = chunkSize;
}

Terrain::~Terrain()
{
	for (int i = 0; i < chunks; ++i) {
		delete[] chunkHeights[i];
	}
	delete[] chunkHeights;
}

int Terrain::interpolate(float aP, int a, int b) {
	return (int)(((float)a*aP) + ((float)b*(1.0f  - aP)) + 0.5f);
}

float Terrain::interpolate(float aP, float a, float b) {
	//float tAP = std::max(aP, 1.0f);
	float val = a*aP + b*(1 - aP);
	//printf("%g\n", val);
	return val;
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
	float interpolationHeights[4] = { chunkHeights[chunkY][chunkX], chunkHeights[chunkY + 1][chunkX], chunkHeights[chunkY + 1][chunkX + 1], chunkHeights[chunkY][chunkX + 1] };

	return Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationHeights);
}

void Terrain::generateHeights()
{
	chunkHeights = new int*[chunks];
	for (int i = 0; i < chunks; i++)
	{
		chunkHeights[i] = new int[chunks];
	}
	for (int x = 0; x < chunks; x++)
	{
		for (int y = 0; y < chunks; y++)
		{
			//chunkHeights[x][y] = (int)((Simplex::octave_noise(4, 0.02f, 0.5f, x, y, nc) + 1) * 5);
			chunkHeights[x][y] = 7;
		}
	}
}

void Terrain::generate2D(float** zValues)
{
	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			int current_height = chunkHeights[chY][chX];

			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					float z = Simplex::octave_noise(8, 0.005f, 0.5f, true_x, true_y, nc) * 5; //calculate_height(x, y, chX, chY);
					zValues[true_y][true_x] = z;
				}
			}
		}
	}
}

void Terrain::generate3D(bool* density, int height)
{
	int weirdnessHeight = height / 3;
	int baseHeight = height / 2;
	float densityThreshold = 0.3f;
	float weirdness = 0.1f;

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						//calculate density with simplex noise
						float d = Simplex::octave_noise(8, 0.007f, 0.5f, true_x, true_y, z, nc);

						float fz = (float)z;
						// Create base layer
						if (z <= baseHeight/2)
							d -= (2 * (fz - baseHeight/2) / baseHeight);

						// Weirdness amount
						if (z > baseHeight/2)
							d -= std::exp(((fz - baseHeight/2) / height)) * weirdness;

						// Scale off
						if (z >= baseHeight )
							d -= ((fz - baseHeight) / height) * ((fz - baseHeight) / height);
					
						//determine whether its solid or air
						density[true_y*chunkSize*chunks*height + true_x*height + z] = d > densityThreshold;
					}
				}
			}
		}
	}
}


void Terrain::Generate3DCustom(bool* density, int height, int octaves, float persistance, int heightMult)
{
	int baseHeight = height / 2;

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						//determine whether its solid or air
						density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, octaves, persistance, heightMult);
					}
				}
			}
		}
	}
}

bool Terrain::fillVoxel(int baseHeight, int x, int y, int z, int height, int octaves, float persistance, int heightMult) 
{
	float d = Simplex::octave_noise(octaves, 0.007f, persistance, x, y, z, nc);

	float fz = (float)z;
	// Create base layer
	if (z <= baseHeight / 2)
		d -= (2 * (fz - baseHeight / 2) / baseHeight);

	// Scale off
	float hm = ((fz - (baseHeight / 2)) / height);
	if (z >= (baseHeight / 2))
	{
		for (int i = 0; i < heightMult; ++i)
		{
			hm *= hm;
		}
	}
	d -= hm;

	return d > 0.3f;
}

void Terrain::GenerateMountains(bool* density, int height)
{
	Generate3DCustom(density, height, MOUNTAINS_OCTAVES, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM);
}

void Terrain::GeneratePlains(bool* density, int height)
{
	Generate3DCustom(density, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM);
}

void Terrain::generateMountainsPlains(bool* density, int height)
{
	int baseHeight = height / 2;
	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			if (chY <= (chunks-1)/2) 
			{
				for (int y = 0; y < chunkSize; ++y) {
					int true_y = y + (chunkSize * chY);
					for (int x = 0; x < chunkSize; ++x) {
						int true_x = x + (chunkSize * chX);
						for (int z = 0; z < height; ++z)
						{
							//determine whether its solid or air
							density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM);
						}
					}
				}
			}
			else 
			{
				for (int y = 0; y < chunkSize; ++y) {
					int true_y = y + (chunkSize * chY);
					for (int x = 0; x < chunkSize; ++x) {
						int true_x = x + (chunkSize * chX);
						for (int z = 0; z < height; ++z)
						{
							//determine whether its solid or air
							density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM);
						}
					}
				}
			}
		}
	}
}


void Terrain::generateMountainsPlainsInterpolated(bool* density, int height)
{
	int baseHeight = height / 2;
	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			if (chY <= (chunks - 1) / 2)
			{
				if (chY == (chunks - 1) / 2) 
				{
					for (int y = 0; y < chunkSize; ++y) {
						int true_y = y + (chunkSize * chY);
						for (int x = 0; x < chunkSize; ++x) {
							int true_x = x + (chunkSize * chX);
							for (int z = 0; z < height; ++z)
							{
								float p = 1.0f - ((float)y / (float)(chunkSize-1.0f));
								int octaves = interpolate(p, MOUNTAINS_OCTAVES, PLAINS_OCTAVES);
								float persistance = interpolate(p, MOUNTAINS_PERSISTANCE, PLAINS_PERSISTANCE);
								int hm = interpolate(p, MOUNTAINS_HM, PLAINS_HM);
								//determine whether its solid or air
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, octaves, persistance, MOUNTAINS_HM);
							}
						}
					}
				}
				else 
				{
					for (int y = 0; y < chunkSize; ++y) {
						int true_y = y + (chunkSize * chY);
						for (int x = 0; x < chunkSize; ++x) {
							int true_x = x + (chunkSize * chX);
							for (int z = 0; z < height; ++z)
							{
								//determine whether its solid or air
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM);
							}
						}
					}
				}
			}
			else
			{

				if (chY == ((chunks / 1) / 2) + 1)
				{
					for (int y = 0; y < chunkSize; ++y) {
						int true_y = y + (chunkSize * chY);
						for (int x = 0; x < chunkSize; ++x) {
							int true_x = x + (chunkSize * chX);
							for (int z = 0; z < height; ++z)
							{
								//determine whether its solid or air
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM);
							}
						}
					}
				}
				else
				{
					for (int y = 0; y < chunkSize; ++y) {
						int true_y = y + (chunkSize * chY);
						for (int x = 0; x < chunkSize; ++x) {
							int true_x = x + (chunkSize * chX);
							for (int z = 0; z < height; ++z)
							{
								//determine whether its solid or air
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM);
							}
						}
					}
				}
			}
		}
	}
}

void Terrain::generate2DTunnels(float** zValues)
{

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			int current_height = chunkHeights[chY][chX];

			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					float z = Simplex::turbulence(2, 0.005f, 0.5f, true_x, true_y, nc);
					zValues[true_y][true_x] = z;
				}
			}
		}
	}
}

void Terrain::generate3DTunnels(bool* density, int height)
{
	float densityThreshold = 0.6f;

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						//calculate density with simplex noise
						float d = Simplex::turbulence(2, 0.02f, 0.5f, true_x, true_y, z, nc);

						//determine whether its solid or air
						density[true_y*chunkSize*chunks*height + true_x*height + z] = d < densityThreshold;

						//density[true_y*chunkSize*chunks*height + true_x*height + z] = Simplex::octave_noise(8, 0.005f, 0.5f, true_x, true_y, z, nc) > 0.3f;
						//std::cout << d;
					}
				}
			}
		}
	}
}

void Terrain::generate3DComposite(bool* density, int height)
{
	int offset = 10;
	int baseHeight = height / 2;
	float dtC = 0.6f;
	float dtG = 0.3f;

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						float fz = (float)z;
						float d = 0;

						if (z < baseHeight - offset) // Cavelayer
						{
							d = Simplex::turbulence(2, 0.02f, 0.5f, true_x, true_y, z, nc);
							density[true_y*chunkSize*chunks*height + true_x*height + z] = d < dtC;
						}
						else if (z < baseHeight + offset) // Interpolayer
						{
							float c = (fz - baseHeight + offset) / (2 * offset);

							float dC = (2 * dtC - Simplex::turbulence(2, 0.02f, 0.5f, true_x, true_y, z, nc)) / dtC;
							float dG = Simplex::octave_noise(8, 0.007f, 0.5f, true_x, true_y, z, nc) / dtG;

							d = dC * (1 - c) + dG * c;

							density[true_y*chunkSize*chunks*height + true_x*height + z] = d > 1 ;
						}
						else { // Groundlayer
							d = Simplex::octave_noise(8, 0.007f, 0.5f, true_x, true_y, z, nc);

							// Create base layer
							float dz = fz - baseHeight - offset;

							if (dz <= offset)
								d += dz / offset * 0.5f;
							else if (dz <= 2 * offset)
								d += (2 * offset - dz) / offset * 0.5f;

							// Toplayer
							if (z > height - offset)
							{
								float scale = ((fz - height + offset) / (offset));
								d -=  scale * scale * 0.5f;
								
							}

							density[true_y*chunkSize*chunks*height + true_x*height + z] = d > dtG;
						}
					}
				}
			}
		}
	}
}

void Terrain::generate3DCaverns(bool* density, int height)
{
	int offset = 10;
	int baseHeight = height / 2;
	float densityThreshold = 0.3f;

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						float fz = (float)z;
						float d = 0;

						float f = 0.009f;
						float dz = -(z - height/2) * (z - height / 2) * 2 /height + height/2 + 0.1f;

						d = Simplex::octave_noise(5, f, 0.5f, true_x, true_y, dz, nc) * 0.7f;
						d += Simplex::octave_noise(5, f, 0.5f, true_x, true_y, z, nc) * 0.3f;

						density[true_y*chunkSize*chunks*height + true_x*height + z] = d > densityThreshold;
					}
				}
			}
		}
	}
}

void Terrain::generate3DCliffs(bool* density, int height)
{
	int offset = 10;
	int baseHeight = height / 2;
	float densityThreshold = 0.3f;

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						float fz = (float)z;
						float d = 0;

						float f = 0.01f;
						float dz = z * z / height;

						d += Simplex::octave_noise(5, f, 0.4f, true_x + 20, true_y + 20, dz, nc);
						if (z > baseHeight)
						{
							d += Simplex::octave_noise(5, f, 0.4f, true_x + 20, true_y + 20, z, nc) * 0.5f;
						}
						density[true_y*chunkSize*chunks*height + true_x*height + z] = d > densityThreshold;
					}
				}
			}
		}
	}
}

void Terrain::generate3DSomething(bool* density, int height)
{
	int offset = 10;

	int base = height * .1f;
	int middle = height / 2;
	int top = height - base;

	float densityThreshold = 0.3f;

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						float fz = (float)z;
						float d = 0;

						float xyFreq = 0.009f;
						float zFreq = 0.007f;

						if (z <= base)
						{
							zFreq = 0.007f;
							d += (base - fz) / (base);
						}

						if (z > base && z <= middle)
						{
							zFreq = 0.007f * (middle - fz)/(middle - base) + 0.001f * (fz - base)/(middle - base);
							d -= 0.01f * (middle - fz) / (middle - base) + 0.1f * (fz - base) / (middle - base);
						}
						
						if (z > middle && z <= top)
						{
							zFreq = 0.001f * (top - fz) / (top - middle) + 0.002f * (fz - middle) / (top - middle);
							d -= 0.1f * (top - fz) / (top - middle) + 0.01f * (fz - middle) / (top - middle);
						}

						if (z > top)
						{
							zFreq = 0.002f;
							d -= (top - fz) / (height - top);
						}

						// Scale off
						//if (z >= top) {
						//	d -= ((fz - top) / top) * ((fz - top) / top);
						//}

						d += Simplex::octave_noise(4, xyFreq, zFreq, 0.4f, true_x, true_y, z, nc);
						//determine whether its solid or air
						density[true_y*chunkSize*chunks*height + true_x*height + z] = d > densityThreshold;
					}
				}
			}
		}
	}
}