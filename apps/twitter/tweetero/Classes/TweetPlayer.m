//
//  TweetPlayer.m
//  Tweetero
//
//  Created by Alexander Dymerets on 8/11/09.
//  Copyright 2009 The Product Engine. All rights reserved.
//

#import "TweetPlayer.h"


@implementation TweetPlayer

-(void)didRotate:(NSNotification *)theNotification 
{
  UIInterfaceOrientation interfaceOrientation = [[UIDevice currentDevice] orientation];
  
  [super setOrientation:interfaceOrientation animated:YES];
}

- (id)initWithContentURL:(NSURL *)url
{
	self = [super initWithContentURL:url];
	if(self)
	{
		
		
		[[NSNotificationCenter defaultCenter] addObserver:self
				selector:@selector(didRotate:)
				name:UIDeviceOrientationDidChangeNotification
				object:nil];
	}
	
	return self;
}

- (void)play
{
	[super play];
	UIInterfaceOrientation interfaceOrientation = [[UIDevice currentDevice] orientation];
	[super setOrientation:interfaceOrientation animated:NO];
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self 
				name:UIDeviceOrientationDidChangeNotification 
				object:nil];
				
	[super dealloc];
}


@end
