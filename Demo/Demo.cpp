#include <stack>
#include "../Generator/Code/World/World.h"
#include "../noise/Simplex/simplex.h"

// RENDER
#ifdef _WIN32
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <iostream>

// #include "../Generator/Code/World/World.h"
// !RENDER


using namespace generator;
using namespace landmass;

void startGlut(int argc, char* argv[], World* world);

enum class WaterType { land, sea, lake, river};
enum class Biome { sea, lake, beach, land};

Attribute height {32, AttributeType::Vertex};
Attribute moisture {32, AttributeType::Vertex};
Attribute waterType {2, AttributeType::Vertex};
Attribute biome {8, AttributeType::Cell};
Attribute cellHeight {32, AttributeType::Cell};

struct HeightGenerator: landmass::Generator {
    enum {Height, Moisture, Water};

    static constexpr float groupFrequency = 0.0003f;
    static constexpr float groupPersistence = 0.5f;

    // The maximum lake size we generate.
    static const U32 lakeSize = 200;

    HeightGenerator(): Generator({&height, &moisture, &waterType}) {}

    virtual void generate(landmass::Chunk& chunk, ChunkMatrix& matrix, I32 seed) override {
        NoiseContext noise(120);
        std::vector<bool> checkForLake;
        checkForLake.reserve(chunk.vertices.size());

        auto heightAttribute = attribute(Height);
        auto moistureAttribute = attribute(Moisture);
        auto waterAttribute = attribute(Water);

        // Calculate the height of each vertex.
        for(U32 i = 0; i < chunk.vertices.size(); i++) {
            auto v = chunk.vertices[i];

            float groupA = (Simplex::octave_noise(1, groupFrequency, groupPersistence, v.x, v.y, 0, noise) + 1.0f) * 2;
            float groupB = (Simplex::octave_noise(1, groupFrequency, groupPersistence, v.x, v.y, 1000, noise) + 1.0f) * 0;
            float groupC = (Simplex::octave_noise(1, groupFrequency, groupPersistence, v.x, v.y, 2000, noise) + 1.0f) * 0;
            float sum = groupA + groupB + groupC;
            float multiplier = 1 / (sum > 0.2f ? sum : 0.2f);

            float persistenceB = (Simplex::octave_noise(1, groupFrequency, groupPersistence, v.x, v.y, 2000, noise) + 1.0f) * 0.15f + 0.5f;

            float heightA = (Simplex::octave_noise(5, 0.0003f, 0.5f, v.x, v.y, noise) + 0.5f);
            float heightB = Simplex::octave_noise(3, 0.00025f, persistenceB, v.x, v.y, noise) + 0.25f;
            float heightC = Simplex::octave_noise(3, 0.000002f, 0.5f, v.x, v.y, noise) + 0.5f;
            float height = (heightA * groupA + heightB * groupB + heightC * groupC) * multiplier;

            WaterType water = WaterType::land;
            float moisture = 0.f;
            bool isWater = height < 0.3;
            if(isWater) {
                water = WaterType::sea;
                moisture = 1.f;
            }
            checkForLake.push_back(isWater);

            chunk.attributes.set(heightAttribute, i, height);
            chunk.attributes.set(moistureAttribute, i, moisture);
            chunk.attributes.set(waterAttribute, i, water);
        }

        // Convert small seas to lakes
        for (size_t i = 0; i < checkForLake.size(); ++i) {
            if (checkForLake[i]) {
                std::stack<U32> stack;
                std::vector<U32> visitedVerts;
                stack.push(i);

                // Whole stack has to be emptied so we don't start work on connected water twice
                while(!stack.empty()) {
                    auto it = stack.top();
                    VertexIndex vi{0, it};
                    stack.pop();

                    for(EdgeIndex j : chunk.vertexEdges[it]) {
                        auto& edgeChunk = chunk.neighbour(matrix, j.chunkIndex);
                        auto ni = nextVertexIndex(vi, edgeChunk.edges[j.index]);
                        if(!ni.chunkIndex && checkForLake[ni.index]) {
                            checkForLake[ni.index] = false;
                            stack.push(ni.index);
                            visitedVerts.push_back(ni.index);
                        }
                    }
                }

                if(visitedVerts.size() <= lakeSize) {
                    for(U32 j : visitedVerts) {
                        chunk.attributes.set(waterAttribute, j, WaterType::lake);
                    }
                }
            }
        }
    }
};

struct BiomeGenerator: landmass::Generator {
    enum {BiomeType, Height, VertexHeight, VertexWater};

    BiomeGenerator(): Generator({&biome, &cellHeight, &height, &waterType}) {}

    virtual void generate(landmass::Chunk& chunk, ChunkMatrix& matrix, I32 seed) override {
        auto biomeAttribute = attribute(BiomeType);
        auto heightAttribute = attribute(Height);
        auto vertexHeightAttribute = attribute(VertexHeight);
        auto vertexWaterAttribute = attribute(VertexWater);

        // Calculate average height etc. for cell
        for(U32 i = 0; i < chunk.cellEdges.size(); ++i) {
            float totalHeight = 0;
            int count = 0;
            int land = 0;
            int sea = 0;
            int lake = 0;

            for(const auto& ei : chunk.cellEdges[i]) {
                auto& edgeChunk = chunk.neighbour(matrix, ei.chunkIndex);
                const auto& edge = edgeChunk.edges[ei.index];
                auto& vertexChunk = edgeChunk.neighbour(matrix, edge.a.chunkIndex);
                totalHeight += edgeChunk.attributes.get<float>(vertexHeightAttribute, i);

                // Count WaterTypes
                switch(edgeChunk.attributes.get<WaterType>(vertexWaterAttribute, i)) {
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

            Biome biome;
            if(sea == count) {
                biome = Biome::sea;
            } else if(lake == count) {
                biome = Biome::lake;
            } else {
                if(sea > 1) {
                    biome = Biome::beach;
                } else {
                    biome = Biome::land;
                }
            }

            chunk.attributes.set(biomeAttribute, i, biome);
            chunk.attributes.set(heightAttribute, i, count > 0 ? totalHeight / count : 0);
        }
    }
};


int main(int argc, char* argv[]) {
    I32 seed = 1;
    World world(seed);

    world.pipeline.landmass += std::make_unique<HeightGenerator>();
    world.pipeline.landmass += std::make_unique<BiomeGenerator>();

    startGlut(argc, argv, &world);
    exit(0);
    return 0;
}


// RENDER


void color(double clr) {
  glColor3d(clr, clr, clr);
}


I32 seed = 1;
World * world;

F64 worldX = 0;
F64 worldY = 0;
F64 zoom = 1;

int lastMouseX = 0;
int lastMouseY = 0;


generator::landmass::Chunk& getVertexChunk(generator::landmass::Chunk& chunk, const VertexIndex& vi) {
  if (!vi.chunkIndex) {
    return chunk;
  }
  return chunk.neighbour(world->pipeline.landmass.matrix, vi.chunkIndex);
}

void render() {
  glClear(GL_COLOR_BUFFER_BIT);
  I32 tileSize = world->pipeline.landmass.matrix.getTileSize();
  I32 chunkX = Tritium::Math::floorInt(worldX / tileSize);
  I32 chunkY = Tritium::Math::floorInt(worldY / tileSize);

  auto width = glutGet(GLUT_WINDOW_WIDTH);
  auto windowheight = glutGet(GLUT_WINDOW_HEIGHT);
  auto top = worldY - (windowheight / 2);
  auto left = worldX - (width / 2);

  std::cout << "World " << worldX << ", " << worldY << ", " << tileSize << "\n";
  std::cout << "In chunk " << chunkX << ", " << chunkY << "\n";

  for (I32 x = chunkX - 2; x <= chunkX + 2; ++x) {
    for (I32 y = chunkY - 2; y <= chunkY + 2; ++y) {
      world->pipeline.landmass.generate(x, y, seed);
    }
  }

  for (I32 x = chunkX - 1; x <= chunkX + 1; ++x) {
    for (I32 y = chunkY - 1; y <= chunkY + 1; ++y) {
      std::cout << "Render " << x << ", " << y << "\n";
      world->pipeline.landmass.generate(x, y, seed);
      auto& chunk = world->pipeline.landmass.matrix.getChunk(x, y);

      // Unsafe get
      auto& heightAttribute = *(world->pipeline.landmass.attribute(&height).get());
      auto& moistureAttribute = *(world->pipeline.landmass.attribute(&moisture).get());
      auto& vertexWaterAttribute = *(world->pipeline.landmass.attribute(&waterType).get());


      auto& edges = chunk.edges;
      for (size_t i = 0; i < edges.size(); ++i) {
        glLineWidth(1);
        //glColor3f(180.f / 255.f, 60.f / 255.f, 50.f / 255.f);
        glBegin(GL_LINES);
        auto edge = chunk.edges[i];
        auto& aChunk = getVertexChunk(chunk, edge.a);
        Vertex a = aChunk.vertices[edge.a.index];
        float aHeight = aChunk.attributes.get<float>(heightAttribute, edge.a.index);
        auto& bChunk = getVertexChunk(chunk, edge.b);
        Vertex b = bChunk.vertices[edge.b.index];
        float bHeight = bChunk.attributes.get<float>(heightAttribute, edge.b.index);
        
        color(aHeight);
        glVertex2d(a.x - left, a.y - top);
        color(bHeight);
        glVertex2d(b.x - left, b.y - top);
        glEnd();
      }

      std::cout << "render water points!\n";
      glPointSize(5.0f);
      glBegin(GL_POINTS);
      color(1);


      

      int j = 0;

      for (size_t i = 0; i < chunk.vertices.size(); ++i)
      {
        auto vertex = chunk.vertices[i];
        if (chunk.attributes.get<WaterType>(vertexWaterAttribute, i) == WaterType::lake) {
          glColor3f(0.f / 255.f, 255.f / 255.f, 255.f / 255.f);//Aqua
          glVertex2d(vertex.x - left, vertex.y - top);
          ++j;
        }
        if (chunk.attributes.get<WaterType>(vertexWaterAttribute, i) == WaterType::river) {
          glColor3f(255.f / 255.f, 0.f / 255.f, 255.f / 255.f);//Lila
          glVertex2d(vertex.x - left, vertex.y - top);
          ++j;
        }
        if (chunk.attributes.get<WaterType>(vertexWaterAttribute, i) == WaterType::sea) {
          glColor3f(255.f / 255.f, 255.f / 255.f, 0.f / 255.f);//Gul
          glVertex2d(vertex.x - left, vertex.y - top);
          ++j;
        }
      }
      glEnd();
      std::cout << chunk.vertices.size() << " water points done: " << j << "\n";
    }
  }
  std::cout << "flush\n";
  glFlush();
}

void handleKeyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 'a':
    worldX -= 10 / zoom;
    break;
  case 'd':
    worldX += 10 / zoom;
    break;
  case 'w':
    worldY -= 10 / zoom;
    break;
  case 's':
    worldY += 10 / zoom;
    break;
  case '+':
    zoom *= 2.0;
    glScalef(2.0f, 2.0f, 1.0f);
    break;
  case '-':
    zoom *= 0.5;
    glScalef(0.5f, 0.5f, 1.0f);
    break;
  }
  std::cout << key << ", " << worldX << ", " << worldY << ", " << zoom << "\n";

  glutPostRedisplay();
}

void handleClick(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    lastMouseX = x;
    lastMouseY = y;
  }
}

void handleMotion(int x, int y) {
  int deltaX = lastMouseX - x;
  int deltaY = lastMouseY - y;
  lastMouseX = x;
  lastMouseY = y;
  worldX += deltaX / zoom;
  worldY += deltaY / zoom;
  glutPostRedisplay();
}

void startGlut(int argc, char* argv[], World* world_) {
  std::cout << "startglut\n";
  world = world_;

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
  glShadeModel(GL_SMOOTH);
  glLoadIdentity();
  glutDisplayFunc(render);
  glutKeyboardFunc(handleKeyboard);
  glutMotionFunc(handleMotion);
  glutMouseFunc(handleClick);
  std::cout << "glutMainLoop!\n";
  glutMainLoop();
}
