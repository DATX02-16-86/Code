#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>
#import "TreeLSystemBridge.h"
#include "TreeLSystem.hpp"
#include "SimpleTree.hpp"
#include "AbstractTree.hpp"
#include "ComplexTree.hpp"
#include "RealTree.hpp"

@implementation SCNVector3Wrapper

- (instancetype)initWithVector:(SCNVector3)vector {
  _vector = vector;
  return self;
}

@end

@implementation TreeLSystemBridge

- (instancetype) init {
  TreeLSystem tree = RealTree();
  auto voxels = tree.generateVoxels(6);
  NSMutableArray<SCNVector3Wrapper*>* tmpVoxels = [NSMutableArray<SCNVector3Wrapper*> array];
  for (vector<double> voxel: voxels) {
    [tmpVoxels addObject:[[SCNVector3Wrapper alloc] initWithVector:SCNVector3Make(voxel[0]/10, voxel[2]/10, -voxel[1]/10)]];
  }
  _voxels = tmpVoxels;
  return self;
}

@end

//* -------------------------------------------------------------------------------------------- *//

//#import <Foundation/Foundation.h>
//#import <SceneKit/SceneKit.h>
//#import "TreeLSystemBridge.h"
//#include "TreeLSystem.hpp"
//#include "SimpleTree.hpp"
//#include "ComplexTree.hpp"
//
//@implementation SCNVector3Wrapper
//
//- (instancetype)initWithVector:(SCNVector3)vector {
//  _vector = vector;
//  return self;
//}
//
//@end
//
//@implementation TreeLSystemBridge
//
//- (instancetype) init {
//  TreeLSystem tree = SimpleTree();
//  auto mesh = tree.generateMesh(2);
//  NSMutableArray<SCNVector3Wrapper*>* tmpVertices = [NSMutableArray<SCNVector3Wrapper*> array];
//  for (vector<double> vertex: mesh.vertices) {
//    [tmpVertices addObject:[[SCNVector3Wrapper alloc] initWithVector:SCNVector3Make(vertex[0], vertex[1], vertex[2])]];
//  }
//  _vertices = tmpVertices;
//
//  NSMutableArray<NSNumber*>* tmpIndices = [NSMutableArray<NSNumber*> array];
//  for (int index: mesh.indices) {
//    [tmpIndices addObject:[NSNumber numberWithInt:index]];
//  }
//  _indices = tmpIndices;
//  return self;
//}
//
//@end