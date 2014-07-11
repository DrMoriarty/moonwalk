//
//  Game.m
//  cosmos
//
//  Created by Vasiliy Makarov on 01.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Game.h"
#import "SHThumbstick.h"
#import "SXParticleSystem.h"

@implementation Game

- (id)initWithWidth:(float)width height:(float)height
{
    if (self = [super initWithWidth:width height:height])
    {
        lvl = [[Level alloc] initWithBackground:[SPTexture textureWithContentsOfFile:@"Bg_sky_8.png"] width:width height:height];
        [self addChild:lvl];
        [lvl loadLevelFromFile:@"demoLevel.xml"];
       
        /*
        //initialize the splash screen with an idle time of 5.0f and default transitions
        SHSplashScreen *splashScreen = [SHSplashScreen splashScreenWithTime:5.0f];
        //initialize and add an image to the splash screen
        SPImage *logo = [SPImage imageWithContentsOfFile:@"Stone.png"];
        [splashScreen addChild:logo];
        //add the splash screen to the stage, it will automatically start and automatically remove itself from the stage when finished.
        [self addChild:splashScreen];
        
        
        SPQuad *quad = [SPQuad quadWithWidth:300 height:150];
        quad.color = 0xff0000;
        quad.x = 50;
        quad.y = 50;
        [self addChild:quad];
        
        NSString *bmpFontName = [SPTextField registerBitmapFontFromFile:@"arial_narrow_bold_italic_17.fnt"];
        SPTextField *bmpFontTF = [SPTextField textFieldWithWidth:300 height:150 
                                                            text:@"It is very easy to use Bitmap fonts, as well! ToTaTeTi IiIoIjds lksdjflksdjf lsdkfjlskd ksjdfl sdlkfs dflksd flkjsdflkf skdjf"];
        bmpFontTF.fontSize = SP_NATIVE_FONT_SIZE; // use the native bitmap font size, no scaling
        bmpFontTF.fontName = bmpFontName;
        bmpFontTF.color = SP_WHITE; // use white if you want to use the texture as it is
        bmpFontTF.hAlign = SPHAlignCenter;
        bmpFontTF.vAlign = SPVAlignCenter;
        bmpFontTF.kerning = YES;
        bmpFontTF.x = 50;
        bmpFontTF.y = 50;
        [self addChild:bmpFontTF];
         */


        /*  dark effect
        [self addChild:[SPImage imageWithContentsOfFile:@"screenline_effect.png"]];
        SPImage *img = [SPImage imageWithContentsOfFile:@"screendark_effect.png"];
        SPTween *tween = [SPTween tweenWithTarget:img time:2.5f];
        [tween animateProperty:@"alpha" targetValue:.5f];
        tween.loop = SPLoopTypeReverse;
        [self.stage.juggler addObject:tween];
        [self addChild:img];
         */
       
        //initialize a thumbstick
        SHThumbstick *absoluteThumbstick = [SHThumbstick thumbstick];
        //absoluteThumbstick.innerImage = [SPImage imageWithContentsOfFile:@"innerThumbstick.png"];
        //absoluteThumbstick.outerImage = [SPImage imageWithContentsOfFile:@"outerThumbstick.png"];
        absoluteThumbstick.type = SHThumbstickStatic;
        //absoluteThumbstick.bounds = [SPRectangle rectangleWithX:0 y:240 width:80 height:80];
        absoluteThumbstick.centerX = 60;
        absoluteThumbstick.centerY = height-40;
        absoluteThumbstick.innerRadius = 20;
        absoluteThumbstick.outerRadius = 40;
        absoluteThumbstick.debugDraw = YES;
        [self addEventListener:@selector(onThumbstickChanged:) atObject:self forType:SH_THUMBSTICK_EVENT_CHANGED];
        [self addChild:absoluteThumbstick];
        
        SPButton *b = [SPButton buttonWithUpState:[SPTexture textureWithContentsOfFile:@"gun.png"]];
        b.x = 410;
        b.y = 250;
        [b addEventListener:@selector(onUseButton:) atObject:self forType:SP_EVENT_TYPE_TRIGGERED];
        [self addChild:b];
        
        SXFPSMeter *meter = [[SXFPSMeter alloc] initWithText:@""];
        [self addChild:meter];

    }
    return self;
}

#pragma mark -

- (void)onThumbstickChanged:(SHThumbstickEvent *)event 
{
    //NSLog(@"onThumbstickChanged distance:%f direction:%f", event.distance, event.direction);        
    float angle = event.direction * M_PI / 180.f;
    float x = sin(angle) * event.distance;
    float y = -cos(angle) * event.distance;
    [lvl setMovement:x y:y];
}

#pragma mark -

- (void)onUseButton:(SPEvent*)event
{
    [lvl useItem];
}

@end
