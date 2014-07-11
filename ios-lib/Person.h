//
//  Person.h
//  cosmos
//
//  Created by Vasiliy Makarov on 11.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

#define ANIMATION_EVENT @"animation_event"

@interface AnimationEvent : SPEvent {
@private
}
@property (nonatomic, copy) NSString* animationName;

-(id) initWithAnimation:(NSString*)animation;
@end

/********* Key *************/

@interface Key : NSObject {
@private
    float _x, _y, _angle;
    int _frame;
}
@property (nonatomic, assign) float x;
@property (nonatomic, assign) float y;
@property (nonatomic, assign) float angle;
@property (nonatomic, assign) int frame;

-(id) initWithX:(float)x y:(float)y angle:(float)angle frame:(int)frame;
-(id) initFromAttributes:(NSDictionary*)attributeDict;
@end

/********* BoneAnim *************/

//@interface BoneAnim : NSObject {
//@private
//    NSString* _boneName;
//    NSMutableArray* _frames;
//}
//@property (nonatomic, copy) NSString* boneName;
//@property (nonatomic, readonly) NSMutableArray* frames;
//
//-(id)initWithBoneName:(NSString*)boneName;
//@end

/********* PersonAnim *************/

@interface PersonAnim : SPEventDispatcher {
@private
    NSString* _name;
    NSMutableDictionary* _boneAnims;
    float speed;
    float _length;
    SPSprite *_object;
    BOOL started;
}
@property (nonatomic, copy) NSString* name;
@property (nonatomic, assign) float length;
@property (nonatomic, assign) SPSprite* object;

-(id) initFromAttributes:(NSDictionary*)attributeDict;
-(void) appendKey:(NSString*)boneName key:(Key*)key;
-(void) startOn:(SPSprite*)object;
-(void) start;
@end

/********* Bone *************/

@interface Bone : SPSprite {
@private
    float baseX, baseY, baseAngle;
}
@property (nonatomic, assign) float dx;
@property (nonatomic, assign) float dy;
@property (nonatomic, assign) float dangle;

-(void) setBasePos:(float)x y:(float)y angle:(float)angle;
@end

/********* Person *************/

@interface Person : SPSprite <NSXMLParserDelegate> {
    NSString* _fileName;
    BOOL finishParse;
    SPSprite *currentBone;
    PersonAnim *currentAnim;
    NSString *curBoneName;
    NSMutableDictionary *animations;
    NSMutableSet *loopedAnims;
}

@property (nonatomic, copy) NSString* fileName;

-(id)initFromXml:(NSString*)fileName;
-(void)startAnimation:(NSString*)animName loop:(BOOL)loop;
-(void)stopAnimation:(NSString*)animName;

@end
