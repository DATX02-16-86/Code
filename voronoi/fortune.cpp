#include <stdlib.h>
#include <stdio.h>
#include "fortune.h"
#include <random>
#include <cmath>

#define TESTING

#ifdef TESTING
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
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

TEST_CASE("Intersection of parallel lines") {
  Vector a{0, 0, 1, 0};
  Vector b{1, 1, 1, 0};
  Vector c{2, 1, 1, 0};

  SECTION("different lines returns Nothing") {
    REQUIRE(!intersect(a, b).just);
  }

  SECTION("same lines returns Nothing") {
    REQUIRE(!intersect(b, c).just);
  }
}

TEST_CASE("Intersection of lines with same point") {
  Point p{1, 1};
  Vector a{p, 1, 1};
  Vector b{p, 1, 0};
  auto c = intersect(a, b);
  REQUIRE(c.just);
  REQUIRE(c.value == p);
}

TEST_CASE("Intersection") {
  Vector a{0, 0, 1, 1};
  Vector b{0, 1, 1, 0};
  auto c = intersect(a, b);
  REQUIRE(c.just);
  Point p{1, 1};
  REQUIRE(c.value == p);
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

Optional<Point> intersect(Vector a, Vector b) {
  // http://www.gamedev.net/topic/647810-intersection-point-of-two-vectors/
  auto c = a.point - b.point;
  auto l = c.x * b.direction.y - c.y * b.direction.x;
  auto r = a.direction.y * b.direction.x - a.direction.x * b.direction.y;
  if (r == 0) {
    return {false};
  }
  auto t = l / r;
  return {true, a.point + a.direction * t};
}

// Will a new parabola at point p intersect with arc i?
Optional<Point> intersect(Point p, arc & arc)
{
  if (arc.p.x == p.x) return{ false };

  bool intersectsWithPrev = !arc.prev || intersection(arc.prev->p, arc.p, p.x).y <= p.y;
  bool intersectsWithNext = !arc.next || intersection(arc.p, arc.next->p, p.x).y <= p.y;

  if (intersectsWithPrev && intersectsWithNext) {
    double y = p.y;
    // Plug it back into the parabola equation.
    double x = (pow(arc.p.x, 2) + pow(arc.p.y - y, 2) - pow(p.x, 2))
      / (2 * arc.p.x - 2 * p.x);
    return{ true, {x, y} };
  }

  return{ false };
}

Optional<std::pair<double, Point>> circle(Point a, Point b, Point c)
{
  // Check that bc is a "right turn" from ab.
  if ((b.x - a.x)*(c.y - a.y) - (c.x - a.x)*(b.y - a.y) > 0)
    return{ false };

  // Algorithm from O'Rourke 2ed p. 189.
  double A = b.x - a.x, B = b.y - a.y,
    C = c.x - a.x, D = c.y - a.y,
    E = A*(a.x + b.x) + B*(a.y + b.y),
    F = C*(a.x + c.x) + D*(a.y + c.y),
    G = 2 * (A*(c.y - b.y) - B*(c.x - b.x));

  if (G == 0) return{ false };  // Points are co-linear.

  Point o = { (D*E - B*F) / G, (A*F - C*E) / G };
  double x = o.x + sqrt(pow(a.x - o.x, 2) + pow(a.y - o.y, 2));

  return{ true, {x, o}};
}



// Where do two parabolas intersect?
Point intersection(Point p0, Point p1, double l)
{
  Point p = p0;
  double y;
  if (p0.x == p1.x) {
    y = (p0.y + p1.y) / 2;
  } else if (p1.x == 1) {
    y = p1.y;
  } else if (p0.x == 1) {
    y = p0.y;
    p = p1;
  } else {
    // Use the quadratic formula.
    double z0 = 2 * (p0.x - l);
    double z1 = 2 * (p1.x - l);

    double a = 1 / z0 - 1 / z1;
    double b = -2 * (p0.y / z0 - p1.y / z1);
    double c = (p0.y*p0.y + p0.x*p0.x - l*l) / z0
      - (p1.y*p1.y + p1.x*p1.x - l*l) / z1;
    y = (-b - sqrt(b*b - 4 * a*c)) / (2 * a);
  }

  double x = (pow(p.x, 2) + pow(p.y - y, 2) - pow(l, 2)) / (2 * p.x - 2 * l);
  return{ x, y };
}

void pop(arc *arc, Segment *s) {
  if (arc->prev) {
    arc->prev->next = arc->next;
    arc->prev->s1 = s;
  }
  if (arc->next) {
    arc->next->prev = arc->prev;
    arc->next->s0 = s;
  }
}

void finishSegments(arc *arc, Point p) {
  if (arc->s0) arc->s0->finish(p);
  if (arc->s1) arc->s1->finish(p);
}


void Voronoi::processNextCircleEvent()
{
  auto event = circleEvents.top();
  circleEvents.pop();
  
  if (event->valid) {
    Segment *s = createSegment( event->p );
    arc *arc = event->a;

    pop(arc, s);
    finishSegments(arc, event->p);

    checkAdjacentArcEvents(*arc, event->x);
  }
}



void Voronoi::checkCircleEvent(arc &arc, double x0)
{
  if (arc.event && arc.event->x != x0) {
    arc.event->valid = false;
  }
  arc.event = nullptr;

  if (!arc.prev || !arc.next)
    return;

  auto circ = circle(arc.prev->p, arc.p, arc.next->p);
  if (circ.just) {
    arc.event = new CircleEvent(circ.value.first, circ.value.second, &arc);
    circleEvents.push(arc.event);
  }
}

void Voronoi::frontInsert(Point p)
{
  if (!root) {
    root = new arc(p);
    return;
  }

  // Find the current arc(s) at height p.y (if there are any).
  for (arc *i = root; i; i = i->next) {
    auto inter = intersect(p, *i);
    if (inter.just) {
      if (i->next && intersect(p, *i->next).just) {
        // New parabola intersects arc i -> duplicate i.
        // Doesn't copy i->event
        i->next->prev = new arc(i->p, i, i->next);
        i->next = i->next->prev;
      } else {
        i->next = new arc(i->p, i);
      }

      // Copy segment s1 to next arc
      i->next->s1 = i->s1;

      // Add p between i and i->next.
      i->next->prev = new arc(p, i, i->next);
      i->next = i->next->prev;

      arc next = *i->next; 

      // Add new half-edges connected to i's endpoints.
      next.prev->s1 = next.s0 = createSegment(inter.value);
      next.next->s0 = next.s1 = createSegment(inter.value);

      // Check for new circle events around the new arc:
      checkCircleEvent(next, p.x);
      checkAdjacentArcEvents(next, p.x);
    }
  }
}

void Voronoi::checkAdjacentArcEvents(arc &arc, double x)
{
  if (arc.prev) checkCircleEvent(*arc.prev, x);
  if (arc.next) checkCircleEvent(*arc.next, x);
}

Segment * Voronoi::createSegment(Point p)
{
  auto seg = new Segment(p);
  result.push_back(seg);
  return seg;
}

void Voronoi::compute()
{
  for (auto siteEvent : siteEvents) {
    while (!circleEvents.empty() && circleEvents.top()->x <= siteEvent.x) {
      processNextCircleEvent();
    }
    frontInsert(siteEvent);
  }

  while (!circleEvents.empty()) {
    processNextCircleEvent();
  }
}

bool gtpoint(Point a, Point b) { return a.x == b.x ? a.y > b.y : a.x > b.x; };

Voronoi::Voronoi(std::vector<Point> points)
{
  std::sort(points.begin(), points.end(), gtpoint);
  siteEvents = points;
}
