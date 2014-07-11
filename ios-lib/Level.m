//
//  Level.m
//  cosmos
//
//  Created by Vasiliy Makarov on 02.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Level.h"
#import "SXParticleSystem.h"

const float CenterPosition = 240.f;

@implementation Level

@synthesize pPoint = playerPoint;
@synthesize name = lvlName;

-(id)initWithBackground:(SPTexture *)background width:(float)width height:(float)height {
    self = [super init];
    dx = dy = 0.f;
    self.pPoint = [SPPoint pointWithX:0 y:0];
    backgroundImg = [SPImage imageWithTexture:background];
    [backgroundImg setWidth:width];
    [backgroundImg setHeight:height];
    [self addChild:backgroundImg];
    
    [self addEventListener:@selector(update:) atObject:self forType:SP_EVENT_TYPE_ENTER_FRAME];
    
    return self;
}

-(void) loadLevelFromFile:(NSString *)fileName
{
    if (!fileName) return;
    float scaleFactor = [SPStage contentScaleFactor];
    NSString *mPath = [[SPUtils absolutePathToFile:fileName withScaleFactor:scaleFactor] retain];
    if (!mPath) [NSException raise:SP_EXC_FILE_NOT_FOUND format:@"file not found: %@", fileName];
    
    finishParse = NO;
    SP_CREATE_POOL(pool);
    NSData *xmlData = [[NSData alloc] initWithContentsOfFile:mPath];
    NSXMLParser *xmlParser = [[NSXMLParser alloc] initWithData:xmlData];
    [xmlData release];
    xmlParser.delegate = self;
    BOOL success = [xmlParser parse];
    SP_RELEASE_POOL(pool);
    
    if (!success)
        [NSException raise:SP_EXC_FILE_INVALID format:@"could not parse level xml %@. Error code: %d, domain: %@", 
         fileName, xmlParser.parserError.code, xmlParser.parserError.domain];
    
    [xmlParser release];

    playerPoint.x = player.x - CenterPosition;
}

-(void) parser:(NSXMLParser*)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
    if([elementName isEqualToString:@"level"]) {
        finishParse = YES;
    } else if([elementName isEqualToString:@"element"]) {
        [currentElement release];
        currentElement = nil;
    }
}

- (void)readAttributes:(SPDisplayObject*)obj attributes:(NSDictionary*)attributeDict
{
    if([attributeDict objectForKey:@"x"] != nil) obj.x = [[attributeDict valueForKey:@"x"] floatValue];
    if([attributeDict objectForKey:@"y"] != nil) obj.y = [[attributeDict valueForKey:@"y"] floatValue];
    if([attributeDict objectForKey:@"scale"] != nil) {
        float scale = [[attributeDict valueForKey:@"scale"] floatValue];
        obj.scaleX = scale;
        obj.scaleY = scale;
    }
}

- (void)parser:(NSXMLParser*)parser didStartElement:(NSString*)elementName 
    namespaceURI:(NSString*)namespaceURI 
    qualifiedName:(NSString*)qName 
    attributes:(NSDictionary*)attributeDict 
{
    assert(!finishParse);
    if([elementName isEqualToString:@"level"]) {
        lvlName = [[attributeDict valueForKey:@"name"] copy];
    } else if([elementName isEqualToString:@"element"]) {
        assert(currentElement == nil);
        float _x=0.f, _y=0.f, _p=1.f;
        if([attributeDict objectForKey:@"x"] != nil) _x = [[attributeDict valueForKey:@"x"] floatValue];
        if([attributeDict objectForKey:@"y"] != nil) _y = [[attributeDict valueForKey:@"y"] floatValue];
        if([attributeDict objectForKey:@"parallax"] != nil) _p = [[attributeDict valueForKey:@"parallax"] floatValue];
        currentElement = [[BgElement alloc] initWithOrigin:[SPPoint pointWithX:_x y:_y] andParallax:_p];
        currentElement.z = (_p - 1.f) * 50.f;
        currentElement.depthEnabled = YES;
        [self addChild:currentElement];
    } else if([elementName isEqualToString:@"image"]) {
        assert(currentElement != nil);
        SPImage *image = [SPImage imageWithContentsOfFile:[attributeDict valueForKey:@"texture"]];
        [self readAttributes:image attributes:attributeDict];
        [currentElement addChild:image];
    } else if([elementName isEqualToString:@"particle"]) {
        assert(currentElement != nil);
        SXParticleSystem *ps = [[SXParticleSystem alloc] initWithContentsOfFile:[attributeDict valueForKey:@"system"]];
        [self readAttributes:ps attributes:attributeDict];
        [currentElement addChild:ps];
        [self.stage.juggler addObject:ps];
        [ps start];
        [ps release];
    } else if([elementName isEqualToString:@"person"]) {
        assert(currentElement != nil);
        Person *pc = [[[Person alloc] initFromXml:[attributeDict valueForKey:@"file"]] autorelease];
        [self readAttributes:pc attributes:attributeDict];
        if([[attributeDict valueForKey:@"player"] isEqualToString:@"yes"]) {
            player = [pc retain];
        }
        [currentElement addChild:pc];
    }
}

-(void) update:(SPEnterFrameEvent*)event
{
    playerPoint.x += event.passedTime * dx;
    playerPoint.y += event.passedTime * dy;
    player.x = playerPoint.x + CenterPosition;
    for(int i=0; i<self.numChildren; i++) {
        SPDisplayObject *ch = [self childAtIndex:i];
        if([ch isKindOfClass:[BgElement class]]) {
            BgElement *bge = (BgElement*) ch;
            [bge setPosition:-playerPoint.x y:-playerPoint.y];
        }
    }
}

-(void) setMovement:(float)x y:(float)y
{
    dx = x * 100.f;
    dy = y * 100.f;
    if(dx != 0.f) {
        [player startAnimation:@"moonwalk3" loop:YES];
        if(dx < 0.f) player.scaleX = - fabs(player.scaleX);
        else player.scaleX = fabs(player.scaleX);
    } else {
        [player stopAnimation:@"moonwalk3"];
    }
}

-(void)useItem
{
    [player startAnimation:@"press button" loop:NO];
}

-(void) dealloc
{
    [player release];
    [backgroundImg release];
    [super dealloc];
}

@end
