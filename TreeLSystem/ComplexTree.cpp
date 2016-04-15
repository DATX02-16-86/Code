#include "TreeLSystem.hpp"
#include "ComplexTree.hpp"
#include <string>

#define COMPLEX_RULES ((map<char, string>){{'F', "Y[\\\\\\\\\\\\MF][/////NF][^^^^^OF][&&&&&PF]"},{'M', "Z/M"},{'N', "Z\\N"},{'O', "Z&O"},{'P', "Z^P"},{'Y', "Z/ZY\\"},{'Z', "ZZ"}})

ComplexTree::ComplexTree():TreeLSystem(DEGREES_TO_RADIANS(4.5), DEGREES_TO_RADIANS(4), 8.0/12.0, 8.0/12.0, "F", COMPLEX_RULES) {};