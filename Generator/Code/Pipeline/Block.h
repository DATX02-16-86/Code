
#ifndef GENERATOR_BLOCK_H
#define GENERATOR_BLOCK_H

#include "Base.h"

namespace generator {

/// Currently, block ids are represented as bytes. We may want to change this to 16-bit values
/// in the future if it turns out that more than 256 block types are used in one world.
using BlockId = U8;

struct Block {
    enum Phase {
        Solid,
        Fluid,
        Gaseous
    };

    /// The block id, which is stored and used as index to metadata.
    U32 id;

    /// The opacity of the block from a lighting perspective.
    /// Values under 1 indicate that light can go through the block.
    F32 opacity;

    /// The average amount of friction the block surface has.
    F32 friction;

    /// How "hard" the block material is. This affects the created shape when removing blocks.
    F32 hardness;

    /// The phase of the block material. Affects how it interacts with the world.
    Phase phase;
};

/**
 * Registers a new Block and returns its block id.
 * This must be done for all blocks that are used through the pipeline.
 * Block types are used in voxel chunk generation and determine how voxels are rendered.
 */
const Block& registerBlock(Block::Phase phase = Block::Solid, F32 opacity = 1.f, F32 friction = 1.f, F32 hardness = 1.f);


/*
 * Default block types.
 */

namespace block {

/// Represents an empty block. Empty voxels should have this block type, as it may be used for compression.
extern const Block& air;

/// Represents a generic solid block, used for testing.
extern const Block& solid;

} // namespace block
} // namespace generator

#endif // GENERATOR_BLOCK_H
