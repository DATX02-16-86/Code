#pragma once

namespace Tools {

	// heights in order: (0,0), (1,0), (1,1), (0,1)
	float bilinearInterpolation(float x, float y, float* interpolationHeights);

	float clamp(float a, float downBound, float upBound);
};
