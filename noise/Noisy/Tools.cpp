#include "Tools.h"


// heights in order: (0,0), (1,0), (1,1), (0,1)
float Tools::bilinearInterpolation(float x, float y, float* interpolationHeights)
{
	return interpolationHeights[0] * (1 - x) * (1 - y) + interpolationHeights[3] * x*(1 - y) + interpolationHeights[1] * (1 - x)*y + interpolationHeights[2] * x*y;
}

float Tools::clamp(float a, float downBound, float upBound)
{
	return a < downBound ? downBound : a > upBound ? upBound : a;
}
