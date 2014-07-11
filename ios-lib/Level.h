//
//  Level.h
//  cosmos
//
//  Created by Vasiliy Makarov on 02.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BgElement.h"
#import "Person.h"

@interface Level : SPSprite <NSXMLParserDelegate> {
    SPImage *backgroundImg;
    SPPoint* playerPoint;
    NSString * lvlName;
    float dx, dy;
    BOOL finishParse;
    BgElement *currentElement;
    Person *player;
}

@property (nonatomic, copy) SPPoint *pPoint;
@property (nonatomic, copy) NSString *name;


-(id) initWithBackground:(SPTexture*)background width:(float)width height:(float)height;
-(void) loadLevelFromFile:(NSString*)fileName;
-(void) setMovement:(float)x y:(float)y;
-(void) useItem;
@end
