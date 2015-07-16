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

#import <UIKit/UIKit.h>
#import "yFrogImageDownoader.h"
#import "TwitterConnectionProtocol.h"


@interface CenteredImageView : UIImageView
{
}

@end

@interface ImageViewController : UIViewController <ImageDownoaderDelegate, UIAlertViewDelegate>
{
    IBOutlet UIImageView* imageView;
	NSString *_yFrogURL;
	UIImage *_image;
	CGAffineTransform _tabBarTransform;
	CGRect _tabBarFrame;

	IBOutlet UISegmentedControl *imageActionsSegmentedControl;
	UIColor *defaultTintColor;
	IBOutlet UIBarButtonItem *saveButton;

	id <TwitterConnectionProtocol>  connectionDelegate;
	NSDictionary *originalMessage;
}

- (id)initWithYFrogURL:(NSString*)yFrogURL;
- (id)initWithImage:(UIImage*)image;
- (void)toggleFullScreen;

- (void)receivedImage:(UIImage*)image sender:(ImageDownoader*)sender;

- (IBAction)imageSegmentedActions:(id)sender;
- (IBAction)saveActions:(id)sender;


@property (nonatomic, retain) UIImage *_image;
@property (nonatomic, retain) NSDictionary *originalMessage;
@property (nonatomic, retain) NSString *_yFrogURL;

@property (nonatomic, retain) id <TwitterConnectionProtocol> connectionDelegate;

@end
