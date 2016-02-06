#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "cellular_automaton.h"
#include "prng.h"

int main(int argc, char const *argv[]) {
  int i, j, n = 60, m = 80, t = 3;
  float p = 0.17;

  printf("Usage: %s [height] [width] [passes] [distribution]\n", argv[0]);
  if (argc > 1) {
    n = atoi(argv[1]);
  }
  if (argc > 2) {
    m = atoi(argv[2]);
  }
  if (argc > 3) {
    t = atoi(argv[3]);
  }
  if (argc > 4) {
    p = atof(argv[4]);
  }
  bool* values = malloc(sizeof(bool)*n*m);
  bool** map = malloc(sizeof(bool*)*n);

  seed(time(NULL));
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      values[i*n+j] = next() > p;
    }
    map[i] = values + i*n;
  }


  bool*** mapptr = malloc(sizeof(bool**));
  mapptr = &map;
  for (i = 0; i < t; i++) {
    next_state_neumann(mapptr, n, m);
  }
  for (i = 0; i < 2; i++) {
    next_state_moore(mapptr, n, m);
  }

  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      if (map[i][j]) {
        printf("X");
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }
  return 0;
}
