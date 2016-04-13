
#include "Generator.h"

namespace generator {
namespace landmass {

void LandmassStage::generate(Chunk& chunk, Size stage, I32 seed) {
    chunk.build(matrix, filler, attributes.data(), attributes.size());

    // Make sure all neighbours have generated any data this stage may depend on.
    if(stage > 1) {
        chunk.mapNeighbours(matrix, [=](Chunk &n) {
            generate(n, stage - 1, seed);
        });
    }

    // Generate up to the requested stage.
    Size i = chunk.generatorStage;
    while(i < stage) {
        generators[stage]->generate(chunk, seed);
        i++;
    }

    chunk.generatorStage = (U16)stage;
}

void LandmassStage::generate(I32 x, I32 y, I32 seed) {
    auto& chunk = matrix.getChunk(x, y);
    generate(chunk, generators.size(), seed);
}

LandmassStage& LandmassStage::operator += (std::unique_ptr<Generator> generator) {
    generators.push_back(::move(generator));
    for(Attribute* a: generator->usedAttributes) {
        U32 i = 0;
        auto max = attributeSources.size();
        for(; i < max; i++) {
            if(a == attributeSources[i]) {
                generator->attributes.push_back(attributes[i]);
            }
        }

        if(i >= max) {
            attributes.push_back(AttributeId((U16)attributes.size(), a->itemBits, a->type));
            generator->attributes.push_back(attributes[max]);
        }
    }
    return *this;
}

}}