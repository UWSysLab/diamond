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

#import "NewMessageController.h"
#import "MGTwitterEngine.h"
#import "TweetterAppDelegate.h"

@implementation NewMessageController

- (void)viewDidLoad
{
	[super viewDidLoad];
	_textModified = NO;
	self.navigationItem.rightBarButtonItem = sendButton;
	self.navigationItem.leftBarButtonItem = cancelButton;
	_twitter = [[MGTwitterEngine alloc] initWithDelegate:self];
	_message = nil;
	_user = nil;
	textEdit.text = @"";
	textEdit.delegate = self;
	[self textViewDidChange:textEdit];	
}


- (void)dealloc
{
	int connectionsCount = [_twitter numberOfConnections];
	[_twitter closeAllConnections];
	[_twitter removeDelegate];
	[_twitter release];
	while(connectionsCount-- > 0)
		[TweetterAppDelegate decreaseNetworkActivityIndicator];

	[_message release];
	[_user release];
	[super dealloc];
}

- (void)textViewDidChange:(UITextView *)textView
{
	charsCount.text = [NSString stringWithFormat:@"%d", MAX_SYMBOLS_COUNT_IN_TEXT_VIEW - [textView.text length]];
	_textModified = YES;
}

- (BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text
{
	return MAX_SYMBOLS_COUNT_IN_TEXT_VIEW >= [textView.text length] - range.length + [text length];
}


- (void)setReplyToMessage:(NSDictionary*)message
{
	_message = [message retain];
	NSString *replyToUser = [[_message objectForKey:@"user"] objectForKey:@"screen_name"];
	textEdit.text = [NSString stringWithFormat:@"@%@ ", replyToUser];
	_textModified = NO;
	[self textViewDidChange:textEdit];
}

- (void)setRetwit:(NSString*)body whose:(NSString*)username
{
	if(username)
		textEdit.text = [NSString stringWithFormat:NSLocalizedString(@"ReTwitFormat", @""), username, body];
	else
		textEdit.text = body;
	_textModified = NO;
	[self textViewDidChange:textEdit];
}

- (void)viewDidAppear:(BOOL)animated
{
	[super viewDidAppear:animated];
	[textEdit becomeFirstResponder];
}


- (IBAction)send 
{
    int replyToId = 0;
	
	if(_message)
	{
		replyToId = [[_message objectForKey:@"id"] intValue];
	}
	
	[sendButton setEnabled:NO];
	NSString* connectionID = [_twitter sendDirectMessage:textEdit.text to:_user];
	if(connectionID)
	{
		[TweetterAppDelegate increaseNetworkActivityIndicator];
		[cancelButton setEnabled:NO];
	}
}

- (IBAction)cancel;
{
	if(!_textModified || [textEdit.text length] == 0)
	{
		[self.navigationController popViewControllerAnimated:YES];
		return;
	}
	
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"The message is not sent" message:@"Your changes will be lost"
												   delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"OK", nil];
	[alert show];
	[alert release];
		
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	if(buttonIndex > 0)
		[self.navigationController popViewControllerAnimated:YES];
}


#pragma mark MGTwitterEngineDelegate methods


- (void)requestSucceeded:(NSString *)connectionIdentifier
{
	[cancelButton setEnabled:YES];
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	[[NSNotificationCenter defaultCenter] postNotificationName:@"DirectMessageSent" object:nil];
	[self.navigationController popViewControllerAnimated:YES];
}


- (void)requestFailed:(NSString *)connectionIdentifier withError:(NSError *)error
{
	[sendButton setEnabled:YES];
	[cancelButton setEnabled:YES];
	
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Failed!" message:[error localizedDescription]
												   delegate:nil cancelButtonTitle:@"OK" otherButtonTitles: nil];
	[alert show];	
	[alert release];
}


- (void)setUser:(NSString*)user
{
	_user = [user retain];
	toField.text = [NSString stringWithFormat:@"Direct Message to %@", _user];
}


@end
