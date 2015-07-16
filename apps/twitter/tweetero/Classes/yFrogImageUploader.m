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

#import "yFrogImageUploader.h"
#import "TweetterAppDelegate.h"
#import "MGTwitterEngine.h"
#import "LocationManager.h"
#include "util.h"

#define		JPEG_CONTENT_TYPE			@"image/jpeg"
#define		MP4_CONTENT_TYPE			@"video/mp4"

@implementation ImageUploader

@synthesize connection;
@synthesize contentXMLProperty;
@synthesize newURL;
@synthesize userData;
@synthesize delegate;
@synthesize contentType;

-(id)init
{
	self = [super init];
	if(self)
	{
		result = [[NSMutableData alloc] initWithCapacity:128];
		canceled = NO;
		scaleIfNeed = NO;
	}
	return self;
}

/*
- (id)retain
{
	return [super retain];
}
- (oneway void)release
{
	[super release];
}

- (id)autorelease
{
	return [super autorelease];
}
*/

-(void)dealloc
{
	self.delegate = nil;
	self.connection = nil;
	self.contentXMLProperty = nil;
	self.newURL = nil;
	self.userData = nil;
	self.contentType = nil;
	[result  release];
	[super dealloc];
}

- (void) postData:(NSData*)data
{
	if(canceled)
		return;
		
	if(!self.contentType)
	{
		NSLog(@"Content-Type header was not setted\n");
		return;
	}
	
	NSString* login = [MGTwitterEngine username];
	NSString* pass = [MGTwitterEngine password];
	
	NSString *boundary = [NSString stringWithFormat:@"------%ld__%ld__%ld", random(), random(), random()];
	
	NSURL *url = [NSURL URLWithString:@"http://yfrog.com/api/upload"];
	NSMutableURLRequest *req = tweeteroMutableURLRequest(url);
	[req setHTTPMethod:@"POST"];

	NSString *multipartContentType = [NSString stringWithFormat:@"multipart/form-data; boundary=%@", boundary];
	[req setValue:multipartContentType forHTTPHeaderField:@"Content-type"];
	
	//adding the body:
	NSMutableData *postBody = [NSMutableData data];
	[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n", boundary] dataUsingEncoding:NSUTF8StringEncoding]];
	
	[postBody appendData:[@"Content-Disposition: form-data; name=\"media\"; filename=\"iPhoneMedia\"\r\n" dataUsingEncoding:NSUTF8StringEncoding]];
	[postBody appendData:[[NSString stringWithFormat:@"Content-Type: %@\r\n", self.contentType] dataUsingEncoding:NSUTF8StringEncoding]];
//	[postBody appendData:[@"Content-Type: image/jpeg\r\n" dataUsingEncoding:NSUTF8StringEncoding]];
	[postBody appendData:[@"Content-Transfer-Encoding: binary\r\n\r\n" dataUsingEncoding:NSUTF8StringEncoding]];
	[postBody appendData:data];
	[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n", boundary] dataUsingEncoding:NSUTF8StringEncoding]];
	
	[postBody appendData:[@"Content-Disposition: form-data; name=\"username\"\r\n\r\n" dataUsingEncoding:NSUTF8StringEncoding]];
	[postBody appendData:[login dataUsingEncoding:NSUTF8StringEncoding]];
	[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n", boundary] dataUsingEncoding:NSUTF8StringEncoding]];
	
	[postBody appendData:[@"Content-Disposition: form-data; name=\"password\"\r\n\r\n" dataUsingEncoding:NSUTF8StringEncoding]];
	[postBody appendData:[pass dataUsingEncoding:NSUTF8StringEncoding]];
	
	if([[LocationManager locationManager] locationDefined])
	{
		[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n", boundary] dataUsingEncoding:NSUTF8StringEncoding]];
		
		[postBody appendData:[@"Content-Disposition: form-data; name=\"tags\"\r\n\r\n" dataUsingEncoding:NSUTF8StringEncoding]];
		[postBody appendData:[[NSString stringWithFormat:@"geotagged, geo:lat=%+.6f, geo:lon=%+.6f", [[LocationManager locationManager] latitude], [[LocationManager locationManager] longitude]] dataUsingEncoding:NSUTF8StringEncoding]];
	}

	[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@--\r\n",boundary] dataUsingEncoding:NSUTF8StringEncoding]];
	
	[req setHTTPBody:postBody];

	[self retain];
	self.connection = [[NSURLConnection alloc] initWithRequest:req 
												  delegate:self 
										  startImmediately:YES];
	if (!self.connection) 
	{
		[delegate uploadedImage:nil sender:self];
		[self release];
	}
	[TweetterAppDelegate increaseNetworkActivityIndicator];
}

- (void) postData:(NSData*)data contentType:(NSString*)mediaContentType
{
	self.contentType = mediaContentType;
	[self postData:data];
}


- (void)postJPEGData:(NSData*)imageJPEGData delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data
{
	self.delegate = dlgt;
	self.userData = data;
	
	if(!imageJPEGData)
	{
		[delegate uploadedImage:nil sender:self];
		return;
	}
		

	[self postData:imageJPEGData contentType:JPEG_CONTENT_TYPE];
}

- (void)postMP4Data:(NSData*)movieData delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data
{
	self.delegate = dlgt;
	self.userData = data;
	
	if(!movieData)
	{
		[delegate uploadedImage:nil sender:self];
		return;
	}
		

	[self postData:movieData contentType:MP4_CONTENT_TYPE];
}

- (void)convertImageThreadAndStartUpload:(UIImage*)image
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSData* imData = UIImageJPEGRepresentation(image, 1.0f);
	self.contentType = JPEG_CONTENT_TYPE;
	[self performSelectorOnMainThread:@selector(postData:) withObject:imData waitUntilDone:NO];

	[pool release];
}

- (void)postImage:(UIImage*)image delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data
{
	delegate = [dlgt retain];
	self.userData = data;

	UIImage* modifiedImage = nil;
	
	BOOL needToResize;
	BOOL needToRotate;
	int newDimension = isImageNeedToConvert(image, &needToResize, &needToRotate);
	if(needToResize || needToRotate)		
		modifiedImage = imageScaledToSize(image, newDimension);

	[NSThread detachNewThreadSelector:@selector(convertImageThreadAndStartUpload:) toTarget:self withObject:modifiedImage ? modifiedImage : image];
}

#pragma mark NSURLConnection delegate methods


- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    [result setLength:0];
}


- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
    [result appendData:data];
}


- (void) connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];
	[delegate uploadedImage:nil sender:self];
    [self release];
}


- (NSCachedURLResponse *) connection:(NSURLConnection *)connection 
                   willCacheResponse:(NSCachedURLResponse *)cachedResponse
{
     return cachedResponse;
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict
{
    if (qName) 
        elementName = qName;

    if ([elementName isEqualToString:@"mediaurl"])
		self.contentXMLProperty = [NSMutableString string];
	else
		self.contentXMLProperty = nil;
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{     
    if (qName)
        elementName = qName;
    
    if ([elementName isEqualToString:@"mediaurl"])
	{
        self.newURL = [self.contentXMLProperty stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
		[parser abortParsing];
	}
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string
{
    if (self.contentXMLProperty)
		[self.contentXMLProperty appendString:string];
}


- (void) connectionDidFinishLoading:(NSURLConnection *)connection
{
	[TweetterAppDelegate decreaseNetworkActivityIndicator];

	NSXMLParser *parser = [[NSXMLParser alloc] initWithData:result];
	[parser setDelegate:self];
	[parser setShouldProcessNamespaces:NO];
	[parser setShouldReportNamespacePrefixes:NO];
	[parser setShouldResolveExternalEntities:NO];
	
	[parser parse];
	[parser release];

	[result setLength:0];
	
	[delegate uploadedImage:self.newURL sender:self];
	[self release];
}

- (void)cancel
{
	canceled = YES;
	if(connection)
	{
		[connection cancel];
		[TweetterAppDelegate decreaseNetworkActivityIndicator];
		[self release];
	}
	[delegate uploadedImage:nil sender:self];
}

- (BOOL)canceled
{
	return canceled;
}


@end
