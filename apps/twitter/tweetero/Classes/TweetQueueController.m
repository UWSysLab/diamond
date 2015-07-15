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

#import "LoginController.h"
#import "MGTwitterEngine.h"
#import "TweetQueueController.h"
#import "TweetQueue.h"
#import "TwitEditorController.h"
#include "util.h"

@implementation TweetQueueController

@synthesize progressSheet;
@synthesize _connection;

+ (NSString*)queueTitle
{
	int count = [[TweetQueue sharedQueue] count];
	NSString *title = nil;
	if(count)
		title = [NSString stringWithFormat:NSLocalizedString(@"QueueTitleFormat", @""), count];
	else
		title = NSLocalizedString(@"Pending Tweets", @"");
	return title;
}

- (void) reloadTablesInSubview:(UIView*)parentView
{
	for (UIView* view in parentView.subviews) 
	{
		if ([view respondsToSelector:@selector(reloadData)]) 
			[view performSelector:@selector(reloadData)]; 
		[self reloadTablesInSubview:view];
	}
}

- (void)setQueueTitle
{
	NSString* title = [[self class] queueTitle];
	self.navigationItem.title = title;
	self.tabBarItem.title = title;

	UINavigationController *more = self.tabBarController.moreNavigationController;
	[self reloadTablesInSubview:more.view];
}

- (id)initWithNibName:(NSString *)nibName bundle:(NSBundle *)nibBundle
{
	self = [super initWithNibName:nibName bundle:nibBundle];
	if(self)
	{
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(setQueueTitle) name:@"QueueChanged" object:nil];
	}
	return self;
}

- (id)init
{
	return [self initWithNibName:@"TweetQueue" bundle:nil];
}

- (void)dealloc 
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	_connection = nil;
	[defaultTintColor release];
    [super dealloc];
}


- (void)enableModifyButtons:(BOOL)enable
{
	[queueSegmentedControl setEnabled:enable forSegmentAtIndex:0];
	[queueSegmentedControl setEnabled:enable forSegmentAtIndex:1];
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	if(buttonIndex > 0)
	{
		NSIndexPath *index = [self.tableView indexPathForSelectedRow];
		[[TweetQueue sharedQueue] deleteMessage:index.row];
		
		[self enableModifyButtons:NO];
		[self.tableView deselectRowAtIndexPath:index animated:NO];
		[self.tableView reloadData];
	}
}

- (void)deleteTweet 
{
	NSIndexPath *index = [self.tableView indexPathForSelectedRow];
	if(!index || index.row < 0 || index.row >= [[TweetQueue sharedQueue] count])
		return;


	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Do you wish to delete this tweet?" message:@"This operation cannot be undone"
												   delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"OK", nil];
	[alert show];
	[alert release];
}

- (void)editTweet
{
	NSIndexPath *index = [self.tableView indexPathForSelectedRow];
	if(!index || index.row < 0 || index.row >= [[TweetQueue sharedQueue] count])
		return;
		
	TwitEditorController *msgView = [[TwitEditorController alloc] init];
	[self.navigationController pushViewController:msgView animated:YES];
	[msgView editUnsentMessage:index.row];
	[msgView release];
}

- (IBAction)segmentedActions:(id)sender
{
	switch([sender selectedSegmentIndex])
	{
		case 0:
			[self editTweet];
			break;
		case 1:
			[self deleteTweet];
			break;
		case 2:
			[self sendAllTweets];
			break;
		default:
			break;
	}
}

-(void)dismissProgressSheetIfExist
{
	if(self.progressSheet)
	{
		[self.progressSheet dismissWithClickedButtonIndex:0 animated:YES];
		self.progressSheet = nil;
	}
}



- (void)postNextMessage
{
	if(![[TweetQueue sharedQueue] count] /*|| canceled*/)
	{
		[self dismissProgressSheetIfExist];
		return;
	}
			
	NSString* text;
	NSData* imageData;
	NSURL*  movieURL;
	int replyTo = 0;
	if([[TweetQueue sharedQueue] getMessage:&text andImageData:&imageData movieURL:&movieURL inReplyTo:&replyTo atIndex:0])
	{
		[self retain];
		MessageUploader * uploader = [[MessageUploader alloc] initWithText:text 
													imageJPEGData:imageData
													video:movieURL 
													replayTo:replyTo 
													delegate:self];
		self._connection = uploader;
		[uploader send];
		[uploader release];
	}
	else
	{
		[self dismissProgressSheetIfExist];
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Failed!", @"") message:NSLocalizedString(@"Error occure while reading the message", @"")
													   delegate:nil cancelButtonTitle:NSLocalizedString(@"OK", @"") otherButtonTitles: nil];
		[alert show];	
		[alert release];
		return;
	}
}

- (void)MessageUploadFinished:(BOOL)uploaded sender:(MessageUploader *)sender
{
	if([sender canceled])
	{
		[self dismissProgressSheetIfExist];
		self._connection = nil;
		[self release];
		return;
	}

	self._connection = nil;
	if(uploaded)
	{
		[[TweetQueue sharedQueue] deleteMessage:0];
		[self enableModifyButtons:NO];
		[self.tableView reloadData];
		[self postNextMessage];
	}
	else
	{
		[self dismissProgressSheetIfExist];
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Failed!", @"") message:NSLocalizedString(@"Error occure while sending the message", @"")
													   delegate:nil cancelButtonTitle:NSLocalizedString(@"OK", @"") otherButtonTitles: nil];
		[alert show];	
		[alert release];
	}
	
	
	[self release];
}



- (void)sendAllTweets 
{
	if(![[TweetQueue sharedQueue] count]) 
		return;

	NSString* login = [MGTwitterEngine username];
	NSString* pass = [MGTwitterEngine password];
	
	if(!login || !pass)
	{
		[LoginController showModal:self.navigationController];
		return;
	}

	self.progressSheet = ShowActionSheet(NSLocalizedString(@"Uploading the messages to Twitter...", @""), self, NSLocalizedString(@"Cancel", @""), self.tabBarController.view);
	[self postNextMessage];
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if(_connection)
		[_connection cancel];
}


- (void)viewDidLoad 
{
    [super viewDidLoad];

	queueSegmentedControl.frame = CGRectMake(0, 0, 130, 30);
	[queueSegmentedControl setWidth:35 forSegmentAtIndex:0];
	[queueSegmentedControl setWidth:40 forSegmentAtIndex:1];
	queueSegmentedControl.autoresizingMask = UIViewAutoresizingFlexibleWidth;
	queueSegmentedControl.segmentedControlStyle = UISegmentedControlStyleBar;
	UIBarButtonItem *segmentBarItem = [[[UIBarButtonItem alloc] initWithCustomView:queueSegmentedControl] autorelease];
	self.navigationItem.rightBarButtonItem = segmentBarItem;
	defaultTintColor = [queueSegmentedControl.tintColor retain];	// keep track of this for later

	[self enableModifyButtons:NO];
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	[self.tableView reloadData];
	[self enableModifyButtons:NO];

	if (self.navigationController.navigationBar.barStyle == UIBarStyleBlackTranslucent || self.navigationController.navigationBar.barStyle == UIBarStyleBlackOpaque) 
		queueSegmentedControl.tintColor = [UIColor darkGrayColor];
	else
		queueSegmentedControl.tintColor = defaultTintColor;
}


- (void)didReceiveMemoryWarning 
{
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload 
{
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


#pragma mark Table view methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView 
{
    return 1;
}


// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section 
{
	int tweetCount = [[TweetQueue sharedQueue] count];
    return tweetCount? tweetCount: 1;
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
	int tweetCount = [[TweetQueue sharedQueue] count];
	
    NSString *CellIdentifier = tweetCount?@"QueueCell": @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) 
	{
        cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:CellIdentifier] autorelease];
		if(tweetCount)
		{
			CGRect rect = CGRectInset(cell.frame, 2, 2);
			UILabel *label = [[UILabel alloc] initWithFrame:rect];
			label.tag = 1;
			label.lineBreakMode = UILineBreakModeWordWrap;
			label.highlightedTextColor = [UIColor whiteColor];
			label.numberOfLines = 0;
			[cell.contentView addSubview:label];
			label.opaque = NO;
			label.backgroundColor = [UIColor clearColor];
			[label release];
		}
    }
    
	if(tweetCount)
	{
		UILabel *label = (UILabel *)[cell viewWithTag:1];
		NSString *text;
		if([[TweetQueue sharedQueue] getMessage:&text andImageData:NULL movieURL:nil inReplyTo:nil atIndex:indexPath.row])
		{
			CGRect cellFrame = [cell frame];
			cellFrame.origin = CGPointMake(0, 0);
			
			label.text = text;
			CGRect rect = CGRectInset(cellFrame, 2, 2);
			label.frame = rect;
			[label sizeToFit];
			if(label.frame.size.height > 46)
			{
				cellFrame.size.height = 50 + label.frame.size.height - 46;
			}
			else
			{
				cellFrame.size.height = 50;
			}
			
			[cell setFrame:cellFrame];
		}
	}
	else
	{
		cell.textAlignment = UITextAlignmentCenter;
		cell.font = [UIFont systemFontOfSize:16];
		cell.text = @"No Unsent Tweets";
	}
	
	cell.contentView.backgroundColor = indexPath.row % 2? [UIColor colorWithRed:0.95 green:0.95 blue:0.95 alpha:1]: [UIColor whiteColor];
	
    return cell;
}


- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath 
{
	int tweetCount = [[TweetQueue sharedQueue] count];
	[self enableModifyButtons: tweetCount > 0];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	if([[TweetQueue sharedQueue] count] == 0) return 50;
	
	UITableViewCell *cell = [self tableView:tableView cellForRowAtIndexPath:indexPath];
	return cell.frame.size.height;
}


@end

