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


struct vertexmeta {
  double height;
};

struct cellmeta {
  double avarageheight;
};

struct edgemeta {
  bool isRiver;
};


struct Chunk {
  Chunk(int x, int y) : x(x), y(y), voronoi(std::make_shared<VD>()) {}
  //Chunk(const Chunk& c): x(c.x), y(c.y), points(c.points), voronoi(c.voronoi), metadata(c.metadata) {}

  int x;
  int y;
  std::vector<Point> points;
  std::vector<Point> neighbourPoints;
  std::shared_ptr<VD> voronoi;
  std::vector<vertexmeta> vertexmetas;
  std::vector<cellmeta> cellmetas;
  std::vector<edgemeta> edgemetas;
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

std::pair<Optional<vertexmeta>, Optional<vertexmeta>> getEdgeMetas(const boost::polygon::voronoi_edge<double>& edge, const std::vector<vertexmeta>& metadata) {
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

      auto meta = chunk.cellmetas[it.color()];
      auto avarageHeight = meta.avarageheight;

      do {
        edge = edge->next();
        auto points = getEdgePoints(*edge, chunk.neighbourPoints);

        if (avarageHeight < 0.3) {
          glColor3f(44.f / 255.f, 127.f / 255.f, 1);

        }
        else if (avarageHeight < 0.43) {
          glColor3f(246.f / 255.f, 223.f / 255.f, 179.f / 255.f);
        }
        else if (avarageHeight < 0.5) {
          glColor3f(44.f / 255.f, 104.f / 255.f, 3.f / 255.f);
        }
        else if (avarageHeight < 0.6) {
          glColor3f(53.f / 255.f, 114.f / 255.f, 13.f / 255.f);
        }
        else if (avarageHeight < 0.7) {
          glColor3f(70.f / 255.f, 138.f / 255.f, 13.f / 255.f);
        }
        else if (avarageHeight < 0.8) {
          glColor3f(80.f / 255.f, 166.f / 255.f, 21.f / 255.f);
        }
        else if (avarageHeight < 0.9) {
          glColor3f(94.f / 255.f, 181.f / 255.f, 30.f / 255.f);
        }
        else {
          color(avarageHeight);
        }
        glVertex2d(points.first.x(), points.first.y());

      } while (edge != it.incident_edge());
      glEnd();
    }
    glLineWidth(30);
    glBegin(GL_LINES);
    glColor3f(44.f / 255.f, 127.f / 255.f, 1);
    
    for (auto& it : chunk.voronoi->edges())
    {
      if (chunk.edgemetas[it.color()].isRiver) {
        auto points = getEdgePoints(it, chunk.neighbourPoints);
        if (!pointIsInChunk(chunk.x, chunk.y, points.first)) {
          continue;
        }
        glVertex2d(points.first.x(), points.first.y());
        glVertex2d(points.second.x(), points.second.y());
      }
    }
    glEnd();
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

void runRiver(Chunk& chunk, const boost::polygon::voronoi_edge<double>* edge) {
  const boost::polygon::voronoi_edge<double>* it = edge->next();
  const boost::polygon::voronoi_edge<double>* best = nullptr;
  do {
    it = it->next();
    auto itMetas = getEdgeMetas(*it, chunk.vertexmetas);
    auto edgeMetas = getEdgeMetas(*edge, chunk.vertexmetas);
    if (itMetas.second.just && itMetas.second.value.height < edgeMetas.second.value.height) {
      if (best == nullptr) {
        best = it;
      }
      else {
        auto bestMetas = getEdgeMetas(*it, chunk.vertexmetas);
        if (itMetas.second.value.height < bestMetas.second.value.height) {
          best = it;
        }
      }
    }
  } while (it != edge);

  if (best != nullptr) {
    auto& meta = chunk.edgemetas[best->color()];
    if (meta.isRiver) {
      // Early return if it catches an edge already in a river
      return;
    }
    meta.isRiver = true;
    runRiver(chunk, best);
  }
};

int main(int argc, char* argv[])
{

  int minX = -1;
  int maxX = 2;
  int minY = -1;
  int maxY = 2;

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
        //insertRandomPoints(x, y, 700, 0, points);
        //insertHexPoints(x, y, 10, points);
        insertHexPointsWithRandomness(x, y, 35, 0, points);
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


      // Calculate height of vertex
      for (auto& it : chunk.voronoi->vertices())
      {
        auto height = (Simplex::octave_noise(5, 0.003f, 0.5f, it.x(), it.y(), a) + 0.2);
        //height += (Simplex::octave_noise(3, 0.010f, 0.5f, it.x(), it.y(), a) + 0.5) * 0.7;
        //auto height = Simplex::octave_noise(3, 0.002f, 0.5f, it.x(), it.y(), a) + 0.5;
        it.color(chunk.vertexmetas.size());
        chunk.vertexmetas.push_back({ height });
      }


      // Calculate avarage height of cell
      for (auto& it : chunk.voronoi->cells()) {
        if (!pointIsInChunk(chunk.x, chunk.y, chunk.neighbourPoints[it.source_index()])) {
          continue;
        }
        const voronoi_edge<double>* edge = it.incident_edge();
        double totalHeight = 0;
        int count = 0;
        do {
          edge = edge->next();
          auto metas = getEdgeMetas(*edge, chunk.vertexmetas);


          if (metas.first.just) {
            totalHeight += metas.first.value.height;
            ++count;
          }
        } while (edge != it.incident_edge());

        it.color(chunk.cellmetas.size());

        double avarageHeight = count > 0 ? totalHeight / count : 0;
        chunk.cellmetas.push_back({ avarageHeight });
      }

      // Give each edge an index
      for (auto& it : chunk.voronoi->edges())
      {
        it.color(chunk.edgemetas.size());
        chunk.edgemetas.push_back({ });
      }

      // Add rivers
      std::vector<int> sourceCandidates;
      auto& vertices = chunk.voronoi->vertices();
      for (int i = 0; i < vertices.size(); ++i) {
        auto height = chunk.vertexmetas[vertices[i].color()].height;
        if (height > 0.8 && height < 0.9) {
          sourceCandidates.push_back(i);
        }
      }

      std::vector<int> sources;
      for (int i : sourceCandidates) {
        auto& vertex = vertices[i];
        if (!std::any_of(sources.begin(), sources.end(), [&vertices, &vertex](int j) {return vertices[j].x() <= vertex.x() + 100
                                                                                        && vertices[j].x() >= vertex.x() - 100
                                                                                        && vertices[j].y() <= vertex.y() + 100
                                                                                        && vertices[j].y() >= vertex.y() - 100; })) {
          sources.push_back(i);
        }
      }

      for (int i: sources) {
        auto& vertex = vertices[i];
        {
          runRiver(chunk, vertex.incident_edge());
        }
      }
    }
  }


  int width = 800, height = 800;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE);
  glutInitWindowSize(width, height);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Compelling Landscape");
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
