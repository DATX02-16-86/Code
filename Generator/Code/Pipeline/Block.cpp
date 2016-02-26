#include <vector>
#include "Block.h"

namespace generator {

static std::vector<Block> registeredBlocks;

const Block& registerBlock(Block::Phase phase, F32 opacity, F32 friction, F32 hardness) {
    registeredBlocks.push_back(Block {(U32)registeredBlocks.size(), opacity, friction, hardness, phase});
    return registeredBlocks.back();
}

	
namespace block {
	const Block air = registerBlock(Block::Gaseous, 0, 0, 0);
	const Block solid = registerBlock();
} // namespace Block
} // namespace generator
