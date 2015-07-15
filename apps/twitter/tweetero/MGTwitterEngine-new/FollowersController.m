//Copyright (c) 2009 Imageshack Corp.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//3. The name of the author may not be used to endorse or promote products
//   derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
//IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
//NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
//THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#import "FollowersController.h"
#import "LoginController.h"
#import "MGTwitterEngine.h"
#import "ImageLoader.h"
#import "UserInfo.h"
#import "TweetterAppDelegate.h"

#define NAME_TAG 1
#define REAL_NAME_TAG 2
#define IMAGE_TAG 3
#define ROW_HEIGHT 60

@implementation UserListController



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

	[_users release];
	[super dealloc];
}

- (void)viewDidLoad 
{
    [super viewDidLoad];
	
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
	
	if(!_users)
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

- (BOOL)noUsers
{
	return !_users || [_users count] == 0;
}

// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section 
{
    return [self noUsers] ? 1: [_users count];
}


#define IMAGE_SIDE 48
#define BORDER_WIDTH 5
#define TEXT_OFFSET_X (BORDER_WIDTH * 2 + IMAGE_SIDE)
#define LABEL_HEIGHT 20
#define LABEL_WIDTH 180
#define TEXT_OFFSET_Y (BORDER_WIDTH * 2 + LABEL_HEIGHT)

- (UITableViewCell *)tableviewCellWithReuseIdentifier:(NSString *)identifier 
{
	if([identifier isEqualToString:@"UICell"])
	{
		UITableViewCell *uiCell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:identifier] autorelease];
		uiCell.textAlignment = UITextAlignmentCenter;
		uiCell.font = [UIFont systemFontOfSize:16];
		return uiCell;
	}
	
	
	
	if([identifier isEqualToString:@"UserListCell"])
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
		
		

		//Real Name
		rect = CGRectMake(TEXT_OFFSET_X, TEXT_OFFSET_Y, LABEL_WIDTH, LABEL_HEIGHT);
		label = [[UILabel alloc] initWithFrame:rect];
		label.tag = REAL_NAME_TAG;
		label.font = [UIFont boldSystemFontOfSize:[UIFont labelFontSize]];
		label.highlightedTextColor = [UIColor whiteColor];
		[cell.contentView addSubview:label];
		label.opaque = NO;
		label.backgroundColor = [UIColor clearColor];
		
		[label release];
		
		return cell;
	}
	
	return nil;
}

- (NSString*)noUsersString
{
	return @"No Followers";
}

- (NSString*)loadingMessagesString
{
	return @"Loading the List of Followers...";
}

- (void)configureCell:(UITableViewCell *)cell forIndexPath:(NSIndexPath *)indexPath 
{
	
	if([self noUsers])
	{
		cell.text = _loading? [self loadingMessagesString]: [self noUsersString];
		return;
	}
    
	if(indexPath.row < [_users count])
	{
		NSDictionary *userData = [_users objectAtIndex:indexPath.row];
		
		
		//Set userpic
		UIImageView *imageView = (UIImageView *)[cell viewWithTag:IMAGE_TAG];
		imageView.image = nil;
		[[ImageLoader sharedLoader] setImageWithURL:[userData objectForKey:@"profile_image_url"] toView:imageView];
		
		UILabel *label;
		//Set user name
		label = (UILabel *)[cell viewWithTag:NAME_TAG];
		label.text = [userData objectForKey:@"screen_name"];
		
		//Set real user name
		label = (UILabel *)[cell viewWithTag:REAL_NAME_TAG];
		label.text = [userData objectForKey:@"name"];
	} 
	else
	{
		cell.text = @"Load More...";
	}
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath 
{
    
    NSString *CellIdentifier = ![self noUsers] && indexPath.row < [_users count]? @"UserListCell": @"UICell";
    
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
	if(indexPath.row >= [_users count]) return 50;
	
	return ROW_HEIGHT;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath 
{	
	if([self noUsers])
		return;
	
	if(indexPath.row < [_users count])
	{
		id userInfo = [_users objectAtIndex:indexPath.row];
		UserInfo *infoView = [[UserInfo alloc] initWithUserName:[userInfo objectForKey:@"screen_name"]];
		[self.navigationController pushViewController:infoView animated:YES];
		[infoView release];
	}

}



- (void)accountChanged:(NSNotification*)notification
{
	[self reloadAll];
}

- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{
	//NSLog(@"%@", viewController);
}

#pragma mark MGTwitterEngineDelegate methods


- (void)requestSucceeded:(NSString *)connectionIdentifier
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	_loading = NO;
    //NSLog(@"Request succeeded for connectionIdentifier = %@", connectionIdentifier);
}


- (void)requestFailed:(NSString *)connectionIdentifier withError:(NSError *)error
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	_loading = NO;
    /*NSLog(@"Request failed for connectionIdentifier = %@, error = %@ (%@)", 
          connectionIdentifier, 
          [error localizedDescription], 
          [error userInfo]);*/
	
	[self releaseActivityIndicator];
	
	if(self.tabBarController.selectedViewController == self.navigationController && [error code] == 401)
		[LoginController showModal:self.navigationController];
	
	if(_users)
	{
		[_users release];
		_users = nil;
	}
	
	[self.tableView reloadData];
}

- (void)userInfoReceived:(NSArray *)userInfo forRequest:(NSString *)connectionIdentifier;
{
//	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	if(_users)
		[_users release];
	
	_users = [userInfo retain];
	
	[self.tableView reloadData];
	
	[self releaseActivityIndicator];
}

#pragma mark ===



- (void)loadFollowers
{
	if([MGTwitterEngine password] != nil)
	{
		_loading = YES;
		[self retainActivityIndicator];
	}
}

- (void)reloadAll
{
	if(_users)
	{
		[_users release];
		_users = nil;
	}
	
	[self loadFollowers];
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

@implementation FollowersController

- (NSString*)noUsersString
{
	return @"No Followers";
}

- (NSString*)loadingMessagesString
{
	return @"Loading the List of Followers...";
}

- (void)loadFollowers
{
	[super loadFollowers];
	
	if([MGTwitterEngine password] != nil)
	{
		[TweetterAppDelegate increaseNetworkActivityIndicator];
		[_twitter getFollowersIncludingCurrentStatus:YES];
	}
}


@end


@implementation FollowingController

- (NSString*)noUsersString
{
	return @"No Following Users";
}

- (NSString*)loadingMessagesString
{
	return @"Loading the List of Following Users...";
}

- (void)loadFollowers
{
	[super loadFollowers];
	
	if([MGTwitterEngine password] != nil)
	{
		[TweetterAppDelegate increaseNetworkActivityIndicator];
		[_twitter getRecentlyUpdatedFriendsFor:nil startingAtPage:0];
	}
}


@end
