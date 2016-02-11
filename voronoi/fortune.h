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
    : p(pp), prev(a), next(b), s0(0), s1(0) {};
};

struct CircleEvent {
  double x;
  Point p;
  arc *a;
  bool valid;
};

// "Greater than" comparison, for reverse sorting in priority queue.
struct gt {
  bool operator()(CircleEvent a, CircleEvent b) { return true; }
};

class Voronoid {
  std::vector<Point> siteEvents;
  std::priority_queue<CircleEvent, std::vector<CircleEvent>, gt> circleEvents;
  std::vector<Edge> result;
  void processNextCircleEvent();
  void checkCircleEvent(arc *arc, double x);
  void frontInsert(Point p);
public:
  void compute();
  Voronoid(std::vector<Point> points);
};


bool pointIsOnVector(Point p, Vector v, double epsilon = 0.001);
Point randomPoint(double xmax, double ymax);
Vector bisector(Point a, Point b);

Optional<Point> intersect(Vector a, Vector b);

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
