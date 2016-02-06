#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "cellular_automata.h"

void next_state(bool*** arr, int xc, int yc, int neighbors, int (*f)(bool**, int, int, int, int)) {
  int a;
  bool** tmp = alloc_2d_array(xc, yc);
  for (int i = 0; i < xc; i++) {
    for (int j = 0; j < yc; j++) {
      if ((a = f(*arr, i, j, xc, yc)) > neighbors) {
        tmp[i][j] = true;
      } else if (a < neighbors) {
        tmp[i][j] = false;
      }
    }
  }
  *arr = tmp;
}

void next_state_moore(bool*** arr, int xc, int yc) {
  next_state(arr, xc, yc, 5, moore_neighbors);

  //int a;
  //bool** tmp = alloc_2d_array(xc, yc);
  // for (int i = 0; i < xc; i++) {
  //   for (int j = 0; j < yc; j++) {
  //     if ((a = moore_neighbors(*arr, i, j, xc, yc)) > 5) {
  //       tmp[i][j] = true;
  //     } else if (a < 5) {
  //       tmp[i][j] = false;
  //     }
  //   }
  // }
  // *arr = tmp;
}

void next_state_neumann(bool*** arr, int xc, int yc) {
  next_state(arr, xc, yc, 3, neumann_neighbors);

  // int a;
  // bool** tmp = alloc_2d_array(xc, yc);
  // for (int i = 0; i < xc; i++) {
  //   for (int j = 0; j < yc; j++) {
  //     if ((a = von_neumann_neighbors(*arr, i, j, xc, yc)) > 3) {
  //       tmp[i][j] = true;
  //     } else if (a < 3) {
  //       tmp[i][j] = false;
  //     }
  //   }
  // }
  // *arr = tmp;
}

bool** alloc_2d_array(int xc, int yc) {
  int i, j;
  bool* values = malloc(sizeof(bool)*xc*yc);
  bool** res = malloc(sizeof(bool*)*xc);
  for (i = 0; i < xc; i++) {
    res[i] = values + i*xc;
  }
  return res;
}

int moore_neighbors(bool** arr, int x, int y, int xc, int yc) {
  int sum = 0, i, j;
  for (i = (x == 0 ? 0 : x-1); i < (x+1 == xc ? xc : x+2); i++) {
    for (j = (y == 0 ? 0 : y-1); j < (y+1 == yc ? yc : y+2); j++) {
      sum += (int)(arr[i][j]);
    }
  }
  return sum;
}

int neumann_neighbors(bool** arr, int x, int y, int xc, int yc) {
  int sum = (int)arr[x][y];
  if (x > 0 && y > 0) {
    sum += (int)arr[x-1][y-1];
  }
  if (x+2 < xc && y > 0) {
    sum += (int)arr[x+1][y-1];
  }
  if (x > 0 && y+2 < yc) {
    sum += (int)arr[x-1][y+1];
  }
  if (x+2 < xc && y+2 < yc) {
    sum += (int)arr[x+1][y+1];
  }
  return sum;
}
