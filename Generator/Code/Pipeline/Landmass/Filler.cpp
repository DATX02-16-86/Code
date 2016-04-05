
#include "Filler.h"
#include <random>

namespace generator {
namespace landmass {

static bool vertexIsInChunk(Int chunkX, Int chunkY, Vertex p, Size chunkSize) {
    return p.x >= chunkX * chunkSize &&
           p.x < (chunkX + 1) * chunkSize &&
           p.y >= chunkY * chunkSize &&
           p.y < (chunkY + 1) * chunkSize;
}

static bool pointIsInChunk(Int chunkX, Int chunkY, Point p, Size chunkSize) {
    return vertexIsInChunk(chunkX, chunkY, {p.x(), p.y()}, chunkSize);
}

static U32 chunkSeed(I32 chunkX, I32 chunkY, I32 seed, Size chunkWidth) {
    return (U32)(chunkX * 31 + chunkY * chunkWidth) * 31 * seed;
}

static void relaxPoints(FillContext& context, Size iterations) {
    auto chunkSize = context.chunkSize;
    auto cx = context.chunkX;
    auto cy = context.chunkY;
    auto& points = context.points;

    for(int i = 0; i < iterations; ++i) {
        Diagram diagram;
        construct_voronoi(points.begin(), points.end(), &diagram);

        for(const Cell& it : diagram.cells()) {
            auto edge = it.incident_edge();
            CoordinateType x = 0;
            CoordinateType y = 0;
            int count = 0;
            do {
                edge = edge->next();
                auto edgePoints = getEdgePoints(*edge, points);
                x += edgePoints.first.x() - cx * chunkSize;
                y += edgePoints.first.y() - cy * chunkSize;
                ++count;
            } while (edge != it.incident_edge());
            points[it.source_index()].x(x / count + cx * chunkSize);
            points[it.source_index()].y(y / count + cy * chunkSize);
        }
    }
}

static void filterPointsOutsideChunks(FillContext& context) {
    auto chunkSize = context.chunkSize;
    auto cx = context.chunkX;
    auto cy = context.chunkY;
    auto& points = context.points;

    points.erase(std::remove_if(points.begin(), points.end(), [&](Point p) {
        return !pointIsInChunk(cx, cy, p, chunkSize);
    }), points.end());
}

//-----------------------------------------------------------------------------------

void RandomFiller::fill(FillContext& context) {
    auto chunkSize = context.chunkSize;
    auto cx = context.chunkX;
    auto cy = context.chunkY;
    auto& points = context.points;

    std::mt19937 gen(chunkSeed(context.chunkX, context.chunkY, seed, chunkSize));
    std::uniform_int_distribution<> distribution(0, (I32)(chunkSize - 1));

    for(int i = 0; i < count; i++) {
        int x = distribution(gen);
        int y = distribution(gen);
        points.push_back(Point(chunkSize * cx + x, chunkSize * cy + y));
    }
}

void RandomRelaxationFiller::fill(FillContext& context) {
    RandomFiller::fill(context);
    relaxPoints(context, iterations);
    filterPointsOutsideChunks(context);
}

void GridFiller::fill(FillContext& context) {
    auto chunkSize = context.chunkSize;
    auto cx = context.chunkX;
    auto cy = context.chunkY;
    auto& points = context.points;

    for(int y = 0; y <= chunkSize; y += spacing) {
        for(int x = 0; x <= chunkSize; x += spacing) {
            points.push_back(Point(chunkSize * cx + x, chunkSize * cy + y));
        }
    }
}

void HexFiller::fill(FillContext& context) {
    auto chunkSize = context.chunkSize;
    auto cx = context.chunkX;
    auto cy = context.chunkY;
    auto& points = context.points;
    auto stepsNeeded = chunkSize / spacing;

    for(Size i = 0; i < stepsNeeded; ++i) {
        for(Size j = 0; j < stepsNeeded; ++j) {
            Size x = j * spacing;
            Size y = i * spacing;
            auto offset = i & 1 ? 0 : (spacing / 2);
            points.push_back(Point(chunkSize * cx + x + offset, chunkSize * cy + y));
        }
    }
}

void RandomHexFiller::fill(FillContext& context) {
    auto cx = context.chunkX;
    auto cy = context.chunkY;
    auto chunkSize = context.chunkSize;
    auto& points = context.points;
    auto stepsNeeded = chunkSize / delta;
    auto seed = chunkSeed(cx, cy, this->seed, chunkSize);

    std::mt19937 gen(seed);
    F32 halfMaxDistance = delta / 8.f;
    std::uniform_int_distribution<> dis(0, int(halfMaxDistance));
    std::bernoulli_distribution bdis(0.5);

    for(int i = 0; i < stepsNeeded; ++i) {
        for(int j = 0; j < stepsNeeded; ++j) {
            int x = j * delta;
            int y = i * delta;
            auto hexOffset = i & 1 ? 0 : (delta / 2);
            int xOffset = int(bdis(gen) ? halfMaxDistance + dis(gen) : -halfMaxDistance - dis(gen));
            int yOffset = int(bdis(gen) ? halfMaxDistance + dis(gen) : -halfMaxDistance - dis(gen));
            points.push_back(Point(chunkSize * cx + x + hexOffset + xOffset, chunkSize * cy + y + yOffset));
        }
    }
}

}} // namespace generator::landmass
