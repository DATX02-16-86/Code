
#ifndef GENERATOR_GENERATOR_H
#define GENERATOR_GENERATOR_H

#include <Base.h>
#include <vector>
#include "Matrix.h"
#include <memory>
#ifndef _WIN32
#include <alloca.h>
#endif //!_WIN32

namespace generator {

struct StreamId {
    U16 id;
    U16 itemBits;
};

struct StreamCounter {
    static Size currentId;
};

#define GeneratorStream(name, bits) static const StreamId name {(U16)StreamCounter::currentId++, (U16)bits};


struct Pipeline;

/// Defines the location of a subset of the world.
struct Segment {
    I32 x; /// The x-position of the chunk left-upper corner.
    I32 y; /// The y-position of the chunk left-upper corner.
    U32 width; /// The size over the x-axis in world units.
    U32 height; /// The size over the y-axis in world units.
    F32 baseScale; /// The base scale to use for the generated features.
    U32 detail; /// The detail factor of the segment, used to determine sampling from different maps.

    /// Calls the provided mapper for each element in this segment.
    template<class F> void map(F&& f) const {
        auto step = Size(1) << detail;
        auto yMax = y + height;
        auto xMax = x + width;
        for(auto row = y; row < yMax; row += step) {
            for(auto column = x; column < xMax; column += step) {
                f(column, row);
            }
        }
    }
};

/// The common interface for generators inside a stage.
struct Generator {
    Generator(std::vector<StreamId> auxiliaryStreams): auxiliaryStreams(auxiliaryStreams) {}
    virtual void generate(const Segment& segment, IdMatrix** auxiliaries, Pipeline& pipeline) = 0;

    const std::vector<StreamId> auxiliaryStreams;
};

/// The common interface for generation stages inside the pipeline.
struct Stage {
    virtual void generate(const Segment& segment, Pipeline& pipeline) {
        for(auto& g: generators) {
            auto streams = (IdMatrix**)alloca(sizeof(IdMatrix*) * g->auxiliaryStreams.size());
            g->generate(segment, streams, pipeline);
        }
    }

    Stage& operator += (std::unique_ptr<Generator> generator) {
        generators.push_back(move(generator));
        return *this;
    }

private:
    std::vector<std::unique_ptr<Generator>> generators;
};

} // namespace generator

#endif //GENERATOR_GENERATOR_H
