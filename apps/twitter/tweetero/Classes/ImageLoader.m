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

#import "ImageLoader.h"


@implementation ImageLoader

- (id)init
{
	self = [super init];
	if(self)
	{
		_cache = [[NSMutableDictionary alloc] initWithCapacity:20];
		_activeDownloads = [[NSMutableDictionary alloc] initWithCapacity:20];
	}
	
	return self;
}


+ (ImageLoader*)sharedLoader
{
	static ImageLoader *loader;
	if(!loader)
		loader = [[ImageLoader alloc] init];
	
	return loader;
}

- (UIImage*)imageWithURL:(NSString*)url
{
	UIImage *image = [_cache objectForKey:url];
	if(image) return image;
	
	NSData *imageData = [NSData dataWithContentsOfURL:[NSURL URLWithString:url]];
	if(!imageData) return nil;
	
	image = [UIImage imageWithData:imageData];
	if(image)
		[_cache setObject:image forKey:url];
	
	return image;
}

- (void)setImageWithURL:(NSString*)url toView:(UIImageView*)imageView
{
	UIImage *image = [_cache objectForKey:url];
	if(image)
	{
		imageView.image = image;
		return;
	}
	
	NSMutableArray *pendingViews = [_activeDownloads objectForKey:url];
	if(pendingViews)
	{
		[pendingViews addObject:imageView];
	}
	else
	{
		ImageDownoader *downloader = [[ImageDownoader alloc] init];
		[downloader getImageFromURL:url imageType:nonYFrog delegate:self];
		[downloader release];
		pendingViews = [NSMutableArray arrayWithObject:imageView];
		[_activeDownloads setObject:pendingViews forKey:url];
	}
	
}

- (void)receivedImage:(UIImage*)image sender:(ImageDownoader*)sender
{
	if(image)
	{
		[_cache setObject:image forKey:sender.origURL];
		
		NSMutableArray *pendingViews = [_activeDownloads objectForKey:sender.origURL];
		if(pendingViews)
		{
			for(UIImageView *view in pendingViews)
				view.image = image;
		}
	}
	
	[_activeDownloads removeObjectForKey:sender.origURL];
}

@end
