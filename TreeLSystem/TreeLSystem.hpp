
#ifndef TreeLSystem_hpp
#define TreeLSystem_hpp
#define _USE_MATH_DEFINES

#include <math.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <list>
#include <vector>

#define DEBUG_TREELSYSTEM
#ifdef DEBUG_TREELSYSTEM
  #define VERTEX_COUNT BRANCH_RADIAL_COUNT*2+2
#else
  #define VERTEX_COUNT BRANCH_RADIAL_COUNT*2
#endif

#define DEGREES_TO_RADIANS(a) (a/180.0*M_PI)
#define RADIANS_TO_DEGREES(a) (a/M_PI*180.0)
#define RX(angle) ((Matrix){{1, 0, 0}, {0, cos(angle), -sin(angle)}, {0, sin(angle), cos(angle)}})
#define RY(angle) ((Matrix){{cos(angle), 0, sin(angle)}, {0, 1, 0}, {-sin(angle), 0, cos(angle)}})
#define RZ(angle) ((Matrix){{cos(angle), -sin(angle), 0}, {sin(angle), cos(angle), 0}, {0, 0, 1}})
#define VEC_LENGTH(vec) (sqrt(left[0]*left[0]+left[1]*left[1]+left[2]*left[2]))
#define BRANCH_RADIAL_COUNT 5

using namespace std;

typedef vector<double> Vector;
typedef vector<vector<double>> Matrix;

Matrix PerformRotation(Matrix mat1, Matrix mat2);

struct Mesh {
  list<Vector> vertices;
  list<int> indices;
};

enum DrawRule {
  TurnLeft = '-',
  TurnRight = '+',
  RollLeft = '\\',
  RollRight = '/',
  PitchUp = '^',
  PitchDown = '&',
  Push = '[',
  Pop = ']'
};


struct TreeLSystem {
  TreeLSystem(double branchingAngle, double phyllotacticAngle, double lengthRatio, double diameterRatio, const string axiom, map<char, string> rules);
  
  Mesh generateMesh(int iterations);
  
protected:
  double branchingAngle, phyllotacticAngle, lengthRatio, diameterRatio;
  const string axiom;
  map<char, string> rules;
private:
  string nextGeneration(string currentGeneration);
  string produce(int iterations);
};

#endif /* TreeLSystem_hpp */
