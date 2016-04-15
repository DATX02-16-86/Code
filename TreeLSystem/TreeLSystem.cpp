#include "TreeLSystem.hpp"
#include "ComplexTree.hpp"
#include <stdio.h>
#include <random>
#include <cmath>
#include <stack>
#include <list>

Matrix PerformRotation(Matrix mat1, Matrix mat2) {
  Matrix product = {
    {0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0}
  };
  
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      for (int inner = 0; inner < 3; inner++) {
        product[row][col] += mat1[row][inner]*mat2[inner][col];
      }
    }
  }
  
  return product;
}

Vector PerformRotation(Matrix rot, Vector vec) {
  Vector product = {0.0, 0.0, 0.0};
  for (int row = 0; row < 3; row++) {
    for (int inner = 0; inner < 3; inner++) {
      product[row] += rot[row][inner]*vec[inner];
    }
  }
  return product;
}

TreeLSystem::TreeLSystem(double branchingAngle,
                         double phyllotacticAngle,
                         double lengthRatio,
                         double diameterRatio,
                         const string axiom,
                         map<char, string> rules): branchingAngle(branchingAngle), phyllotacticAngle(phyllotacticAngle), lengthRatio(lengthRatio), diameterRatio(diameterRatio), axiom(axiom), rules(rules) {};

string TreeLSystem::nextGeneration(string currentGeneration) {
  string rule, result = "";
  for (char elem: currentGeneration) {
    if (rules.count(elem)) {
      rule = rules[elem];
      result += rule;
    } else {
      result += elem;
    }
  }
  return result;
}

string TreeLSystem::produce(int iterations) {
  string result = axiom;
  for (; iterations > 0; iterations--) {
    result = nextGeneration(result);
  }
  return result;
}

double randRange(double low, double high) {
  return (double)rand()/RAND_MAX*(high-low)+low;
}

void addBranch(double lengthRatio, double diameterRatio, Vector *position, Matrix rotation, Mesh* mesh) {
  auto noisedLengthRatio = randRange(lengthRatio*0.9, lengthRatio*1.1);
  Vector points[VERTEX_COUNT];
  auto _pos = (*position);
  
  Vector dir = PerformRotation(rotation, {0.0, 1.0, 0.0});
  for (int i = 0; i < 3; i++) {
    (*position)[i] += dir[i]*noisedLengthRatio;
  }
  for (int i = 0; i < BRANCH_RADIAL_COUNT; i++) {
    points[i] = {
      cos(2*M_PI/BRANCH_RADIAL_COUNT*i)/10*diameterRatio,
      0,
      sin(2*M_PI/BRANCH_RADIAL_COUNT*i)/10*diameterRatio
    };
  }
  for (int i = BRANCH_RADIAL_COUNT; i < BRANCH_RADIAL_COUNT*2; i++) {
    points[i] = {
      cos(2*M_PI/BRANCH_RADIAL_COUNT*i)/10*diameterRatio,
      noisedLengthRatio,
      sin(2*M_PI/BRANCH_RADIAL_COUNT*i)/10*diameterRatio
    };
  }
#ifdef DEBUG_TREELSYSTEM
  points[VERTEX_COUNT-2] = {0, 0, 0};
  points[VERTEX_COUNT-1] = {0, noisedLengthRatio, 0};
#endif
  for (int i = 0; i < VERTEX_COUNT; i++) {
    // Rotate point in the direction of the turtle.
    points[i] = PerformRotation(rotation, points[i]);
    // Translate point to the current position of the turtle.
    for (int j = 0; j < 3; j++) {
      points[i][j] += _pos[j];
    }
  }
  
  // Push the points and its indices to the mesh.
  for (Vector vector: points) {
    mesh->vertices.push_back(vector);
  }
  auto verticesCount = (int)mesh->vertices.size();
  for (int i = 0; i < BRANCH_RADIAL_COUNT; i++) {
    mesh->indices.push_back(verticesCount+i);
    mesh->indices.push_back(verticesCount+BRANCH_RADIAL_COUNT+i);
    mesh->indices.push_back(verticesCount+(i+1)%BRANCH_RADIAL_COUNT);
    
    mesh->indices.push_back(verticesCount+BRANCH_RADIAL_COUNT+i);
    mesh->indices.push_back(verticesCount+BRANCH_RADIAL_COUNT+(i+1)%BRANCH_RADIAL_COUNT);
    mesh->indices.push_back(verticesCount+(i+1)%BRANCH_RADIAL_COUNT);
  }
}

Mesh TreeLSystem::generateMesh(int iterations) {
  Mesh mesh;
  Vector currentPosition = { 0, 0, 0 };
  Matrix currentRotation = {
    { 1.0, 0.0, 0.0  },
    { 0.0, 1.0, 0.0  },
    { 0.0, 0.0, 1.0 }
  };
  stack<Vector> positionStack;
  stack<Matrix> rotationStack;
  auto generation = produce(iterations);
  for (char elem: generation) {
    switch (elem) {
      case TurnLeft:
        currentRotation = PerformRotation(currentRotation, RY(randRange(phyllotacticAngle*0.9, phyllotacticAngle*1.1)));
        break;
      case TurnRight:
        currentRotation = PerformRotation(currentRotation, RY(randRange(-phyllotacticAngle*1.1, -phyllotacticAngle*0.9)));
        break;
      case RollLeft:
        currentRotation = PerformRotation(currentRotation, RZ(randRange(branchingAngle*0.9, branchingAngle*1.1)));
        break;
      case RollRight:
        currentRotation = PerformRotation(currentRotation, RZ(randRange(-branchingAngle*1.1, -branchingAngle*0.9)));
        break;
      case PitchDown:
        currentRotation = PerformRotation(currentRotation, RX(randRange(-branchingAngle*1.1, -branchingAngle*0.9)));
        break;
      case PitchUp:
        currentRotation = PerformRotation(currentRotation, RX(randRange(branchingAngle*0.9, branchingAngle*1.1)));
        break;
      case Push:
        positionStack.push(currentPosition);
        rotationStack.push(currentRotation);
        break;
      case Pop:
      {
        currentPosition = positionStack.top();
        currentRotation = rotationStack.top();
        positionStack.pop();
        rotationStack.pop();
        break;
      }
      default:
        addBranch(pow(this->lengthRatio, positionStack.size()),
                  pow(this->diameterRatio, positionStack.size()),
                  &currentPosition,
                  currentRotation,
                  &mesh);
        break;
    }
  }
  
  return mesh;
}
