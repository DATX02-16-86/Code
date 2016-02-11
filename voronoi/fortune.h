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


struct SweepLine {
  double value;
  double stepSize;
};

typedef std::pair<Point, Point> Edge;
typedef Point CircleEvent;

std::vector<Edge> voronoi(std::vector<Point> points);


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


template <class T>
bool isSiteEvent(T answer, T value, T epsilon);
