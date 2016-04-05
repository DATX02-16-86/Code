
#include <GLUT/glut.h>
#include <iostream>

#include "Voronoi.h"
#include "../../noise/Simplex/simplex.h"
#include "Generate.h"

ChunkMap hexChunks;

void color(double clr) {
    glColor3d(clr, clr, clr);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    std::cout << "render cells!\n";

    for (auto& kv : hexChunks) {
        auto& chunk = kv.second;
        if (!(chunk.state == ChunkState::BIOMES)) {
            continue;
        }
        std::cout << "render " << chunk.x << ", " << chunk.y << "\n";

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

            const auto& edgeIndexes = chunk.cellEdges[i];
            size_t edgeCount = edgeIndexes.size();
            // Vertex old;
            // ChunkWithIndexes* oldC;

            // Probably unneccessary check
            if (edgeCount < 2) {
                continue;
            }

            for (int j = 0; j < edgeCount; ++j) {
                const auto& ei = edgeIndexes[j];

                const auto& edgeChunk = findChunkWithChunkIndex(hexChunks, chunk, ei.chunkIndex);
                const auto& edge = edgeChunk.edges[ei.index];
                const auto& nexti = edgeIndexes[(j + 1) % edgeCount];
                const auto& nextChunk = findChunkWithChunkIndex(hexChunks, chunk, nexti.chunkIndex);
                const auto& next = nextChunk.edges[nexti.index];
                VertexIndex ind = sharedVertexIndex(edge, next, chunkIndex(edgeChunk, nextChunk));

                Vertex vertex = findChunkWithChunkIndex(hexChunks, edgeChunk, ind.chunkIndex).vertices[ind.index];

                glVertex2d(vertex.x, vertex.y);


            }
            glEnd();
        }
    }

    std::cout << "cells done!\n";
    std::cout << "render lines around cells!\n";
    // Lines around cells
    for (auto& kv : hexChunks) {
        auto& chunk = kv.second;
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
            Vertex a = getVertex(hexChunks, chunk, edge.a);
            Vertex b = getVertex(hexChunks, chunk, edge.b);
            glVertex2d(a.x, a.y);
            glVertex2d(b.x, b.y);
            glEnd();
        }
    }
    std::cout << "lines around cells done!\n";

    std::cout << "render water points!\n";
    for (auto& kv : hexChunks) {
        auto& chunk = kv.second;
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
    std::cout << "water points done!\n";

    std::cout << "render boxes!\n";
    for (auto& kv : hexChunks) {
        auto& chunk = kv.second;
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
    std::cout << "flush!\n";

    glFlush();
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
            std::pair<int, int> pos{ x, y };
            hexChunks.insert({ pos, std::move(chunk) });
        }
    }

    for (auto& kv : hexChunks) {
        auto& chunk = kv.second;
        if (chunk.x >= minX && chunk.x <= maxX && chunk.y >= minY && chunk.y <= maxY) {
            //addMoisture(chunk);
        }
    }

    for (auto& kv : hexChunks) {
        auto& chunk = kv.second;
        if (chunk.x >= minX && chunk.x <= maxX && chunk.y >= minY && chunk.y <= maxY) {
            addBiomes(hexChunks, chunk);
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
