#include <stdbool.h>

void next_state(bool*** arr, int xc, int yc, int neighbors, int (*f)(bool**, int, int, int, int));
void next_state_moore(bool*** arr, int xc, int yc);
void next_state_neumann(bool*** arr, int xc, int yc);
bool** alloc_2d_array(int xc, int yc);
int moore_neighbors(bool** arr, int x, int y, int xc, int yc);
int neumann_neighbors(bool** arr, int x, int y, int xc, int yc);
