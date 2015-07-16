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

#import "MGTwitterEngine.h"
#import "MGConnectionWrap.h"

@implementation MGConnectionWrap

@synthesize mgTwitterConnectionID;

- (id)initWithTwitter:(MGTwitterEngine *)twitter 
				connection:(NSString *)connectionID delegate:(id <MGConnectionDelegate>)delegate
{
	self = [super init];
	if(self)
	{
		_twitter = [twitter retain];
		mgTwitterConnectionID = [connectionID retain];
		_delegate = [delegate retain];
		canceled = NO;
	}
	return self;
}

- (void)dealloc 
{
	[_twitter closeAllConnections];
	[_twitter removeDelegate];
	[_twitter release];

	[mgTwitterConnectionID release];
	[_delegate release];
    [super dealloc];
}


- (void)cancel
{
	canceled = YES;
	[_twitter closeConnection:mgTwitterConnectionID];
	[_delegate MGConnectionCanceled:mgTwitterConnectionID];
}

- (BOOL)canceled
{
	return canceled;
}


@end
