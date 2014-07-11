#import "SXFPSMeter.h"

@implementation SXFPSMeter

- (id)initWithText:(NSString *)text{
	self = [super initWithText:text];
	self.hAlign = SPHAlignLeft;
	self.vAlign = SPVAlignTop;
	self.fontSize = 16;
	self.color = 0xFF0000;
	[self addEventListener:@selector(update:) atObject:self forType:SP_EVENT_TYPE_ENTER_FRAME];
	return self;
}

- (void)update:(SPEnterFrameEvent*)event{
	static int frameCount = 0;
	static double totalTime = 0;
	totalTime += event.passedTime;
	if (++frameCount % 60 == 0){
		self.text = [NSString stringWithFormat:@"FPS: %d", (int)(frameCount/totalTime)];
		frameCount = totalTime = 0;
	}
}

- (void)dealloc{
	[self removeEventListener:@selector(update:) atObject:self forType:SP_EVENT_TYPE_ENTER_FRAME];
	[super dealloc];
}

@end
