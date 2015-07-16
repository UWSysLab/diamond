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

#import "DirectMessagesController.h"
#import "MGTwitterEngine.h"
#import "TweetterAppDelegate.h"

@implementation DirectMessagesController

- (void)viewDidLoad 
{
    [super viewDidLoad];
	self.navigationItem.title = @"Direct Messages";
	
	UIBarButtonItem *reloadButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
		target:self action:@selector(reload)];
	self.navigationItem.leftBarButtonItem = reloadButton;
	[reloadButton release];
	
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reload) name:@"DirectMessageSent" object:nil];
}

- (void)accountChanged:(NSNotification*)notification
{
	[self reloadAll];
}

- (NSString*)noMessagesString
{
	return @"No Direct Messages";
}

- (NSString*)loadingMessagesString
{
	return @"Loading Direct Messages...";
}

- (void)loadMessagesStaringAtPage:(int)numPage count:(int)count
{
	[super loadMessagesStaringAtPage:numPage count:count];
	if([MGTwitterEngine password] != nil)
	{
		[self retainActivityIndicator];
		[TweetterAppDelegate increaseNetworkActivityIndicator];
		[_twitter getDirectMessagesSince:nil startingAtPage:numPage];
		[TweetterAppDelegate increaseNetworkActivityIndicator];
		[_twitter getSentDirectMessagesSince:nil startingAtPage:numPage];
		self.navigationItem.title = [MGTwitterEngine username];
	}
}

- (void)reload
{
	[self reloadAll];
}

@end
