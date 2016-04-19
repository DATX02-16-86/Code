#include "Tools.h"

bool Tools::isInsideTraingle(std::tuple<float, float> p, std::tuple<float, float> a, std::tuple<float, float> b, std::tuple<float, float> c)
{
	std::tuple<float, float, float> bc = Tools::getBarycentric(p, a, b, c);
	if (std::get<0>(bc) >= 0 && std::get<0>(bc) <= 1 && std::get<1>(bc) >= 0 && std::get<1>(bc) <= 1 && std::get<2>(bc) >= 0 && std::get<2>(bc) <= 1) {
		return true;
	}
	return false;
}

std::tuple<float, float, float> Tools::getBarycentric(std::tuple<float, float> p, std::tuple<float, float> p1, std::tuple<float, float> p2, std::tuple<float, float> p3)
{
	double det = ((std::get<1>(p2) - std::get<1>(p3)) * (std::get<0>(p1) - std::get<0>(p3))) + ((std::get<0>(p3) - std::get<0>(p2)) * (std::get<1>(p1) - std::get<1>(p3)));

	float alpha = (((std::get<1>(p2) - std::get<1>(p3)) * (std::get<0>(p) - std::get<0>(p3))) + ((std::get<0>(p3) - std::get<0>(p2)) * (std::get<1>(p) - std::get<1>(p3)))) / det;

	float beta = (((std::get<1>(p3) - std::get<1>(p1)) * (std::get<0>(p) - std::get<0>(p3))) + ((std::get<0>(p1) - std::get<0>(p3)) * (std::get<1>(p) - std::get<1>(p3)))) / det;

	float gamma = 1.0f - alpha - beta;
	return std::make_tuple(alpha, beta, gamma);
}

// heights in order: (0,0), (1,0), (1,1), (0,1)
float Tools::bilinearInterpolation(float x, float y, float* interpolationHeights)
{
	return interpolationHeights[0] * (1 - x) * (1 - y) + interpolationHeights[3] * x*(1 - y) + interpolationHeights[1] * (1 - x)*y + interpolationHeights[2] * x*y;
}
