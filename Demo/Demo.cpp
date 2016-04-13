
#include "../Generator/Code/World/World.h"

using namespace generator;
using namespace landmass;

int main() {
    World world;

    for(int x = 0; x < 20; x++) {
        for(int y = 0; y < 20; y++) {
            world.pipeline.landmass.generate(x, y);
        }
    }

    exit(0);
    return 0;
}