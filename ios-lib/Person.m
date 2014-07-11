//
//  Person.m
//  cosmos
//
//  Created by Vasiliy Makarov on 11.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Person.h"

@implementation AnimationEvent

@synthesize animationName;

-(id)initWithAnimation:(NSString *)animation
{
    self = [super initWithType:ANIMATION_EVENT bubbles:YES];
    self.animationName = animation;
    return self;
}

@end

/********* Key *************/

@implementation Key

@synthesize x = _x;
@synthesize y = _y;
@synthesize angle = _angle;
@synthesize frame = _frame;

-(id)initWithX:(float)x y:(float)y angle:(float)angle frame:(int)frame
{
    self = [super init];
    _x = x;
    _y = y;
    _angle = angle;
    _frame = frame;
    return self;
}

-(id) initFromAttributes:(NSDictionary*)attributeDict
{
    self = [super init];
    _x = [[attributeDict valueForKey:@"x"] floatValue];
    _y = [[attributeDict valueForKey:@"y"] floatValue];
    _angle = SP_D2R([[attributeDict valueForKey:@"angle"] floatValue]);
    _frame = [[attributeDict valueForKey:@"frame"] intValue];
    return self;
}

@end

/********* BoneAnim *************/

//@implementation BoneAnim
//
//@synthesize boneName = _boneName;
//@synthesize frames = _frames;
//
//-(id)initWithBoneName:(NSString*)boneName
//{
//    self = [super init];
//    self.boneName = boneName;
//    _frames = [[NSMutableArray alloc] init];
//    return self;
//}
//
//-(void)dealloc
//{
//    [_frames release];
//}
//
//@end

/********* PersonAnim *************/

@implementation PersonAnim

@synthesize name = _name;
@synthesize object = _object;

-(id)initFromAttributes:(NSDictionary *)attributeDict
{
    self = [super init];
    speed = 1.f / 25.f;
    self.name = [attributeDict valueForKey:@"name"];
    _length = speed * [[attributeDict valueForKey:@"length"] intValue];
    _boneAnims = [[NSMutableDictionary alloc] init];
    started = NO;
    return self;
}

-(void)appendKey:(NSString *)boneName key:(Key *)key
{
    NSMutableArray *bkeys = [_boneAnims valueForKey:boneName];
    if(bkeys == nil) {
        bkeys = [NSMutableArray array];
        [_boneAnims setValue:bkeys forKey:boneName];
    }
    [bkeys addObject:key];
}

-(void)endAnimation
{
    started = NO;
    [_object dispatchEvent:[[AnimationEvent alloc] initWithAnimation:self.name]];
}

-(void)start
{
    if(!started) [self startOn:_object];
}

-(void)startOn:(SPSprite *)object
{
    started = YES;
    for (NSString* bone in [_boneAnims allKeys]) {
        SPSprite *curBone = (SPSprite*)[object childByName:bone];
        NSMutableArray *bkeys = [_boneAnims valueForKey:bone];
        float startTime = 0.f;
        Key *prevKey = [[[Key alloc] init] autorelease];
        for (Key* key in bkeys) {
            if(key.frame == 0) prevKey = key;
            float endTime = speed * key.frame;
            SPTween *tween = [SPTween tweenWithTarget:curBone time:endTime - startTime];
            tween.delay = startTime;
            [tween animateProperty:@"dx" targetValue:key.x];
            [tween animateProperty:@"dy" targetValue:key.y];
            [tween animateProperty:@"dangle" targetValue:key.angle];
            [object.stage.juggler addObject:tween];
            startTime = endTime;
        }
    }
    [[object.stage.juggler delayInvocationAtTarget:self byTime:_length] endAnimation];
}

-(void)setLength:(float)frames
{
    _length = speed * frames;
}

-(float)length
{
    return _length / speed;
}

-(void)dealloc
{
    [_name release];
    [_boneAnims release];
    [super dealloc];
}

@end

/********* Bone *************/

@implementation Bone

-(void)setBasePos:(float)x y:(float)y angle:(float)angle
{
    baseX = x;
    baseY = y;
    baseAngle = angle;
    mX = baseX;
    mY = baseY;
    mRotationZ = baseAngle;
}

-(void)setDx:(float)x
{
    mX = baseX + x;
}

-(float)dx
{
    return mX - baseX;
}

-(void)setDy:(float)y
{
    mY = baseY + y;
}

-(float)dy
{
    return mY - baseY;
}

-(void)setDangle:(float)rotation
{
    mRotationZ = baseAngle + rotation;
}

-(float)dangle
{
    return mRotationZ - baseAngle;
}

@end

/********* Person *************/

@implementation Person

@synthesize fileName = _fileName;

-(id)initFromXml:(NSString *)fileName
{
    self = [super init];
    self.fileName = fileName;
    animations = [[NSMutableDictionary dictionary] retain];
    loopedAnims = [[NSMutableSet alloc] init];
    [self addEventListener:@selector(onAddedToStage:) atObject:self forType:SP_EVENT_TYPE_ADDED_TO_STAGE];
    [self addEventListener:@selector(onEndAnimation:) atObject:self forType:ANIMATION_EVENT];
    return self;
}

- (void)onAddedToStage:(SPEvent *)event
{
    [self removeEventListener:@selector(onAddedToStage:) atObject:self forType:SP_EVENT_TYPE_ADDED_TO_STAGE];
    if (!self.fileName) return;
    float scaleFactor = [SPStage contentScaleFactor];
    _fileName = [[SPUtils absolutePathToFile:self.fileName withScaleFactor:scaleFactor] retain];
    if (!_fileName) [NSException raise:SP_EXC_FILE_NOT_FOUND format:@"file not found: %@", self.fileName];

    finishParse = NO;
    currentBone = self;
    SP_CREATE_POOL(pool);
    NSData *xmlData = [[NSData alloc] initWithContentsOfFile:_fileName];
    NSXMLParser *xmlParser = [[NSXMLParser alloc] initWithData:xmlData];
    [xmlData release];
    xmlParser.delegate = self;
    BOOL success = [xmlParser parse];
    SP_RELEASE_POOL(pool);
    assert(currentBone == self);
    
    if (!success)
        [NSException raise:SP_EXC_FILE_INVALID format:@"could not parse level xml %@. Error code: %d, domain: %@", 
         self.fileName, xmlParser.parserError.code, xmlParser.parserError.domain];
    
    [xmlParser release];      
}

-(void) parser:(NSXMLParser*)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
    if([elementName isEqualToString:@"Person"]) {
        finishParse = YES;
    } else if([elementName isEqualToString:@"bone"]) {
        [currentBone release];
        currentBone = (SPSprite*)currentBone.parent;
    } else if([elementName isEqualToString:@"pose"]) {
        [currentAnim release];
        currentAnim = nil;
    } else if([elementName isEqualToString:@"keys"]) {
        [curBoneName release];
        curBoneName = nil;
    }
}
    
- (void)parser:(NSXMLParser*)parser didStartElement:(NSString*)elementName 
  namespaceURI:(NSString*)namespaceURI 
 qualifiedName:(NSString*)qName 
    attributes:(NSDictionary*)attributeDict 
{
    if([elementName isEqualToString:@"Person"]) {
        self.name = [attributeDict valueForKey:@"name"];
    } else if([elementName isEqualToString:@"bone"]) {
        Bone *newbone = [[Bone alloc] init];
        float posx = [[attributeDict valueForKey:@"x"] floatValue];
        float posy = [[attributeDict valueForKey:@"y"] floatValue];
        float pivotX = [[attributeDict valueForKey:@"pivotX"] floatValue];
        float pivotY = [[attributeDict valueForKey:@"pivotY"] floatValue];
        if(currentBone != nil) {
            posx += currentBone.pivotX;
            posy += currentBone.pivotY;
        }
        newbone.pivotX = -pivotX;
        newbone.pivotY = -pivotY;
        [newbone setBasePos:posx y:posy angle:SP_D2R([[attributeDict valueForKey:@"angle"] floatValue])];
        newbone.z = [[attributeDict valueForKey:@"z"] floatValue] * 0.1f;
        //BOOL underlayer = currentBone != nil && newbone.z < currentBone.z;
        NSString *file = [attributeDict valueForKey:@"file"];
        newbone.name = [attributeDict valueForKey:@"ID"];
        newbone.depthEnabled = YES;
        [newbone addChild:[SPImage imageWithContentsOfFile:file]];
        /*if(underlayer) {
            int sprInd = 0;
            while ([[currentBone childAtIndex:sprInd] isKindOfClass:[SPSprite class]]) {
                sprInd ++;
            }
            [currentBone addChild:newbone atIndex:sprInd];
        }
        else*/ 
            [currentBone addChild:newbone];
        currentBone = newbone;
    } else if([elementName isEqualToString:@"pose"]) {
        currentAnim = [[PersonAnim alloc] initFromAttributes:attributeDict];
        currentAnim.object = self;
        [animations setValue:currentAnim forKey:currentAnim.name];
    } else if([elementName isEqualToString:@"keys"]) {
        assert(currentAnim != nil);
        curBoneName = [[attributeDict valueForKey:@"bone"] retain];
    } else if([elementName isEqualToString:@"key"]) {
        assert(currentAnim != nil);
        [currentAnim appendKey:curBoneName key:[[[Key alloc] initFromAttributes:attributeDict] autorelease]];
    }
}

-(void)onEndAnimation:(AnimationEvent*)event
{
    PersonAnim *a = [animations valueForKey:event.animationName];
    if(a == nil) return;
    if([loopedAnims containsObject:event.animationName]) {
        [a startOn:self];
    }
}

-(void)startAnimation:(NSString *)animName loop:(BOOL)loop
{
    PersonAnim *a = [animations valueForKey:animName];
    if(a != nil) {
        [a start];
        if(loop) {
            [loopedAnims addObject:animName];
        }
    }
}

-(void)stopAnimation:(NSString *)animName
{
    if([loopedAnims containsObject:animName]) {
        [loopedAnims removeObject:animName];
    }
}

-(void)dealloc
{
    [self removeEventListener:@selector(onEndAnimation:) atObject:self forType:ANIMATION_EVENT];
    [animations release];
    [loopedAnims release];
    [super dealloc];
}

@end
