//
//  GamePlayViewController.m
//  cosmos
//
//  Created by Vasiliy Makarov on 02.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GamePlayViewController.h"


@implementation GamePlayViewController

@synthesize stage;

#pragma mark -
#pragma mark View Life Cycle

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onTransitionToInactive:) name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(stopGameNotification) name:@"StopGame" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onTransitionToActive:) name:UIApplicationDidBecomeActiveNotification object:nil];
    
}

- (void)viewDidDisappear:(BOOL)animated {
	[super viewDidDisappear:animated];
	self.stage = nil;
}

- (void)viewDidUnload {
    [super viewDidUnload];
}

#pragma mark -
#pragma mark Rotation

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return (interfaceOrientation == UIInterfaceOrientationLandscapeRight) || (interfaceOrientation == UIInterfaceOrientationLandscapeLeft);
}

#pragma mark -
#pragma mark Accessor Methods

- (void)setStage:(SPStage *)aStage {
	if (aStage) {
		SPView *spView = (SPView *)[self view];
        
		spView.stage = aStage;
		spView.frameRate = 60.0;
		[spView start];
        
		[SPAudioEngine start:SPAudioSessionCategory_AmbientSound];
	}
}

#pragma -
#pragma mark Notifications

- (void)stopGameNotification {
    SPView *spView = (SPView *)[self view];
	[spView stop];
    
	[SPAudioEngine stop];
    
    [self.parentViewController dismissModalViewControllerAnimated:YES];
}

- (void)onTransitionToActive:(NSNotification *)sender {
    SPView *spView = (SPView *)[self view];
    [spView start];
    
    [SPAudioEngine start];
}

- (void)onTransitionToInactive:(NSNotification *)sender {
    SPView *spView = (SPView *)[self view];
    [spView stop];
    
    [SPAudioEngine stop];
}


#pragma mark -
#pragma mark Memory Managment

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

- (void)dealloc {
    [self.stage release];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self name:@"StopGame" object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidBecomeActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationWillResignActiveNotification object:nil];
    
    [super dealloc];
}

@end
