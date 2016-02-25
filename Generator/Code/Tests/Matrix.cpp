
#include "../Pipeline/Matrix.h"
#include <catch.hpp>

using namespace generator;

TEST_CASE("Matrix types") {
    IdMatrix matrix(128, 128, 0, 11);
    matrix.set(33, 33, 0, 5);
    REQUIRE(matrix.get(33, 33, 0) == 5);

    for(Size x = 0; x < 128; x++) {
        for(Size y = 0; y < 128; y++) {
            matrix.set(x, y, 0, (x * y) & 0b1111);
        }
    }

    for(Size x = 0; x < 128; x++) {
        for(Size y = 0; y < 128; y++) {
            REQUIRE(matrix.get(x, y, 0) == ((x * y) & 0b1111));
        }
    }
}