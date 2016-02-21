#define BOOST_POLYGON_NO_DEPS

// Example from http://whyalgorithm.com/blog/2015/08/26/reliable-voronoi-implementation/

#include <iostream>
#include <random>

#include <boost/polygon/voronoi.hpp>

#include <GL/glut.h>

using namespace boost::polygon;

typedef double coordinate_type;
typedef point_data<coordinate_type> Point;
typedef voronoi_diagram<double> VD;

VD vd;
std::vector<Point> points;
std::vector<int> vertex0height;
std::vector<int> vertex1height;
int maxHeight = 0;
int j = 0;
int clrs[5][3] = {
  { 1, 0, 0 },
  { 1, 1, 0 },
  { 0, 0, 1 },
  { 1, 0, 0 },
  { 1, 0, 1 }
};


void nextColor() {
  j = (j + 1) % 5;
  auto clr = clrs[j];
  int r = clr[0];
  int g = clr[1];
  int b = clr[2];
  glColor3d(r, g, b);
}


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
        nextColor();
        glVertex2d(it->vertex0()->x(), it->vertex0()->y());
        nextColor();
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
          nextColor();
          glVertex2d(origin.x() - direction.x() * koef, origin.y() - direction.y() * koef);
        }
        else {
          nextColor();
          glVertex2d(it->vertex0()->x(), it->vertex0()->y());
        }

        if (it->vertex1() == NULL) {
          nextColor();
          glVertex2d(origin.x() + direction.x() * koef, origin.y() + direction.y() * koef);
        }
        else {
          nextColor();
          glVertex2d(it->vertex1()->x(), it->vertex1()->y());
        }
      }
    }
  }
  glEnd();
  glFlush();
}

#define CHUNK_SIZE 200

int chunkSeed(int chunkX, int chunkY, int seed) {
  return (chunkX * 31 + chunkY * CHUNK_SIZE) * 31 * seed;
}
void insertRandomPoints(int chunkX, int chunkY, int count, int seed) {
  std::random_device rd;
  std::mt19937 gen(chunkSeed(chunkX, chunkY, seed));
  std::uniform_int_distribution<> dis(0, CHUNK_SIZE - 1);
  for (int i = 0; i < count; i++) {
    int x = dis(gen);
    int y = dis(gen);
    points.push_back(Point(CHUNK_SIZE * chunkX + x, CHUNK_SIZE * chunkY + y));
  }
}

int main(int argc, char* argv[])
{

  points.push_back(Point(0, 0));
  points.push_back(Point(800, 0));
  points.push_back(Point(0, 800));
  points.push_back(Point(800, 800));
  insertRandomPoints(1, 1, 100, 0);
  insertRandomPoints(1, 2, 100, 0);
  insertRandomPoints(2, 1, 100, 0);

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
  glEnable(GL_TEXTURE_2D);
  glShadeModel(GL_SMOOTH);
  glLoadIdentity();
  glutDisplayFunc(displayMe);
  glutMainLoop();


  return 0;
}
