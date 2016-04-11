
#pragma once

#include <Base.h>
#include "Voronoi.h"

namespace generator {
namespace landmass {

struct FillContext {
    std::vector<Point>& points;
    const I32 chunkX;
    const I32 chunkY;
    const U32 chunkSize;
};

/// Interface for voronoi diagram point fillers.
/// This defines the base shape of the diagram on which metadata is laid out
/// by laying out the center points of each voronoi cell.
struct Filler {
    virtual void fill(FillContext& context) = 0;
};

/// Fills the voronoi diagram with completely random data (not recommended).
struct RandomFiller: Filler {
    RandomFiller(U32 count, I32 seed): count(count), seed(seed) {}

    const U32 count;
    const I32 seed;

    virtual void fill(FillContext& context) override;
};

/// Creates a voronoi diagram from random data with lloyd relaxation.
struct RandomRelaxationFiller: RandomFiller {
    RandomRelaxationFiller(U32 count, I32 seed, U32 iterations): RandomFiller(count, seed), iterations(iterations) {}

    const U32 iterations;

    virtual void fill(FillContext& context) override;
};

/// Creates a regular square grid.
struct GridFiller: Filler {
    GridFiller(U32 spacing): spacing(spacing) {}

    const U32 spacing;

    virtual void fill(FillContext& context) override;
};

/// Creates a regular hexagonal grid.
struct HexFiller: Filler {
    HexFiller(U32 spacing): spacing(spacing) {}

    const U32 spacing;

    virtual void fill(FillContext& context) override;
};

/// Creates a semi-random hexagonal grid - this gives the best result.
struct RandomHexFiller: Filler {
    RandomHexFiller(U32 delta, I32 seed): delta(delta), seed(seed) {}

    const U32 delta;
    const I32 seed;

    virtual void fill(FillContext& context) override;
};

}} // namespace generator::landmass
