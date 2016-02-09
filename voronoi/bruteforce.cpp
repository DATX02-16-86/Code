#include <stdlib.h>
#include <stdio.h>
#include "bruteforce.h"

int main(int argc, char const *argv[]) {
  int n = 100, m = 100, k = 250;
  auto parr = new Point[k];

  // Generate random points.
  for (int i = 0; i < k; i++) {
    parr[i] = randomPoint(100, 100);
  }

  // Print out all the points as coordinates.
  // for (int i = 0; i < k; i += 2) {
  //   printf("x1: %d, y1: %d\n", parr[i].x, parr[i].y);
  //   printf("x1: %d, y1: %d\n", parr[i].x, parr[i].y);
  // }

  printf("y = x\n");

  Point a = (Point) {1, 1};
  printf("a = {%f, %f}\n", a.x, a.y);

  Point b = (Point) {2, 2};
  printf("b = {%f, %f}\n", b.x, b.y);

  Line l = bisector(a, b);
  printf("Bisector: y = %.1f · x + %.1f\n", l.k, l.m);

  return 0;
}

double frand(double fmin, double fmax) {
  double f = (double) rand() / RAND_MAX;
  return fmin + f * (fmax - fmin);
}

// Generates a randaom point;
Point randomPoint(double xmax, double ymax) {
  return (Point) {frand(0, xmax), frand(0, ymax)};
}

// Returns the bisector line between two points.
Line bisector(Point a, Point b) {
  Point p = (Point) {(a.x + b.x) / 2, (a.y + b.y) / 2};

  double k = -(b.y - a.y) / (b.x - a.x);
  double m = p.y - k * p.x;
  return (Line) {k, m};
}
