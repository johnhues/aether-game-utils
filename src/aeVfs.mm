//------------------------------------------------------------------------------
// aeVfs.mm
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import "aeVfs.h"

//------------------------------------------------------------------------------
// aeVfs functions
//------------------------------------------------------------------------------
bool aeVfs_AppleGetCacheDir( aeStr256* outDir )
{
  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
  NSString* cachesPath = [paths lastObject];
  if ( [cachesPath length] )
  {
    *outDir = [cachesPath UTF8String];
    return true;
  }
  return false;
}

bool aeVfs_AppleCreateFolder( const char* dir )
{
  NSString* path = [NSString stringWithUTF8String:dir];
  NSError* error = nil;
  BOOL success = [[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:&error];
  return success && !error;
}

void aeVfs_AppleOpenFolder( const char* _path )
{
  NSString* path = [NSString stringWithUTF8String:_path];
  [[NSWorkspace sharedWorkspace] selectFile:path inFileViewerRootedAtPath:@""];
}

aeStr256 aeVfs_AppleGetAbsolutePath( const char* _path )
{
  NSString* path = [NSString stringWithUTF8String:_path];
  NSString* currentPath = [[NSFileManager defaultManager] currentDirectoryPath];
  AE_ASSERT( [currentPath characterAtIndex:0] != '~' );
  NSURL* currentPathUrl = [NSURL fileURLWithPath:currentPath];
  NSURL* absoluteUrl = [NSURL URLWithString:path relativeToURL:currentPathUrl];
  return [absoluteUrl.path UTF8String];
}
