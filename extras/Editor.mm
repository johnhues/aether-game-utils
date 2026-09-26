//------------------------------------------------------------------------------
// Editor.mm
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/Editor.h"
#if _AE_OSX_
#import <AppKit/AppKit.h>

namespace ae {

//------------------------------------------------------------------------------
// Editor app icon
//------------------------------------------------------------------------------
void _EditorSetAppIcon( const uint8_t* rgba, uint32_t width, uint32_t height )
{
	@autoreleasepool
	{
		NSImage* image = nil;
		if( rgba && width && height )
		{
			NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
				initWithBitmapDataPlanes:nullptr
				pixelsWide:width
				pixelsHigh:height
				bitsPerSample:8
				samplesPerPixel:4
				hasAlpha:YES
				isPlanar:NO
				colorSpaceName:NSDeviceRGBColorSpace
				bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
				bytesPerRow:( width * 4 )
				bitsPerPixel:32];
			memcpy( [rep bitmapData], rgba, width * height * 4 );
			image = [[NSImage alloc] initWithSize:NSMakeSize( width, height )];
			[image addRepresentation:rep];
			[rep release];
		}
		[[NSApplication sharedApplication] setApplicationIconImage:image];
		[image release];
	}
}

bool _EditorCopyAppIcon( uint8_t* rgbaOut, uint32_t width, uint32_t height )
{
	bool result = false;
	@autoreleasepool
	{
		NSImage* image = [[NSApplication sharedApplication] applicationIconImage];
		if( image && rgbaOut && width && height )
		{
			NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
				initWithBitmapDataPlanes:nullptr
				pixelsWide:width
				pixelsHigh:height
				bitsPerSample:8
				samplesPerPixel:4
				hasAlpha:YES
				isPlanar:NO
				colorSpaceName:NSDeviceRGBColorSpace
				bitmapFormat:0
				bytesPerRow:( width * 4 )
				bitsPerPixel:32];
			memset( [rep bitmapData], 0, width * height * 4 );
			NSGraphicsContext* context = [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
			if( context )
			{
				[NSGraphicsContext saveGraphicsState];
				[NSGraphicsContext setCurrentContext:context];
				[image drawInRect:NSMakeRect( 0, 0, width, height )];
				[NSGraphicsContext restoreGraphicsState];
				const uint8_t* src = [rep bitmapData];
				for( uint32_t i = 0; i < width * height; i++ )
				{
					const uint32_t alpha = src[ i * 4 + 3 ];
					for( uint32_t c = 0; c < 3; c++ )
					{
						const uint32_t v = alpha ? ( src[ i * 4 + c ] * 255 + alpha / 2 ) / alpha : 0;
						rgbaOut[ i * 4 + c ] = (uint8_t)( ( v > 255 ) ? 255 : v );
					}
					rgbaOut[ i * 4 + 3 ] = (uint8_t)alpha;
				}
				result = true;
			}
			[rep release];
		}
	}
	return result;
}

} // End ae namespace

#endif
