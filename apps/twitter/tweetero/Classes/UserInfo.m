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

#import "UserInfo.h"
#import "MGTwitterEngine.h"
#import "ImageLoader.h"
#import "WebViewController.h"
#import "NewMessageController.h"
#import "UserMessageListController.h"
#import "TweetterAppDelegate.h"
#import "TwitEditorController.h"
#include "util.h"

@implementation UserInfo

@synthesize isUserReceivingUpdatesForConnectionID;

- (id)initWithUserName:(NSString*)uname
{
	self = [super initWithNibName:@"UserInfo" bundle:nil];
	
	if(self)
	{
		_gotInfo = NO;
		_twitter = [[MGTwitterEngine alloc] initWithDelegate:self];
		_username = [uname retain];
	}
	
	return self;
}


- (void)viewDidLoad 
{
    [super viewDidLoad];

	sendDirectMessage.hidden = YES;
	[TweetterAppDelegate increaseNetworkActivityIndicator];
	self.isUserReceivingUpdatesForConnectionID = [_twitter isUser:_username receivingUpdatesFor:[MGTwitterEngine username]];
	[TweetterAppDelegate increaseNetworkActivityIndicator];
	[_twitter getUserInformationFor:_username];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (IBAction)sendMessage 
{
	NewMessageController *msgView = [[NewMessageController alloc] initWithNibName:@"NewMessage" bundle:nil];
	[self.navigationController pushViewController:msgView animated:YES];
	[msgView setUser:_username];
	[msgView release];
}

- (IBAction)sendReply 
{
	TwitEditorController *msgView = [[TwitEditorController alloc] init];
	[self.navigationController pushViewController:msgView animated:YES];
	[msgView setReplyToMessage:	[NSDictionary dictionaryWithObject:	[NSDictionary dictionaryWithObject:_username forKey:@"screen_name"]	
															forKey:@"user"]];
	[msgView release];
}

- (IBAction)showTwitts 
{
    UserMessageListController *msgView = [[UserMessageListController alloc] initWithUserName:_username];
	[self.navigationController pushViewController:msgView animated:YES];
	[msgView release];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}


- (void)dealloc 
{
	infoView.delegate = nil;
	if(infoView.loading)
	{
		[infoView stopLoading];
		[TweetterAppDelegate decreaseNetworkActivityIndicator];
	}
	int connectionsCount = [_twitter numberOfConnections];
	[_twitter closeAllConnections];
	[_twitter removeDelegate];
	[_twitter release];
	while(connectionsCount-- > 0)
		[TweetterAppDelegate decreaseNetworkActivityIndicator];
	
	[_username release];
	self.isUserReceivingUpdatesForConnectionID = nil;
    [super dealloc];
}

- (void)requestFailed:(NSString *)connectionIdentifier withError:(NSError *)error
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Network Failure" message:[error localizedDescription]
												   delegate:self cancelButtonTitle:@"OK" otherButtonTitles: nil];
	[alert show];	
	[alert release];
}

- (void)miscInfoReceived:(NSArray *)miscInfo forRequest:(NSString *)connectionIdentifier
{
	if(![self.isUserReceivingUpdatesForConnectionID isEqualToString:connectionIdentifier])
		return;

	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	NSDictionary *followData = [miscInfo objectAtIndex:0];
	
	BOOL friends = NO;
	id friendsObj = [followData objectForKey:@"friends"];
	if(friendsObj)
		friends = [friendsObj boolValue];
	sendDirectMessage.hidden = !friends;
}

- (void)userInfoReceived:(NSArray *)userInfo forRequest:(NSString *)connectionIdentifier;
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	NSDictionary *userData = [userInfo objectAtIndex:0];
	
	UIImage *avatar = [[ImageLoader sharedLoader] imageWithURL:[userData objectForKey:@"profile_image_url"]];
	CGSize avatarViewSize = avatarView.frame.size;
	if(avatar.size.width > avatarViewSize.width || avatar.size.height > avatarViewSize.height)
		avatar = imageScaledToSize(avatar, avatarViewSize.width);
	avatarView.image = avatar;
	nameField.text = [userData objectForKey:@"screen_name"];
	realNameField.text = [userData objectForKey:@"name"];
	self.navigationItem.title = [userData objectForKey:@"screen_name"];
	
	
	NSMutableString* info = [NSMutableString stringWithCapacity:256];
	[info appendFormat:@"<html><body style=\"width:%d\">", (int)infoView.frame.size.width - 10];
	
	NSString *item;
	BOOL infoEmpty = YES;
	
	if(item = [userData objectForKey:@"description"])
	{
		infoEmpty = NO;
		[info appendString:item];
		[info appendString:@"<br>"];
	}
	
	if(item = [userData objectForKey:@"url"])
	{
		infoEmpty = NO;
		[info appendFormat:@"<a href=%@>%@</a>", item, item];
		[info appendString:@"<br>"];
	}
	
	if((item = [userData objectForKey:@"location"]) && [item length] != 0)
	{
		infoEmpty = NO;
		NSScanner *scanner = [NSScanner scannerWithString:item];
		[info appendString:@"Location: "];
		
		NSString *textPart;
		[scanner scanUpToCharactersFromSet:[NSCharacterSet decimalDigitCharacterSet] intoString:&textPart];
		if([textPart length] > 0 && [textPart characterAtIndex:[textPart length] - 1] == (unichar)'-')
		{
			textPart = [textPart substringToIndex:[textPart length] - 1];
			[scanner setScanLocation:[scanner scanLocation] - 1];
		}
		
		[info appendString:textPart];
		double x, y;
		if(![scanner isAtEnd])
		{
			[scanner scanDouble:&x];
			[scanner scanUpToCharactersFromSet:[NSCharacterSet decimalDigitCharacterSet] intoString:&textPart];
			if([scanner isAtEnd])
				[info appendFormat:@"%f%@", x, textPart];
			else
			{
				if([textPart length] > 0 && [textPart characterAtIndex:[textPart length] - 1] == (unichar)'-')
				{
					textPart = [textPart substringToIndex:[textPart length] - 1];
					[scanner setScanLocation:[scanner scanLocation] - 1];
				}
				[scanner scanDouble:&y];
				[info appendFormat:@"<a href = http://maps.google.com/maps?q=%f,%f>%@</a>", x, y, @"Current Location"];
				[info appendString:[item substringFromIndex:[scanner scanLocation]]];
				
			}
		}
		
		[info appendString:@"<br>"];
	}
	
	if(infoEmpty)
	{
		CGRect infoframe = infoView.frame, ctrlframe = controlView.frame;
		ctrlframe.origin = infoframe.origin;
		controlView.frame = ctrlframe;
		infoView.hidden = YES;
	}
	else
	{
		[info appendString:@"</body></html>"];
		infoView.scalesPageToFit = NO;
		[infoView loadHTMLString:info baseURL:nil];
	}
	
	NSString* notifyOn = [userData objectForKey:@"notifications"];
	if(notifyOn)
	{
		notifySwitch.on = [notifyOn isEqualToString:@"true"];
	}
		
	_gotInfo = YES;
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	if(!_gotInfo)
		[self.navigationController popViewControllerAnimated:YES];
	else
	{
		_gotInfo = NO;
		[TweetterAppDelegate increaseNetworkActivityIndicator];
		[_twitter getUserInformationFor:_username];
	}
}

- (void)webViewDidStartLoad:(UIWebView *)webView
{
	[TweetterAppDelegate increaseNetworkActivityIndicator];
}


- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
	if([[[request URL] absoluteString] isEqualToString:@"about:blank"])
		return YES;

	UIViewController *webViewCtrl = [[WebViewController alloc] initWithRequest:request];
	[self.navigationController pushViewController:webViewCtrl animated:YES];
	[webViewCtrl release];
	
	return NO;
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
}

- (void)webViewDidFinishLoad:(UIWebView *)webView
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
}


- (IBAction)notifySwitchChanged
{
	[TweetterAppDelegate increaseNetworkActivityIndicator];
	if(notifySwitch.on)
		[_twitter enableNotificationsFor:_username];
	else
		[_twitter disableNotificationsFor:_username];
	
	[TweetterAppDelegate increaseNetworkActivityIndicator];
	[_twitter getUserInformationFor:_username];
}

@end
