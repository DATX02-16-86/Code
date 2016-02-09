#include <vector>
#include <string>

struct Point{
  double x;
  double y;
};

struct Line{
  double k;
  double m;
  bool operator==(const Line& rhs) {
    return k == rhs.k && m == rhs.m;
  }
};

struct Polygon{
  //Ordered list of points
  std::vector<Point> lines;
};

bool pointIsOnLine(Point p, Line l, double allowedDiff);
Point randomPoint(double xmax, double ymax);
Line bisector(Point a, Point b);

std::ostream& operator << (std::ostream& os, Line const& value) {
  os << (std::string) "k: " << value.k << (std::string) ", m: " << value.m;
  return os;
}

std::ostream& operator << (std::ostream& os, Point const& value) {
  os << (std::string) "(" << value.x << (std::string) ", " << value.y << (std::string) ")";
  return os;
}