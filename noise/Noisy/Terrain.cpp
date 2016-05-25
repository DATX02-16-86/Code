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
	this->chunkSizeZ = 0;
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
	for (int i = 0; i < chunks; ++i) {
		delete[] biomes[i];
	}
	delete[] biomes;
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
			chunkHeights[x][y] = 40 + (int)(Simplex::octave_noise(2, 1, 0.5f, x, y, nc) * 20);
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

// 			biomes[x][y] = plainBiome;


			//one-line random version
			if(x < chunks / 2 + 2*Simplex::noise(x, nc)) {
				biomes[x][y] = plainBiome;
			}
			else {
				biomes[x][y] = mountainBiome;
			}

			//random version
// 			float val = (Simplex::octave_noise(4, 0.5f, 0.5f, x, y, nc) + 1) * 2;
// 			if (val < 2) {
// 				biomes[x][y] = plainBiome;
// 			}
// 			else {
// 				biomes[x][y] = mountainBiome;
// 			}
				
			//one line 3-split version
// 			if (x < chunks / 3) {
// 				biomes[x][y] = plainBiome;
// 			} else if (x < 2 * chunks / 3) {
// 				biomes[x][y] = hillBiome;
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

	float interpolationCaviness[4] = { biomes[chunkY][chunkX].cavernness, biomes[chunkY + 1][chunkX].cavernness, biomes[chunkY + 1][chunkX + 1].cavernness, biomes[chunkY][chunkX + 1].cavernness };

	float interpolationTunnelnes[4] = { biomes[chunkY][chunkX].tunnellness, biomes[chunkY + 1][chunkX].tunnellness, biomes[chunkY + 1][chunkX + 1].tunnellness, biomes[chunkY][chunkX + 1].tunnellness };

	float interpolationFloatingIslands[4] = { biomes[chunkY][chunkX].floatingIslands, biomes[chunkY + 1][chunkX].floatingIslands, biomes[chunkY + 1][chunkX + 1].floatingIslands, biomes[chunkY][chunkX + 1].floatingIslands };


	float mountains = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationMountainness);
	float plains = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationPlainness);
	float ridges = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationRidgyness);
	float weird = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationWeirdness);
	float pillars = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationPillarness);
	float caves = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationCaviness);
	float tunnels = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationTunnelnes);
	float floatingIslands = Tools::bilinearInterpolation(interpX / chunkSize, interpY / chunkSize, interpolationFloatingIslands);

	return BiomeRepresentation(mountains, plains, ridges, weird, pillars, caves, tunnels, floatingIslands);
}

int Terrain::worldBiomeFunction(int x, int y, int z, float baseHeight, BiomeRepresentation biome)
{

// 	if (biome.plainness < 0.5f) {
// 		float asd = 0;
// 	}

	float trueHeight = baseHeight + plainHeightOffset(x, y) *biome.plainness +mountainHeightOffset(x, y)*biome.mountainness + ridgeHeightOffset(x, y)*std::pow(biome.ridgyness,10);
	
	trueHeight = trueHeight < 0 ? 0 : trueHeight;
	//return z < trueHeight;

//	Combine 2d with 3d noise
	float density = 0.f;

	// All layers
	if (biome.weirdness > 0) {
		density += weirdDensity(x, y, z, trueHeight, 15)*biome.weirdness*biome.weirdness;
	}
	if (biome.pillarness > 0)
	{
		density += pillarDensity(x, y, z, trueHeight, 20, biome.pillarness);
	}

	// Underground layer
	if (z < trueHeight)
	{
		if (biome.cavernness > 0) {
			density += cavernDensity(x, y, z, trueHeight > baseHeight ? baseHeight + (trueHeight-baseHeight)/2 : trueHeight , biome.cavernness);
		}
		if (biome.tunnellness > 0) {
			density += tunnelDensity(x, y, z, std::min(trueHeight, baseHeight), biome.tunnellness, 15);
		}
	}

	// Overground layer
	if (z > trueHeight) 
	{
		if (biome.floatingIslands > 0)
		{
			density += floatingIslandsDensity(x, y, z, trueHeight, biome.plainness);
		}
	}

	
	float density2D = Tools::clamp((trueHeight - z)/trueHeight+0.5f,0,1);
	
	//testing
	//density2D = z > trueHeight ? 0.f : 1.f;
	//end of testing
	if (density + density2D < 0.5f) 
	{
		return 0;
	}
	else
	{
		if (z > trueHeight + 2) {
			//floating islands
			return 1;
		}

		float topLayers[2] = {0,0}; //grass, snow
		float otherLayers[2] = { 0,0 }; //earth, rock

		if (biome.plainness > 0)
		{
			// plains with grassy surface
			topLayers[0] += 2 * biome.plainness;
			otherLayers[0] += baseHeight*biome.plainness*biome.plainness;
			otherLayers[1] += trueHeight*biome.plainness;
		}
		if (biome.mountainness > 0)
		{
			//mountains with snowy tips and grassy valleys
			if (trueHeight > baseHeight + 50) {
				topLayers[1] += 2 * biome.mountainness;
			}
			//grass
			topLayers[0] += Tools::clamp(baseHeight-trueHeight,-10,2) * biome.mountainness;
			//earth
			otherLayers[0] += (baseHeight+15 - trueHeight)*biome.mountainness;
			otherLayers[1] = trueHeight* biome.mountainness;
		}

		//evaluate layers
		int currentLayer = trueHeight;

		// top layer
		if (topLayers[0] >= 1) {
			if (z > currentLayer - (int)topLayers[0]) {
				return 2;
			}
			currentLayer -= (int)topLayers[0];
		}
		else if (topLayers[1] >= 1) {
			if (z > currentLayer - (int)topLayers[1]) {
				return 4;
			}
			currentLayer -= (int)topLayers[1];
		}
		//  other layers
		if (otherLayers[0] >= 1 && z > currentLayer - (int)otherLayers[0])
		{
			return 3;
		}
		
		return 5;
	}
		//display non-interpolated biomes
// 		if (biome.mountainness == 1.f && biome.plainness == 0.f) {
// 			return 2;
// 		}
// 		if (biome.plainness == 1.f && biome.mountainness == 0.f) {
// 			return 3;
// 		}
// 		if (biome.plainness == 0.5f && biome.mountainness == 0.5f) {
// 			return 4;
// 		}
	//if interpolated
	return 1;
}

float Terrain::plainHeightOffset(int x, int y)
{
	return Simplex::octave_noise(2, 0.01f, 0.5f, x, y, nc) * 5;
}

float Terrain::mountainHeightOffset(int x, int y)
{
	float d = Simplex::octave_noise(4, 0.006f, 0.4f, x, y, nc);
	return d > 0 ? d*100 : d*15;
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

	return 2*d+0.2f;
}

float Terrain::pillarDensity(int x, int y, int z, float baseHeight, float pillarHeight, float pillarParameter)
{
	if (z < baseHeight)
	{
		return 0;
	}
	bool isPillar = std::pow(Simplex::octave_noise(1, 0.007, 0.5, x, y, nc),2)>0.7f;
	if (isPillar) 
	{
		if (z < baseHeight + pillarHeight ) {
			return pillarParameter*(0.7f - std::abs(Simplex::octave_noise(4,0.01,0.7,x,y,z,nc)));
		}
	}
	return 0;
}

float Terrain::tunnelDensity(int x, int y, int z, float baseHeight, float caveParameter, float tunnelRadius) 
{
	//Check if is tunnel
	float tunnelFactor = Simplex::turbulence(2, 0.01f, 0.5f, x, y, nc);
	if (tunnelFactor > 0.3f) {
		//Check if is within tunnelheight
		float tunnelHeightCenter = Simplex::octave_noise(2, 0.005f, 0.5f, x, y, nc) * (baseHeight / 3) + baseHeight/2;
		float heightDistanceFromCenter = std::abs(z-tunnelHeightCenter);
		if (heightDistanceFromCenter < tunnelRadius) {
			return -tunnelFactor*(tunnelRadius - heightDistanceFromCenter) / tunnelRadius;// +Simplex::octave_noise(3, 0.05f, 0.5f, x, y, z, nc)*0.3f;
		}
	}
	return 0;
}

float Terrain::cavernDensity(int x, int y, int z, float surfaceHeight, float cavernParameter)
{
	float cavernHeight = (surfaceHeight - 5);

	float adjustedZ = std::abs(z - cavernHeight*0.5f);
	float d1 = 1 - std::pow(adjustedZ / (0.5*cavernHeight), 4);
	float d2 = -2*std::abs(Simplex::octave_noise(2, 0.002f, 0.7f, x, y, adjustedZ, nc));
	return d1*d2 + 0.2*Simplex::octave_noise(5,0.005f,0.5f,x,y,z,nc);
}

float Terrain::floatingIslandsDensity(int x, int y, int z, float trueHeight, float plainness)
{
	float bottom = trueHeight + 20;
	float top = chunkSizeZ;
	int topSmoothing = 20;
	int bottomSmoothing = 40;
	
	// return 0 if outside active range
	if (z < bottom || z > top) 
	{
		return 0;
	}

	float islandDensity = Simplex::octave_noise(6, 0.003f, 0.5f, x, y, z, nc);
	
	if (islandDensity < 0.1f)
	{
		return 0;
	}

	float smoothingFactor = 1.f;

	if (z < bottom + bottomSmoothing)
	{
		smoothingFactor = (z - bottom) / bottomSmoothing;
	}
	else if (z > top - topSmoothing) 
	{
		smoothingFactor = (top - z) / topSmoothing;
	}

	return smoothingFactor*islandDensity;
}


void Terrain::generateFromBiomes(int* allValues, bool interpolate)
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

					float z = calculateHeight(x, y, chX, chY) + Simplex::octave_noise(2, 0.01f, 0.5f, true_x, true_y, nc) * 5;
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
						density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, octaves, persistance, heightMult, 0.0f);
					}
				}
			}
		}
	}
}

bool Terrain::fillVoxel(int baseHeight, int x, int y, int z, int height, int octaves, float persistance, int heightMult, int baseMult) 
{
	return getVoxelDensity(baseHeight, x, y, z, height, octaves, persistance, heightMult, baseMult) > 0.3f;
}

float Terrain::getVoxelDensity(int baseHeight, int x, int y, int z, int height, int octaves, float persistance, int heightMult, int baseMult)
{
	float d = Simplex::octave_noise(octaves, 0.007f, persistance, x, y, z, nc);

	float fz = (float)z;
	// Create base layer
	float bm = (2 * (fz - baseHeight / 2) / baseHeight);
	if (z <= baseHeight / 2)
	{
		for (int i = 0; i < heightMult; ++i)
		{
			bm *= bm;
		}
	}
	d -= bm;

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

	return d;
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
							density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM, MOUNTAINS_BM);
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
							density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, octaves, persistance, hm, MOUNTAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM, MOUNTAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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
								float mountainD = getVoxelDensity(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM, MOUNTAINS_BM);
								float plainsD = getVoxelDensity(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, MOUNTAINS_OCTAVES, MOUNTAINS_PERSISTANCE, MOUNTAINS_HM, MOUNTAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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
								density[true_y*chunkSize*chunks*height + true_x*height + z] = fillVoxel(baseHeight, true_x, true_y, z, height, PLAINS_OCTAVES, PLAINS_PERSISTANCE, PLAINS_HM, PLAINS_BM);
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

void Terrain::generate3DRaw(bool * point_z_values, int height)
{

	for (int chY = 1; chY < chunks - 1; chY++) {
		for (int chX = 1; chX < chunks - 1; chX++) {
			for (int y = 0; y < chunkSize; ++y) {
				int true_y = y + (chunkSize * chY);
				for (int x = 0; x < chunkSize; ++x) {
					int true_x = x + (chunkSize * chX);
					for (int z = 0; z < height; ++z)
					{
						float d = Simplex::octave_noise(4, 0.01, 0.4f, true_x, true_y, z, nc);
						//determine whether its solid or air
						point_z_values[true_y*chunkSize*chunks*height + true_x*height + z] = d > 0;
					}
				}
			}
		}
	}
}
