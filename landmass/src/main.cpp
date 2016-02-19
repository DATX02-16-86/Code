#define BOOST_POLYGON_NO_DEPS

// Example from http://whyalgorithm.com/blog/2015/08/26/reliable-voronoi-implementation/

#include <iostream>

#include <boost/polygon/voronoi.hpp>

using namespace boost::polygon;

typedef double coordinate_type;
typedef point_data<coordinate_type> Point;
typedef voronoi_diagram<double> VD;

int main(int argc, char* argv[])
{
  std::vector<Point> points;

  //    points.push_back(Point(0, 0));
  //    points.push_back(Point(1, 0));
  //    points.push_back(Point(0, 1));
  //    points.push_back(Point(1, 1));
  int n;
  std::cin >> n;
  double x, y;
  for (int i = 0; i<n; i++) {
    std::cin >> x >> y;
    points.push_back(Point(x, y));
  }
  VD vd;
  construct_voronoi(points.begin(), points.end(), &vd);

  for (VD::const_edge_iterator it = vd.edges().begin(); it != vd.edges().end(); ++it)
  {
    if (it->is_primary())
    {
      if (it->is_finite())
      {
        std::cout << "(" << it->vertex0()->x() << "," << it->vertex0()->y() << ") --- (" << it->vertex1()->x() << "," << it->vertex1()->y() << ")" << std::endl;
      }
      else
      {
        Point p1 = points[it->cell()->source_index()];
        Point p2 = points[it->twin()->cell()->source_index()];
        Point origin;
        Point direction;
        coordinate_type koef = 1.0;

        origin.x((p1.x() + p2.x()) * 0.5);
        origin.y((p1.y() + p2.y()) * 0.5);
        direction.x(p1.y() - p2.y());
        direction.y(p2.x() - p1.x());
        if (it->vertex0() == NULL) {
          std::cout << "(" << origin.x() - direction.x() * koef << "," << origin.y() - direction.y() * koef << ") --- ";
        }
        else {
          std::cout << "(" << it->vertex0()->x() << "," << it->vertex0()->y() << ")  --- ";
        }

        if (it->vertex1() == NULL) {
          std::cout << "(" << origin.x() + direction.x() * koef << "," << origin.y() + direction.y() * koef << ")" << std::endl;
        }
        else {
          std::cout << "(" << it->vertex1()->x() << "," << it->vertex1()->y() << ")" << std::endl;
        }
      }
    }
  }
  return 0;
}
