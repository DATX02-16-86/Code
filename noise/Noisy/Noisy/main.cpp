#include "simplexnoise.h"
#include <iostream>

int main() {

	int chunk_width = 5;
	const int chunks_x = 1;
	const int chunks_y = 3;

	int heights[chunks_x][chunks_y] = { { 1, 2, 3 } };

	for (int i = 0; i < chunks_x; i++)
	{
		for (int j = 0; j < chunks_y; j++)
		{
			int current_height = heights[i][j];

			for (int x = 0; x < chunk_width; ++x)
			{
				for (int y = 0; y < chunk_width; ++y)
				{
					float z = octave_noise_2d(5, 0.25, 0.15, x + (chunk_width * i), y + (chunk_width * j)) + current_height;

					std::cout << z;

					if (y < chunk_width - 1)
					{
						std::cout << ",";
					}
				}

				std::cout << std::endl;
			}
		}

	}


	std::cin.get();

	return 0;
}
