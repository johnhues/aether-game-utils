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
#ifndef AEVFS_H
#define AEVFS_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aePlatform.h"
#include "aeString.h"

//------------------------------------------------------------------------------
// aeVfs class
//------------------------------------------------------------------------------
class aeVfs
{
public:
  enum Root { Data, User };

  void Initialize( const char* dataDir, const char* organizationName, const char* applicationName );

  uint32_t GetSize( Root root, const char* fileName );
  uint32_t Read( Root root, const char* fileName, void* buffer, uint32_t bufferSize );
  uint32_t Write( Root root, const char* fileName, const void* buffer, uint32_t bufferSize );
  void ShowFolder( Root root, const char* fileDir );
  const char* GetRootDir( Root root );

  static uint32_t GetSize( const char* fileDir );
  static uint32_t Read( const char* fileDir, void* buffer, uint32_t bufferSize );
  static uint32_t Write( const char* fileDir, const void* buffer, uint32_t bufferSize );
  static void ShowFolder( const char* fileDir );

private:
  aeStr256 m_dataDir;
  aeStr256 m_userDir;
};

#endif
