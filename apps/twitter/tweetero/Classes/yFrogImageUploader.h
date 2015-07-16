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

#import <Foundation/Foundation.h>
#import "TwitterConnectionProtocol.h"

@class ImageUploader;

@protocol ImageUploaderDelegate<NSObject>

- (void)uploadedImage:(NSString*)yFrogURL sender:(ImageUploader*)sender;

@end



@interface ImageUploader : NSObject <TwitterConnectionProtocol>
{
	NSMutableData*	result;
	id <ImageUploaderDelegate> delegate;
	id userData;
	
	NSURLConnection *connection;
	
	NSMutableString* contentXMLProperty;
	NSString*		newURL;
	BOOL			canceled;
	BOOL			scaleIfNeed;
	
	NSString*		contentType;

}

- (void)postJPEGData:(NSData*)imageJPEGData delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data;
- (void)postMP4Data:(NSData*)movieData delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data;
- (void)postImage:(UIImage*)image delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data; // call postJPEGData:delegate:userData:
- (void)cancel;
- (BOOL)canceled;


@property (nonatomic, retain) NSURLConnection *connection;
@property (nonatomic, retain) NSMutableString* contentXMLProperty;
@property (nonatomic, retain) NSString* newURL;
@property (nonatomic, retain) NSString* contentType;
@property (nonatomic, retain) id userData;
@property (nonatomic, retain) id <ImageUploaderDelegate> delegate;

@end
