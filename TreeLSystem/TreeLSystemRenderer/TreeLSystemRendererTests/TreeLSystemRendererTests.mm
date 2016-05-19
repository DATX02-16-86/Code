//
//  TreeLSystemRendererTests.m
//  TreeLSystemRenderer
//
//  Created by Hampus Lidin on 2016-04-02.
//  Copyright © 2016 DATX02-16-86. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "TreeLSystemBridge.h"
#include "TreeLSystem.hpp"
#include <iostream>
#include <string>
#include <cstdio>

using namespace std;

@interface TreeLSystemRendererTests : XCTestCase

@end

void print_matrix(Matrix mat) {
  NSString* m = @"⎡                   ⎤\n";
  auto max = mat.size();
  for (int i = 0; i < max; i++) {
    m = [m stringByAppendingString:@"⎢ "];
    for (auto col: mat[i]) {
      m = [m stringByAppendingString:[(col < 0 ? @"" : @" ") stringByAppendingString:[NSString stringWithFormat:@"%.2f ", col]]];
    }
    m = [m stringByAppendingString:@"⎥\n"];
    if (i < max-1) {
      m = [m stringByAppendingString:@"⎢                   ⎥\n"];
    } else {
      m = [m stringByAppendingString:@"⎣                   ⎦\n"];
    }
  }
  printf("%s", [m UTF8String]);
}

void print_vector(SCNVector3 vec) {
  printf("[ %0.2f %0.2f %0.2f ]\n", vec.x, vec.y, vec.z);
}

bool scnvectors_equal(SCNVector3 vec1, SCNVector3 vec2) {
  return vec1.x == vec2.x && vec1.y == vec2.y && vec1.z == vec2.z;
}

bool matrices_equal(Matrix mat1, Matrix mat2, double eps = 1e-10) {
  bool result = true;
  for (int i = 0; i < mat1.size(); i++) {
    for (int j = 0; j < mat1[i].size(); j++) {
      double diff = mat1[i][j] - mat2[i][j];
      if (diff < 0) {
        diff = -diff;
      }
      result = result && diff <= eps;
    }
  }
  return result;
}

bool text_matrix_multiplication(Matrix mat1, Matrix mat2, Matrix expected) {
  auto result = PerformRotation(mat1, mat2);
  printf("================================\nPerforming matrix multiplication with matrices:\n\n");
  print_matrix(mat1);
  print_matrix(mat2);
  printf("\nThe resulting matrix is:\n\n");
  print_matrix(result);
  printf("\nExpectation:\n\n");
  print_matrix(expected);
  printf("\n================================\n\n");
  return matrices_equal(result, expected);
}

@implementation TreeLSystemRendererTests

- (void)testPrintPoints {
  TreeLSystemBridge* tree = [TreeLSystemBridge new];
  printf("Vertices:\n");
  for (int i = 0; i < tree.voxels.count; i++) {
    print_vector(tree.voxels[i].vector);
  }
}

- (void)testMatrixMultiplication {
  Matrix rot_mat, operand, expected;
  
  rot_mat = RZ(M_PI_2);
  operand = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
  };
  expected = {
    {0, -1, 0},
    {1, 0, 0},
    {0, 0, 1}
  };
  XCTAssert(text_matrix_multiplication(rot_mat, operand, expected), "Matrix operation is incorrect.");
  
  rot_mat = RX(-M_PI_2);
  expected = {
    {1, 0, 0},
    {0, 0, 1},
    {0, -1, 0}
  };
  XCTAssert(text_matrix_multiplication(rot_mat, operand, expected), "Matrix operation is incorrect.");
  
  rot_mat = RY(-M_PI_2);
  expected = {
    {0, 0, -1},
    {0, 1, 0},
    {1, 0, 0}
  };
  XCTAssert(text_matrix_multiplication(rot_mat, operand, expected), "Matrix operation is incorrect.");
  
  rot_mat = RZ(-M_PI_2);
  rot_mat = PerformRotation(RY(-M_PI), rot_mat);
  expected = {
    {0, -1, 0},
    {-1, 0, 0},
    {0, 0, -1}
  };
  XCTAssert(text_matrix_multiplication(rot_mat, operand, expected), "Matrix operation is incorrect.");
}

@end
