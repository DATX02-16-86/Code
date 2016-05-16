#include "TreeLSystem.hpp"
#include "RealTree.hpp"
#include <string>

#define REAL_RULES ((map<char, string>){{'F', "Y[^^^^F*]++++++[^^^^F*]++++++[^^^^F*]+++++[^ZF*]"}, {'Y', "Y^Z&+++"}})

RealTree::RealTree():TreeLSystem(DEGREES_TO_RADIANS(10), DEGREES_TO_RADIANS(18), 0.9, 0.65, 50, 5, "F", REAL_RULES) {};