//
//  BgElement.h
//  cosmos
//
//  Created by Vasiliy Makarov on 02.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BgElement : SPSprite {
    float mParallax;
    SPPoint *mOrigin;
}

@property (nonatomic, assign) float parallax;
@property (nonatomic, copy) SPPoint *origin;

-(id) initWithOrigin:(SPPoint*)origin andParallax:(float)parallax;
-(void) setPosition:(float)x y:(float)y;

@end
