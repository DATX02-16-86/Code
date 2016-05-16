#include "SimpleTree.hpp"
#include <string>

#define SIMPLE_RULES ((map<char, string>){{'X', "Y[/X]+[/X]+[/X]+[/X]"},{'Y', "YY"}})

SimpleTree::SimpleTree():TreeLSystem(M_PI_4, M_PI_2, 0.67, 0.5, 30, 6, "X", SIMPLE_RULES) {};