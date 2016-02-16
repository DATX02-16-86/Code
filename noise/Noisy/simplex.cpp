#include "simplex.h"
#include "..\..\Tritium\Code\Core\Math\Math.h";

const float F = 1 / 3;
const float G = 1 / 6;

float simplex3D(float x, float y, float z)
{
	float n0, n1, n2, n3;

	float s = (x + y + z) * F;

	int i = Tritium::Math::floorInt(x + s);
	int j = Tritium::Math::floorInt(y + s);
	int k = Tritium::Math::floorInt(z + s);
	
	float t = (i + j + k) * G;

	float X0 = i - t;
	float Y0 = j - t;
	float Z0 = k - t;
	
	float x0 = x - X0;
	float y0 = y - Y0;
	float z0 = z - Z0;

	int i1, j1, k1;
	int i2, j2, k2;

	if (x0 >= y0)
	{
		if (y0 >= z0)
		{
			i1 = 1;
			j1 = 0;
			k1 = 0;

			i2 = 1;
			j2 = 1;
			k2 = 0;
		}
		else if (x0 >= z0)
		{
			i1 = 1;
			j1 = 0;
			k1 = 0;

			i2 = 1;
			j2 = 0;
			k2 = 1;
		}
		else
		{
			i1 = 0;
			j1 = 0;
			k1 = 1;

			i2 = 1;
			j2 = 0;
			k2 = 1;
		}
	}
	else
	{
		if (y0 < z0)
		{
			i1 = 0;
			j1 = 0;
			k1 = 1;

			i2 = 0;
			j2 = 1;
			k2 = 1;
		}
		else if (x0 < z0)
		{
			i1 = 0;
			j1 = 1;
			k1 = 0;

			i2 = 0;
			j2 = 1;
			k2 = 1;
		}
		else
		{
			i1 = 0;
			j1 = 1;
			k1 = 0;

			i2 = 1;
			j2 = 1;
			k2 = 0;
		}
	}

	float x1 = x0 - i1 + G;
	float y1 = y0 - j1 + G;
	float z1 = z0 - k1 + G;

	float x2 = x0 - i2 + 2.0*G;
	float y2 = y0 - j2 + 2.0*G;
	float z2 = z0 - k2 + 2.0*G;

	float x3 = x0 - 1.0 + 3.0*G;
	float y3 = y0 - 1.0 + 3.0*G;
	float z3 = z0 - 1.0 + 3.0*G;

	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;

	int gi0 = perm[ii + perm[jj + perm[kk]]] % 12;
	int gi1 = perm[ii + i1 + perm[jj + j1 + perm[kk + k1]]] % 12;
	int gi2 = perm[ii + i2 + perm[jj + j2 + perm[kk + k2]]] % 12;
	int gi3 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1]]] % 12;

	float t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;

	if (t0 < 0)
	{
		n0 = 0.0;
	}
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
	}

	float t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
	if (t2<0)
	{
		n2 = 0;
	}
	else
	{
		t2 *= t2;
		n2 = t2*t2*dot(grad3[gi2], x2, y2, z2);
	}

	float t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
	if (t3<0)
	{
		n3 = 0;
	}
	else
	{
		t3 *= t3;
		n3 = t3*t3*dot(grad3[gi3], x3, y3, z3);
	}

	return 32.0f * (n0 + n1 + n2 + n3);
}

float dot(const int * grad, float x, float y, float z)
{
	return grad[0] * x + grad[1] * y + grad[2]*z;
}
