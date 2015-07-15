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

#import "MessageListController.h"
#import "LoginController.h"
#import "MGTwitterEngine.h"
#import "ImageLoader.h"
#import "MessageViewController.h"
#import "TweetterAppDelegate.h"
#include "util.h"

#define NAME_TAG 1
#define TIME_TAG 2
#define IMAGE_TAG 3
#define TEXT_TAG 4
#define ROW_HEIGHT 70

@implementation MessageListController



- (void)dealloc
{
	while (_indicatorCount) 
	{
		[self releaseActivityIndicator];
	}
	
	[_indicator release];
	
	int connectionsCount = [_twitter numberOfConnections];
	[_twitter closeAllConnections];
	[_twitter removeDelegate];
	[_twitter release];
	while(connectionsCount-- > 0)
		[TweetterAppDelegate decreaseNetworkActivityIndicator];

	[_messages release];
	
	
	if(_errorDesc)
		[_errorDesc release];
	
	[super dealloc];
}

- (void)viewDidLoad 
{
    [super viewDidLoad];
	
	_errorDesc = nil;
	_lastMessage = NO;
	_loading = [MGTwitterEngine username] != nil;
	_indicatorCount = 0;
	_indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
	
	CGRect frame = self.tableView.frame;
	CGRect indFrame = _indicator.frame;
	frame.origin.x += (frame.size.width - indFrame.size.width) * 0.5f;
	frame.origin.y += (frame.size.height - indFrame.size.height) * 0.3f;
	frame.size = indFrame.size;
	_indicator.frame = frame;
	
	_twitter = [[MGTwitterEngine alloc] initWithDelegate:self];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(accountChanged:) name:@"AccountChanged" object:nil];

	[self performSelector:@selector(reloadAll) withObject:nil afterDelay:0.5f];
}

- (void)viewWillAppear:(BOOL)animated 
{
    [super viewWillAppear:animated];
	
	if(!_messages)
		[self.tableView reloadData];
}

- (void)didReceiveMemoryWarning 
{
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

#pragma mark Table view methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView 
{
    return 1;
}

- (BOOL)noMessages
{
	return !_messages || [_messages count] == 0;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section 
{
    return [self noMessages] ? 1:
		_lastMessage? [_messages count]: [_messages count] + 1;
}


#define IMAGE_SIDE 48
#define BORDER_WIDTH 5
#define TEXT_OFFSET_X (BORDER_WIDTH * 2 + IMAGE_SIDE)
#define LABEL_HEIGHT 20
#define LABEL_WIDTH 130
#define TEXT_WIDTH (320 - TEXT_OFFSET_X - BORDER_WIDTH)
#define TEXT_OFFSET_Y (BORDER_WIDTH * 2 + LABEL_HEIGHT)
#define TEXT_HEIGHT (ROW_HEIGHT - TEXT_OFFSET_Y - BORDER_WIDTH)

- (UITableViewCell *)tableviewCellWithReuseIdentifier:(NSString *)identifier 
{
	if([identifier isEqualToString:@"UICell"])
	{
		UITableViewCell *uiCell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:identifier] autorelease];
		uiCell.textAlignment = UITextAlignmentCenter;
		uiCell.font = [UIFont systemFontOfSize:16];
		return uiCell;
	}
		
	
	
	if([identifier isEqualToString:@"TwittListCell"])
	{
		CGRect rect;
			
		rect = CGRectMake(0.0, 0.0, 320.0, ROW_HEIGHT);
		
		UITableViewCell *cell = [[[UITableViewCell alloc] initWithFrame:rect reuseIdentifier:identifier] autorelease];
		
		//Userpic view
		rect = CGRectMake(BORDER_WIDTH, (ROW_HEIGHT - IMAGE_SIDE) / 2.0, IMAGE_SIDE, IMAGE_SIDE);
		UIImageView *imageView = [[UIImageView alloc] initWithFrame:rect];
		imageView.tag = IMAGE_TAG;
		[cell.contentView addSubview:imageView];
		[imageView release];
		
		
		UILabel *label;
		
		//Username
		rect = CGRectMake(TEXT_OFFSET_X, BORDER_WIDTH, LABEL_WIDTH, LABEL_HEIGHT);
		label = [[UILabel alloc] initWithFrame:rect];
		label.tag = NAME_TAG;
		label.font = [UIFont boldSystemFontOfSize:[UIFont labelFontSize]];
		label.highlightedTextColor = [UIColor whiteColor];
		[cell.contentView addSubview:label];
		label.opaque = NO;
		label.backgroundColor = [UIColor clearColor];
		
		[label release];
		
		//Message creation time
		rect = CGRectMake(TEXT_OFFSET_X + LABEL_WIDTH, BORDER_WIDTH, LABEL_WIDTH, LABEL_HEIGHT);
		label = [[UILabel alloc] initWithFrame:rect];
		label.tag = TIME_TAG;
		label.font = [UIFont systemFontOfSize:[UIFont smallSystemFontSize]];
		label.textAlignment = UITextAlignmentRight;
		label.highlightedTextColor = [UIColor whiteColor];
		label.textColor = [UIColor lightGrayColor];
		[cell.contentView addSubview:label];
		label.opaque = NO;
		label.backgroundColor = [UIColor clearColor];
		
		[label release];

		//Message body
		rect = CGRectMake(TEXT_OFFSET_X, TEXT_OFFSET_Y, TEXT_WIDTH, TEXT_HEIGHT);
		label = [[UILabel alloc] initWithFrame:rect];
		label.tag = TEXT_TAG;
		label.lineBreakMode = UILineBreakModeWordWrap;
		label.font = [UIFont systemFontOfSize:[UIFont systemFontSize]];
		label.highlightedTextColor = [UIColor whiteColor];
		label.numberOfLines = 0;
		[cell.contentView addSubview:label];
		label.opaque = NO;
		label.backgroundColor = [UIColor clearColor];
		
		[label release];
		
		return cell;
	}
	
	return nil;
}

- (NSString*)noMessagesString
{
	return @"";
}

- (NSString*)loadingMessagesString
{
	return @"";
}

- (void)configureCell:(UITableViewCell *)cell forIndexPath:(NSIndexPath *)indexPath 
{

	if([self noMessages])
	{
		if(_errorDesc)
			cell.text = _errorDesc;
		else
			cell.text = _loading? [self loadingMessagesString]: [self noMessagesString];
		return;
	}
    
	if(indexPath.row < [_messages count])
	{
		static NSDateFormatter *dateFormatter = nil;
		if (dateFormatter == nil) 
			dateFormatter = [[NSDateFormatter alloc] init];

		static NSCalendar *calendar;
		if(calendar == nil)
			calendar= [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
		
		NSDictionary *messageData = [_messages objectAtIndex:indexPath.row];
		NSDictionary *userData = [messageData objectForKey:@"user"];
		if(!userData)
			userData = [messageData objectForKey:@"sender"];
		
		CGRect cellFrame = [cell frame];
		//Set message text
		UILabel *label;
		label = (UILabel *)[cell viewWithTag:TEXT_TAG];
		label.text = DecodeEntities([messageData objectForKey:@"text"]);
		[label setFrame:CGRectMake(TEXT_OFFSET_X, TEXT_OFFSET_Y, TEXT_WIDTH, TEXT_HEIGHT)];
		[label sizeToFit];
		if(label.frame.size.height > TEXT_HEIGHT)
		{
			cellFrame.size.height = ROW_HEIGHT + label.frame.size.height - TEXT_HEIGHT;
		}
		else
		{
			cellFrame.size.height = ROW_HEIGHT;
		}
		
		[cell setFrame:cellFrame];

		
		//Set message date and time
		NSCalendarUnit unitFlags = NSYearCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit;
		label = (UILabel *)[cell viewWithTag:TIME_TAG];
		NSDate *createdAt = [messageData objectForKey:@"created_at"];
		NSDateComponents *nowComponents = [calendar components:unitFlags fromDate:[NSDate date]];
		NSDateComponents *yesterdayComponents = [calendar components:unitFlags fromDate:[NSDate dateWithTimeIntervalSinceNow:-60*60*24]];
		NSDateComponents *createdAtComponents = [calendar components:unitFlags fromDate:createdAt];
		
		if([nowComponents year] == [createdAtComponents year] &&
			[nowComponents month] == [createdAtComponents month] &&
			[nowComponents day] == [createdAtComponents day])
		{
			[dateFormatter setDateStyle:NSDateFormatterNoStyle];
			[dateFormatter setTimeStyle:NSDateFormatterShortStyle];
			label.text = [dateFormatter stringFromDate:createdAt];
		}
		else if([yesterdayComponents year] == [createdAtComponents year] &&
			[yesterdayComponents month] == [createdAtComponents month] &&
			[yesterdayComponents day] == [createdAtComponents day])
		{
			[dateFormatter setDateStyle:NSDateFormatterNoStyle];
			[dateFormatter setTimeStyle:NSDateFormatterShortStyle];
			label.text = [NSString stringWithFormat:@"Yesterday, %@", [dateFormatter stringFromDate:createdAt]];
		}
		else
		{
			[dateFormatter setDateStyle:NSDateFormatterMediumStyle];
			[dateFormatter setTimeStyle:NSDateFormatterShortStyle];
			label.text = [dateFormatter stringFromDate:createdAt];
		}
		
				
		//Set userpic
		UIImageView *imageView = (UIImageView *)[cell viewWithTag:IMAGE_TAG];
		imageView.image = nil;
		[[ImageLoader sharedLoader] setImageWithURL:[userData objectForKey:@"profile_image_url"] toView:imageView];
		
		//Set user name
		label = (UILabel *)[cell viewWithTag:NAME_TAG];
		label.text = [userData objectForKey:@"screen_name"];
	} 
	else
	{
		cell.text = @"Load More...";
	}
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath 
{
    NSString *CellIdentifier = ![self noMessages] && indexPath.row < [_messages count]? @"TwittListCell": @"UICell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) 
	{
        cell = [self tableviewCellWithReuseIdentifier:CellIdentifier];
    }
    
    [self configureCell:cell forIndexPath:indexPath];
	
	cell.contentView.backgroundColor = indexPath.row % 2? [UIColor colorWithRed:0.95 green:0.95 blue:0.95 alpha:1]: [UIColor whiteColor];
	
    return cell;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	if(indexPath.row >= [_messages count]) return 50;
	
	UITableViewCell *cell = [self tableView:tableView cellForRowAtIndexPath:indexPath];
	return cell.frame.size.height;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath 
{	
	if([self noMessages])
		return;
		
	if(indexPath.row < [_messages count])
	{
		NSMutableDictionary *messageData = [NSMutableDictionary dictionaryWithDictionary:[_messages objectAtIndex:indexPath.row]];
		id userInfo = [messageData objectForKey:@"sender"];
		if(userInfo && [messageData objectForKey:@"user"] == nil)
		{
			[messageData setObject:userInfo forKey:@"user"];
			[messageData setObject:[NSNumber numberWithBool:YES] forKey:@"DirectMessage"];
		}
			
		MessageViewController *msgView = [[MessageViewController alloc] initWithMessage:messageData];
		[self.navigationController pushViewController:msgView animated:YES];
		[msgView release];
	}
	else
	{
		[self loadMessagesStaringAtPage:++_pagenum count:MESSAGES_PER_PAGE];
	}
}



- (void)accountChanged:(NSNotification*)notification
{
	[self reloadAll];
}

- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{

}

#pragma mark MGTwitterEngineDelegate methods


- (void)requestSucceeded:(NSString *)connectionIdentifier
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	_loading = NO;
}


- (void)requestFailed:(NSString *)connectionIdentifier withError:(NSError *)error
{
	if(self.navigationItem.leftBarButtonItem)
			self.navigationItem.leftBarButtonItem.enabled = YES;
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	_loading = NO;
	_errorDesc = [[[error localizedDescription] capitalizedString] retain];
	
	[self releaseActivityIndicator];
	
	if(self.tabBarController.selectedViewController == self.navigationController && [error code] == 401)
		[LoginController showModal:self.navigationController];
		
	if(_messages)
	{
		[_messages release];
		_messages = nil;
	}
	
	[self.tableView reloadData];
}


- (void)statusesReceived:(NSArray *)statuses forRequest:(NSString *)connectionIdentifier
{
	if([statuses count] < MESSAGES_PER_PAGE)
	{
		_lastMessage = YES;
		if(_messages)
			[self.tableView deleteRowsAtIndexPaths:
					[NSArray arrayWithObject: [NSIndexPath indexPathForRow:[_messages count] inSection:0]]
				withRowAnimation:UITableViewRowAnimationTop];
	}
	
	if(!_messages)
	{
		if([statuses count] > 0)
			_messages = [statuses retain];
		[self.tableView reloadData];
	}
	else
	{
		NSArray *messages = _messages;
		_messages = [[messages arrayByAddingObjectsFromArray:statuses] retain];
		NSMutableArray *indices = [NSMutableArray arrayWithCapacity:[statuses count]];
		for(int i = [messages count]; i < [_messages count]; ++i)
			[indices addObject:[NSIndexPath indexPathForRow:i inSection:0]];
			
		@try
		{
			[self.tableView insertRowsAtIndexPaths:indices withRowAnimation:UITableViewRowAnimationTop];
		}
		@catch (NSException * e) 
		{
			NSLog(@"Tweet List Error!!!\nNumber of rows: %d\n_messages: %@\nstatuses: %@\nIndices: %@\n",
				[self tableView:self.tableView numberOfRowsInSection:0],
				_messages, statuses, indices);
		}
		
		[messages release];
	}
	
	[self releaseActivityIndicator];
	
	if(self.navigationItem.leftBarButtonItem)
		self.navigationItem.leftBarButtonItem.enabled = YES;
}


NSInteger dateReverseSort(id num1, id num2, void *context)
{
	NSDate *d1 = [num1 objectForKey:@"created_at"];
	NSDate *d2 = [num2 objectForKey:@"created_at"];
	return [d2 compare:d1];
}

- (void)directMessagesReceived:(NSArray *)statuses forRequest:(NSString *)connectionIdentifier;
{
	if([statuses count] < MESSAGES_PER_PAGE)
	{
		_lastMessage = YES;
		if(_messages && [_messages count] > 0 && [self.tableView numberOfRowsInSection:0] > [_messages count])
			[self.tableView deleteRowsAtIndexPaths:
					[NSArray arrayWithObject: [NSIndexPath indexPathForRow:[_messages count] inSection:0]]
				withRowAnimation:UITableViewRowAnimationTop];
	}
	
	if(!_messages)
	{
		if([statuses count] > 0)
			_messages = [statuses retain];
		[self.tableView reloadData];
	}
	else
	{
		NSArray *messages = _messages;
		
		[statuses setValue:[NSNumber numberWithBool:YES] forKey:@"NewItem"];
		_messages = [[[messages arrayByAddingObjectsFromArray:statuses] sortedArrayUsingFunction:dateReverseSort context:nil] retain];
		NSMutableArray *indices = [NSMutableArray arrayWithCapacity:[statuses count]];
		for(int i = 0; i < [_messages count]; ++i)
		{
			if([[_messages objectAtIndex:i] valueForKey:@"NewItem"])
			{
				[indices addObject:[NSIndexPath indexPathForRow:i inSection:0]];
			}
		}
		
		@try 
		{
			[self.tableView insertRowsAtIndexPaths:indices withRowAnimation:UITableViewRowAnimationTop];
		}
		@catch (NSException * e) 
		{
			NSLog(@"Direct Messages Error!!!\nNumber of rows: %d\n_messages: %@\nstatuses: %@\nIndices: %@\n",
				[self tableView:self.tableView numberOfRowsInSection:0],
				_messages, statuses, indices);
		}
		@finally 
		{
			[_messages setValue:nil forKey:@"NewItem"];
		}
		
		[messages release];
	}
	
	
	[self releaseActivityIndicator];
	
	if(self.navigationItem.leftBarButtonItem)
		self.navigationItem.leftBarButtonItem.enabled = YES;
}

#pragma mark ===



- (void)loadMessagesStaringAtPage:(int)numPage count:(int)count
{
	if([MGTwitterEngine password] != nil)
	{
		if(_errorDesc)
		{
			[_errorDesc release];
			_errorDesc = nil;
		}
		_loading = YES;
		[self retainActivityIndicator];
		if(self.navigationItem.leftBarButtonItem)
			self.navigationItem.leftBarButtonItem.enabled = NO;
		if([self noMessages])
			[self.tableView reloadData];
	}
}

- (void)reloadAll
{
	_lastMessage = NO;
	_pagenum = 1;
	
	if(_messages)
	{
		[_messages release];
		_messages = nil;
	}
	
	[self loadMessagesStaringAtPage:_pagenum count:MESSAGES_PER_PAGE];
}

- (void)retainActivityIndicator
{
	if(++_indicatorCount == 1)
	{
		[self.tableView.superview addSubview:_indicator];
		[_indicator startAnimating];
	}
}

- (void)releaseActivityIndicator
{
	if(_indicatorCount > 0)
	{
		if(--_indicatorCount == 0)
		{
			[_indicator stopAnimating];
			[_indicator removeFromSuperview];
		}
	}
}

@end

