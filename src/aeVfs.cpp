//------------------------------------------------------------------------------
// aeString.h
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
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
// Header files
//------------------------------------------------------------------------------
#include "aeVfs.h"
#include "SDL.h"
#if _AE_APPLE_
  #include <CoreFoundation/CoreFoundation.h>
#endif
#if _AE_WINDOWS_
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <shellapi.h>
#endif

//------------------------------------------------------------------------------
// Member functions
//------------------------------------------------------------------------------
void aeVfs::Initialize( const char* dataDir, const char* organizationName, const char* applicationName )
{
  AE_ASSERT_MSG( organizationName[ 0 ], "Organization name must not be empty" );
  AE_ASSERT_MSG( applicationName[ 0 ], "Application name must not be empty" );

  const char* validateOrgName = organizationName;
  while ( *validateOrgName )
  {
    AE_ASSERT_MSG( isalnum( *validateOrgName ) || ( *validateOrgName == '_' ), "Invalid VFS organization name '#'. Only alphanumeric characters and undersrcores are supported.", organizationName );
    validateOrgName++;
  }
  const char* validateAppName = applicationName;
  while ( *validateAppName )
  {
    AE_ASSERT_MSG( isalnum( *validateAppName ) || ( *validateAppName == '_' ), "Invalid VFS application name '#'. Only alphanumeric characters and undersrcores are supported.", applicationName );
    validateAppName++;
  }

  char* sdlUserDir = SDL_GetPrefPath( organizationName, applicationName );

  m_dataDir = dataDir;
  m_userDir = sdlUserDir;

#if _AE_WINDOWS_
  #define AE_PATH_CHAR '\\'
#else
  #define AE_PATH_CHAR '/'
#endif

  // Allow data dir prefix to be empty
  if ( m_dataDir.Length() && m_dataDir[ m_dataDir.Length() - 1 ] != AE_PATH_CHAR )
  {
    m_dataDir.Append( aeStr16( 1, AE_PATH_CHAR ) );
  }
  AE_ASSERT( m_userDir.Length() );
  if ( m_userDir[ m_userDir.Length() - 1 ] != AE_PATH_CHAR )
  {
    m_userDir.Append( aeStr16( 1, AE_PATH_CHAR ) );
  }

  SDL_free( sdlUserDir );
}

uint32_t aeVfs::GetSize( Root root, const char* fileName )
{
  aeStr256 fullName = GetRootDir( root );
  fullName += fileName;
  return GetSize( fullName.c_str() );
}

uint32_t aeVfs::Read( Root root, const char* fileName, void* buffer, uint32_t bufferSize )
{
  aeStr256 fullName = GetRootDir( root );
  fullName += fileName;
  return Read( fullName.c_str(), buffer, bufferSize );
}

uint32_t aeVfs::Write( Root root, const char* fileName, const void* buffer, uint32_t bufferSize )
{
  aeStr256 fullName = GetRootDir( root );
  fullName += fileName;
  return Write( fullName.c_str(), buffer, bufferSize );
}

void aeVfs::ShowFolder( Root root, const char* fileDir )
{
  aeStr256 fullName = GetRootDir( root );
  fullName += fileDir;
  return ShowFolder( fullName.c_str() );
}

const char* aeVfs::GetRootDir( Root root )
{
  if ( root == aeVfs::Data )
  {
    return m_dataDir.c_str();
  }
  if ( root == aeVfs::User )
  {
    return m_userDir.c_str();
  }
  return nullptr;
}

//------------------------------------------------------------------------------
// Static member functions
//------------------------------------------------------------------------------
uint32_t aeVfs::GetSize( const char* fileDir )
{
#if _AE_APPLE_
  CFStringRef fileDirIn = CFStringCreateWithCString( kCFAllocatorDefault, fileDir, kCFStringEncodingUTF8 );
  CFURLRef appUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), fileDirIn, nullptr, nullptr );
  CFStringRef bundlePath = nullptr;
  if ( appUrl )
  {
    bundlePath = CFURLCopyFileSystemPath( appUrl, kCFURLPOSIXPathStyle );
    fileDir = CFStringGetCStringPtr( bundlePath, kCFStringEncodingUTF8 );
  }
#endif
  
  uint32_t fileSize = 0;
  if ( FILE* file = fopen( fileDir, "rb" ) )
  {
    fseek( file, 0, SEEK_END );
    fileSize = (uint32_t)ftell( file );
    fclose( file );
  }
  
#if _AE_APPLE_
  if ( bundlePath ) { CFRelease( bundlePath ); }
  if ( appUrl ) { CFRelease( appUrl ); }
  CFRelease( fileDirIn );
#endif
  
  return fileSize;
}

uint32_t aeVfs::Read( const char* fileDir, void* buffer, uint32_t bufferSize )
{
#if _AE_APPLE_
  CFStringRef fileDirIn = CFStringCreateWithCString( kCFAllocatorDefault, fileDir, kCFStringEncodingUTF8 );
  CFURLRef appUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), fileDirIn, nullptr, nullptr );
  CFStringRef bundlePath = nullptr;
  if ( appUrl )
  {
    CFStringRef bundlePath = CFURLCopyFileSystemPath( appUrl, kCFURLPOSIXPathStyle );
    fileDir = CFStringGetCStringPtr( bundlePath, kCFStringEncodingUTF8 );
  }
#endif

  uint32_t resultLen = 0;
  
  if ( FILE* file = fopen( fileDir, "rb" ) )
  {
    fseek( file, 0, SEEK_END );
    resultLen = (uint32_t)ftell( file );
    fseek( file, 0, SEEK_SET );

    if ( resultLen <= bufferSize )
    {
      size_t readLen = fread( buffer, sizeof(uint8_t), resultLen, file );
      AE_ASSERT( readLen == resultLen );
    }
    else
    {
      resultLen = 0;
    }

    fclose( file );
  }
  
#if _AE_APPLE_
  if ( bundlePath ) { CFRelease( bundlePath ); }
  if ( appUrl ) { CFRelease( appUrl ); }
  CFRelease( fileDirIn );
#endif

  return resultLen;
}

uint32_t aeVfs::Write( const char* fileDir, const void* buffer, uint32_t bufferSize )
{
  FILE* file = fopen( fileDir, "wb" );
  if ( !file )
  {
    file = fopen( fileDir, "wb" );
    if ( !file )
    {
      return 0;
    }
  }

  fwrite( buffer, sizeof(uint8_t), bufferSize, file );
  fclose( file );

  return bufferSize;
}

void aeVfs::ShowFolder( const char* fileDir )
{
  // @TODO: OSX and Linux
#if _AE_WINDOWS_
  ShellExecuteA( NULL, "explore", fileDir, NULL, NULL, SW_SHOWDEFAULT );
#endif
}
