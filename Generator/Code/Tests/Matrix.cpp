
#include <catch.hpp>
#include "../Pipeline/Matrix.h"

using namespace generator;

TEST_CASE("IdMatrix") {
    IdMatrix matrix(128, 128, 0, 11);
    matrix.set(33, 33, 0, 5);
    REQUIRE(matrix.get(33, 33, 0) == 5);

	SECTION("Medium matrix") {
		for(Size x = 0; x < 128; x++) {
			for(Size y = 0; y < 128; y++) {
				matrix.set(x, y, 0, (x * y) & 0b11111111111);
			}
		}
		
		for(Size x = 0; x < 128; x++) {
			for(Size y = 0; y < 128; y++) {
				CAPTURE(x);
				CAPTURE(y);
				REQUIRE(matrix.get(x, y, 0) == ((x * y) & 0b11111111111));
			}
		}
	}

	SECTION("Large matrix") {
		IdMatrix largeMatrix(256, 256, 0, 32);
		for(Size x = 0; x < 256; x++) {
			for(Size y = 0; y < 256; y++) {
				largeMatrix.set(x, y, 0, (x * y) & 0xffffffff);
			}
		}
		
		for(Size x = 0; x < 256; x++) {
			for(Size y = 0; y < 256; y++) {
				CAPTURE(x);
				CAPTURE(y);
				REQUIRE(largeMatrix.get(x, y, 0) == ((x * y) & 0xffffffff));
			}
		}
	}
	
	SECTION("Small matrix") {
		IdMatrix smallMatrix(64, 64, 0, 2);
		for(Size x = 0; x < 64; x++) {
			for(Size y = 0; y < 64; y++) {
				smallMatrix.set(x, y, 0, (x * y) & 0b11);
			}
		}
		
		for(Size x = 0; x < 64; x++) {
			for(Size y = 0; y < 64; y++) {
				CAPTURE(x);
				CAPTURE(y);
				REQUIRE(smallMatrix.get(x, y, 0) == ((x * y) & 0b11));
			}
		}
	}
}

TEST_CASE("TiledMatrix single tile") {
	TiledMatrix matrix(0, 16, 7);
	for(Int x = 0; x < 128; x++) {
		for(Int y = 0; y < 128; y++) {
			matrix.set(x, y, 0, (x * y) & 0b1111111);
		}
	}
	
	for(Int x = 0; x < 128; x++) {
		for(Int y = 0; y < 128; y++) {
			CAPTURE(x);
			CAPTURE(y);
			REQUIRE(matrix.get(x, y, 0) == ((x * y) & 0b1111111));
		}
	}
}

TEST_CASE("TiledMatrix multiple tiles") {
	TiledMatrix matrix(0, 16, 4);
	for(Int x = -231; x < 111; x++) {
		for(Int y = -17; y < 33; y++) {
			matrix.set(x, y, 0, (x * y) & 0xffff);
		}
	}
	
	for(Int x = -231; x < 111; x++) {
		for(Int y = -17; y < 33; y++) {
			CAPTURE(x);
			CAPTURE(y);
			REQUIRE(matrix.get(x, y, 0) == ((x * y) & 0xffff));
		}
	}
}

TEST_CASE("TiledMatrix OOB") {
	TiledMatrix matrix(0, 16, 4);
	for(Int x = -231; x < 111; x++) {
		for(Int y = -17; y < 33; y++) {
			matrix.set(x, y, 0, (x * y) & 0xffff);
		}
	}
	
	REQUIRE(matrix.get(300, 300, 0) == 0);
}