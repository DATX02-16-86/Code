# pragma once

// SimplexNoise1234
// Copyright © 2003-2011, Stefan Gustavson
//
// Contact: stegu@itn.liu.se
//
// This library is public domain software, released by the author
// into the public domain in February 2011. You may do anything
// you like with it. You may even remove all attributions,
// but of course I'd appreciate it if you kept my name somewhere.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

/** \file
		\brief Declares the SimplexNoise1234 class for producing Perlin simplex noise.
		\author Stefan Gustavson (stegu@itn.liu.se)
*/

/*
 * This is a clean, fast, modern and free Perlin Simplex noise class in C++.
 * Being a stand-alone class with no external dependencies, it is
 * highly reusable without source code modifications.
 *
 *
 * Note:
 * Replacing the "float" type with "double" can actually make this run faster
 * on some platforms. A templatized version of SimplexNoise1234 could be useful.
 * http://staffwww.itn.liu.se/~stegu/aqsis/aqsis-newnoise/
 */


 /*
 * Noise Context contains the context that is needed to generate noise.
 * The same context needs to always generate exactly the same noise!
 * It needs to be exactly the same on all platforms, so it can either be
 * kept as static explicit data or be generated using a platform-independent
 * seeded, deterministic random number generator.
 *
 * Currently only holds the permutation table.
 *
 * Note that making this an int[] instead of a char[] might make the
 * code run faster on platforms with a high penalty for unaligned single
 * byte addressing. Intel x86 is generally single-byte-friendly, but
 * some other CPUs are faster with 4-aligned reads.
 * However, a char[] is smaller, which avoids cache trashing, and that
 * is probably the most important aspect on most architectures.
 * This array is accessed a *lot* by the noise functions.
 * A vector-valued noise over 3D accesses it 96 times, and a
 * float-valued 4D noise 64 times. We want this to fit in the cache!
 */
struct NoiseContext {

	/** Creates default perm table*/
	NoiseContext();
	
	/** Creates perm table based on seed*/
	NoiseContext(int seed);

	/*
 	* Permutation table. This is just a random jumble of all numbers 0-255,* repeated twice to avoid wrapping the index at 255 for each lookup.
	*
	* Note that making this an int[] instead of a char[] might make the
	* code run faster on platforms with a high penalty for unaligned single
	* A vector-valued noise over 3D accesses it 96 times, and a
	* byte addressing. Intel x86 is generally single-byte-friendly, but
	* some other CPUs are faster with 4-aligned reads.
	* However, a char[] is smaller, which avoids cache trashing, and that
	* is probably the most important aspect on most architectures.
	* This array is accessed a *lot* by the noise functions.
	* float-valued 4D noise 64 times. We want this to fit in the cache!
	*/
	unsigned char perm[512];
};

class Simplex
{
public:
	Simplex() {}
	~Simplex() {}

	/** 1D, 2D, 3D and 4D float Simplex noise */
	static float noise(float x, NoiseContext& nc = defaultNoiseContext);
	static float noise(float x, float y, NoiseContext& nc = defaultNoiseContext);
	static float noise(float x, float y, float z, NoiseContext& nc = defaultNoiseContext);
	static float noise(float x, float y, float z, float w, NoiseContext& nc = defaultNoiseContext);

	static float octave_noise(int octaves, float freq, float persistence, float x, float y, NoiseContext& nc = defaultNoiseContext);
	static float octave_noise(int octaves, float freq, float persistence, float x, float y, float z, NoiseContext& nc = defaultNoiseContext);

private:
	static NoiseContext defaultNoiseContext;

	static float grad(int hash, float x);
	static float grad(int hash, float x, float y);
	static float grad(int hash, float x, float y, float z);
	static float grad(int hash, float x, float y, float z, float t);
};
