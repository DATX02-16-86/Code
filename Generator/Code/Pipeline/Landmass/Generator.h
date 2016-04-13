
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
    virtual void generate(Chunk& chunk) = 0;

    AttributeId attribute(U32 index) {return attributes[index];}

private:
    friend struct LandmassStage;
    std::vector<AttributeId> attributes;
    std::vector<Attribute*> usedAttributes;
};

struct LandmassStage {
    LandmassStage(Filler& filler): filler(filler) {}

    void generate(Chunk& chunk, Size stage);
    virtual void generate(I32 x, I32 y);
    LandmassStage& operator += (std::unique_ptr<Generator> generator);

private:
    std::vector<AttributeId> attributes;
    std::vector<Attribute*> attributeSources;
    std::vector<std::unique_ptr<Generator>> generators;

    Filler& filler;
    ChunkMatrix matrix;
};

}}