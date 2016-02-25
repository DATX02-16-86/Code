#include <stdlib.h>
#include <stdio.h>
#include "bruteforce.h"
#include <random>

#define TESTING

#ifdef TESTING
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../Libraries/catch.hpp"
#endif // TESTING


#ifndef TESTING

int main(int argc, char const *argv[]) {
  int n = 100, m = 100, k = 10;
  auto parr = new Point[k];

  std::random_device rd;
  std::mt19937 gen{ rd() };
  std::uniform_real_distribution<> xdist(0, n);
  std::uniform_real_distribution<> ydist(0, m);

  // Generate random points.
  for (int i = 0; i < k; i++) {
    parr[i] = { xdist(gen), ydist(gen) };
  }

  auto bisections = new Line[k*k];
  auto polys = new Polygon[k];
  for (int i = 0; i < k; i++) {
    auto p1 = parr[i];
    std::vector<Point> lines{ { 0L, 0L },{ 0L, (double)m },{ (double)n, (double)m },{ (double)n, 0L } };
    for (int j = 0; j < k; j++) {
      auto p2 = parr[j];
      bisections[i*k + j] = bisector(p1, p2);
    }
  }

  for (int i = 0; i < k * k; i++) {
    auto l = bisections[i];
    printf("Bisector: y = %.1f * x + %.1f\n", l.k, l.m);
  }

  delete parr;
  delete bisections;

  return 0;
}

#endif // !TESTING

#ifdef TESTING
TEST_CASE("Bisector line should go through central point") {
  auto a = bisector({ 0, 0 }, { 2, 2 });
  CAPTURE(a);
  REQUIRE(pointIsOnVector({ 1, 1 }, a));
}

TEST_CASE("Bisector") {
  auto a = bisector({ 0, 0 }, { 2, 2 });
  auto b = bisector({ 0, -5 }, { 1, 5 });
  CAPTURE(a);
  CAPTURE(b);
  REQUIRE(pointIsOnVector({ 0, 2 }, bisector({ 0, 0 }, { 2, 2 })));
  
  REQUIRE(pointIsOnVector({ -1, 0.15 }, bisector({ 0, -5 }, { 1, 5 })));
}


TEST_CASE("Point is on vector") {
  Vector a{ 0, 0, 0, 0 };
  REQUIRE(pointIsOnVector({ 0, 0 }, a, 0.001));

  Vector b{ 1, 0, 1, 3 };
  REQUIRE(!pointIsOnVector({ 0, 0 }, b, 0.001));

  Vector c{ 1, 1, 1, 19 };
  CAPTURE(c);
  REQUIRE(pointIsOnVector({ 2, 20 }, c, 0.001));

}

TEST_CASE("Divide polygon does nothing if line doesn't intersect") {

}

#endif // TESTING



double frand(double fmin, double fmax) {
  double f = (double) rand() / RAND_MAX;
  return fmin + f * (fmax - fmin);
}

// Generates a random point;
Point randomPoint(double xmax, double ymax) {
  return {frand(0, xmax), frand(0, ymax)};
}

// Returns the line perpendicular to the segment bisector,
// going through their central point
Vector bisector(Point a, Point b) {
  Point p = {(a.x + b.x) / 2, (a.y + b.y) / 2};
  Point direction = {a.y - b.y, b.x - a.x };
  return{ p, direction };
}

bool pointIsOnVector(Point p, Vector v, double epsilon) {
  auto xdistance = p.x - v.point.x;
  if (v.direction.y == 0) {
    return closeEnough(v.point.x, p.x, epsilon);
  }
  if (v.direction.x == 0) {
    return closeEnough(v.point.y, p.y, epsilon);
  }
  return closeEnough(v.point.y + xdistance * (v.direction.y / v.direction.x), p.y, epsilon);
}

template <class T>
bool closeEnough(T answer, T value, T epsilon) {
  return value >= answer - epsilon && value <= answer + epsilon;
}