//------------------------------------------------------------------------------
// aeVfs.h
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
#ifndef AEVFS_H
#define AEVFS_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeString.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
enum class aeVfsRoot
{
  Data, // A given existing directory
  User, // A directory for storing preferences and savedata
  Cache, // A directory for storing expensive to generate data (computed, downloaded, etc)
  UserShared, // Same as above but shared accross the 'organization name'
  CacheShared // Same as above but shared accross the 'organization name'
};

#if _AE_WINDOWS_
  #define AE_PATH_SEPARATOR '\\'
#else
  #define AE_PATH_SEPARATOR '/'
#endif

//------------------------------------------------------------------------------
// aeVfs class
//------------------------------------------------------------------------------
class aeVfs
{
public:
  // Passing an empty string to dataDir is equivalent to using
  // the applications working directory. Organization name should be your name
  // or your companies name and should be consistent across apps. Application
  // name should be the name of this application. Initialize() creates missing
  // folders for aeVfsRoot::User and aeVfsRoot::Cache.
  void Initialize( const char* dataDir, const char* organizationName, const char* applicationName );

  // Member functions for use of aeVfsRoot directories
  bool GetRootDir( aeVfsRoot root, aeStr256* outDir ) const;
  uint32_t GetSize( aeVfsRoot root, const char* filePath ) const;
  uint32_t Read( aeVfsRoot root, const char* filePath, void* buffer, uint32_t bufferSize ) const;
  uint32_t Write( aeVfsRoot root, const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs ) const;
  bool CreateFolder( aeVfsRoot root, const char* folderPath ) const;
  void ShowFolder( aeVfsRoot root, const char* folderPath ) const;

  // Static member functions intended to be used when not creating a aeVfs instance
  static uint32_t GetSize( const char* filePath );
  static uint32_t Read( const char* filePath, void* buffer, uint32_t bufferSize );
  static uint32_t Write( const char* filePath, const void* buffer, uint32_t bufferSize, bool createIntermediateDirs );
  static bool CreateFolder( const char* folderPath );
  static void ShowFolder( const char* folderPath );
  
  // Static helpers
  static aeStr256 GetAbsolutePath( const char* filePath );
  static const char* GetFileNameFromPath( const char* filePath );
  static const char* GetFileExtFromPath( const char* filePath );
  static aeStr256 GetDirectoryFromPath( const char* filePath );
  static void AppendToPath( aeStr256* path, const char* str );

private:
  void m_SetDataDir( const char* dataDir );
  void m_SetUserDir( const char* organizationName, const char* applicationName );
  void m_SetCacheDir( const char* organizationName, const char* applicationName );
  void m_SetUserSharedDir( const char* organizationName );
  void m_SetCacheSharedDir( const char* organizationName );

  aeStr256 m_dataDir;
  aeStr256 m_userDir;
  aeStr256 m_cacheDir;
  aeStr256 m_userSharedDir;
  aeStr256 m_cacheSharedDir;
};

#endif
