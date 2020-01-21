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
#if _AE_WINDOWS_
#include "atlstr.h"
#include "Combaseapi.h"
#endif

void aeVfs::Initialize( const char* dataDir, const char* tempDir, const char* userDir )
{
  m_dataDir = dataDir;
  m_tempDir = tempDir;
  m_userDir = userDir;
  if ( m_dataDir[ m_dataDir.Length() - 1 ] != '/' )
  {
    m_dataDir.Append( "/" );
  }
  if ( m_tempDir[ m_tempDir.Length() - 1 ] != '/' )
  {
    m_tempDir.Append( "/" );
  }
  if ( m_userDir[ m_userDir.Length() - 1 ] != '/' )
  {
    m_userDir.Append( "/" );
  }

#if _AE_WINDOWS_
  CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE ); // @TODO: CoUninitialize();
#endif
}

uint32_t aeVfs::GetSize( Root root, const char* fileName )
{
  char fullName[ 256 ];
  AE_ASSERT( sizeof(m_dataDir) + sizeof(fileName) < sizeof(fullName) );
  strcpy( fullName, GetRootDir( root ) );
  strcat( fullName, fileName );

  return GetSize( fullName );
}

uint32_t aeVfs::Read( Root root, const char* fileName, void* buffer, uint32_t bufferSize )
{
  char fullName[ 256 ];
  AE_ASSERT( sizeof(m_dataDir) + sizeof(fileName) < sizeof(fullName) );
  strcpy( fullName, GetRootDir( root ) );
  strcat( fullName, fileName );

  return Read( fullName, buffer, bufferSize );
}

uint32_t aeVfs::Write( Root root, const char* fileName, const void* buffer, uint32_t bufferSize )
{
  char fullName[ 256 ];
  AE_ASSERT( sizeof(m_dataDir) + sizeof(fileName) < sizeof(fullName) );
  strcpy( fullName, GetRootDir( root ) );
  strcat( fullName, fileName );

  return Write( fullName, buffer, bufferSize );
}

uint32_t aeVfs::GetSize( const char* fileDir )
{
  FILE* file = fopen( fileDir, "rb" );
  if ( !file )
  {
    return 0;
  }

  fseek( file, 0, SEEK_END );
  uint32_t fileSize = (uint32_t)ftell( file );
  fclose( file );

  return fileSize;
}

uint32_t aeVfs::Read( const char* fileDir, void* buffer, uint32_t bufferSize )
{
  FILE* file = fopen( fileDir, "rb" );
  if ( !file )
  {
    return 0;
  }

  fseek( file, 0, SEEK_END );
  uint32_t fileSize = (uint32_t)ftell( file );
  fseek( file, 0, SEEK_SET );

  if ( fileSize > bufferSize )
  {
    fclose( file );
    return 0;
  }

  size_t readLen = fread( buffer, sizeof(uint8_t), fileSize, file );
  AE_ASSERT( readLen == fileSize );
  fclose( file );

  return fileSize;
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

const char* aeVfs::GetRootDir( Root root )
{
  if ( root == aeVfs::Data ) { return m_dataDir.c_str(); }
  if ( root == aeVfs::Temp ) { return m_tempDir.c_str(); }
  if ( root == aeVfs::User ) { return m_userDir.c_str(); }
  return nullptr;
}