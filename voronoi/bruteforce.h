#include <vector>
#include <string>

struct Point{
  double x;
  double y;
};

struct Vector {
  Point point;
  Point direction;
};

struct Polygon{
  //Ordered list of points
  std::vector<Point> lines;
};

bool pointIsOnVector(Point p, Vector v, double epsilon = 0.001);
Point randomPoint(double xmax, double ymax);
Vector bisector(Point a, Point b);

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
