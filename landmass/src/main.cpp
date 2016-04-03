#define BOOST_POLYGON_NO_DEPS

// Example from http://whyalgorithm.com/blog/2015/08/26/reliable-voronoi-implementation/

#include <iostream>
#include <random>
#include <algorithm>
#include <memory>
#include <stack>
#include <stdexcept>

#include <boost/polygon/voronoi.hpp>

#include <GL/glut.h>

#include "../../noise/Simplex/simplex.h"
#include <Base.h>
#include "main.h"

#define CHUNK_SIZE 400
#define GRID_DIVISIONS 10


using namespace boost::polygon;

typedef double coordinate_type;
typedef point_data<coordinate_type> Point;
typedef voronoi_diagram<double> VD;

enum class WaterType { land, sea, lake, river};
enum class Biome { sea, lake, beach, land};

struct vertexmeta {
  double height;
  WaterType wt;
  double moisture;
};

struct cellmeta {
  double avarageheight;
  Biome biome;
  double avarageMoisture;
};

struct edgemeta {
  bool isRiver;
};


struct Chunk {
  Chunk(int x, int y) : x(x), y(y), voronoi(std::make_shared<VD>()) {}

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
bool closeEnough(T answer, T value, T epsilon) {
  return value >= answer - epsilon && value <= answer + epsilon;
}

struct Vertex {
  double x;
  double y;
};

bool operator==(const Vertex& lhs, const Vertex& rhs)
{
  return closeEnough(lhs.x, rhs.x, 0.0000000001) && closeEnough(lhs.y, rhs.y, 0.0000000001);
}

bool operator<(const Vertex& lhs, const Vertex& rhs)
{
  if (lhs.x < rhs.x) {
    return true;
  }
  else if (lhs.x > rhs.x) {
    return false;
  }
  else {
    return lhs.y < rhs.y;
  }
}

struct VertexIndex { U32 chunkIndex : 4; U32 index : 28; };
struct EdgeIndex { U32 chunkIndex : 4; U32 index : 28; };

bool operator==(const VertexIndex& lhs, const VertexIndex& rhs)
{
  return lhs.chunkIndex == rhs.chunkIndex && lhs.index == rhs.index;
}

struct Edge {
  VertexIndex a;
  VertexIndex b;
};

struct UnconnectedEdge {
  Vertex a;
  Vertex b;
  U8 connectToChunk : 4;
  size_t position;
};

bool operator==(const Edge& lhs, const Edge& rhs)
{
  return (lhs.a == rhs.a && lhs.b == rhs.b) || (lhs.a == rhs.b && lhs.b == rhs.a);
}

enum ChunkState {
  NOTHING, POINTS_ADDED, VERTICES, EDGES, CONNECTED_EDGES, VERTEXMETA, RIVERS, MOISTURE, MOISTURE_NEIGHBOURS, BIOMES
};

struct ChunkWithIndexes {
  int x;
  int y;
  ChunkState state;
  std::vector<Point> cellPoints;
  std::vector<std::vector<int>> cellEdges;
  std::vector<Vertex> vertices;
  std::vector<std::vector<EdgeIndex>> verticeEdges;
  std::vector<std::vector<UnconnectedEdge>> unconnectedVerticeEdges;
  std::vector<size_t> edgesConnectCandidates;
  std::vector<Edge> edges;
  std::vector<vertexmeta> vertexmetas;
  std::vector<cellmeta> cellmetas;
  std::vector<edgemeta> edgemetas;
};

template <class T>
struct Optional {
  bool just;
  T value;
};

void vertexMeta(ChunkWithIndexes& chunk);

std::vector<Chunk> chunks;
std::vector<ChunkWithIndexes> hexChunks;

int findChunk(int x, int y) {
  for (size_t i = 0; i < hexChunks.size(); ++i) {
    if (hexChunks[i].x == x && hexChunks[i].y == y) {
      return i;
    }
  }

  throw std::range_error("Couldn't find chunk");
}

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

bool vertexIsInChunk(int chunkX, int chunkY, Vertex p) {
  return p.x >= chunkX * CHUNK_SIZE &&
    p.x < (chunkX + 1) * CHUNK_SIZE &&
    p.y >= chunkY * CHUNK_SIZE &&
    p.y < (chunkY + 1) * CHUNK_SIZE;
}

bool pointIsInChunk(int chunkX, int chunkY, Point p) {
  return vertexIsInChunk(chunkX, chunkY, { p.x(), p.y() });
}

const U8 CURRENT_CHUNK_INDEX = 4;
const int chunkIndexRelativeX[9] = { -1,  0,  1, -1, 0, 1, -1, 0, 1 };
int chunkIndexRelativeY[9] = { -1, -1, -1,  0, 0, 0,  1, 1, 1 };

// Gives the relative index of the chunk where p is in
// 0 1 2
// 3 4 5
// 6 7 8
U8 chunkIndex(int chunkX, int chunkY, Vertex p) {
  for (int i = 0; i < 9; ++i) {
    if (vertexIsInChunk(chunkX + chunkIndexRelativeX[i], chunkY + chunkIndexRelativeY[i], p)) {
      return i;
    }
  }

  throw std::range_error("Vertex wasn't in any adjacent chunks");
}

ChunkWithIndexes& findChunkWithChunkIndex(ChunkWithIndexes& chunk, U8 chunkIndex) {
  if (chunkIndex == CURRENT_CHUNK_INDEX) {
    return chunk;
  }
  else {
    return hexChunks[findChunk(chunk.x + chunkIndexRelativeX[chunkIndex], chunk.y + chunkIndexRelativeY[chunkIndex])];
  }
}

const ChunkWithIndexes& findChunkWithChunkIndex(const ChunkWithIndexes& chunk, U8 chunkIndex) {
  if (chunkIndex == CURRENT_CHUNK_INDEX) {
    return chunk;
  }
  else {
    return hexChunks[findChunk(chunk.x + chunkIndexRelativeX[chunkIndex], chunk.y + chunkIndexRelativeY[chunkIndex])];
  }
}

vertexmeta& getVertexMeta(ChunkWithIndexes& c, VertexIndex i, void (*prepare) (ChunkWithIndexes&)) {
  auto& chunk = findChunkWithChunkIndex(c, i.chunkIndex);
  prepare(chunk);
  return chunk.vertexmetas[i.index];
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

void render2() {
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
      double avarageMoisture = meta.avarageMoisture;

      do {
        edge = edge->next();
        auto points = getEdgePoints(*edge, chunk.neighbourPoints);


        if (meta.biome == Biome::lake) {
          glColor3f(40.f / 255.f, 120.f / 255.f, 0.8f);

        }
        else if (meta.biome == Biome::sea) {
          glColor3f(35.f / 255.f, 115.f / 255.f, 0.5);

        }
        else if (meta.biome == Biome::beach) {
          glColor3f(246.f / 255.f, 223.f / 255.f, 179.f / 255.f);
        }
        else if (avarageHeight < 0.5 && avarageMoisture < 0.4) {
          glColor3d(210 / 255.f, 180 / 255.f, 140 / 255.f);
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
    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3f(60.f / 255.f, 140.f / 255.f, 1);

    
    for (auto& it : chunk.voronoi->edges())
    {
      if (chunk.edgemetas[it.color()].isRiver || true) {
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

Vertex getVertex(const ChunkWithIndexes& chunk, const VertexIndex& index) {
  return findChunkWithChunkIndex(chunk, index.chunkIndex).vertices[index.index];
}

VertexIndex neighbourVertexIndex(const ChunkWithIndexes& c, const Edge& e, const Vertex& v);

VertexIndex sharedVertexIndex(Edge first, Edge second) {
  if (first.a == second.a || first.a == second.b) {
    return first.a;
  }
  return first.b;
}

VertexIndex nextVertexIndex(VertexIndex current, Edge next) {
  if (current == next.b) {
    return next.a;
  }
  return next.b;
}

void render() {
  glClear(GL_COLOR_BUFFER_BIT);

  for (auto& chunk : hexChunks) {
    if (!(chunk.state == ChunkState::BIOMES)) {
      continue;
    }

    for (size_t i = 0; i < chunk.cellmetas.size(); ++i) {
      glBegin(GL_POLYGON);

      auto meta = chunk.cellmetas[i];
      double avarageHeight = meta.avarageheight;
      double avarageMoisture = meta.avarageMoisture;

      if (meta.biome == Biome::lake) {
        glColor3f(40.f / 255.f, 120.f / 255.f, 0.8f);
      }
      else if (meta.biome == Biome::sea) {
        glColor3f(35.f / 255.f, 115.f / 255.f, 0.5);
      }/*
      else if (meta.biome == Biome::beach) {
        glColor3f(246.f / 255.f, 223.f / 255.f, 179.f / 255.f);
      }
      else if (avarageHeight < 0.5 && avarageMoisture < 0.4) {
        glColor3d(210 / 255.f, 180 / 255.f, 140 / 255.f);
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
      }*/
      else {
        color(avarageMoisture);
      }


      std::vector<Edge> eis;

      for (int j : chunk.cellEdges[i]) {
        eis.push_back(chunk.edges[j]);
      }

      const auto& edgeIndexes = chunk.cellEdges[i];
      size_t edgeCount = edgeIndexes.size();

      // Probably unneccessary check
      if (edgeCount < 2) {
        continue;
      }

      for (int j = 0; j < edgeCount; ++j) {
        auto ei = chunk.edges[edgeIndexes[j]];
        auto next = chunk.edges[edgeIndexes[(j + 1) % edgeCount]];
        VertexIndex ind = sharedVertexIndex(ei, next);

        auto vertex = findChunkWithChunkIndex(chunk, ind.chunkIndex).vertices[ind.index];

        glVertex2d(vertex.x, vertex.y);
        
        
      }
      glEnd();
    }
  }


  // Lines around cells
  for (auto& chunk : hexChunks) {
    if (!(chunk.state >= ChunkState::RIVERS)) {
      continue;
    }

    for (size_t i = 0; i < chunk.edges.size(); ++i)
    {
      auto edgemeta = chunk.edgemetas[i];
      if (edgemeta.isRiver) {
        std::cout << "River!";
        glLineWidth(2);
       glColor3f(60.f / 255.f, 140.f / 255.f, 1);
      }
      else {
        glLineWidth(1);
        glColor3f(180.f / 255.f, 60.f / 255.f, 50.f / 255.f);
      }

      glBegin(GL_LINES);
      auto edge = chunk.edges[i];
      Vertex a = getVertex(chunk, edge.a);
      Vertex b = getVertex(chunk, edge.b);
      glVertex2d(a.x, a.y);
      glVertex2d(b.x, b.y);
      glEnd();
    }
  }

  for (auto& chunk : hexChunks) {
    if (!(chunk.state >= ChunkState::RIVERS)) {
      continue;
    }
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    color(1);

    for (size_t i = 0; i < chunk.vertices.size(); ++i)
    {
      auto meta = chunk.vertexmetas[i];
      auto vertex = chunk.vertices[i];
      if (meta.wt == WaterType::lake) {
        glColor3f(0.f / 255.f, 255.f / 255.f, 255.f / 255.f);
        glVertex2d(vertex.x, vertex.y);
      }
      if (meta.wt == WaterType::river) {
        glColor3f(255.f / 255.f, 0.f / 255.f, 255.f / 255.f);
        glVertex2d(vertex.x, vertex.y);
      }
    }
    glEnd();

  }

  for (auto& chunk : hexChunks) {
    if (!(chunk.state >= ChunkState::RIVERS)) {
      continue;
    }

    glLineWidth(2);
    glColor3f(255.f / 255.f, 0.f / 255.f, 0.f / 255.f);
    glBegin(GL_LINES);

    glVertex2d(chunk.x * CHUNK_SIZE, chunk.y * CHUNK_SIZE);
    glVertex2d((chunk.x + 1) * CHUNK_SIZE, chunk.y * CHUNK_SIZE);

    glVertex2d((chunk.x + 1) * CHUNK_SIZE, chunk.y * CHUNK_SIZE);
    glVertex2d((chunk.x + 1) * CHUNK_SIZE, (chunk.y + 1) * CHUNK_SIZE);

    glVertex2d((chunk.x + 1) * CHUNK_SIZE, (chunk.y + 1) * CHUNK_SIZE);
    glVertex2d(chunk.x * CHUNK_SIZE, (chunk.y + 1) * CHUNK_SIZE);

    glVertex2d(chunk.x * CHUNK_SIZE, (chunk.y + 1) * CHUNK_SIZE);
    glVertex2d(chunk.x * CHUNK_SIZE, chunk.y * CHUNK_SIZE);

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
  double maxDistance = pointDistance / 4.0;
  std::uniform_int_distribution<> dis(0, int (maxDistance / 2.0));
  std::bernoulli_distribution bdis(0.5);

  for (int i = 0; i < stepsNeeded; ++i) {
    for (int j = 0; j < stepsNeeded; ++j) {
      int x = j * pointDistance;
      int y = i * pointDistance;
      auto hexOffset = i & 1 ? 0 : (pointDistance / 2);
      int xOffset = int(bdis(gen) ? maxDistance / 2.0 + dis(gen) : - maxDistance / 2.0 - dis(gen));
      int yOffset = int(bdis(gen) ? maxDistance / 2.0 + dis(gen) : -maxDistance / 2.0 - dis(gen));
      points.push_back(Point(CHUNK_SIZE * chunkX + x + hexOffset + xOffset, CHUNK_SIZE * chunkY + y + yOffset));
    }
  }
}

//Neighoburs to a chunk, inclouding the chunk itself
std::vector<Chunk*> neighbourChunks(int x, int y, std::vector<Chunk>& in) {
  std::vector<Chunk*> out;
  for (auto& chunk: in) {
    if (chunk.x >= x - 1 && chunk.x <= x + 1 && chunk.y >= y - 1 && chunk.y <= y + 1 && !(chunk.y == y && chunk.x == x)) {
      out.push_back(&chunk);
    }
  }
  return out;
}

//Neighoburs to a chunk, inclouding the chunk itself
std::vector<ChunkWithIndexes*> neighbourChunks(int x, int y, std::vector<ChunkWithIndexes>& in) {
  std::vector<ChunkWithIndexes*> out;
  for (auto& chunk : in) {
    if (chunk.x >= x - 1 && chunk.x <= x + 1 && chunk.y >= y - 1 && chunk.y <= y + 1 && !(chunk.y == y && chunk.x == x)) {
      out.push_back(&chunk);
    }
  }
  return out;
}

VertexIndex neighbourVertexIndex(const ChunkWithIndexes& c, const Edge& e, const Vertex& v) {
  if (findChunkWithChunkIndex(c, e.a.chunkIndex).vertices[e.a.index] == v) {
    return e.a;
  }
  return e.b;
}

// Sets isRiver recursively and greedily for the edge that has the most slope.
// v is the index of the vertex that is the water source.
void runRiver(ChunkWithIndexes& chunk, U32 v) {
  const auto& vertex = chunk.vertices[v];
  vertexmeta& bestMeta = chunk.vertexmetas[v];
  VertexIndex bestVertex;
  EdgeIndex bestEdge;
  bool anyBest = false;

  for (EdgeIndex i : chunk.verticeEdges[v]) {
    auto& edgeChunk = findChunkWithChunkIndex(chunk, i.chunkIndex);
    auto j = neighbourVertexIndex(edgeChunk, edgeChunk.edges[i.index], vertex);
    auto& meta = getVertexMeta(edgeChunk, j, vertexMeta);
    if (meta.height < bestMeta.height) {
      anyBest = true;
      bestVertex = j;
      bestMeta = meta;
      bestEdge = i;
    }
  }

  if (anyBest) {
    auto& edgeChunk = findChunkWithChunkIndex(chunk, bestEdge.chunkIndex);
    auto& edgeMeta = edgeChunk.edgemetas[bestEdge.index];
    auto& aMeta = getVertexMeta(edgeChunk, edgeChunk.edges[bestEdge.index].a, vertexMeta);
    auto& bMeta = getVertexMeta(edgeChunk, edgeChunk.edges[bestEdge.index].b, vertexMeta);
    bool aIsLand = aMeta.wt == WaterType::land;
    bool bIsLand = bMeta.wt == WaterType::land;
    bool bestIsLand = bestMeta.wt == WaterType::land;

    if (aIsLand) {
      aMeta.wt = WaterType::river;
      aMeta.moisture = 1;
    }
    if (bIsLand) {
      bMeta.wt = WaterType::river;
      bMeta.moisture = 1;
    }

    if (edgeMeta.isRiver) {
      // Early return if it catches an edge already in a river
      return;
    }
    edgeMeta.isRiver = true;
    if (!bestIsLand) {
      // Don't run rivers into water
      return;
    }
    runRiver(findChunkWithChunkIndex(edgeChunk, bestVertex.chunkIndex), bestVertex.index);
  }
}

bool calculationWorthy(double x, double y, const Chunk &chunk) {
  return x >= (chunk.x - 0.3) * CHUNK_SIZE &&
    x < (chunk.x + 1.3) * CHUNK_SIZE &&
    y >= (chunk.y - 0.3) * CHUNK_SIZE &&
    y < (chunk.y + 1.3) * CHUNK_SIZE;
}

bool calculationWorthy(double x, double y, const ChunkWithIndexes &chunk) {
  return x >= (chunk.x - 0.3) * CHUNK_SIZE &&
    x < (chunk.x + 1.3) * CHUNK_SIZE &&
    y >= (chunk.y - 0.3) * CHUNK_SIZE &&
    y < (chunk.y + 1.3) * CHUNK_SIZE;
}

void addPoints(ChunkWithIndexes& chunk) {
  if (chunk.state >= ChunkState::POINTS_ADDED) {
    return;
  }

  insertHexPointsWithRandomness(chunk.x, chunk.y, 20, 0, chunk.cellPoints);
  chunk.state = ChunkState::POINTS_ADDED;
}

void addVertices(ChunkWithIndexes& chunk) {
  if (chunk.state >= ChunkState::VERTICES) {
    return;
  }

  addPoints(chunk);

  auto neighbours = neighbourChunks(chunk.x, chunk.y, hexChunks);
  int size = chunk.cellPoints.size();

  for (auto* nei : neighbours) {
    addPoints(*nei);
    size += nei->cellPoints.size();
  }

  std::vector<Point> cells;

  cells.reserve(size);
  for (Point point : chunk.cellPoints) {
    cells.push_back(point);
  }

  for (auto& neighbour : neighbours) {
    for (Point point : neighbour->cellPoints) {
      if (calculationWorthy(point.x(), point.y(), chunk)) {
        cells.push_back(point);
      }
    }
  }

  VD voronoi;
  construct_voronoi(cells.begin(), cells.end(), &voronoi);

  // Add vertices to chunk and remember their index
  for (const auto& vertex : voronoi.vertices()) {
    Vertex point{ vertex.x(), vertex.y() };
    if (vertexIsInChunk(chunk.x, chunk.y, point)) {
      chunk.vertices.push_back(point);
    }
  }

  chunk.state = ChunkState::VERTICES;
}

// TODO: speed up
int findVertexIndex(const ChunkWithIndexes& chunk, Vertex v) {
  for (size_t i = 0; i < chunk.vertices.size(); ++i) {
    if (chunk.vertices[i] == v) {
      return i;
    }
  }

  std::cout << "Vertex wasn't in the chunk";
  throw std::range_error("Vertex wasn't in the chunk");
}

int findEdgeIndex(const ChunkWithIndexes& chunk, Vertex a, Vertex b) {
  for (auto i : chunk.edgesConnectCandidates) {
    auto e = chunk.edges[i];
    auto ea = findChunkWithChunkIndex(chunk, e.a.chunkIndex).vertices[e.a.index];

    if (ea == a) {
      auto eb = findChunkWithChunkIndex(chunk, e.b.chunkIndex).vertices[e.b.index];
      if (eb == b) {
        return i;
      }
    }
    else if (ea == b) {
      auto eb = findChunkWithChunkIndex(chunk, e.b.chunkIndex).vertices[e.b.index];
      if (eb == a) {
        return i;
      }
    }
  }

  std::cout << "Edge wasn't in the chunk";
  throw std::range_error("Edge wasn't in the chunk");
}

void addVoronoi(ChunkWithIndexes& chunk) {
  if (chunk.state >= ChunkState::EDGES) {
    return;
  }
  addVertices(chunk);

  auto neighbours = neighbourChunks(chunk.x, chunk.y, hexChunks);
  int size = chunk.cellPoints.size();

  for (auto* nei : neighbours) {
    addVertices(*nei);
    size += nei->cellPoints.size();
  }

  std::vector<Point> cells;

  cells.reserve(size);
  for (Point point : chunk.cellPoints) {
    cells.push_back(point);
  }

  for (auto& neighbour : neighbours) {
    for (Point point : neighbour->cellPoints) {
      if (calculationWorthy(point.x(), point.y(), chunk)) {
        cells.push_back(point);
      }
    }
  }

  VD voronoi;
  construct_voronoi(cells.begin(), cells.end(), &voronoi);

  int i = 1;
  for (auto& vertex : voronoi.vertices()) {
    Vertex point{ vertex.x(), vertex.y() };
    if (vertexIsInChunk(chunk.x, chunk.y, point)) {
      vertex.color(i++);
    }
  }

  chunk.verticeEdges.resize(i - 1);
  chunk.unconnectedVerticeEdges.resize(i - 1);

  for (auto& vertex : voronoi.vertices()) {
    auto vi = vertex.color();

    if (vi != 0) {
      auto* incident_edge = vertex.incident_edge();
      auto* it = incident_edge;
      size_t position = 0;
      do {
        it = it->rot_next();
        auto& edge = *it;
        // Can process edge and it's not yet processed via its twin
        if (edge.is_primary() && edge.is_finite() && edge.color() == 0)
        {
          auto& vert0 = *edge.vertex0();
          Vertex point0{ vert0.x(), vert0.y() };
          auto& vert1 = *edge.vertex1();
          Vertex point1{ vert1.x(), vert1.y() };

          U8 chunkIndex0 = chunkIndex(chunk.x, chunk.y, point0);
          U8 chunkIndex1 = chunkIndex(chunk.x, chunk.y, point1);
          U32 vertIndex0 = vert0.color();
          U32 vertIndex1 = vert1.color();

          // Is one of the vertices outside this chunk?
          if (chunkIndex0 != CURRENT_CHUNK_INDEX || chunkIndex1 != CURRENT_CHUNK_INDEX) {
            // Should this edge be in this chunk?
            auto decidingVertex = std::min(point0, point1);
            U8 decidingIndex = chunkIndex(chunk.x, chunk.y, decidingVertex);
            if (decidingIndex != CURRENT_CHUNK_INDEX) {
              edge.color(1);
              edge.twin()->color(1);
              UnconnectedEdge ue{ point0, point1, decidingIndex, position++ };
              chunk.unconnectedVerticeEdges[vi - 1].push_back(std::move(ue));
              continue;
            }
            chunk.edgesConnectCandidates.push_back(chunk.edges.size());
            if (chunkIndex0 != CURRENT_CHUNK_INDEX) {
              vertIndex0 = findVertexIndex(findChunkWithChunkIndex(chunk, chunkIndex0), point0) + 1;
            }
            if (chunkIndex1 != CURRENT_CHUNK_INDEX) {
              vertIndex1 = findVertexIndex(findChunkWithChunkIndex(chunk, chunkIndex1), point1) + 1;
            }
          }

          // 0 and 1 are reserved numbers
          edge.color(chunk.edges.size() + 2);
          edge.twin()->color(chunk.edges.size() + 2);

          chunk.edges.push_back({ { chunkIndex0, vertIndex0 - 1 },{ chunkIndex1, vertIndex1 - 1} });
        }
        if (edge.color() >= 2) {
          // Add the edge index, subtract 1 since 1 was added to avoid 0
          chunk.verticeEdges[vi - 1].push_back({ CURRENT_CHUNK_INDEX, edge.color() - 2 });
          ++position;
        }
      } while (it != incident_edge);
      
    }
  }

  chunk.edgemetas.resize(chunk.edges.size());

  auto cellCount = chunk.cellPoints.size();
  chunk.cellEdges.resize(cellCount);


  for (auto& cell : voronoi.cells()) {
    auto cellIndex = cell.source_index();
    
    // Cell is in this chunk
    if (cellIndex < cellCount) {
      auto* incident_edge = cell.incident_edge();
      auto* it = incident_edge;
      do {
        it = it->next();

        if (it->color() >= 2) {
          // Add the edge index, subtract 2 since 0 and 1 were special numbers
          chunk.cellEdges[cellIndex].push_back(it->color() - 2);
        }
        else {
          // std::cout << "Skipped one\n";
        }
      } while (it != incident_edge);
    }
  }

  chunk.state = ChunkState::EDGES;
}

void connectEdges(ChunkWithIndexes& chunk) {
  if (chunk.state >= ChunkState::CONNECTED_EDGES) {
    return;
  }
  addVoronoi(chunk);

  auto neighbours = neighbourChunks(chunk.x, chunk.y, hexChunks);

  for (auto* nei : neighbours) {
    addVoronoi(*nei);
  }

  for (int i = 0; i < chunk.verticeEdges.size(); ++i) {
    for (auto e : chunk.unconnectedVerticeEdges[i]) {
      auto echunk = findChunkWithChunkIndex(chunk, e.connectToChunk);
      EdgeIndex ei{ e.connectToChunk, findEdgeIndex(echunk, e.a, e.b) };
      chunk.verticeEdges[i].insert(chunk.verticeEdges[i].begin() + e.position, ei);
    }
  }

  chunk.state = CONNECTED_EDGES;
}


void vertexMeta(ChunkWithIndexes& chunk) {
  if (chunk.state >= ChunkState::VERTEXMETA) {
    return;
  }
  connectEdges(chunk);

  NoiseContext a(120);
  std::vector<bool> checkForLake;
  checkForLake.reserve(chunk.vertices.size());

  // Calculate height of vertex
  for (size_t i = 0; i < chunk.vertices.size(); ++i)
  {
    auto& it = chunk.vertices[i];
    float groupA = (Simplex::octave_noise(1, 0.0003f, 0.5f, float(it.x), float(it.y), 0, a) + 1.0f) * 2;
    float groupB = (Simplex::octave_noise(1, 0.0003f, 0.5f, float(it.x), float(it.y), 1000, a) + 1.0f) * 0;
    float groupC = (Simplex::octave_noise(1, 0.0003f, 0.5f, float(it.x), float(it.y), 2000, a) + 1.0f) * 0;
    float sum = groupA + groupB + groupC;
    float multiplier = 1 / (sum > 0.2f ? sum : 0.2f);

    float persistanceB = (Simplex::octave_noise(1, 0.0003f, 0.5f, float(it.x), float(it.y), 2000, a) + 1.0f) * 0.15f + 0.5f;

    float heightA = (Simplex::octave_noise(5, 0.003f, 0.5f, float(it.x), float(it.y), a) + 0.5f);
    float heightB = Simplex::octave_noise(3, 0.0025f, persistanceB, float(it.x), float(it.y), a) + 0.25f;
    float heightC = Simplex::octave_noise(3, 0.00002f, 0.5f, float(it.x), float(it.y), a) + 0.5f;
    float height = (heightA * groupA + heightB * groupB + heightC * groupC) * multiplier;
    WaterType wt = WaterType::land;
    bool isWater = height < 0.3;
    double moisture = 0;
    if (isWater) {
      wt = WaterType::sea;
      moisture = 1;
    }
    checkForLake.push_back(isWater);
    chunk.vertexmetas.push_back({ height, wt, moisture });
  }

  //Has to be small for now since chunks aren't handled well
  size_t LAKE_SIZE = 200;

  // Convert small seas to lakes
  for (size_t i = 0; i < checkForLake.size(); ++i) {
    if (checkForLake[i]) {
      std::stack<int> stack;
      std::vector<int> visitedVerts;
      stack.push(i);
      // Whole stack has to be emptied so we don't start work on connected water twice
      while (!stack.empty()) {
        int it = stack.top();
        VertexIndex vi{ CURRENT_CHUNK_INDEX, it };
        stack.pop();
        for (EdgeIndex j : chunk.verticeEdges[it]) {
          auto& edgeChunk = findChunkWithChunkIndex(chunk, j.chunkIndex);
          auto ni = nextVertexIndex(vi, edgeChunk.edges[j.index]);
          if (ni.chunkIndex == CURRENT_CHUNK_INDEX && checkForLake[ni.index]) {
            checkForLake[ni.index] = false;
            stack.push(ni.index);
            visitedVerts.push_back(ni.index);
          }
        }
      }

      if (visitedVerts.size() <= LAKE_SIZE) {
        for (int j : visitedVerts) {
          chunk.vertexmetas[j].wt = WaterType::lake;
        }
      }

    }
  }

  chunk.state = ChunkState::VERTEXMETA;
}

void addRivers(ChunkWithIndexes& chunk) {
  if (chunk.state >= ChunkState::RIVERS) {
    return;
  }
  vertexMeta(chunk);

  chunk.state = ChunkState::RIVERS;
  return;

  // Add rivers
  std::vector<int> sourceCandidates;
  for (U32 i = 0; i < chunk.vertices.size(); ++i) {
    auto height = chunk.vertexmetas[i].height;
    if (height > 0.8 && height < 0.9) {
      sourceCandidates.push_back(i);
    }
  }

  std::vector<int> sources;
  for (int i : sourceCandidates) {
    auto& vertex = chunk.vertices[i];
    if (!std::any_of(sources.begin(), sources.end(), [&chunk, &vertex](int j) {return chunk.vertices[j].x <= vertex.x + 100
      && chunk.vertices[j].x >= vertex.x - 100
      && chunk.vertices[j].y <= vertex.y + 100
      && chunk.vertices[j].y >= vertex.y - 100; })) {
      sources.push_back(i);
    }
  }

  for (U32 i : sources) {
    {
      runRiver(chunk, i);
    }
  }

  chunk.state = ChunkState::RIVERS;
}

void addMoisture(ChunkWithIndexes& chunk) {
  if (chunk.state >= ChunkState::MOISTURE) {
    return;
  }
  addRivers(chunk);

  

  std::stack<std::pair<ChunkWithIndexes*, U32>> stack;
  for (size_t i = 0; i < chunk.vertices.size(); ++i) {
    auto wt = chunk.vertexmetas[i].wt;
    if (wt == WaterType::lake || wt == WaterType::river) {
      stack.push({ &chunk, i });
    }
  }

  while (!stack.empty()) {
    auto pair = stack.top();
    auto* pairChunk = pair.first;
    VertexIndex vi{ CURRENT_CHUNK_INDEX, pair.second };
    stack.pop();
    double vertMoist = pairChunk->vertexmetas[pair.second].moisture;

    for (auto i : pairChunk->verticeEdges[pair.second]) {
      auto& edgeChunk = findChunkWithChunkIndex(*pairChunk, i.chunkIndex);
      auto ni = nextVertexIndex(vi, edgeChunk.edges[i.index]);
      auto& niChunk = findChunkWithChunkIndex(edgeChunk, ni.chunkIndex);
      addRivers(niChunk);
      auto& meta = niChunk.vertexmetas[ni.index];
      double newMoist = vertMoist * 0.9;
      if (meta.moisture < newMoist && newMoist > 0.05) {
        meta.moisture = newMoist;
        stack.push({ &niChunk, ni.index });
      }
    }
  }
  chunk.state = ChunkState::MOISTURE;
}

void addMoistureNeighbours(ChunkWithIndexes& chunk) {
  if (chunk.state >= ChunkState::MOISTURE_NEIGHBOURS) {
    return;
  }

  auto neighbours = neighbourChunks(chunk.x, chunk.y, hexChunks);

  for (auto* nei : neighbours) {
    addMoisture(*nei);
  }

  chunk.state = ChunkState::MOISTURE_NEIGHBOURS;
}


void addBiomes(ChunkWithIndexes& chunk) {
  if (chunk.state >= ChunkState::BIOMES) {
    return;
  }
  addMoistureNeighbours(chunk);

  // Calculate avarage height etc. for cell
  for (size_t i = 0; i < chunk.cellEdges.size(); ++i) {
    double totalHeight = 0;
    double totalMoisture = 0;
    int count = 0;
    int land = 0;
    int sea = 0;
    int lake = 0;
    for (int j : chunk.cellEdges[i]) {
      auto ei = chunk.edges[j];
      auto vertexChunk = findChunkWithChunkIndex(chunk, ei.a.chunkIndex);
      auto vertex = getVertexMeta(chunk, ei.a, addMoisture);
      totalHeight += vertex.height;
      totalMoisture += vertex.moisture;

      //Count WaterTypes
      switch (vertex.wt)
      {
      case WaterType::land:
        land++;
        break;
      case WaterType::sea:
        sea++;
        break;
      case WaterType::lake:
        lake++;
        break;
      case WaterType::river:
        land++;
        break;
      }
      ++count;
    }

    double avarageHeight = count > 0 ? totalHeight / count : 0;
    double avarageMoisture = count > 0 ? totalMoisture / count : 0;
    Biome biome;
    if (sea == count) {
      biome = Biome::sea;
    }
    else if (lake == count) {
      biome = Biome::lake;
    }
    else {
      if (sea > 1) {
        biome = Biome::beach;
      }
      else {
        biome = Biome::land;
      }
    }

    chunk.cellmetas.push_back({ avarageHeight, biome, avarageMoisture });
  }

  chunk.state = ChunkState::BIOMES;
}

int main(int argc, char* argv[])
{
  // Min/max chunks that are drawable
  int minX = -1;
  int maxX = 3;
  int minY = -1;
  int maxY = 3;

  NoiseContext a(120);

  // Create chunks
  // Chunks used for padding in voronoi are added as well
  for (int x = minX - 4; x <= maxX + 4; ++x) {
    for (int y = minY - 4; y <= maxY + 4; ++y) {
      ChunkWithIndexes chunk{ x, y };
      hexChunks.push_back(std::move(chunk));
    }
  }

  for (auto& chunk : hexChunks) {
    if (chunk.x >= minX && chunk.x <= maxX && chunk.y >= minY && chunk.y <= maxY) {
      //addMoisture(chunk);
    }
  }

  for (auto& chunk : hexChunks) {
    if (chunk.x >= minX && chunk.x <= maxX && chunk.y >= minY && chunk.y <= maxY) {
      addBiomes(chunk);
    }
  }

  int width = 1600, height = 800;
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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel(GL_SMOOTH);
  glLoadIdentity();
  glutDisplayFunc(render);
  glutMainLoop();

  return 0;
}
