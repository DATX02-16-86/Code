
#include "../Generator/Code/World/World.h"

using namespace generator;
using namespace landmass;

int main() {
    I32 seed = 1;
    World world(seed);

    for(int x = 0; x < 20; x++) {
        for(int y = 0; y < 20; y++) {
            world.pipeline.landmass.generate(x, y, seed);
        }
    }

    exit(0);
    return 0;
}