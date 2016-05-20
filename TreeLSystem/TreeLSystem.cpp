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

vector<int> PerformRotationDiscrete(Matrix rot, vector<double> vec) {
  vector<int> product {0, 0, 0};
  
  for (int row = 0; row < 3; row++) {
    auto result = 0.0;
    for (int inner = 0; inner < 3; inner++) {
      result += rot[row][inner]*vec[inner];
    }
    product[row] = round(result);
  }
  
  return product;
}

/**
 Rasterizes a 2D line going through origin (0,0) to point (x,y).
 
 @param x The x-coordinate.
 @param y The y-coordinate.
 @return An array of 2D-coordinates representing the line.
 */
vector<vector<int>> rasterize2DLine(int endX, int endY, int startX = 0, int startY = 0) {
  auto negDirX = endX < startX;
  auto negDirY = endY < startY;
  if (negDirX) swap(startX, endX);
  if (negDirY) swap(startY, endY);
  
  auto dx = endX-startX, dy = endY-startY;
  auto tall = dx < dy;
  if (tall) { swap(startX, startY); swap(endX, endY); swap(dx, dy); }
  
  auto line = vector<vector<int>>(dx, vector<int>(2));
  
  auto D = dy - dx;
  auto ystep = 0;
  for (int xstep = 0; xstep < dx; xstep++) {
    line[xstep][0] = negDirX ? (tall ? endY-ystep : endX-xstep) : (tall ? startX+ystep : startX+xstep);
    line[xstep][1] = negDirY ? (tall ? endX-xstep : endY-ystep) : (tall ? startX+xstep : startY+ystep);
    if (D >= 0) {
      ystep++;
      D -= dx;
    }
    D += dy;
  }
  return line;
}

/**
 Rasterizes a 3D line going through origin (0,0,0) to point (x,y,z).
 
 @param x The x-coordinate.
 @param y The y-coordinate.
 @param z The y-coordinate.
 @return An array of #D-coordinates representing the line.
 */
vector<vector<int>> rasterize3DLine(int endX, int endY, int endZ, int startX = 0, int startY = 0, int startZ = 0) {
  auto negDirX = endX < startX;
  auto negDirY = endY < startY;
  auto negDirZ = endZ < startZ;
  if (negDirX) swap(startX, endX);
  if (negDirY) swap(startY, endY);
  if (negDirZ) swap(startZ, endZ);
  
  auto dx = endX - startX, dy = endY - startY, dz = endZ - startZ;
  auto deep = dx < dy && dz < dy;
  auto tall = !deep && dx < dz;
  if      (deep) { swap(startX, startY); swap(endX, endY); swap(dx, dy); }
  else if (tall) { swap(startX, startZ); swap(endX, endZ); swap(dx, dz); }
  
  auto line = vector<vector<int>>(dx, vector<int>(3));
  
  auto Dy = dy - dx;
  auto Dz = dz - dx;
  auto ystep = 0;
  auto zstep = 0;
  for (int xstep = 0; xstep < dx; xstep++) {
    line[xstep][0] = negDirX ? (deep ? endY-ystep : (tall ? endZ-zstep : endX-xstep)) : (deep ? startY+ystep : (tall ? startZ+zstep : startX+xstep));
    line[xstep][1] = negDirY ? (deep ? endX-xstep : endY-ystep) : (deep ? startX+xstep : startY+ystep);
    line[xstep][2] = negDirZ ? (tall ? endX-xstep : endZ-zstep) : (tall ? startX+xstep : startZ+zstep);
    if (Dy >= 0) {
      ystep++;
      Dy -= dx;
    }
    if (Dz >= 0) {
      zstep++;
      Dz -= dx;
    }
    Dy += dy;
    Dz += dz;
  }
  return line;
}

typedef int Axis;
Axis calculateDrivingAxis(int x, int y, int z) {
  auto posX = abs(x), posY = abs(y), posZ = abs(z);
  if (posX >= posY && posX >= posZ) {
    return 0b001;
  }
  if (posY >= posX && posY >= posZ) {
    return 0b010;
  }
  return 0b100;
}

// TODO: Fix this.
vector<int> convert3DPoint(int x, int y, int z, Axis to, Axis from = 0b001) {
//  switch (~to & ~from & 0b111) {
//    case 0b001:
//      return {y, x, z};
//    case 0b010:
//      return {x, z, y};
//    case 0b100:
//      return {z, y, x};
//  }
  return {x, y, z};
}

// TODO: Add skew.
vector<vector<int>> rasterize3DCircle(int radiusX, int radiusY, int radiusZ, int originX = 0, int originY = 0, int originZ = 0) {
  auto r = max({abs(radiusX), abs(radiusY), abs(radiusZ)});
  auto drivingAxis = calculateDrivingAxis(radiusX, radiusY, radiusZ);
//  auto tangentXZ = rasterize2DLine(radiusX+1, radiusZ+1, -radiusX, -radiusZ);
//  auto tangentYZ = rasterize2DLine(radiusY+1, radiusZ+1, -radiusY, -radiusZ);
  
  auto arc = vector<vector<int>>();
  auto x = r, y = 0;
  auto dx = 1-2*r, dy = 1;
  auto re = 0;
  auto smoothEndPoint = false;
  while (x >= y) {
    if (x == y) smoothEndPoint = true;
    arc.push_back({x, y});
    y += 1;
    re += dy;
    dy += 2;
    if (2*re+dx > 0) {
      x -= 1;
      re += dx;
      dx += 2;
    }
  }
  
  auto quadrantSize = 2*arc.size()-1-smoothEndPoint;
  auto circle = vector<vector<int>>(quadrantSize == 0 ? 1 : 4*quadrantSize, vector<int>(3));
  for (auto coordinate : arc) {
    auto x = coordinate[0], y = coordinate[1];
    vector<vector<int>> a1 = {{x, y}, {-y, x}, {-x, -y}, {y, -x}};
    vector<vector<int>> a2 = {{y, x}, {-x, y}, {-y, -x}, {x, -y}};
    for (int i = 0; i < (quadrantSize == 0 ? 1 : 4); i++) {
      auto c = a1[i];
      circle[quadrantSize*i+y] = convert3DPoint(originX + c[0], originY + c[1], originZ, drivingAxis);
      if (y != 0 && y < quadrantSize-y) {
        c = a2[i];
        circle[quadrantSize*(i+1)-y] = convert3DPoint(originX + c[0], originY + c[1], originZ, drivingAxis);
      }
    }
  }

  return circle;
}

TreeLSystem::TreeLSystem(double branchingAngle, double phyllotacticAngle, double lengthRatio, double diameterRatio,
                         int initialBranchLength, int initialBranchDiameter, string axiom, map<char, string> rules)
: branchingAngle(branchingAngle), phyllotacticAngle(phyllotacticAngle), branchLengthRatio(lengthRatio), branchDiameterRatio(diameterRatio),
initialBranchLength(initialBranchLength), initialBranchDiameter(initialBranchDiameter), axiom(axiom), rules(rules) {};

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

//void addBranch(double lengthRatio, double diameterRatio, Vector *position, Matrix rotation, Mesh* mesh) {
//  auto noisedLengthRatio = randRange(lengthRatio*0.9, lengthRatio*1.1);
//  Vector points[VERTEX_COUNT];
//  auto _pos = (*position);
//
//  Vector dir = PerformRotation(rotation, {0.0, 1.0, 0.0});
//  for (int i = 0; i < 3; i++) {
//    (*position)[i] += dir[i]*noisedLengthRatio;
//  }
//  for (int i = 0; i < BRANCH_RADIAL_COUNT; i++) {
//    points[i] = {
//      cos(2*M_PI/BRANCH_RADIAL_COUNT*i)/10*diameterRatio,
//      0,
//      sin(2*M_PI/BRANCH_RADIAL_COUNT*i)/10*diameterRatio
//    };
//  }
//  for (int i = BRANCH_RADIAL_COUNT; i < BRANCH_RADIAL_COUNT*2; i++) {
//    points[i] = {
//      cos(2*M_PI/BRANCH_RADIAL_COUNT*i)/10*diameterRatio,
//      noisedLengthRatio,
//      sin(2*M_PI/BRANCH_RADIAL_COUNT*i)/10*diameterRatio
//    };
//  }
//#ifdef DEBUG_TREELSYSTEM
//  points[VERTEX_COUNT-2] = {0, 0, 0};
//  points[VERTEX_COUNT-1] = {0, noisedLengthRatio, 0};
//#endif
//  for (int i = 0; i < VERTEX_COUNT; i++) {
//    // Rotate point in the direction of the turtle.
//    points[i] = PerformRotation(rotation, points[i]);
//    // Translate point to the current position of the turtle.
//    for (int j = 0; j < 3; j++) {
//      points[i][j] += _pos[j];
//    }
//  }
//
//  // Push the points and its indices to the mesh.
//  for (Vector vector: points) {
//    mesh->vertices.push_back(vector);
//  }
//  auto verticesCount = (int)mesh->vertices.size();
//  for (int i = 0; i < BRANCH_RADIAL_COUNT; i++) {
//    mesh->indices.push_back(verticesCount+i);
//    mesh->indices.push_back(verticesCount+BRANCH_RADIAL_COUNT+i);
//    mesh->indices.push_back(verticesCount+(i+1)%BRANCH_RADIAL_COUNT);
//
//    mesh->indices.push_back(verticesCount+BRANCH_RADIAL_COUNT+i);
//    mesh->indices.push_back(verticesCount+BRANCH_RADIAL_COUNT+(i+1)%BRANCH_RADIAL_COUNT);
//    mesh->indices.push_back(verticesCount+(i+1)%BRANCH_RADIAL_COUNT);
//  }
//}

VoxelCollection TreeLSystem::generateVoxels(int iterations) {
  VoxelCollection voxels;
  vector<double> currentPosition = { 0, 0, 0 };
  Matrix currentRotation = {
    { 1.0, 0.0, 0.0  },
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0  }
  };
  int currentBranchLength   = this->initialBranchLength;
  int currentBranchDiameter = this->initialBranchDiameter;
  stack<vector<double>> positionStack;
  stack<Matrix> rotationStack;
  auto generation = produce(iterations);
  for (char elem: generation) {
    double variation = 1.0; //randRange(0.9, 1.1);
    switch (elem) {
      case TurnLeft:
        currentRotation = PerformRotation(currentRotation, RZ(phyllotacticAngle*variation));
        break;
      case TurnRight:
        currentRotation = PerformRotation(currentRotation, RZ(-phyllotacticAngle*variation));
        break;
      case RollLeft:
        currentRotation = PerformRotation(currentRotation, RY(branchingAngle*variation));
        break;
      case RollRight:
        currentRotation = PerformRotation(currentRotation, RY(-branchingAngle*variation));
        break;
      case PitchDown:
        currentRotation = PerformRotation(currentRotation, RX(branchingAngle*variation));
        break;
      case PitchUp:
        currentRotation = PerformRotation(currentRotation, RX(-branchingAngle*variation));
        break;
      case Push:
        positionStack.push(currentPosition);
        rotationStack.push(currentRotation);
        currentBranchLength   = pow(this->branchLengthRatio, positionStack.size()) * this->initialBranchLength;
        currentBranchDiameter = pow(this->branchDiameterRatio, positionStack.size()) * this->initialBranchDiameter;
        break;
      case Pop:
        currentPosition = positionStack.top();
        currentRotation = rotationStack.top();
        positionStack.pop();
        rotationStack.pop();
        currentBranchLength   = pow(this->branchLengthRatio, positionStack.size()) * this->initialBranchLength;
        currentBranchDiameter = pow(this->branchDiameterRatio, positionStack.size()) * this->initialBranchDiameter;
        break;
      case Leaf:
        // TODO: Add sphere of green voxels.
        break;
      default:
        auto variedBranchLength = currentBranchLength * randRange(0.9, 1.1);
        auto radius = currentBranchDiameter / 2;
        
        double x0 = (int)currentPosition[0];
        double y0 = (int)currentPosition[1];
        double z0 = (int)currentPosition[2];
        
        auto shift = PerformRotationDiscrete(currentRotation, {0.0, 0.0, (double)variedBranchLength});
        auto skew = PerformRotationDiscrete(currentRotation, {(double)radius, 0.0, 0.0});
        auto shiftLine = rasterize3DLine(shift[0], shift[1], shift[2]);
        
        for (int i = 0; i < 3; i++)
          currentPosition[i] += shift[i];
        
        for (auto shiftCoordinate : shiftLine) {
          auto shiftX = shiftCoordinate[0], shiftY = shiftCoordinate[1], shiftZ = shiftCoordinate[2];
          auto circle = rasterize3DCircle(radius, 0, 0);//rasterize3DCircle(skew[0], skew[1], skew[2]);
          
          for (auto coordinate : circle) {
            voxels.push_back({x0 + shiftX + coordinate[0], y0 + shiftY + coordinate[1], z0 + shiftZ});
          }
        }
        break;
    }
  }
  
  return voxels;
}