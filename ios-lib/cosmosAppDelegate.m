//
//  cosmosAppDelegate.m
//  cosmos
//
//  Created by Vasiliy Makarov on 01.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "cosmosAppDelegate.h"
#import "Game.h"

@implementation cosmosAppDelegate


@synthesize window=_window;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Override point for customization after application launch.
    SP_CREATE_POOL(pool);    
    
    [SPStage setSupportHighResolutions:YES];
    // force high resolution if we're on the iPad:
    float w, h;
    if ([[UIDevice currentDevice].model rangeOfString:@"iPad"].location == 0) {
        [SPStage setContentScaleFactor:2.0f];
        w = 480;
        h = 360;
    } else {
        w = 480;
        h = 320;
    }
    [SPAudioEngine start];
    
    Game *game = [[Game alloc] initWithWidth:w height:h];
    [self.window.rootViewController setStage:game];
    //sparrowView.stage = game;
    [game release];
    [self.window makeKeyAndVisible];
    [sparrowView start];
    
    SP_RELEASE_POOL(pool);
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
    [sparrowView stop];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
    [sparrowView start];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    /*
     Called when the application is about to terminate.
     Save data if appropriate.
     See also applicationDidEnterBackground:.
     */
}

- (void)dealloc
{
    [sparrowView release];
    [_window release];
    [super dealloc];
}

@end
