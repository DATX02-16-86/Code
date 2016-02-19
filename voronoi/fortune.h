#include <vector>
#include <string>
#include <ostream>
#include <queue>
#include <utility>

// ---- Types ---- //
struct Point{
  double x;
  double y;
  bool operator==(const Point& rhs) const
  {
    return x == rhs.x && y == rhs.y;
  }
  Point operator+(const Point& rhs) const
  {
    return {x + rhs.x,  y + rhs.y};
  }
  Point operator-(const Point& rhs) const
  {
    return {x - rhs.x,  y - rhs.y};
  }
  Point operator*(const double rhs) const
  {
    return {x * rhs,  y * rhs};
  }
};

struct Vector {
  Point point;
  Point direction;
};

struct Polygon{
  //Ordered list of points creating the border lines
  std::vector<Point> points;
  // Currently only == if points are the same, rotation not supported
  bool operator==(const Polygon& rhs) const
  {
    return points == rhs.points;
  }
};

template <class T>
struct Optional {
  bool just;
  T value;
};

struct Segment {
  Point start;
  Point end = { 0, 0 };
  bool done = false;
  Segment(Point start) : start(start) {};
  void finish(Point p) {
    if (done) {
      return;
    }
    done = true;
    end = p;
  }
};

struct SweepLine {
  double value;
  double stepSize;
};

typedef std::pair<Point, Point> Edge;

struct CircleEvent;

struct arc {
  Point p;
  arc *prev;
  arc *next;
  // So we can invalidate old events in checkCircleEvent
  CircleEvent *event;
  Segment *s0;
  Segment *s1;
  arc(Point pp, arc *a = 0, arc *b = 0)
    : p(pp), prev(a), next(b), event(nullptr),s0(nullptr), s1(nullptr) {};
};

struct CircleEvent {
  double x;
  Point p;
  arc *a;
  bool valid;
  CircleEvent(double xx, Point pp, arc *aa)
    : x(xx), p(pp), a(aa), valid(true) {}
};

// "Greater than" comparison, for reverse sorting in priority queue.
struct gt {
    bool operator()(CircleEvent *a, CircleEvent *b) { return a->x > b->x; }
};

class Voronoi {
  std::vector<Point> siteEvents;
  std::priority_queue<CircleEvent*, std::vector<CircleEvent*>, gt> circleEvents;
  arc *root = nullptr;
  void processNextCircleEvent();
  void checkCircleEvent(arc &arc, double x0);
  void frontInsert(Point p);
  void checkAdjacentArcEvents(arc &arc, double x);
  Segment* createSegment(Point p);
  void finishEdges();
  
  double X0 = 0;
  double X1 = 0;
  double Y0 = 0;
  double Y1 = 0;
public:
  void compute();
  Voronoi(std::vector<Point> points);
  std::vector<Segment*> result;
};

// ---- General helper functions ---- //

/**
    Returns a random **double** between ´fmin´ inclusive and ´fmax´ exclusive.
 
    @param fmin The lower bound.
    @param fmax The upper bound.
    @return A random **double** between the bounds.
 */
double frand(double fmin, double fmax);

/**
    Returns a random **Point** with the x coordinate between 0 and ´xmax´ and the y coordinate between 0 and ´ymax´.
 
    @param xmax The upper bound for the x coordinate.
    @param ymax The upper bound for the y coordinate.
    @return A random **Point** in the bounding rectangle.
 */
Point randomPoint(double xmax, double ymax);

/**
    Returns the bisector of the line going through points ´a´ and ´b´.
 
    @param a First point in the line.
    @param b Second point in the line.
    @return The bisector of the line through `a` and `b`.
 */
Vector bisector(Point a, Point b);

/**
    Determines whether **Point** ´p´ is on **Vector** ´v´, with the error ´epsilon´.
 
    @param p A **Point**.
    @param v A **Vector**.
    @param epsilon The error.
    @return ´true´ if ´p´ is on ´v´ with error ´epsilon´, ´false´ otherwise.
 */
bool pointIsOnVector(Point p, Vector v, double epsilon = 0.001);

/**
    Determines whether ´value´ is close enough to ´answer´ with error ´espilon´.
 
    @param answer The exact value.
    @param value The approximate value.
    @param epsilon The error.
    @return ´true´ if ´value´ is close enough to ´answer´ with error ´epsilon´, ´false´ otherwise.
 */
template <class T>
bool closeEnough(T answer, T value, T epsilon);

/**
    Returns the intersection point of two vectors, if any.
 
    @param a First **Vector**.
    @param b Second **Vector**
    @return An optional either containing a **Point** of the intersection, or nothing.
 */
Optional<Point> intersect(Vector a, Vector b);

// ---- Voronoi helper functions ---- //

/**
    Checks whether a new parabola at ´p´ intersects with ´arc´?
 
    @param p A **Point**.
    @param arc An **arc**.
    @return An optional either containing a **Point** of the intersection, or nothing.
 */
Optional<Point> intersect(Point p, arc &arc);

Optional<std::pair<double, Point>> circle(Point a, Point b, Point c);

/**
    Checks where two parabolas intersect.
 
    @param p0 First **Point**.
    @param p1 Second **Point**.
    @param sweep The sweep line.
    @return The point of intersection.
 */
Point intersection(Point p0, Point p1, double l);

void pop(arc *arc, Segment *s);

void finishSegments(arc *arc, Point p);

std::ostream& operator << (std::ostream& os, Point const& value) {
  os << (std::string) "(" << value.x << (std::string) ", " << value.y << (std::string) ")";
  return os;
}

std::ostream& operator << (std::ostream& os, Vector const& value) {
  os << (std::string) "point: " << value.point << (std::string) ", direction: " << value.direction;
  return os;
}
