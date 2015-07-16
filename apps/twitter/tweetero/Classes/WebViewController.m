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

#import "WebViewController.h"
#import "TweetterAppDelegate.h"

@implementation WebViewController

- (id)initWithRequest:(NSURLRequest*)request
{
	self = [super initWithNibName:@"WebView" bundle:nil];
	if(self)
	{
		_request = [request retain];
		self.hidesBottomBarWhenPushed = YES;
	}
	
	return self;
}

- (void)dealloc
{
	webView.delegate = nil;
	if(webView.loading)
	{
		[webView stopLoading];
		[TweetterAppDelegate decreaseNetworkActivityIndicator];
	}
	[_request release];
	[super dealloc];
}

- (void)viewDidLoad 
{
    [super viewDidLoad];
	[webView loadRequest:_request];
	self.navigationItem.title = @"Loading...";
	// important for view orientation rotation
	self.view.autoresizesSubviews = YES;
	self.view.autoresizingMask = (UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight);
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (void)webViewDidStartLoad:(UIWebView *)webView
{
	// starting the load, show the activity indicator in the status bar
	[TweetterAppDelegate increaseNetworkActivityIndicator];
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	self.navigationItem.title = @"Failed";
}

- (void)webViewDidFinishLoad:(UIWebView *)webView
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	self.navigationItem.title = [webView stringByEvaluatingJavaScriptFromString:@"document.title"];
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
	if([[[request URL] host] isEqualToString:@"maps.google.com"])
	{
		BOOL success = NO;
		success = [[UIApplication sharedApplication] openURL: [request URL]];
		if(success)
			return NO;
	}

	return YES;
}

@end
