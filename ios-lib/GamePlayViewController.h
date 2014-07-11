//
//  GamePlayViewController.h
//  cosmos
//
//  Created by Vasiliy Makarov on 02.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Sparrow.h"

@interface GamePlayViewController : UIViewController {
}

@property (nonatomic, retain) SPStage *stage;

- (void)stopGameNotification;

- (void)onTransitionToActive:(NSNotification *)sender;
- (void)onTransitionToInactive:(NSNotification *)sender;

@end
