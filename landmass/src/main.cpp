#define BOOST_POLYGON_NO_DEPS

// Example from http://whyalgorithm.com/blog/2015/08/26/reliable-voronoi-implementation/

#include <iostream>

#include <boost/polygon/voronoi.hpp>

#include <GL/glut.h>

using namespace boost::polygon;

typedef double coordinate_type;
typedef point_data<coordinate_type> Point;
typedef voronoi_diagram<double> VD;

VD vd;
std::vector<Point> points;

void displayMe(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glBegin(GL_LINES);
  for (VD::const_edge_iterator it = vd.edges().begin(); it != vd.edges().end(); ++it)
  {
    if (it->is_primary())
    {
      if (it->is_finite())
      {
        glVertex2d(it->vertex0()->x(), it->vertex0()->y());
        glVertex2d(it->vertex1()->x(), it->vertex1()->y());
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
          glVertex2d(origin.x() - direction.x() * koef, origin.y() - direction.y() * koef);
        }
        else {
          glVertex2d(it->vertex0()->x(), it->vertex0()->y());
        }

        if (it->vertex1() == NULL) {
          glVertex2d(origin.x() + direction.x() * koef, origin.y() + direction.y() * koef);
        }
        else {
          glVertex2d(it->vertex1()->x(), it->vertex1()->y());
        }
      }
    }
  }
  glEnd();
  glFlush();
}

int main(int argc, char* argv[])
{

  points.push_back(Point(0, 0));
  points.push_back(Point(100, 0));
  points.push_back(Point(0, 100));
  points.push_back(Point(100, 100));
  points.push_back(Point(800, 800));

  construct_voronoi(points.begin(), points.end(), &vd);

  int width = 800, height = 800;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE);
  glutInitWindowSize(width, height);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Hello world :D");
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, height, 0, 1, -1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glutDisplayFunc(displayMe);
  glutMainLoop();


  return 0;
}
