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

#import "TweetQueue.h"
#import "util.h"


@implementation TweetQueue

- (NSString*)libraryFolderName
{
	return [@"~/Library" stringByExpandingTildeInPath];
}


- (NSString*)tweetterQueueImagesFolderName:(BOOL*)folderExists
{
	NSString *libFolderPathMain = [[self libraryFolderName] stringByAppendingPathComponent:@"TweetterQueueImages"];
	NSString *libFolderPath = libFolderPathMain;
	NSFileManager* fm = [NSFileManager defaultManager];
	BOOL isDir = NO;
	*folderExists = NO;
	int i = 1;
	while(((*folderExists) = [fm fileExistsAtPath:libFolderPath isDirectory:&isDir]) && !isDir)
		libFolderPath = [NSString stringWithFormat:@"%@ %d", libFolderPathMain, i++];
	
	return libFolderPath;
}

- (NSString*) saveJPEGDataInLibrary:(NSData*)jpegData // return local path
{
	NSFileManager* fm = [NSFileManager defaultManager];
	BOOL folderExists = NO;
	NSString* libFolderPath = [self tweetterQueueImagesFolderName:&folderExists];
	if(!folderExists)
		[fm createDirectoryAtPath:libFolderPath attributes:nil];

	NSString *imagePathMain = [libFolderPath stringByAppendingPathComponent:@"TweetterImage"];
	NSString *imagePath = nil;
	do
	{
		imagePath = [[NSString stringWithFormat:@"%@_%ld", imagePathMain, random()] stringByAppendingPathExtension:@"jpg"];
	}
	while([fm fileExistsAtPath:imagePath]);
		
	BOOL success = [jpegData writeToFile:imagePath atomically:YES];	
	
	NSString* localPath = [imagePath stringByReplacingOccurrencesOfString:[self libraryFolderName] withString:@""];
	return success ? localPath : nil;
}

- (NSString*) saveImageInLibrary:(UIImage*)image
{
	return [self saveJPEGDataInLibrary:UIImageJPEGRepresentation(image, 1.0f)];
}






+ (TweetQueue*)sharedQueue
{
	static TweetQueue *queue;
	if(!queue) queue = [[TweetQueue alloc] init];
	
	return queue;
}

- (BOOL)addMessage:(NSString*)text withImage:(UIImage*)image withMovie:(NSURL*)movieURL inReplyTo:(int)inReplyTo
{
	return [self addMessage:text 
				withImageData:image ? UIImageJPEGRepresentation(image, 1.0f) : nil 
				withMovie:movieURL
				inReplyTo:inReplyTo];
}

- (BOOL)addMessage:(NSString*)text withImageData:(NSData*)imageData withMovie:(NSURL*)movieURL inReplyTo:(int)inReplyTo
{
	NSMutableDictionary *entry = [NSMutableDictionary dictionaryWithObject:text ? text : @"" forKey:@"text"];

	[entry setObject:[NSNumber numberWithInt:inReplyTo] forKey:@"inReplyTo"];
	
	if(imageData) 
	{
		NSString* localPath = [self saveJPEGDataInLibrary:imageData];
		if(!localPath)
			return NO;
			
		[entry setObject:localPath forKey:@"imagePath"];
	}
	
	if(movieURL)
		[entry setObject:[movieURL path] forKey:@"videoPath"];
	
	NSArray *array = [[NSUserDefaults standardUserDefaults] objectForKey:@"TweetQueue"];
	array = array? [array arrayByAddingObject:entry]: [NSArray arrayWithObject:entry];
	
	[[NSUserDefaults standardUserDefaults] setObject:array forKey:@"TweetQueue"];
	[[NSNotificationCenter defaultCenter] postNotificationName: @"QueueChanged" object: nil];
	return YES;
}

- (BOOL)replaceMessage:(NSString*)text withImage:(UIImage*)image withMovie:(NSURL*)movieURL inReplyTo:(int)inReplyTo atIndex:(int)index
{
	return [self replaceMessage:text 
				withImageData:image ? UIImageJPEGRepresentation(image, 1.0f) : nil
				withMovie:movieURL 
				inReplyTo:inReplyTo
				atIndex:index];
}

- (BOOL)replaceMessage:(NSString*)text withImageData:(NSData*)imageData withMovie:(NSURL*)movieURL inReplyTo:(int)inReplyTo atIndex:(int)index
{
	NSArray *prefArray = [[NSUserDefaults standardUserDefaults] objectForKey:@"TweetQueue"];
	if(!prefArray || index >= [prefArray count])
		return NO;
		
	
	NSMutableDictionary *entry = [NSMutableDictionary dictionaryWithObject:text ? text : @"" forKey:@"text"];

	[entry setObject:[NSNumber numberWithInt:inReplyTo] forKey:@"inReplyTo"];
	
	if(imageData) 
	{
		NSString* localPath = [self saveJPEGDataInLibrary:imageData];
		if(!localPath)
			return NO;
			
		[entry setObject:localPath forKey:@"imagePath"];
	}
	
	if(movieURL)
		[entry setObject:[movieURL path] forKey:@"videoPath"];
	
	NSMutableArray *array = [NSMutableArray arrayWithArray:prefArray];
	[array replaceObjectAtIndex:index withObject:entry];
	
	
	[[NSUserDefaults standardUserDefaults] setObject:array forKey:@"TweetQueue"];
	[[NSNotificationCenter defaultCenter] postNotificationName: @"QueueChanged" object: nil];
	return YES;
}

- (int)count
{
	NSArray *array = [[NSUserDefaults standardUserDefaults] objectForKey:@"TweetQueue"];
	return array? [array count]: 0;
}

- (BOOL)getMessage:(NSString**)text andImageData:(NSData**)imageData movieURL:(NSURL**)movieURL inReplyTo:(int*)inReplyTo atIndex:(int)index
{
	NSArray *array = [[NSUserDefaults standardUserDefaults] objectForKey:@"TweetQueue"];
	if(!array || index >= [array count])
	{
		if(text) *text = nil;
		if(imageData) *imageData = nil;
		return NO;
	}
	
	NSDictionary *entry = [array objectAtIndex:index];
	
	if(text) 
		*text = [entry objectForKey:@"text"];
		
	if(inReplyTo)
	{
		NSNumber *replyNSNumber = [entry objectForKey:@"inReplyTo"];
		*inReplyTo = replyNSNumber ? [replyNSNumber intValue] : 0;
	}
	
	if(imageData) 
	{
		NSString* imagePath = [entry objectForKey:@"imagePath"];
		if(imagePath)
			imagePath = [[self libraryFolderName] stringByAppendingPathComponent:imagePath];
		*imageData = imagePath ? [NSData dataWithContentsOfFile:imagePath] : nil;
	}
	
	if(movieURL)
	{
		NSString* videoPath = [entry objectForKey:@"videoPath"];
		*movieURL = videoPath ? [NSURL fileURLWithPath:videoPath] : nil;
	}
	
	return YES;
}

- (BOOL)deleteMessage:(int)index
{
	NSArray *array = [[NSUserDefaults standardUserDefaults] objectForKey:@"TweetQueue"];
	if(!array || index >= [array count])
		return NO;

	NSDictionary *entry = [array objectAtIndex:index];
	NSString* imageLocalPath = [entry objectForKey:@"imagePath"];

	if(imageLocalPath)
	{
		NSError *error;
		NSString* fullPath = [[self libraryFolderName] stringByAppendingPathComponent:imageLocalPath];

		if(![[NSFileManager defaultManager] removeItemAtPath:fullPath error:&error])
		{
			return NO;
		}
	}
			
	NSMutableArray *newArray = [NSMutableArray arrayWithArray:array];
	[newArray removeObjectAtIndex:index];
	[[NSUserDefaults standardUserDefaults] setObject:newArray forKey:@"TweetQueue"];
	[[NSNotificationCenter defaultCenter] postNotificationName: @"QueueChanged" object: nil];
	
	return YES;
}

- (BOOL)deleteAllMessages
{
	BOOL folderExists = NO;
	NSString* libFolderPath = [self tweetterQueueImagesFolderName:&folderExists];
	if(folderExists)
		if(![[NSFileManager defaultManager] removeItemAtPath:libFolderPath error:nil])
			return NO;
			
	[[NSUserDefaults standardUserDefaults] setObject:[NSArray array] forKey:@"TweetQueue"];
	[[NSNotificationCenter defaultCenter] postNotificationName: @"QueueChanged" object: nil];
	return YES;
}

@end
