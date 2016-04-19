
#pragma once

#include <Base.h>
#include <memory>
#include <vector>
#include "Chunk.h"
#include "ChunkMatrix.h"
#include "Filler.h"

namespace generator {
namespace landmass {

struct Generator {
    Generator(std::vector<Attribute*>&& attributes = std::vector<Attribute*>{}): usedAttributes(::move(attributes)) {}
    virtual void generate(Chunk& chunk, ChunkMatrix& matrix, I32 seed) = 0;

    AttributeId attribute(U32 index) {return attributes[index];}

private:
    friend struct LandmassStage;
    std::vector<AttributeId> attributes;
    std::vector<Attribute*> usedAttributes;
};

struct LandmassStage {
    LandmassStage(Filler& filler, U32 gridSize, U32 gridSpread): filler(filler), matrix(0, 13, gridSize, gridSpread) {}

    /// Generates attributes up to the provided stage into this voronoi chunk
    void generate(Chunk& chunk, Size stage, I32 seed);

    /// Generates a chunk of voronoi data at the provided position.
    virtual Chunk& generate(I32 x, I32 y, I32 seed);

    /// Adds an attribute generator to be applied to generated voronoi chunks.
    LandmassStage& operator += (std::unique_ptr<Generator> generator);

    /// Returns the attribute id for this attribute if it was generated.
    Maybe<AttributeId> attribute(Attribute*);

    Filler& filler;
    ChunkMatrix matrix;

private:
    std::vector<AttributeId> attributes;
    std::vector<Attribute*> attributeSources;
    std::vector<std::unique_ptr<Generator>> generators;
};

}}