#define BOOST_POLYGON_NO_DEPS

// Example from http://whyalgorithm.com/blog/2015/08/26/reliable-voronoi-implementation/

#include <iostream>
#include <random>
#include <algorithm>
#include <memory>

#include <boost/polygon/voronoi.hpp>

#include <GL/glut.h>

#include "../../noise/Simplex/simplex.h"

#define CHUNK_SIZE 400
#define GRID_DIVISIONS 10


using namespace boost::polygon;

typedef double coordinate_type;
typedef point_data<coordinate_type> Point;
typedef voronoi_diagram<double> VD;


struct meta {
  double height;
};

struct Chunk {
  Chunk(int x, int y) : x(x), y(y), voronoi(std::make_shared<VD>()) {}
  //Chunk(const Chunk& c): x(c.x), y(c.y), points(c.points), voronoi(c.voronoi), metadata(c.metadata) {}

  int x;
  int y;
  std::vector<Point> points;
  std::vector<Point> neighbourPoints;
  std::shared_ptr<VD> voronoi;
  std::vector<meta> metadata;
};

template <class T>
struct Optional {
  bool just;
  T value;
};


std::vector<Chunk> chunks;

void color(double clr) {
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

bool pointIsInChunk(int chunkX, int chunkY, Point p) {
  return p.x() >= chunkX * CHUNK_SIZE &&
    p.x() < (chunkX + 1) * CHUNK_SIZE &&
    p.y() >= chunkY * CHUNK_SIZE &&
    p.y() < (chunkY + 1) * CHUNK_SIZE;
}

std::pair<Optional<meta>, Optional<meta>> getEdgeMetas(const boost::polygon::voronoi_edge<double>& edge, const std::vector<meta>& metadata) {
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

  for (auto& chunk : chunks) {
    for (auto it : chunk.voronoi->cells()) {
      if (!pointIsInChunk(chunk.x, chunk.y, chunk.neighbourPoints[it.source_index()])) {
        continue;
      }
      const voronoi_edge<double>* edge = it.incident_edge();
      glBegin(GL_POLYGON);
      double totalHeight = 0;
      int count = 0;
      do {
        edge = edge->next();
        auto metas = getEdgeMetas(*edge, chunk.metadata);


        if (metas.first.just) {
          totalHeight += metas.first.value.height;
          ++count;
        }
      } while (edge != it.incident_edge());

      double avarageHeight = count > 0 ? totalHeight / count : 0;

      do {
        edge = edge->next();
        auto points = getEdgePoints(*edge, chunk.neighbourPoints);

        if (avarageHeight > 0.4) {
          color(avarageHeight);
          glVertex2d(points.first.x(), points.first.y());
        }

      } while (edge != it.incident_edge());
      glEnd();
    }
  }

  
  glFlush();
}


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


void relaxPoints(int chunkX, int chunkY, std::vector<Point>& points, int iterations) {
  for (int i = 0; i < iterations; ++i) {
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
  points.erase(std::remove_if(points.begin(), points.end(), [&](Point p) {return !pointIsInChunk(chunkX, chunkY, p); }), points.end());
  }

void insertGridPoints(int chunkX, int chunkY, int pointDistance, std::vector<Point>& points) {

  for (int y = 0; y <= CHUNK_SIZE; y += pointDistance) {
    for (int x = 0; x <= CHUNK_SIZE; x += pointDistance) {
      points.push_back(Point(CHUNK_SIZE * chunkX + x, CHUNK_SIZE * chunkY + y));
    }
  }
}

void insertHexPoints(int chunkX, int chunkY, int pointDistance, std::vector<Point>& points) {
  int stepsNeeded = CHUNK_SIZE / pointDistance;
  for (int i = 0; i < stepsNeeded; ++i) {
    for (int j = 0; j < stepsNeeded; ++j) {
      int x = j * pointDistance;
      int y = i * pointDistance;
      auto offset = i & 1 ? 0 : (pointDistance / 2);
      points.push_back(Point(CHUNK_SIZE * chunkX + x + offset, CHUNK_SIZE * chunkY + y));
    }
  }
}

void insertHexPointsWithRandomness(int chunkX, int chunkY, int pointDistance, int seed, std::vector<Point>& points) {
  int stepsNeeded = CHUNK_SIZE / pointDistance;

  std::random_device rd;
  std::mt19937 gen(chunkSeed(chunkX, chunkY, seed));
  int maxDistance = pointDistance / 3;
  std::uniform_int_distribution<> dis(-maxDistance, maxDistance);

  for (int i = 0; i < stepsNeeded; ++i) {
    for (int j = 0; j < stepsNeeded; ++j) {
      int x = j * pointDistance;
      int y = i * pointDistance;
      auto hexOffset = i & 1 ? 0 : (pointDistance / 2);
      int xOffset = dis(gen);
      int yOffset = dis(gen);
      points.push_back(Point(CHUNK_SIZE * chunkX + x + hexOffset + xOffset, CHUNK_SIZE * chunkY + y + yOffset));
    }
  }
}


//Neighoburs to a chunk, inclouding the chunk itself
std::vector<Chunk*> neighbourChunks(int x, int y, std::vector<Chunk>& in) {
  std::vector<Chunk*> out;
  for (auto& chunk: in) {
    if (chunk.x >= x - 1 && chunk.x <= x + 1 && chunk.y >= y - 1 && chunk.y <= y + 1) {
      out.push_back(&chunk);
    }
  }
  return out;
}

int main(int argc, char* argv[])
{

  int minX = 0;
  int maxX = 1;
  int minY = 0;
  int maxY = 1;

  for (int x = minX - 1; x <= maxX + 1; ++x) {
    for (int y = minY - 1; y <= maxY + 1; ++y) {
      Chunk chunk{ x, y };
      chunks.push_back(std::move(chunk));
    }
  }

  NoiseContext a(3);

  for (auto& chunk: chunks) {
    for (int x = chunk.x - 1; x <= chunk.x + 1; ++x) {
      for (int y = chunk.y - 1; y <= chunk.y + 1; ++y) {
        std::vector<Point> points;
        //insertRandomPoints(x, y, 300, 0, points);
        //insertHexPoints(x, y, 100, points);
        insertHexPointsWithRandomness(x, y, 10, 0, points);
        //relaxPoints(x, y, points, 5);
        //filterPointsOutsideChunks(x, y, points);
        chunk.points.reserve(chunk.points.size() + points.size());
        for (auto point : points) {
          chunk.points.push_back(point);
        }
      }
    }
  }

  for (auto& chunk : chunks) {
    if (chunk.x >= minX && chunk.x <= maxX && chunk.y >= minY && chunk.y <= maxY) {
      auto neighbours = neighbourChunks(chunk.x, chunk.y, chunks);
      int size = 0;
      for (auto chunk : neighbours) {
        size += chunk->points.size();
      }

      chunk.neighbourPoints.reserve(size);
      for (auto& neighbour : neighbours) {
        for (Point point : neighbour->points) {
          chunk.neighbourPoints.push_back(point);
        }
      }

      construct_voronoi(chunk.neighbourPoints.begin(), chunk.neighbourPoints.end(), &*chunk.voronoi);

      for (auto& it : chunk.voronoi->vertices())
      {
        auto height = (Simplex::octave_noise(3, 0.006f, 0.5f, it.x(), it.y(), a) + 0.5) * 0.3;
        height += (Simplex::octave_noise(3, 0.003f, 0.5f, it.x(), it.y(), a) + 0.5) * 0.7;
        it.color(chunk.metadata.size());
        chunk.metadata.push_back({ height });
      }
    }
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
