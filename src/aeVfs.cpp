//------------------------------------------------------------------------------
// aeVfs.cpp
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
  #include <Shlobj_core.h>
#endif

#if _AE_WINDOWS_
#define AE_PATH_CHAR '\\'
#else
#define AE_PATH_CHAR '/'
#endif

//------------------------------------------------------------------------------
// Objective-C helper function declarations for aeVfs.mm
//------------------------------------------------------------------------------
#if _AE_APPLE_
  bool aeVfs_GetCacheDir( aeStr256* outDir );
  bool aeVfs_CreateDir( const char* dir );
#endif

//------------------------------------------------------------------------------
// Member functions
//------------------------------------------------------------------------------
void aeVfs::Initialize( const char* dataDir, const char* organizationName, const char* applicationName )
{
  AE_ASSERT_MSG( organizationName && organizationName[ 0 ], "Organization name must not be empty" );
  AE_ASSERT_MSG( applicationName && applicationName[ 0 ], "Application name must not be empty" );

  const char* validateOrgName = organizationName;
  while ( *validateOrgName )
  {
    AE_ASSERT_MSG( isalnum( *validateOrgName ) || ( *validateOrgName == '_' ), "Invalid aeVFS organization name '#'. Only alphanumeric characters and undersrcores are supported.", organizationName );
    validateOrgName++;
  }
  const char* validateAppName = applicationName;
  while ( *validateAppName )
  {
    AE_ASSERT_MSG( isalnum( *validateAppName ) || ( *validateAppName == '_' ), "Invalid aeVFS application name '#'. Only alphanumeric characters and undersrcores are supported.", applicationName );
    validateAppName++;
  }

  m_SetDataDir( dataDir ? dataDir : "" );
  m_SetUserDir( organizationName, applicationName );
  m_SetCacheDir( organizationName, applicationName );
}

void aeVfs::m_SetDataDir( const char* dataDir )
{
  m_dataDir = dataDir;

  // Append slash if not empty and is currently missing
  if ( m_dataDir.Length() && m_dataDir[ m_dataDir.Length() - 1 ] != AE_PATH_CHAR )
  {
    m_dataDir.Append( aeStr16( 1, AE_PATH_CHAR ) );
  }
}

void aeVfs::m_SetUserDir( const char* organizationName, const char* applicationName )
{
  m_userDir = "";

  char* sdlUserDir = SDL_GetPrefPath( organizationName, applicationName );
  if ( !sdlUserDir )
  {
    return;
  }

  m_userDir = sdlUserDir;
  AE_ASSERT( m_userDir.Length() );

  if ( m_userDir[ m_userDir.Length() - 1 ] != AE_PATH_CHAR )
  {
    m_userDir.Append( aeStr16( 1, AE_PATH_CHAR ) );
  }

  SDL_free( sdlUserDir );
}

void aeVfs::m_SetCacheDir( const char* organizationName, const char* applicationName )
{
  aeStr16 pathChar( 1, AE_PATH_CHAR );
  m_cacheDir = "";

#if _AE_APPLE_
  // Something like /User/someone/Library/Caches
  if ( aeVfs_GetCacheDir( &m_cacheDir ) )
  {
    m_cacheDir += pathChar;
    m_cacheDir += organizationName;
    m_cacheDir += pathChar;
    m_cacheDir += applicationName;
    m_cacheDir += pathChar;
    
    if ( !aeVfs_CreateDir( m_cacheDir.c_str() ) )
    {
      m_cacheDir = "";
    }
  }
#elif _AE_LINUX_
  // Something like /users/someone/.cache
  AE_WARN( "aeVfs::Cache directory not implemented yet on this platform" );
#elif _AE_WINDOWS_
  // Something like C:\Users\someone\AppData\Local\Company\Game
  PWSTR wpath = nullptr;
  HRESULT pathResult = SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, nullptr, &wpath );
  if ( pathResult == S_OK )
  {
    char path[ m_cacheDir.MaxLength() + 1 ];
    int32_t pathLen = wcstombs( path, wpath, m_cacheDir.MaxLength() );
    if ( pathLen > 0 )
    {
      path[ pathLen ] = 0;

      m_cacheDir = path;
      m_cacheDir += pathChar; // SHGetKnownFolderPath does not include trailing backslash
      m_cacheDir += organizationName;
      m_cacheDir += pathChar;
      m_cacheDir += applicationName;
      m_cacheDir += pathChar;

      int createResult = SHCreateDirectoryExA( nullptr, m_cacheDir.c_str(), nullptr );
      if ( createResult != ERROR_SUCCESS && createResult != ERROR_ALREADY_EXISTS )
      {
        m_cacheDir = "";
      }
    }
  }
  CoTaskMemFree( wpath ); // Always free even on failure
#endif
}

uint32_t aeVfs::GetSize( aeVfsRoot root, const char* fileName ) const
{
  aeStr256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += fileName;
    return GetSize( fullName.c_str() );
  }
  return 0;
}

uint32_t aeVfs::Read( aeVfsRoot root, const char* fileName, void* buffer, uint32_t bufferSize ) const
{
  aeStr256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += fileName;
    return Read( fullName.c_str(), buffer, bufferSize );
  }
  return 0;
}

uint32_t aeVfs::Write( aeVfsRoot root, const char* fileName, const void* buffer, uint32_t bufferSize ) const
{
  aeStr256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += fileName;
    return Write( fullName.c_str(), buffer, bufferSize );
  }
  return 0;
}

void aeVfs::ShowFolder( aeVfsRoot root, const char* fileDir ) const
{
  aeStr256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += fileDir;
    ShowFolder( fullName.c_str() );
  }
}

bool aeVfs::GetRootDir( aeVfsRoot root, aeStr256* outDir ) const
{
  switch ( root )
  {
    case aeVfsRoot::Data:
      if ( outDir )
      {
        *outDir = m_dataDir;
      }
      return true;
    case aeVfsRoot::User:
      if ( m_userDir.Length() )
      {
        if ( outDir )
        {
          *outDir = m_userDir;
        }
        return true;
      }
      break;
    case aeVfsRoot::Cache:
      if ( m_cacheDir.Length() )
      {
        if ( outDir )
        {
          *outDir = m_cacheDir;
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
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
