// Copyright (c) 2009 Imageshack Corp.
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#import "TweetterAppDelegate.h"
#import "TwitEditorController.h"
#import "HomeViewController.h"
#import "SelectImageSource.h"
#import "SettingsController.h"
#import "LocationManager.h"
#import "RepliesListController.h"
#import "DirectMessagesController.h"
#import "NavigationRotateController.h"
#import "TweetQueueController.h"
#import "AboutController.h"
#import "LoginController.h"
#import "MGTwitterEngine.h"
#import "FollowersController.h"

static int NetworkActivityIndicatorCounter = 0;

@implementation TweetterAppDelegate

@synthesize window;
@synthesize tabBarController;

- (UINavigationController *)createNavControllerWrappingViewControllerOfClass:(Class)cntrloller 
																			nibName:(NSString*)nibName 
																			tabIconName:(NSString*)iconName
																			tabTitle:(NSString*)tabTitle
{
	UIViewController* viewController = [[cntrloller alloc] initWithNibName:nibName bundle:nil];
	
	NavigationRotateController *theNavigationController;
	theNavigationController = [[NavigationRotateController alloc] initWithRootViewController:viewController];
	viewController.tabBarItem.image = [UIImage imageNamed:iconName];
	viewController.title = NSLocalizedString(tabTitle, @""); 
	[viewController release];
	
	return theNavigationController;
}


- (void)setupPortraitUserInterface 
{
	UINavigationController *localNavigationController;
	
	NSMutableArray *localViewControllersArray = [[NSMutableArray alloc] initWithCapacity:4];

	localNavigationController = [self createNavControllerWrappingViewControllerOfClass:[HomeViewController class] nibName:nil tabIconName:@"HomeTabIcon.tiff" tabTitle:@"Home"];
	[localViewControllersArray addObject:localNavigationController];
	[localNavigationController release];
	if([MGTwitterEngine username] == nil)
		[LoginController showModeless:localNavigationController animated:NO];
		
	localNavigationController = [self createNavControllerWrappingViewControllerOfClass:[RepliesListController class] nibName:@"UserMessageList" tabIconName:@"Replies.tiff" tabTitle:@"Replies"];
	[localViewControllersArray addObject:localNavigationController];
	[localNavigationController release];
	
	localNavigationController = [self createNavControllerWrappingViewControllerOfClass:[DirectMessagesController class] nibName:@"UserMessageList" tabIconName:@"Messages.tiff" tabTitle:@"Messages"];
	[localViewControllersArray addObject:localNavigationController];
	[localNavigationController release];
	
	localNavigationController = [self createNavControllerWrappingViewControllerOfClass:[TweetQueueController class] nibName:@"TweetQueue" tabIconName:@"Queue.tiff" tabTitle:[TweetQueueController queueTitle]];
	[localViewControllersArray addObject:localNavigationController];
	[localNavigationController release];

	localNavigationController = [self createNavControllerWrappingViewControllerOfClass:[FollowersController class] nibName:@"UserMessageList" tabIconName:@"followers.tiff" tabTitle:@"Followers"];
	[localViewControllersArray addObject:localNavigationController];
	[localNavigationController release];
	
	localNavigationController = [self createNavControllerWrappingViewControllerOfClass:[FollowingController class] nibName:@"UserMessageList" tabIconName:@"following.tiff" tabTitle:@"Following"];
	[localViewControllersArray addObject:localNavigationController];
	[localNavigationController release];
	
	localNavigationController = [self createNavControllerWrappingViewControllerOfClass:[SettingsController class] nibName:@"SettingsView" tabIconName:@"SettingsTabIcon.tiff" tabTitle:@"Settings"];
	[localViewControllersArray addObject:localNavigationController];
	[localNavigationController release];
	
	localNavigationController = [self createNavControllerWrappingViewControllerOfClass:[AboutController class] nibName:@"About" tabIconName:@"About.tiff" tabTitle:@"About"];
	[localViewControllersArray addObject:localNavigationController];
	[localNavigationController release];
	
	tabBarController.viewControllers = localViewControllersArray;
	
	[localViewControllersArray release];
	
}





- (void)applicationDidFinishLaunching:(UIApplication *)application 
{
	NSDictionary *appDefaults = [[NSDictionary alloc] initWithContentsOfFile:
						[[NSBundle mainBundle] pathForResource:@"defaults" ofType:@"plist"]];
	[[NSUserDefaults standardUserDefaults] registerDefaults:appDefaults];
	[appDefaults release];


	[self setupPortraitUserInterface];
    [window addSubview:tabBarController.view];

	[[LocationManager locationManager] startUpdates];
	
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
}


- (void)dealloc 
{
    [tabBarController release];
    [window release];
    [super dealloc];
}

+ (void) increaseNetworkActivityIndicator
{
	NetworkActivityIndicatorCounter++;
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NetworkActivityIndicatorCounter > 0;
}

+ (void) decreaseNetworkActivityIndicator
{
	NetworkActivityIndicatorCounter--;
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NetworkActivityIndicatorCounter > 0;
}

@end

