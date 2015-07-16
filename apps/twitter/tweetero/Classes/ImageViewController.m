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

#import "ImageViewController.h"
#import "TwitEditorController.h"

#define	PICTURE_SAVING_ALERT				165
#define	PICTURE_DOWNLOADING_ERROR_ALERT		80


@implementation ImageViewController

@synthesize _image;
@synthesize _yFrogURL;
@synthesize connectionDelegate;
@synthesize originalMessage;

- (id)initWithYFrogURL:(NSString*)yFrogURL
{
    if (self = [super initWithNibName:@"ImageView" bundle:nil]) 
	{
        self._yFrogURL = yFrogURL;
		self._image = nil;
		self.hidesBottomBarWhenPushed = YES;
    }
    return self;
}


- (id)initWithImage:(UIImage*)image
{
    if (self = [super initWithNibName:@"ImageView" bundle:nil]) 
	{
        self._yFrogURL = nil;
		self._image = image;
		self.hidesBottomBarWhenPushed = YES;
    }
    return self;
}

- (void)applyImage
{
	[imageView setImage:self._image];
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad 
{
    [super viewDidLoad];
	
	
	if(!self._image && self._yFrogURL)
	{
		ImageDownoader * downloader = [[ImageDownoader alloc] init];
		self.connectionDelegate = downloader;
		[downloader getImageFromURL:self._yFrogURL imageType:iPhoneYFrog delegate:self];
		[downloader release];
	}
	
	if(self._image)
		[self applyImage];
	
	_tabBarTransform = self.tabBarController.view.transform;
	_tabBarFrame = self.tabBarController.view.frame;
	
	defaultTintColor = [imageActionsSegmentedControl.tintColor retain];	// keep track of this for later
	
	if(self._yFrogURL)
	{
		imageActionsSegmentedControl.frame = CGRectMake(0, 0, 190, 30);
		imageActionsSegmentedControl.autoresizingMask = UIViewAutoresizingFlexibleWidth;
		imageActionsSegmentedControl.segmentedControlStyle = UISegmentedControlStyleBar;
		UIBarButtonItem *segmentBarItem = [[UIBarButtonItem alloc] initWithCustomView:imageActionsSegmentedControl];
		self.navigationItem.rightBarButtonItem = segmentBarItem;
		[segmentBarItem release];
	}
	else
		self.navigationItem.rightBarButtonItem = saveButton;	
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	self.navigationController.navigationBar.barStyle = UIBarStyleBlackTranslucent;
	[[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleBlackTranslucent animated:YES];
	if (self.navigationController.navigationBar.barStyle == UIBarStyleBlackTranslucent || self.navigationController.navigationBar.barStyle == UIBarStyleBlackOpaque) 
		imageActionsSegmentedControl.tintColor = [UIColor darkGrayColor];
	else
		imageActionsSegmentedControl.tintColor = defaultTintColor;
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
	
	if(connectionDelegate)
		[connectionDelegate cancel];

	[[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationPortrait];
	self.tabBarController.view.transform = _tabBarTransform;
	self.tabBarController.view.frame = _tabBarFrame;
	self.navigationController.navigationBar.barStyle = UIBarStyleDefault;
	[[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleDefault animated:YES];
}


- (void)didReceiveMemoryWarning 
{
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}


- (void)dealloc 
{
	self._yFrogURL = nil;
	self._image = nil;
	[defaultTintColor release];
	self.connectionDelegate = nil;
	self.originalMessage = nil;
    [super dealloc];
}


- (UIView *) viewForZoomingInScrollView: (UIScrollView *) scrollView
{
	return imageView;
}

- (void) scrollViewDidEndZooming: (UIScrollView *) scrollView withView: (UIView *) view atScale: (float) scale
{
	CGAffineTransform transform = CGAffineTransformIdentity;
	transform = CGAffineTransformScale(transform, scale, scale);
	imageView.transform = transform;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

-(void)setFullScreen:(BOOL)fullScreen
{
	BOOL isFullScreenNow = self.navigationController.navigationBarHidden;
	if(fullScreen != isFullScreenNow)
		[self toggleFullScreen];
}

- (void)toggleFullScreen
{
	BOOL navBarHidden = self.navigationController.navigationBarHidden;
			
	[self.navigationController 
		setNavigationBarHidden: !navBarHidden
		animated:YES];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event 
{ 
	NSSet *allTouches = [event allTouches];

	if([allTouches count] == 1)
	{
		UITouch *touch = [[allTouches allObjects] objectAtIndex:0];
		if([touch tapCount] == 1)
		{
			[self toggleFullScreen];
		}
	}
}

- (void)receivedImage:(UIImage*)image sender:(ImageDownoader*)sender
{
	self.connectionDelegate = nil;
	if(image)
	{
		self._image = image;
		[self applyImage];
	}
	else if(![sender canceled])
	{
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Failed!", @"") 
													message:NSLocalizedString(@"Failed to download the image.", @"")
													delegate:self cancelButtonTitle:NSLocalizedString(@"OK", @"") otherButtonTitles: nil];
		alert.tag = PICTURE_DOWNLOADING_ERROR_ALERT;
		[alert show];	
		[alert release];
	}
}


- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	if(alertView.tag == PICTURE_SAVING_ALERT)
		return;

	if(alertView.tag == PICTURE_DOWNLOADING_ERROR_ALERT)
	{
		[self setFullScreen:NO];
		[self.navigationController popViewControllerAnimated:YES];
	}
}

- (void) forward
{
	if(!self._yFrogURL)
		return;
		
	BOOL success = NO;
	NSString *mailto = [NSString stringWithFormat:@"mailto:?&subject=%@&body=%%26lt%%3B%@%%26gt%%3B", 
						[NSLocalizedString(@"Mail Subject: Forwarding of an image", @"") stringByAddingPercentEscapesUsingEncoding:NSASCIIStringEncoding],
						[self._yFrogURL stringByAddingPercentEscapesUsingEncoding:NSASCIIStringEncoding]
						];
	success = [[UIApplication sharedApplication] openURL:[NSURL URLWithString:mailto]];
	if(!success)
	{
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Failed!", @"") message:NSLocalizedString(@"Failed to send a mail.", @"")
													   delegate:nil cancelButtonTitle:@"OK" otherButtonTitles: nil];
		[alert show];	
		[alert release];
	}

}

- (void) reTwit
{
	if(!self._yFrogURL)
		return;
		
	TwitEditorController *msgView = [[TwitEditorController alloc] init];
	[self.navigationController pushViewController:msgView animated:YES];
	[msgView setRetwit:self._yFrogURL whose:self.originalMessage ? [[originalMessage objectForKey:@"user"] objectForKey:@"screen_name"] : nil];
	[msgView release];
}

- (IBAction)imageSegmentedActions:(id)sender
{
	switch([sender selectedSegmentIndex])
	{
		case 0:
			[self forward];
			break;
		case 1:
			[self reTwit];
			break;
		case 2:
			[self saveActions:nil];
			break;
		default:
			break;
	}
}

- (IBAction)saveActions:(id)sender
{
	[self retain];
	UIImageWriteToSavedPhotosAlbum(self._image, self, @selector(image:didFinishSavingWithError:contextInfo:), nil);
}

- (void)image:(UIImage *)image didFinishSavingWithError:(NSError *)error contextInfo:(void *)contextInfo
{
	[self release];
	UIAlertView *alert = nil;
	if(error)
	{
		alert = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Failed!", @"") message:[error localizedDescription]
									delegate:self cancelButtonTitle:NSLocalizedString(@"OK", @"") otherButtonTitles:nil];
	}
	else
	{
		alert = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Picture Saved", @"") message:NSLocalizedString(@"The picture has been added to your Photos", @"")
									delegate:self cancelButtonTitle:NSLocalizedString(@"OK", @"") otherButtonTitles:nil];
	}
	alert.tag = PICTURE_SAVING_ALERT;
	[alert show];
	[alert release];
}

@end


@implementation CenteredImageView


- (void)setFrame:(CGRect)frame
{
	[super setFrame:frame];
}


@end