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

#include "util.h"
#import "MGTwitterEngine.h"

void LogStringArray(NSArray* ar, NSString* descriptionString)
{
#ifdef DEBUG
	NSLog(@"========================Start=====================");
	if(descriptionString)
		NSLog(descriptionString);
	NSEnumerator *enumerator = [ar objectEnumerator];
	id obj;
	while (obj = [enumerator nextObject]) 
		if([obj isKindOfClass:[NSString class]])
			NSLog((NSString*)obj);
		else
			NSLog(@"------------------Non-string item");
	NSLog(@"========================End=====================");
#endif
}

void LogDictionaryStringKeys(NSDictionary* dict, NSString* descriptionString)
{
#ifdef DEBUG
	LogStringArray([dict allKeys], descriptionString);
#endif
}

void LogStringSet(NSSet* set, NSString* descriptionString)
{
#ifdef DEBUG
	LogStringArray([set allObjects], descriptionString);
#endif
}

UIActionSheet * ShowActionSheet(NSString* title, id <UIActionSheetDelegate> delegate, 
									NSString *cancelButtonTitle, UIView* forView)
{
	UIActionSheet* progressSheet = [[UIActionSheet alloc] initWithTitle:title delegate:delegate cancelButtonTitle:cancelButtonTitle destructiveButtonTitle:nil otherButtonTitles:nil];
	progressSheet.actionSheetStyle = UIActionSheetStyleDefault;
	
	UIActivityIndicatorView* progressInd = [[UIActivityIndicatorView alloc] initWithFrame:CGRectMake(-18.5, -3.0, 40.0, 40.0)];
	progressInd.activityIndicatorViewStyle = UIActivityIndicatorViewStyleWhite;
	[progressInd sizeToFit];
	progressInd.autoresizingMask = (UIViewAutoresizingFlexibleLeftMargin |
									UIViewAutoresizingFlexibleRightMargin |
									UIViewAutoresizingFlexibleTopMargin |
									UIViewAutoresizingFlexibleBottomMargin);
	[progressInd startAnimating];
	[progressSheet addSubview:progressInd];
	[progressInd autorelease];
	
	[progressSheet showInView:forView];
	return [progressSheet autorelease];
}

// may cause a crash in non main thead
UIImage* imageScaledToSize(UIImage* image, int maxDimension)
{
	CGImageRef imgRef = image.CGImage;  
	
	CGFloat width = CGImageGetWidth(imgRef);  
	CGFloat height = CGImageGetHeight(imgRef);  
	
	CGAffineTransform transform = CGAffineTransformIdentity;
	CGRect bounds = CGRectMake(0, 0, width, height);  
	
	if(maxDimension > 0) //need scale
	{
		 if (width > maxDimension || height > maxDimension) 
		 {  
			 CGFloat ratio = width/height;  
			 if (ratio > 1)
			 {  
				 bounds.size.width = maxDimension;  
				 bounds.size.height = bounds.size.width / ratio;  
			 }  
			 else
			 {  
				 bounds.size.height = maxDimension;  
				 bounds.size.width = bounds.size.height * ratio;  
			 }  
		 }
	}
	CGFloat scaleRatio = bounds.size.width / width;
	CGSize imageSize = CGSizeMake(CGImageGetWidth(imgRef), CGImageGetHeight(imgRef));  
    CGFloat boundHeight;  
	
	UIImageOrientation orient = image.imageOrientation;  
    switch(orient) 
	{  
        case UIImageOrientationUp: //EXIF = 1  
            transform = CGAffineTransformIdentity;  
            break;  
			
        case UIImageOrientationUpMirrored: //EXIF = 2  
            transform = CGAffineTransformMakeTranslation(imageSize.width, 0.0);  
            transform = CGAffineTransformScale(transform, -1.0, 1.0);  
            break;  
			
        case UIImageOrientationDown: //EXIF = 3  
            transform = CGAffineTransformMakeTranslation(imageSize.width, imageSize.height);  
            transform = CGAffineTransformRotate(transform, M_PI);  
            break;  
			
        case UIImageOrientationDownMirrored: //EXIF = 4  
            transform = CGAffineTransformMakeTranslation(0.0, imageSize.height);  
            transform = CGAffineTransformScale(transform, 1.0, -1.0);  
            break;  
			
        case UIImageOrientationLeftMirrored: //EXIF = 5  
            boundHeight = bounds.size.height;  
            bounds.size.height = bounds.size.width;  
            bounds.size.width = boundHeight;  
            transform = CGAffineTransformMakeTranslation(imageSize.height, imageSize.width);  
            transform = CGAffineTransformScale(transform, -1.0, 1.0);  
            transform = CGAffineTransformRotate(transform, 3.0 * M_PI / 2.0);  
            break;  
			
        case UIImageOrientationLeft: //EXIF = 6  
            boundHeight = bounds.size.height;  
            bounds.size.height = bounds.size.width;  
            bounds.size.width = boundHeight;  
            transform = CGAffineTransformMakeTranslation(0.0, imageSize.width);  
            transform = CGAffineTransformRotate(transform, 3.0 * M_PI / 2.0);  
            break;  
			
        case UIImageOrientationRightMirrored: //EXIF = 7  
            boundHeight = bounds.size.height;  
            bounds.size.height = bounds.size.width;  
            bounds.size.width = boundHeight;  
            transform = CGAffineTransformMakeScale(-1.0, 1.0);  
            transform = CGAffineTransformRotate(transform, M_PI / 2.0);  
            break;  
			
        case UIImageOrientationRight: //EXIF = 8  
            boundHeight = bounds.size.height;  
            bounds.size.height = bounds.size.width;  
            bounds.size.width = boundHeight;  
            transform = CGAffineTransformMakeTranslation(imageSize.height, 0.0);  
            transform = CGAffineTransformRotate(transform, M_PI / 2.0);  
            break;  
			
        default:  
            [NSException raise:NSInternalInconsistencyException format:@"Invalid image orientation"];  
			
    }  
	
    UIGraphicsBeginImageContext(bounds.size);
	
    CGContextRef context = UIGraphicsGetCurrentContext();  
	
    if (orient == UIImageOrientationRight || orient == UIImageOrientationLeft)
	{
        CGContextScaleCTM(context, -scaleRatio, scaleRatio);
        CGContextTranslateCTM(context, -height, 0);  
    }
    else
	{  
        CGContextScaleCTM(context, scaleRatio, -scaleRatio);
        CGContextTranslateCTM(context, 0, -height);  
    }  
	
    CGContextConcatCTM(context, transform);  
	
    CGContextDrawImage(UIGraphicsGetCurrentContext(), CGRectMake(0, 0, width, height), imgRef);  
    UIImage *imageCopy = UIGraphicsGetImageFromCurrentImageContext();  
    UIGraphicsEndImageContext();  
	
    return imageCopy;
	
}

int isImageNeedToConvert(UIImage* testImage, BOOL *needToResize, BOOL *needToRotate)
{
	*needToResize = [[NSUserDefaults standardUserDefaults] boolForKey:@"ScalePhotosBeforeUploading"] &&
	(testImage.size.width > IMAGE_SCALING_SIZE || testImage.size.height > IMAGE_SCALING_SIZE);
	*needToRotate = testImage.imageOrientation != UIImageOrientationUp;
	
	
	if(*needToResize)
		return IMAGE_SCALING_SIZE;
	else 
		return -1;
}




NSString* ValidateYFrogLink(NSString *yfrogUrl)
{
	//if host is not yfrog.com it's not a link to yfrog
	NSURL *url = [NSURL URLWithString:yfrogUrl];
	if([[url host] rangeOfString:@"yfrog." options:NSCaseInsensitiveSearch].location == NSNotFound)
		return nil;
		
		
	NSString *path = [url path];
	if(!path || [path length] <= 1)
		return nil; //it's a path to the site start page
	
	//Image and mp4 video URLs don't end with x and y.
	if([path hasSuffix:@"x"] || [path hasSuffix:@"y"])
		return nil;
			
	//If it's a link to a thunbnail, truncate .th.jpg to get a link to an image page
	NSRange range = [path rangeOfString:@".th.jpg"];
	if(range.location != NSNotFound)
		path = [path substringToIndex:range.location];
	
	//If there is a '.' character, it's not a proper link to picture
	range = [path rangeOfString:@"."];
	if(range.location != NSNotFound)
		return nil;
	
	range = [path rangeOfString:@":"];
	if(range.location != NSNotFound)
		path = [path substringToIndex:range.location];
		
	url = [[[NSURL alloc] initWithScheme:[url scheme]  host:[url host] path:path] autorelease];
	return [url absoluteString];
}

BOOL isVideoLink(NSString *yfrogUrl)
{
	NSURL *url = [NSURL URLWithString:yfrogUrl];
	if([[url host] rangeOfString:@"yfrog." options:NSCaseInsensitiveSearch].location == NSNotFound)
		return NO;
		
		
	NSString *path = [url path];
	if(!path || [path length] <= 1)
		return NO; //it's a path to the site start page
	
	if([path hasSuffix:@"z"])
		return YES; //It's mp4 video
	
	return NO;
}

NSString* getLinkWithTag(NSString *tag)
{
    NSString *link = nil;
    NSRange range = [tag rangeOfString:@" href"];
    if (range.length > 0)
    {
        char buf[1024];
        
        unsigned len = [tag length];
        unsigned idx = range.location + range.length;
        while ([tag characterAtIndex:idx] != '=' && idx < len)
            idx++;
        
        unichar ch, endCh = ' ';
        do {
            idx++;
            ch = [tag characterAtIndex:idx];
            if (ch == '"' || ch == '\'')
                endCh = ch;
        } while (ch == ' ' || ch == '"' || ch == '\'' && idx < len);
        
        unsigned i = 0;
        while (((ch = [tag characterAtIndex:idx]) != endCh) && (idx < len))
        {
            buf[i++] = (char)ch;
            idx++;
        }
        buf[i] = 0;
        link = [NSString stringWithCString:buf length:i];
    }
    return link;
}

static struct
{
	NSString *codeForm, *literalForm;
} EntityTable[] = 
{
	{@"&#34;",	@"&quot;"},//	quotation mark
	{@"&#39;",	@"&apos;"},// (does not work in IE)	apostrophe 
	{@"&#38;",	@"&amp;"},//	ampersand
	{@"&#60;",	@"&lt;"},//	less-than
	{@"&#62;",	@"&gt;"},//	greater-than
	{@"&#160;",	@"&nbsp;"},//	non-breaking space
	{@"&#161;",	@"&iexcl;"},//	inverted exclamation mark
	{@"&#162;",	@"&cent;"},//	cent
	{@"&#163;",	@"&pound;"},//	pound
	{@"&#164;",	@"&curren;"},//	currency
	{@"&#165;",	@"&yen;"},//	yen
	{@"&#166;",	@"&brvbar;"},//	broken vertical bar
	{@"&#167;",	@"&sect;"},//	section
	{@"&#168;",	@"&uml;"},//	spacing diaeresis
	{@"&#169;",	@"&copy;"},//	copyright
	{@"&#170;",	@"&ordf;"},//	feminine ordinal indicator
	{@"&#171;",	@"&laquo;"},//	angle quotation mark (left)
	{@"&#172;",	@"&not;"},//	negation
	{@"&#173;",	@"&shy;"},//	soft hyphen
	{@"&#174;",	@"&reg;"},//	registered trademark
	{@"&#175;",	@"&macr;"},//	spacing macron
	{@"&#176;",	@"&deg;"},//	degree
	{@"&#177;",	@"&plusmn;"},//	plus-or-minus 
	{@"&#178;",	@"&sup2;"},//	superscript 2
	{@"&#179;",	@"&sup3;"},//	superscript 3
	{@"&#180;",	@"&acute;"},//	spacing acute
	{@"&#181;",	@"&micro;"},//	micro
	{@"&#182;",	@"&para;"},//	paragraph
	{@"&#183;",	@"&middot;"},//	middle dot
	{@"&#184;",	@"&cedil;"},//	spacing cedilla
	{@"&#185;",	@"&sup1;"},//	superscript 1
	{@"&#186;",	@"&ordm;"},//	masculine ordinal indicator
	{@"&#187;",	@"&raquo;"},//	angle quotation mark (right)
	{@"&#188;",	@"&frac14;"},//	fraction 1/4
	{@"&#189;",	@"&frac12;"},//	fraction 1/2
	{@"&#190;",	@"&frac34;"},//	fraction 3/4
	{@"&#191;",	@"&iquest;"},//	inverted question mark
	{@"&#215;",	@"&times;"},//	multiplication
	{@"&#247;",	@"&divide;"},//	division
	{@"&#192;",	@"&Agrave;"},//	capital a, grave accent
	{@"&#193;",	@"&Aacute;"},//	capital a, acute accent
	{@"&#194;",	@"&Acirc;"},//	capital a, circumflex accent
	{@"&#195;",	@"&Atilde;"},//	capital a, tilde
	{@"&#196;",	@"&Auml;"},//	capital a, umlaut mark
	{@"&#197;",	@"&Aring;"},//	capital a, ring
	{@"&#198;",	@"&AElig;"},//	capital ae
	{@"&#199;",	@"&Ccedil;"},//	capital c, cedilla
	{@"&#200;",	@"&Egrave;"},//	capital e, grave accent
	{@"&#201;",	@"&Eacute;"},//	capital e, acute accent
	{@"&#202;",	@"&Ecirc;"},//	capital e, circumflex accent
	{@"&#203;",	@"&Euml;"},//	capital e, umlaut mark
	{@"&#204;",	@"&Igrave;"},//	capital i, grave accent
	{@"&#205;",	@"&Iacute;"},//	capital i, acute accent
	{@"&#206;",	@"&Icirc;"},//	capital i, circumflex accent
	{@"&#207;",	@"&Iuml;"},//	capital i, umlaut mark
	{@"&#208;",	@"&ETH;"},//	capital eth, Icelandic
	{@"&#209;",	@"&Ntilde;"},//	capital n, tilde
	{@"&#210;",	@"&Ograve;"},//	capital o, grave accent
	{@"&#211;",	@"&Oacute;"},//	capital o, acute accent
	{@"&#212;",	@"&Ocirc;"},//	capital o, circumflex accent
	{@"&#213;",	@"&Otilde;"},//	capital o, tilde
	{@"&#214;",	@"&Ouml;"},//	capital o, umlaut mark
	{@"&#216;",	@"&Oslash;"},//	capital o, slash
	{@"&#217;",	@"&Ugrave;"},//	capital u, grave accent
	{@"&#218;",	@"&Uacute;"},//	capital u, acute accent
	{@"&#219;",	@"&Ucirc;"},//	capital u, circumflex accent
	{@"&#220;",	@"&Uuml;"},//	capital u, umlaut mark
	{@"&#221;",	@"&Yacute;"},//	capital y, acute accent
	{@"&#222;",	@"&THORN;"},//	capital THORN, Icelandic
	{@"&#223;",	@"&szlig;"},//	small sharp s, German
	{@"&#224;",	@"&agrave;"},//	small a, grave accent
	{@"&#225;",	@"&aacute;"},//	small a, acute accent
	{@"&#226;",	@"&acirc;"},//	small a, circumflex accent
	{@"&#227;",	@"&atilde;"},//	small a, tilde
	{@"&#228;",	@"&auml;"},//	small a, umlaut mark
	{@"&#229;",	@"&aring;"},//	small a, ring
	{@"&#230;",	@"&aelig;"},//	small ae
	{@"&#231;",	@"&ccedil;"},//	small c, cedilla
	{@"&#232;",	@"&egrave;"},//	small e, grave accent
	{@"&#233;",	@"&eacute;"},//	small e, acute accent
	{@"&#234;",	@"&ecirc;"},//	small e, circumflex accent
	{@"&#235;",	@"&euml;"},//	small e, umlaut mark
	{@"&#236;",	@"&igrave;"},//	small i, grave accent
	{@"&#237;",	@"&iacute;"},//	small i, acute accent
	{@"&#238;",	@"&icirc;"},//	small i, circumflex accent
	{@"&#239;",	@"&iuml;"},//	small i, umlaut mark
	{@"&#240;",	@"&eth;"},//	small eth, Icelandic
	{@"&#241;",	@"&ntilde;"},//	small n, tilde
	{@"&#242;",	@"&ograve;"},//	small o, grave accent
	{@"&#243;",	@"&oacute;"},//	small o, acute accent
	{@"&#244;",	@"&ocirc;"},//	small o, circumflex accent
	{@"&#245;",	@"&otilde;"},//	small o, tilde
	{@"&#246;",	@"&ouml;"},//	small o, umlaut mark
	{@"&#248;",	@"&oslash;"},//	small o, slash
	{@"&#249;",	@"&ugrave;"},//	small u, grave accent
	{@"&#250;",	@"&uacute;"},//	small u, acute accent
	{@"&#251;",	@"&ucirc;"},//	small u, circumflex accent
	{@"&#252;",	@"&uuml;"},//	small u, umlaut mark
	{@"&#253;",	@"&yacute;"},//	small y, acute accent
	{@"&#254;",	@"&thorn;"},//	small thorn, Icelandic	
	{@"&#255;",	@"&yuml;"},//	small y, umlaut mark
};

static NSString *decodedStringForEntity(int entityIndex)
{
	NSString *codeForm = EntityTable[entityIndex].codeForm;
	int code;
	sscanf([[codeForm substringFromIndex:2] UTF8String], "%d", &code);
	char codeStr[2];
	codeStr[0] = (char)code;
	codeStr[1] = 0;
	return [NSString stringWithCString:codeStr encoding:NSISOLatin1StringEncoding];
}

NSString *DecodeEntities(NSString *str)
{
	NSMutableString *bufstr = [[str mutableCopy] autorelease];
	
	for(int i = 0; i < sizeof(EntityTable)/sizeof(*EntityTable); ++i)
	{
		NSRange entRange = [bufstr rangeOfString:EntityTable[i].codeForm];
		if(entRange.location != NSNotFound)
		{
			[bufstr replaceCharactersInRange:entRange withString:decodedStringForEntity(i)];
		}
			 
		entRange = [bufstr rangeOfString:EntityTable[i].literalForm];
		if(entRange.location != NSNotFound)
		{
			[bufstr replaceCharactersInRange:entRange withString:decodedStringForEntity(i)];
		}
	}
	
	return bufstr;
}

NSURLRequest* tweeteroURLRequest(NSURL* url)
{
	NSMutableURLRequest *req = [NSMutableURLRequest requestWithURL:url];
	[req setValue:[MGTwitterEngine userAgent] forHTTPHeaderField:@"User-Agent"];
	return req;
}

NSMutableURLRequest* tweeteroMutableURLRequest(NSURL* url)
{
	NSMutableURLRequest *req = [NSMutableURLRequest requestWithURL:url];
	[req setValue:[MGTwitterEngine userAgent] forHTTPHeaderField:@"User-Agent"];
	return req;
}

