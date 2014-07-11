//
//  BgElement.m
//  cosmos
//
//  Created by Vasiliy Makarov on 02.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "BgElement.h"


@implementation BgElement

@synthesize parallax = mParallax;
@synthesize origin = mOrigin;

-(id) initWithOrigin:(SPPoint *)origin andParallax:(float)parallax
{
    self = [super init];
    self.parallax = parallax;
    self.origin = origin;
    return self;
}

-(void) setPosition:(float)x y:(float)y
{
    float X = mOrigin.x + x * mParallax;
    float Y = mOrigin.y + y * mParallax;
    self.x = X;
    self.y = Y;
}

@end
