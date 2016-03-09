
#ifndef GENERATOR_PIPELINE_H
#define GENERATOR_PIPELINE_H

#include "Generator.h"

namespace generator {

struct Chunk;

/**
 * Contains the modules used for each generation stage.
 * Manages sending the generated data through each stage to produce an end result.
 */
struct Pipeline {
    Pipeline(U8 tileSize = 10): data(tileSize) {}

    /**
     * Fills a chunk of voxel data from this pipeline.
     * The location is provided through the chunk.
     */
    void fillChunk(Chunk& chunk);

    // Contains the intermediate data maps generated by each stage.
    struct Data {
        Data(U8 tileSize): tileSize(tileSize) {}
        TiledMatrix* get(StreamId stream);
        TiledMatrix* getOrCreate(StreamId stream, Size detail);

    private:
        void resize(Size count);

        TiledMatrix* matrices = nullptr;
        U32 count = 0;
        U8 tileSize;
    } data;

    // The stages a that are dynamically configurable.
    Stage landmassStage;
    Stage heightStage;
    Stage biomeStage;
    Stage structureStage;

    // Determines the level of detail each pipeline stage operates at.
    U8 landmassDetail = 9;
    U8 heightDetail = 7;
    U8 biomeDetail = 5;
    U8 structureDetail = 5;
};

} // namespace generator

#endif //GENERATOR_PIPELINE_H
