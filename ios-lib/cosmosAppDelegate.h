//
//  cosmosAppDelegate.h
//  cosmos
//
//  Created by Vasiliy Makarov on 01.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Sparrow.h"

@interface cosmosAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    SPView *sparrowView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet SPView *sparrowView;

@end
