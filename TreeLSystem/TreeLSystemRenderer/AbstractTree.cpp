#include "AbstractTree.hpp"
#include <string>

#define ABSTRACT_RULES ((map<char, string>){{'X', "F+\\F[X]/F+/F[/X]\\F[/X]\\X"},{'F', "FFF"}})

AbstractTree::AbstractTree():TreeLSystem(M_PI_4, M_PI_4, 1.0, 1.0, 25, 6, "X", ABSTRACT_RULES) {};