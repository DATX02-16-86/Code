#include "Terrain.h"
#include "Tools.h"
#include "..\Simplex\simplex.h"
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <vector>

//Init constants
const float Terrain::PLAINS_PERSISTANCE = 0.3f;
const float Terrain::MOUNTAINS_PERSISTANCE = 0.45f;
const float Terrain::MOUNTAINS_FREQUENCY = 0.08f;

Terrain::Terrain(int chunks, int chunkSize, int seed)
{
	this->nc = NoiseContext(seed);
	this->chunks = chunks;
	this->chunkSize = chunkSize;
}

Terrain::Terrain(int chunks, int chunkSize, int chunkSizeZ, int seed)
{
	this->nc = NoiseContext(seed);
	this->chunks = chunks;
	this->chunkSize = chunkSize;
	this->chunkSizeZ = chunkSizeZ;
}

Terrain::~Terrain()
{
	for (int i = 0; i < chunks; ++i) {
		delete[] chunkHeights[i];
	}
	delete[] chunkHeights;
	/*for (int i = 0; i < chunks; ++i) {
		delete[] biomes[i];
	}
	delete[] biomes;*/
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
			chunkHeights[x][y] = 31 + (int)(Simplex::octave_noise(2, 1, 0.5f, x, y, nc) * 10);
			//chunkHeights[x][y] = chunkSizeZ/2;
		}
	}
}

float Terrain::calculateHeight(int x, int y, int chunkX, int chunkY)
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
	float interpolationHeights[4] = { (float)chunkHeights[chunkY][chunkX], (float)chunkHeights[chunkY + 1][chunkX], (float)chunkHeights[chunkY + 1][chunkX + 1], (float)chunkHeights[chunkY][chunkX + 1] };

	return Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationHeights);
}

//Universal biome test

void Terrain::generateBiomes() 
{
	biomes = new BiomeRepresentation*[chunks];
	for (int i = 0; i < chunks; i++)
	{
		biomes[i] = new BiomeRepresentation[chunks];
	}
	for (int x = 0; x < chunks; x++)
	{
		for (int y = 0; y < chunks; y++)
		{
			//one-line version
// 			if(x < chunks / 2) {
// 				biomes[x][y] = plainBiome;
// 			}
// 			else {
// 				biomes[x][y] = pillarBiome;
// 			}

			//random version
			float val = (Simplex::octave_noise(4, 0.5f, 0.5f, x, y, nc) + 1) * 2;
			if (val < 1.4f) {
				biomes[x][y] = plainBiome;
			}
 			else if (val < 2.f) {
 				biomes[x][y] = mountainBiome;
 			}
			else {
				biomes[x][y] = pillarBiome;
			}
				
			//one line 3-split version
// 			if (x < chunks / 3) {
// 				biomes[x][y] = plainBiome;
// 			} else if (x < 2 * chunks / 3) {
// 				biomes[x][y] = pillarBiome;
// 			} else {
// 				biomes[x][y] = mountainBiome;
// 			}
		}
	}
}

BiomeRepresentation Terrain::calculateBiome(int x, int y, int chunkX, int chunkY)
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

	float interpolationMountainness[4] = { biomes[chunkY][chunkX].mountainness, biomes[chunkY + 1][chunkX].mountainness, biomes[chunkY + 1][chunkX + 1].mountainness, biomes[chunkY][chunkX + 1].mountainness };

	float interpolationPlainness[4] = { biomes[chunkY][chunkX].plainness, biomes[chunkY + 1][chunkX].plainness, biomes[chunkY + 1][chunkX + 1].plainness, biomes[chunkY][chunkX + 1].plainness };

	float interpolationRidgyness[4] = { biomes[chunkY][chunkX].ridgyness, biomes[chunkY + 1][chunkX].ridgyness, biomes[chunkY + 1][chunkX + 1].ridgyness, biomes[chunkY][chunkX + 1].ridgyness };

	float interpolationWeirdness[4] = { biomes[chunkY][chunkX].weirdness, biomes[chunkY + 1][chunkX].weirdness, biomes[chunkY + 1][chunkX + 1].weirdness, biomes[chunkY][chunkX + 1].weirdness};

	float interpolationPillarness[4] = { biomes[chunkY][chunkX].pillarness, biomes[chunkY + 1][chunkX].pillarness, biomes[chunkY + 1][chunkX + 1].pillarness, biomes[chunkY][chunkX + 1].pillarness };

	float interpolationCaviness[4] = { biomes[chunkY][chunkX].caviness, biomes[chunkY + 1][chunkX].caviness, biomes[chunkY + 1][chunkX + 1].caviness, biomes[chunkY][chunkX + 1].caviness };


	float mountains = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationMountainness);
	float plains = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationPlainness);
	float ridges = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationRidgyness);
	float weird = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationWeirdness);
	float pillars = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationPillarness);
	float caves = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationCaviness);


	return BiomeRepresentation(mountains, plains, ridges, weird, pillars, caves);
}

bool Terrain::worldBiomeFunction(int x, int y, int z, float baseHeight, BiomeRepresentation biome)
{

// 	if (biome.plainness < 0.5f) {
// 		float asd = 0;
// 	}

	float trueHeight = baseHeight + plainHeightOffset(x, y) *biome.plainness +mountainHeightOffset(x, y)*biome.mountainness + ridgeHeightOffset(x, y)*std::pow(biome.ridgyness,10);
	
	trueHeight = trueHeight < 0 ? 0 : trueHeight;
	//return z < trueHeight;

//	Combine 2d with 3d noise
	float density = 0.f;
	if (biome.weirdness > 0) {
		density += weirdDensity(x, y, z, trueHeight, 50)*biome.weirdness*biome.weirdness;
	}
	if (biome.pillarness > 0)
	{
		density += pillarDensity(x, y, z, trueHeight, biome.pillarness);
	}
	if (biome.caviness > 0 && z < trueHeight) {
		density += caveDensity(x, y, z, std::min(trueHeight,baseHeight), biome.caviness);
	}
	
	float density2D = Tools::clamp((trueHeight - z)/trueHeight+0.5f,0,1);

	return density2D + density > 0.5f;
}

float Terrain::plainHeightOffset(int x, int y)
{
	return Simplex::octave_noise(2, 0.01f, 0.5f, x, y, nc) * 5;
}

float Terrain::mountainHeightOffset(int x, int y)
{
	float d = Simplex::octave_noise(4, 0.009f, 0.4f, x, y, nc);
	return d > 0 ? d*30 : d*15;
}

float Terrain::ridgeHeightOffset(int x, int y)
{
	return Simplex::turbulence(2, 0.004f, 0.5f, x, y, nc)*50;
}

float Terrain::weirdDensity(int x, int y, int z, float baseHeight, float weirdnessHeight)
{

	//return Simplex::octave_noise(8, 0.007f, 0.5f, x, y, z, nc)*2;

	float height = baseHeight + weirdnessHeight;
	
	//calculate density with simplex noise
	float d = Simplex::octave_noise(8, 0.007f, 0.5f, x, y, z, nc);

	float fz = (float)z;

	// Scale off
	if (z > height - 10)
		d -= std::pow(((fz - height+10) / (10)),2);

	return 3*d+0.2f;
}

float Terrain::pillarDensity(int x, int y, int z, float baseHeight, float pillarParameter)
{
	if (z < baseHeight)
	{
		return 0;
	}
	bool isPillar = std::abs(Simplex::octave_noise(1, 0.01, 0.5, x, y, nc))>0.5f;
	if (isPillar) 
	{
		if (baseHeight < z && z < baseHeight+ 30 ) {
			return pillarParameter*(0.7f - std::abs(Simplex::octave_noise(4,0.01,0.7,x,y,z,nc)));
		}
	}
	return 0;
}

float Terrain::caveDensity(int x, int y, int z, float baseHeight, float caveParameter) 
{
	//Check if is tunnel
	float tunnelFactor = Simplex::turbulence(2, 0.02f, 0.5f, x, y, nc);
	if (tunnelFactor > 0.2f) {
		//Check if is within tunnelheight
		float heightDistanceFromCenter = std::abs(z-((Simplex::octave_noise(2, 0.005f, 0.5f, x, y, nc)+1) * (baseHeight / 2)));
		if (heightDistanceFromCenter < 8) {
			return -tunnelFactor*(8 - heightDistanceFromCenter) / 8 + Simplex::octave_noise(3, 0.05f, 0.5f, x, y, z, nc)*0.3f;
		}
	}
	return 0;
}

void Terrain::generateFromBiomes(bool* allValues, bool interpolate)
{
	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < chunkSizeZ; ++z)
					{
						//determine whether its solid or air
						BiomeRepresentation biome;
						if (interpolate) {
							biome = calculateBiome(x, y, chX, chY);
						}
						else {
							biome = biomes[chX][chY];
						}
						float baseHeight = calculateHeight(x, y, chX, chY);
						allValues[true_y*chunkSize*chunks*chunkSizeZ + true_x*chunkSizeZ + z] = worldBiomeFunction(true_x, true_y, z, baseHeight, biome);
					}
				}
			}
		}
	}
}

//End of universal biome test

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

void Terrain::Generate3DCustom(bool* density, int height, int octaves, float frequency, float persistance, int heightMult)
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
						density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, octaves, frequency, persistance, heightMult, 0.0f);
					}
				}
			}
		}
	}
}

bool Terrain::fillVoxel(int baseHeight, int x, int y, int z, int height, int octaves, float frequency, float persistance, int heightMult, int baseMult) 
{
	return getVoxelDensity(baseHeight, x, y, z, height, octaves, frequency, persistance, heightMult, baseMult) > 0.3f;
}

float Terrain::getVoxelDensity(int baseHeight, int x, int y, int z, int height, int octaves, float frequency, float persistance, int heightMult, int baseMult)
{
	float d = Simplex::octave_noise(octaves, frequency, persistance, x, y, z, nc);

	float fz = (float)z;
	// Create base layer
	/*float bm = (2 * (fz - baseHeight / 2) / baseHeight);
	if (z <= baseHeight / 2)
	{
		for (int i = 0; i < heightMult; ++i)
		{
			bm *= bm;
		}
	}
	d -= bm;*/
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
	//d -= hm;

	return d;
}

void Terrain::GenerateMountains(bool* density, int height)
{
	Generate3DCustom(density, height, MOUNTAINS_OCTAVES, MOUNTAINS_FREQUENCY, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM);
}

void Terrain::GeneratePlains(bool* density, int height)
{
	Generate3DCustom(density, height, PLAINS_OCTAVES, 0.05f, PLAINS_PERSISTANCE, PLAINS_HM);
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
							density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_FREQUENCY, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM, MOUNTAINS_BM);
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
							density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, 0.05f, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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
								float frequency = interpolate(p, MOUNTAINS_FREQUENCY, 0.05f);
								float persistance = interpolate(p, MOUNTAINS_PERSISTANCE, PLAINS_PERSISTANCE);
								int hm = interpolate(p, MOUNTAINS_HM, PLAINS_HM);
								//determine whether its solid or air
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, octaves, frequency, persistance, hm, MOUNTAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_FREQUENCY, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM, MOUNTAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, 0.05f, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, 0.05f, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
							}
						}
					}
				}
			}
		}
	}
}

void Terrain::generateMountainsPlainsInterpolatedD(bool* density, int height)
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
								float p = 1.0f - ((float)y / (float)(chunkSize - 1.0f));
								float mountainD = getVoxelDensity(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_FREQUENCY, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM, MOUNTAINS_BM);
								float plainsD = getVoxelDensity(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, 0.05f, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
								float intD = interpolate(p, mountainD, plainsD);
								density[true_y*chunkSize*chunks*height + true_x*height + z] = intD > 0.3f;
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_FREQUENCY, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM, MOUNTAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, 0.05f, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, 0.05f, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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

void Terrain::generateMountains(std::vector<std::vector<std::vector<bool>>> allValues)
{
	generate3d(allValues, MOUNTAINS_OCTAVES, 0.5f, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM);
}

void Terrain::generate3d(std::vector<std::vector<std::vector<bool>>> allValues, int octaves, float frequency, float persistance, int heightMultiplier)
{
	for (int x = 0; x < allValues.size(); ++x)
	{
		for (int y = 0; y < allValues[0].size(); ++y)
		{
			for (int z = 0; z < allValues[0][0].size(); ++z)
			{
				allValues[x][y][z] = fillVoxel(1, x, y, z, 1, octaves, frequency, persistance, heightMultiplier, 1.0f);
			}
		}
	}
}

void Terrain::removeFloating(bool* allValues, int height)
{
	int chunkVoxelAmount = chunks * chunkSize * chunks * chunkSize * height;
	int *partitions = (int *)std::malloc(chunkVoxelAmount * sizeof(int *));
	bool *checked = (bool *)std::malloc(chunkVoxelAmount * sizeof(bool *));
	int partition_counter = 0;

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						int index = true_y*chunkSize*chunks*height + true_x*height + z;
						if (allValues[index])
						{
							//printf("Assigning %d to %d \n", partition_counter, index);
							partitions[index] = partition_counter;
							//printf("Part %d: %d  \n", index, partitions[index]);
							checked[index] = false;
							partition_counter++;
						}
						else {
							checked[index] = true;
							partitions[index] = -1;
						}
					}
				}
			}
		}
	}

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						int index = y*chunkSize*chunks*height + x*height + z;
						spreadPartition(checked, partitions, height, true_x, true_y, z, partitions[index]);
					}
				}
			}
		}
	}

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						int index = true_y*chunkSize*chunks*height + true_x*height + z;
						//printf("Part %d: %d  \n", index, partitions[index]);
						if (partitions[index] % 2 != 0)
						{
							allValues[index] = false;
						}
					}
				}
			}
		}
	}
}

void Terrain::spreadPartition(bool* checked, int* partitions, int height, int x, int y, int z, int newPartition)
{
	int index = y*chunkSize*chunks*height + x*height + z;
	if (!checked[index])
	{
		//printf("Partition: %d \n", index);
		checked[index] = true;
		partitions[index] = newPartition;
		if (x > 0) 
		{
			spreadPartition(checked, partitions, height, x - 1, y, z, newPartition);
		}
		if (x < 31) //BAD!
		{
			spreadPartition(checked, partitions, height, x + 1, y, z, newPartition);
		}
		if (y > 0)
		{
			spreadPartition(checked, partitions, height, x, y - 1, z, newPartition);
		}
		if (y < 32) //BAD!
		{
			spreadPartition(checked, partitions, height, x, y + 1, z, newPartition);
		}
		if (z > 0)
		{
			spreadPartition(checked, partitions, height, x, y, z + 1, newPartition);
		}
		if (z < 15) //BAD!
		{
			spreadPartition(checked, partitions, height, x, y, z - 1, newPartition);
		}
	}
}

