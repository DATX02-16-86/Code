#define BOOST_POLYGON_NO_DEPS

// Example from http://whyalgorithm.com/blog/2015/08/26/reliable-voronoi-implementation/

#include <iostream>
#include <random>
#include <algorithm>

#include <boost/polygon/voronoi.hpp>

#include <GL/glut.h>

#include "../../noise/Simplex/simplex.h"

#define CHUNK_SIZE 200
#define GRID_DIVISIONS 10


using namespace boost::polygon;

typedef double coordinate_type;
typedef point_data<coordinate_type> Point;
typedef voronoi_diagram<double> VD;


struct meta {
  double height;
};

template <class T>
struct Optional {
  bool just;
  T value;
};



std::vector<meta> metadata;

VD vd;
std::vector<Point> globalPoints;
std::vector<int> vertex0height;
std::vector<int> vertex1height;
int maxHeight = 1;
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

void color(double clr) {
  /*if (clr > 0.6) {
    clr = 1;
  }
  else if (clr < 0.4) {
    clr = 0;
  }*/
  glColor3d(clr, clr, clr);
}

std::pair<Point, Point> getEdgePoints(const boost::polygon::voronoi_edge<double>& edge, std::vector<Point>& points) {
  if (edge.is_primary())
  {
    if (edge.is_finite())
    {
      double x0 = edge.vertex0()->x();
      double y0 = edge.vertex0()->y();

      double x1 = edge.vertex1()->x();
      double y1 = edge.vertex1()->y();
      return{ {x0, y0}, {x1, y1} };
    }
    else
    {
      Point p1 = points[edge.cell()->source_index()];
      Point p2 = points[edge.twin()->cell()->source_index()];
      Point origin;
      Point direction;
      coordinate_type koef = 1.0;

      origin.x((p1.x() + p2.x()) * 0.5);
      origin.y((p1.y() + p2.y()) * 0.5);
      direction.x(p1.y() - p2.y());
      direction.y(p2.x() - p1.x());

      double x0;
      double y0;
      double x1;
      double y1;

      if (edge.vertex0() == NULL) {
        x0 = origin.x() - direction.x() * koef;
        y0 = origin.y() - direction.y() * koef;
      }
      else {
        x0 = edge.vertex0()->x();
        y0 = edge.vertex0()->y();
      }

      if (edge.vertex1() == NULL) {
        x1 = origin.x() + direction.x() * koef;
        y1 = origin.y() + direction.y() * koef;
      }
      else {
        x1 = edge.vertex1()->x();
        y1 = edge.vertex1()->y();
      }

      return{ { x0, y0 },{ x1, y1 } };
    }
  }
  return{ {0, 0}, {0, 0} };
}

std::pair<Optional<meta>, Optional<meta>> getEdgeMetas(const boost::polygon::voronoi_edge<double>& edge) {
  if (edge.is_primary())
  {
    if (edge.is_finite())
    {
      auto index0 = edge.vertex0()->color();
      auto index1 = edge.vertex1()->color();
      return {{true, metadata[index0]},{true, metadata[index1]} };
    }
    else
    {
      return{ {false}, {false} };
    }
  }
  return{ { false },{ false } };
}

void render() {
  glClear(GL_COLOR_BUFFER_BIT);

  for (auto it : vd.cells()) {
    const voronoi_edge<double>* edge = it.incident_edge();
    glBegin(GL_POLYGON);
    double totalHeight = 0;
    int count = 0;
    do {
      edge = edge->next();
      auto metas = getEdgeMetas(*edge);


      if (metas.first.just) {
        totalHeight += metas.first.value.height;
        ++count;
      }
    } while (edge != it.incident_edge());

    double avarageHeight = count > 0 ? totalHeight / count : 0;

    do {
      edge = edge->next();
      auto points = getEdgePoints(*edge, globalPoints);

      if (avarageHeight > 0.4) {
        color(avarageHeight);
        glVertex2d(points.first.x(), points.first.y());
      }

    } while (edge != it.incident_edge());
    glEnd();
  }
  glFlush();
}


void displayMe(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glBegin(GL_LINES);
  
  for (auto it: vd.edges())
  {
    
    if (it.is_primary())
    {
      if (it.is_finite())
      {
        auto index0 = it.vertex0()->color();
        color(metadata[index0].height);
        glVertex2d(it.vertex0()->x(), it.vertex0()->y());
        auto index1 = it.vertex1()->color();
        color(metadata[index1].height);
        glVertex2d(it.vertex1()->x(), it.vertex1()->y());
      }
      else
      {
        Point p1 = globalPoints[it.cell()->source_index()];
        Point p2 = globalPoints[it.twin()->cell()->source_index()];
        Point origin;
        Point direction;
        coordinate_type koef = 1.0;

        origin.x((p1.x() + p2.x()) * 0.5);
        origin.y((p1.y() + p2.y()) * 0.5);
        direction.x(p1.y() - p2.y());
        direction.y(p2.x() - p1.x());
        if (it.vertex0() == NULL) {
          nextColor();
          glVertex2d(origin.x() - direction.x() * koef, origin.y() - direction.y() * koef);
        }
        else {
          nextColor();
          glVertex2d(it.vertex0()->x(), it.vertex0()->y());
        }

        if (it.vertex1() == NULL) {
          nextColor();
          glVertex2d(origin.x() + direction.x() * koef, origin.y() + direction.y() * koef);
        }
        else {
          nextColor();
          glVertex2d(it.vertex1()->x(), it.vertex1()->y());
        }
      }
    }
  }
  glEnd();
  glFlush();
}

#define NUM_LLOYD_RELAXATIONS 3

int chunkSeed(int chunkX, int chunkY, int seed) {
  return (chunkX * 31 + chunkY * CHUNK_SIZE) * 31 * seed;
}

void insertRandomPoints(int chunkX, int chunkY, int count, int seed, std::vector<Point>& points) {
  std::random_device rd;
  std::mt19937 gen(chunkSeed(chunkX, chunkY, seed));
  std::uniform_int_distribution<> dis(0, CHUNK_SIZE - 1);
  for (int i = 0; i < count; i++) {
    int x = dis(gen);
    int y = dis(gen);
    points.push_back(Point(CHUNK_SIZE * chunkX + x, CHUNK_SIZE * chunkY + y));
  }
}


void relaxPoints(int chunkX, int chunkY, std::vector<Point>& points) {
  for (int i = 0; i < NUM_LLOYD_RELAXATIONS; ++i) {
    VD diagram;
    construct_voronoi(points.begin(), points.end(), &diagram);
    for (auto it : diagram.cells()) {
      const voronoi_edge<double>* edge = it.incident_edge();
      double x = 0;
      double y = 0;
      int count = 0;
      do {
        edge = edge->next();
        auto edgepPoints = getEdgePoints(*edge, points);
        x += edgepPoints.first.x() - chunkX * CHUNK_SIZE;
        y += edgepPoints.first.y() - chunkY * CHUNK_SIZE;
        ++count;
      } while (edge != it.incident_edge());
      points[it.source_index()].x(x / count + chunkX * CHUNK_SIZE);
      points[it.source_index()].y( y / count + chunkY * CHUNK_SIZE);
    }
  }
}

void filterPointsOutsideChunks(int chunkX, int chunkY, std::vector<Point>& points) {
  points.erase(std::remove_if(points.begin(), points.end(), [&](Point p) {return p.x() < chunkX * CHUNK_SIZE ||
                                                                                  p.x() > (chunkX + 1) * CHUNK_SIZE ||
                                                                                  p.y() < chunkY * CHUNK_SIZE ||
                                                                                  p.y() > (chunkY + 1) * CHUNK_SIZE; }), points.end());
  }

void insertGridPoints(int chunkX, int chunkY, int pointDistance, std::vector<Point>& points) {

  for (int y = 0; y <= CHUNK_SIZE; y += pointDistance) {
    for (int x = 0; x <= CHUNK_SIZE; x += pointDistance) {
      points.push_back(Point(CHUNK_SIZE * chunkX + x, CHUNK_SIZE * chunkY + y));
    }
  }
}

int main(int argc, char* argv[])
{
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      std::vector<Point> points;
      insertRandomPoints(i, j, 300, 0, points);
      relaxPoints(i, j, points);
      filterPointsOutsideChunks(i, j, points);
      globalPoints.reserve(globalPoints.size() + 1000);
      for (auto point : points) {
        globalPoints.push_back(point);
      }
    }
  }
  

  //insertGridPoints(0, 0, 5, globalPoints);

  construct_voronoi(globalPoints.begin(), globalPoints.end(), &vd);

  NoiseContext a(3);


  for (auto& it: vd.vertices())
  {
    auto height = Simplex::octave_noise(3, 0.002f, 0.5f, it.x(), it.y(), a) + 0.5;
    it.color(metadata.size());
    metadata.push_back({ height });
  }

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
  glutDisplayFunc(render);
  glutMainLoop();


  return 0;
}
