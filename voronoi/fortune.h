#include <vector>
#include <string>
#include <ostream>
#include <queue>
#include <utility>


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


bool pointIsOnVector(Point p, Vector v, double epsilon = 0.001);
Point randomPoint(double xmax, double ymax);
Vector bisector(Point a, Point b);

Optional<Point> intersect(Vector a, Vector b);
Optional<Point> intersect(Point p, arc &arc);

Optional<std::pair<double, Point>> circle(Point a, Point b, Point c);

Point intersection(Point p0, Point p1, double l);

std::ostream& operator << (std::ostream& os, Point const& value) {
  os << (std::string) "(" << value.x << (std::string) ", " << value.y << (std::string) ")";
  return os;
}

std::ostream& operator << (std::ostream& os, Vector const& value) {
  os << (std::string) "point: " << value.point << (std::string) ", direction: " << value.direction;
  return os;
}

template <class T>
bool closeEnough(T answer, T value, T epsilon);
