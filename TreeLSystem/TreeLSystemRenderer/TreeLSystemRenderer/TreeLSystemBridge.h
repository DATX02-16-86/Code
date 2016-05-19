#ifndef TreeLSystemBridge_h
#define TreeLSystemBridge_h

#import <Foundation/Foundation.h>
#import <SceneKit/SceneKit.h>

@interface SCNVector3Wrapper : NSObject
@property SCNVector3 vector;
- (nonnull instancetype) initWithVector:(SCNVector3)vector;
@end

@interface TreeLSystemBridge: NSObject
@property (nonnull) NSArray<SCNVector3Wrapper*>* voxels;
- (nonnull instancetype) init;
@end

#endif /* TreeLSystemBridge_h */

//* -------------------------------------------------------------------------------------------- *//

//#ifndef TreeLSystemBridge_h
//#define TreeLSystemBridge_h
//
//#import <Foundation/Foundation.h>
//#import <SceneKit/SceneKit.h>
//
//@interface SCNVector3Wrapper : NSObject
//@property SCNVector3 vector;
//- (nonnull instancetype) initWithVector:(SCNVector3)vector;
//@end
//
//@interface TreeLSystemBridge: NSObject
//@property (nonnull) NSArray<SCNVector3Wrapper*>* vertices;
//@property (nonnull) NSArray<NSNumber*>* indices;
//- (nonnull instancetype) init;
//@end
//
//#endif /* TreeLSystemBridge_h */

