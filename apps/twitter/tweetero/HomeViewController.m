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

#import "HomeViewController.h"
#import "LoginController.h"
#import "MGTwitterEngine.h"
#import "ImageLoader.h"
#import "MessageViewController.h"
#import "MessageListController.h"
#import "TwitEditorController.h"
#import "TweetterAppDelegate.h"

#define NAME_TAG 1
#define TIME_TAG 2
#define IMAGE_TAG 3
#define TEXT_TAG 4
#define ROW_HEIGHT 70

@implementation HomeViewController

- (void)dealloc 
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}


- (void)viewDidLoad 
{
    [super viewDidLoad];

	self.navigationItem.title = @"Tweetero Home";
	
	UIBarButtonItem *newMsgButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCompose
		target:self action:@selector(newMessage)];
	self.navigationItem.rightBarButtonItem = newMsgButton;
	[newMsgButton release];
	
	UIBarButtonItem *reloadButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
		target:self action:@selector(reload)];
	self.navigationItem.leftBarButtonItem = reloadButton;
	[reloadButton release];

	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reload) name:@"TwittsUpdated" object:nil];
	if([MGTwitterEngine password] == nil)
		[LoginController showModal:self.navigationController];
}


- (void)viewDidAppear:(BOOL)animated 
{
    [super viewDidAppear:animated];
}


- (void)accountChanged:(NSNotification*)notification
{
	[self reloadAll];
	self.navigationItem.title = [MGTwitterEngine username];
}

- (void)loadMessagesStaringAtPage:(int)numPage count:(int)count
{
    NSLog(@"Niel: loadMessagesStaringAtPage");
    
	[super loadMessagesStaringAtPage:numPage count:count];
	if([MGTwitterEngine password] != nil)
	{
		[TweetterAppDelegate increaseNetworkActivityIndicator];
		[_twitter getFollowedTimelineFor:nil since:nil startingAtPage:numPage count:count];
		self.navigationItem.title = [MGTwitterEngine username];
	}
}

- (NSString*)noMessagesString
{
	return @"No Tweets";
}

- (NSString*)loadingMessagesString
{
	return @"Loading Tweets...";
}

- (void)newMessage
{
	TwitEditorController *msgView = [[TwitEditorController alloc] init];
	[self.navigationController pushViewController:msgView animated:YES];
	[msgView release];
}

- (void)reload
{
	if([MGTwitterEngine password] == nil)
		[LoginController showModal:self.navigationController];
	else
		[self reloadAll];
}

@end

