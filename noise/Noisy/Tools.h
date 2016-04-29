#pragma once
#include <tuple>

namespace Tools {
	
	bool isInsideTraingle(std::tuple<float, float> point, std::tuple<float, float> a, std::tuple<float, float> b, std::tuple<float, float> c);

	std::tuple<float, float, float> getBarycentric(std::tuple<float, float> point, std::tuple<float, float> a, std::tuple<float, float> b, std::tuple<float, float> c);

	// heights in order: (0,0), (1,0), (1,1), (0,1)
	float bilinearInterpolation(float x, float y, float* interpolationHeights);

	float clamp(float a, float downBound, float upBound);
};
