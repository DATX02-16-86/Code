
#include "../Generator/Code/Pipeline/Landmass/Chunk.h"

using namespace generator;
using namespace landmass;

int main() {
    landmass::RandomHexFiller filler(64, 1);
    landmass::ChunkMatrix matrix(0, 4096);

    for(int x = -10; x < 10; x++) {
        for(int y = -10; y < 10; y++) {
            matrix.getChunk(x, y).connectEdges(matrix, filler);
        }
    }

    return 0;
}