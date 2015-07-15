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

#import "SettingsController.h"
#import "LocationManager.h"
#import "MGTwitterEngine.h"
#import "LoginController.h"

@implementation SettingsController

-(id)init
{
	self = [super init];
	if(self)
	{
		
	}
	return self;
}


-(id)initWithNibName:(NSString *)nibName bundle:(NSBundle *)nibBundle
{
	self = [super initWithNibName:nibName bundle:nibBundle];
	if(self)
	{
		
	}
	return self;
}

- (void) setNavigatorButtons
{

}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad 
{
    [super viewDidLoad];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(accountChanged:) name:@"AccountChanged" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateLocationDefaultsChanged:) name:@"UpdateLocationDefaultsChanged" object:nil];
	self.navigationItem.title = NSLocalizedString(@"Settings", @"");
	firstMailAddressView.delegate = self;
	firstMailAddressView.hidden = YES;
	postMailLabel.hidden = YES;
	postMailSwitch.hidden = YES;
	
	[self accountChanged:nil];
}

- (void)navigationController:(UINavigationController *)navigationController didShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{

}

- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{

}

- (void)didReceiveMemoryWarning 
{
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}


- (void)dealloc 
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (IBAction) postLocationChanged
{
	[[NSUserDefaults standardUserDefaults] setBool:postLocationsSwitch.on forKey:@"UseLocations"];
	if(postLocationsSwitch.on)
		[[LocationManager locationManager] startUpdates];
	else
		[[LocationManager locationManager] stopUpdates];
	[[NSNotificationCenter defaultCenter] postNotificationName: @"UpdateLocationDefaultsChanged" object: nil];
}


- (IBAction) postMailChanged
{
	[[NSUserDefaults standardUserDefaults] setBool:postMailSwitch.on forKey:@"PostMail"];
}

- (IBAction) postFirstMailAddressChanged
{
	[[NSUserDefaults standardUserDefaults] setObject:[NSArray arrayWithObject:firstMailAddressView.text] forKey:@"PostMailAddresses"];
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	postLocationsSwitch.on = [[NSUserDefaults standardUserDefaults] boolForKey:@"UseLocations"];
	postMailSwitch.on = [[NSUserDefaults standardUserDefaults] boolForKey:@"PostMail"];
	scaleLargeImagesSwitch.on = [[NSUserDefaults standardUserDefaults] boolForKey:@"ScalePhotosBeforeUploading"];

	NSArray* addresses = [[NSUserDefaults standardUserDefaults] arrayForKey:@"PostMailAddresses"];
	firstMailAddressView.text = (addresses && [addresses count]) ? [addresses objectAtIndex:0] : @"";
}


- (IBAction)login:(id)sender;
{
	[LoginController showModal:self.navigationController];
}


- (void)updateLocationDefaultsChanged:(NSNotification*)notification
{
	postLocationsSwitch.on = [[NSUserDefaults standardUserDefaults] boolForKey:@"UseLocations"];
}

- (void)accountChanged:(NSNotification*)notification
{
	if([MGTwitterEngine username] && [MGTwitterEngine password])
	{
		loginInfo.text = [NSString stringWithFormat:@"You are logged in to Twitter as %@", [MGTwitterEngine username]];
		[loginButton setTitle:@"Change Account" forState:0];
	}
	else
	{
		loginInfo.text = @"You are not logged in to Twitter";
		[loginButton setTitle:@"Log In" forState:0];
	}
	
}

- (IBAction) scaleLargeImagesChanged
{
	[[NSUserDefaults standardUserDefaults] setBool:scaleLargeImagesSwitch.on forKey:@"ScalePhotosBeforeUploading"];
}

@end
