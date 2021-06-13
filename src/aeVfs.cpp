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

//------------------------------------------------------------------------------
// Objective-C helper function declarations for aeVfs.mm
//------------------------------------------------------------------------------
#if _AE_APPLE_
  bool aeVfs_AppleCreateFolder( const char* dir );
  void aeVfs_AppleOpenFolder( const char* filePath );
  aeStr256 aeVfs_AppleGetAbsolutePath( const char* filePath );
#endif

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
bool aeVfs_GetCacheDir( aeStr256* outDir );

#if _AE_LINUX_
bool aeVfs_GetCacheDir( aeStr256* outDir )
{
  // Something like /users/someone/.cache
  AE_WARN( "aeVfs::Cache directory not implemented yet on this platform" );
  return false;
}
#elif _AE_WINDOWS_
bool aeVfs_GetCacheDir( aeStr256* outDir )
{
  // Something like C:\Users\someone\AppData\Local\Company\Game
  bool result = false;
  PWSTR wpath = nullptr;
  // SHGetKnownFolderPath does not include trailing backslash
  HRESULT pathResult = SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, nullptr, &wpath );
  if ( pathResult == S_OK )
  {
    char path[ outDir->MaxLength() + 1 ];
    int32_t pathLen = wcstombs( path, wpath, outDir->MaxLength() );
    if ( pathLen > 0 )
    {
      path[ pathLen ] = 0;
      
      *outDir = path;
      result = true;
    }
  }
  CoTaskMemFree( wpath ); // Always free even on failure
  return result;
}
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
  m_SetUserSharedDir( organizationName );
  m_SetCacheSharedDir( organizationName );
}

void aeVfs::m_SetDataDir( const char* dataDir )
{
  m_dataDir = GetAbsolutePath( dataDir );

  // Append slash if not empty and is currently missing
  if ( m_dataDir.Length() && m_dataDir[ m_dataDir.Length() - 1 ] != AE_PATH_SEPARATOR )
  {
    m_dataDir.Append( aeStr16( 1, AE_PATH_SEPARATOR ) );
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

  if ( m_userDir[ m_userDir.Length() - 1 ] != AE_PATH_SEPARATOR )
  {
    m_userDir.Append( aeStr16( 1, AE_PATH_SEPARATOR ) );
  }

  SDL_free( sdlUserDir );
}

void aeVfs::m_SetCacheDir( const char* organizationName, const char* applicationName )
{
  const aeStr16 pathChar( 1, AE_PATH_SEPARATOR );
  m_cacheDir = "";
  
  if ( aeVfs_GetCacheDir( &m_cacheDir ) )
  {
    AE_ASSERT( m_cacheDir.Length() );
    m_cacheDir += pathChar;
    m_cacheDir += organizationName;
    m_cacheDir += pathChar;
    m_cacheDir += applicationName;
    m_cacheDir += pathChar;
    if ( !CreateFolder( m_cacheDir.c_str() ) )
    {
      m_cacheDir = "";
    }
  }
}

void aeVfs::m_SetUserSharedDir( const char* organizationName )
{
  m_userSharedDir = "";

  char* sdlUserDir = SDL_GetPrefPath( organizationName, "shared" );
  if ( !sdlUserDir )
  {
    return;
  }

  m_userSharedDir = sdlUserDir;
  AE_ASSERT( m_userSharedDir.Length() );

  if ( m_userSharedDir[ m_userSharedDir.Length() - 1 ] != AE_PATH_SEPARATOR )
  {
    m_userSharedDir.Append( aeStr16( 1, AE_PATH_SEPARATOR ) );
  }

  SDL_free( sdlUserDir );
}

void aeVfs::m_SetCacheSharedDir( const char* organizationName )
{
  const aeStr16 pathChar( 1, AE_PATH_SEPARATOR );
  m_cacheSharedDir = "";
  
  if ( aeVfs_GetCacheDir( &m_cacheSharedDir ) )
  {
    AE_ASSERT( m_cacheSharedDir.Length() );
    m_cacheSharedDir += pathChar;
    m_cacheSharedDir += organizationName;
    m_cacheSharedDir += pathChar;
    m_cacheSharedDir += "shared";
    m_cacheSharedDir += pathChar;
    if ( !CreateFolder( m_cacheSharedDir.c_str() ) )
    {
      m_cacheSharedDir = "";
    }
  }
}

uint32_t aeVfs::GetSize( aeVfsRoot root, const char* filePath ) const
{
  aeStr256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += filePath;
    return GetSize( fullName.c_str() );
  }
  return 0;
}

uint32_t aeVfs::Read( aeVfsRoot root, const char* filePath, void* buffer, uint32_t bufferSize ) const
{
  aeStr256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += filePath;
    return Read( fullName.c_str(), buffer, bufferSize );
  }
  return 0;
}

uint32_t aeVfs::Write( aeVfsRoot root, const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs ) const
{
  aeStr256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += filePath;
    return Write( fullName.c_str(), buffer, bufferSize, createIntermediateDirs );
  }
  return 0;
}

bool aeVfs::CreateFolder( aeVfsRoot root, const char* folderPath ) const
{
  aeStr256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += folderPath;
    return CreateFolder( fullName.c_str() );
  }
  return false;
}

void aeVfs::ShowFolder( aeVfsRoot root, const char* folderPath ) const
{
  aeStr256 fullName;
  if ( GetRootDir( root, &fullName ) )
  {
    fullName += folderPath;
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
    case aeVfsRoot::UserShared:
      if ( m_userSharedDir.Length() )
      {
        if ( outDir )
        {
          *outDir = m_userSharedDir;
        }
        return true;
      }
      break;
    case aeVfsRoot::CacheShared:
      if ( m_cacheSharedDir.Length() )
      {
        if ( outDir )
        {
          *outDir = m_cacheSharedDir;
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
uint32_t aeVfs::GetSize( const char* filePath )
{
#if _AE_APPLE_
  CFStringRef filePathIn = CFStringCreateWithCString( kCFAllocatorDefault, filePath, kCFStringEncodingUTF8 );
  CFURLRef appUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), filePathIn, nullptr, nullptr );
  CFStringRef bundlePath = nullptr;
  if ( appUrl )
  {
    bundlePath = CFURLCopyFileSystemPath( appUrl, kCFURLPOSIXPathStyle );
    filePath = CFStringGetCStringPtr( bundlePath, kCFStringEncodingUTF8 );
  }
#endif
  
  uint32_t fileSize = 0;
  if ( FILE* file = fopen( filePath, "rb" ) )
  {
    fseek( file, 0, SEEK_END );
    fileSize = (uint32_t)ftell( file );
    fclose( file );
  }
  
#if _AE_APPLE_
  if ( bundlePath ) { CFRelease( bundlePath ); }
  if ( appUrl ) { CFRelease( appUrl ); }
  CFRelease( filePathIn );
#endif
  
  return fileSize;
}

uint32_t aeVfs::Read( const char* filePath, void* buffer, uint32_t bufferSize )
{
#if _AE_APPLE_
  CFStringRef filePathIn = CFStringCreateWithCString( kCFAllocatorDefault, filePath, kCFStringEncodingUTF8 );
  CFURLRef appUrl = CFBundleCopyResourceURL( CFBundleGetMainBundle(), filePathIn, nullptr, nullptr );
  CFStringRef bundlePath = nullptr;
  if ( appUrl )
  {
    CFStringRef bundlePath = CFURLCopyFileSystemPath( appUrl, kCFURLPOSIXPathStyle );
    filePath = CFStringGetCStringPtr( bundlePath, kCFStringEncodingUTF8 );
  }
#endif

  uint32_t resultLen = 0;
  
  if ( FILE* file = fopen( filePath, "rb" ) )
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
  CFRelease( filePathIn );
#endif

  return resultLen;
}

uint32_t aeVfs::Write( const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs )
{
  if ( createIntermediateDirs )
  {
    auto dir = GetDirectoryFromPath( filePath );
    if ( dir.Length() && !aeVfs::CreateFolder( dir.c_str() ) )
    {
      return 0;
    }
  }
  
  FILE* file = fopen( filePath, "wb" );
  if ( !file )
  {
    return 0;
  }

  fwrite( buffer, sizeof(uint8_t), bufferSize, file );
  fclose( file );

  return bufferSize;
}

bool aeVfs::CreateFolder( const char* folderPath )
{
#if _AE_APPLE_
  return aeVfs_AppleCreateFolder( folderPath );
#elif _AE_WINDOWS_
  switch ( SHCreateDirectoryExA( nullptr, folderPath, nullptr ) )
  {
    case ERROR_SUCCESS:
    case ERROR_ALREADY_EXISTS:
      return true;
    default:
      return false;
  }
#endif
  // @TODO: Linux
  return false;
}

void aeVfs::ShowFolder( const char* folderPath )
{
#if _AE_WINDOWS_
  ShellExecuteA( NULL, "explore", folderPath, NULL, NULL, SW_SHOWDEFAULT );
#elif _AE_OSX_
  aeVfs_AppleOpenFolder( folderPath );
#endif
  // @TODO: Linux
}

aeStr256 aeVfs::GetAbsolutePath( const char* filePath )
{
#if _AE_APPLE_
  return aeVfs_AppleGetAbsolutePath( filePath );
#endif
  // @TODO: Windows and Linux
  return filePath;
}

const char* aeVfs::GetFileNameFromPath( const char* filePath )
{
  const char* s0 = strrchr( filePath, '/' );
  const char* s1 = strrchr( filePath, '\\' );
  
  if ( s1 && s0 )
  {
    return ( ( s1 > s0 ) ? s1 : s0 ) + 1;
  }
  else if ( s0 )
  {
    return s0 + 1;
  }
  else if ( s1 )
  {
    return s1 + 1;
  }
  else
  {
    return filePath;
  }
}

const char* aeVfs::GetFileExtFromPath( const char* filePath )
{
  const char* fileName = GetFileNameFromPath( filePath );
  const char* s = strchr( fileName, '.' );
  if ( s )
  {
    return s + 1;
  }
  else
  {
    // @NOTE: Return end of given string in case pointer arithmetic is being done by user
    uint32_t len = strlen( fileName );
    return fileName + len;
  }
}

aeStr256 aeVfs::GetDirectoryFromPath( const char* filePath )
{
  const char* fileName = GetFileNameFromPath( filePath );
  return aeStr256( fileName - filePath, filePath );
}

void aeVfs::AppendToPath( aeStr256* path, const char* str )
{
  if ( !path )
  {
    return;
  }
  
  // @TODO: Handle paths that already have a file name and extension
  
  // @TODO: Handle one or more path separators at end of path
  if ( (*path)[ path->Length() - 1 ] != AE_PATH_SEPARATOR )
  {
    path->Append( aeStr16( 1, AE_PATH_SEPARATOR ) );
  }
  
  // @TODO: Handle one or more path separators at front of str
  *path += str;
}
