#include "SimpleTree.hpp"
#include <string>

#define SIMPLE_RULES ((map<char, string>){{'X', "Y[+X]//[+X]//[+X]//[+X]"},{'Y', "YY"}})

SimpleTree::SimpleTree():TreeLSystem(DEGREES_TO_RADIANS(45), DEGREES_TO_RADIANS(40), 8.0/12.0, 9.0/10.0, "X", SIMPLE_RULES) {};