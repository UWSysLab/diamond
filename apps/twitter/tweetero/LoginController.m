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

@implementation LoginController

 -(id)initWithNibName:(NSString *)nibName bundle:(NSBundle *)nibBundle modal:(BOOL)modal
 {
	self = [super initWithNibName:nibName bundle:nibBundle];
	if(self)
	{
		_modal = modal;
	}
	
	return self;
 }
 
- (IBAction)cancel:(id)sender 
{
	if(_remember)
		[MGTwitterEngine remindPassword];
    [self.navigationController dismissModalViewControllerAnimated:YES];
}

- (IBAction)login:(id)sender 
{
    NSString *login = [loginField text];
	NSString *password = [passwordField text];
    	
	[MGTwitterEngine setUsername:login password:password remember:[rememberSwitch isOn]];
	
	[[NSNotificationCenter defaultCenter] postNotificationName:@"AccountChanged" object:nil 
		userInfo:[NSDictionary dictionaryWithObjectsAndKeys:login, @"login", password, @"password", nil]];
	
	[self.navigationController dismissModalViewControllerAnimated:YES];
}

+ (void)showModeless:(UINavigationController*)parentController animated:(BOOL)anim
{
	static LoginController* sharedController;
	if(!sharedController)
	{
		sharedController = [[LoginController alloc] initWithNibName:@"Login" bundle:nil modal:NO];
	}
	
	[parentController pushViewController:sharedController animated:anim];
}

+ (void)showModal:(UINavigationController*)parentController
{
	static LoginController* sharedController;
	static UINavigationController *navigationController;
	if(!sharedController)
	{
		sharedController = [[LoginController alloc] initWithNibName:@"Login" bundle:nil modal:YES];
		navigationController = [[UINavigationController alloc] initWithRootViewController:sharedController];
	}
	
	[parentController presentModalViewController:navigationController animated:YES];
}

- (void)viewDidLoad 
{
    [super viewDidLoad];

 	self.navigationItem.rightBarButtonItem = loginButton;
	if(_modal) self.navigationItem.leftBarButtonItem = cancelButton;
	self.navigationItem.title = @"Twitter Account";
	[loginField setText:[MGTwitterEngine username]];
	[passwordField setText:[MGTwitterEngine password]];
	_remember = [[loginField text] length] == 0? NO: YES;
	[rememberSwitch setOn: _remember];
	
	UIImage *icon = [UIImage imageNamed:@"Frog.tiff"];
	if(icon)
		[iconView setImage:icon];
		
	if(_remember)
		[MGTwitterEngine forgetPassword];
}

#pragma mark -
#pragma mark <UITextFieldDelegate> Methods

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	[textField resignFirstResponder];
    return YES;
}




@end
