# pragma once

// SimplexNoise1234
// Copyright � 2003-2011, Stefan Gustavson
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

class Simplex {

  public:
    Simplex() {}
    ~Simplex() {}

/** 1D, 2D, 3D and 4D float Simplex noise
 */
	static float Simplex::noise(float x);
	static float Simplex::noise(float x, float y);
	static float Simplex::noise(float x, float y, float z);
	static float Simplex::noise(float x, float y, float z, float w);

	static float Simplex::octave_noise(int octaves, float freq, float persistence, float x, float y);
	static float Simplex::octave_noise(int octaves, float freq, float persistence, float x, float y, float z);

  private:
    static unsigned char Simplex::perm[];
    static float  Simplex::grad( int hash, float x );
    static float  Simplex::grad( int hash, float x, float y );
    static float  Simplex::grad( int hash, float x, float y , float z );
    static float  Simplex::grad( int hash, float x, float y, float z, float t );

};
