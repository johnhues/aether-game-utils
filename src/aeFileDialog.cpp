//------------------------------------------------------------------------------
// aeFileDialog.cpp
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
// Headers
//------------------------------------------------------------------------------
#include <filesystem>
#include "aeFileDialog.h"
#include "aeWindow.h"
#include "SDL.h"
#include "SDL_syswm.h"
#if _AE_WINDOWS_
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <commdlg.h>
#endif

namespace AE_NAMESPACE {

// General Helpers
void FixPathExtension( const char* extension, std::filesystem::path* pathOut )
{
  // Set if path has no extension
  if ( !pathOut->has_extension() )
  {
    pathOut->replace_extension( extension );
    return;
  }

  // Set if extension chars are just periods
  std::string pathExt = pathOut->extension().string();
  if ( pathExt[ pathExt.length() - 1 ] == '.' )
  {
    pathOut->concat( std::string( "." ) + extension );
    return;
  }
}

//------------------------------------------------------------------------------
// FileFilter member functions
//------------------------------------------------------------------------------
FileFilter::FileFilter( const char* desc, const char** ext, uint32_t extensionCount )
{
  extensionCount = aeMath::Min( extensionCount, countof( extensions ) );
  description = desc;
  for ( uint32_t i = 0; i < extensionCount; i++ )
  {
    extensions[ i ] = ext[ i ];
  }
}

#if _AE_WINDOWS_

//------------------------------------------------------------------------------
// Windows helpers
//------------------------------------------------------------------------------
ae::Array< char > CreateFilterString( const Array< FileFilter >& filters )
{
  if ( !filters.Length() )
  {
    return {};
  }

  ae::Array< char > result;
  ae::Array< char > tempFilterStr;

  for ( uint32_t i = 0; i < filters.Length(); i++ )
  {
    const FileFilter& filter = filters[ i ];
    tempFilterStr.Clear();

    uint32_t extCount = 0;
    for ( uint32_t j = 0; j < countof( FileFilter::extensions ); j++ )
    {
      const char* ext = filter.extensions[ j ];
      if ( ext == nullptr )
      {
        continue;
      }

      // Validate extension
      if ( strcmp( "*", ext ) != 0 )
      {
        for ( const char* extCheck = ext; *extCheck; extCheck++ )
        {
          if ( !std::isalnum( *extCheck ) )
          {
            AE_FAIL_MSG( "File extensions must only contain alphanumeric characters: #", ext );
            return {};
          }
        }
      }

      if ( extCount == 0 )
      {
        tempFilterStr.Append( "*.", 2 );
      }
      else
      {
        tempFilterStr.Append( ";*.", 3 );
      }

      tempFilterStr.Append( ext, (uint32_t)strlen( ext ) );
      extCount++;
    }

    if ( extCount == 0 )
    {
      tempFilterStr.Append( "*.*", 3 );
    }

    // Description
    result.Append( filter.description, (uint32_t)strlen( filter.description ) );
    result.Append( " (", 2 );
    result.Append( &tempFilterStr[ 0 ], tempFilterStr.Length() );
    result.Append( ")", 2 ); // Every description must be null terminated

    result.Append( &tempFilterStr[ 0 ], tempFilterStr.Length() );
    result.Append( "", 1 ); // Every filter must be null terminated
  }

  // Append final null terminator because GetOpenFileName requires double termination at end of string
  result.Append( "", 1 );

  return result;
}

//------------------------------------------------------------------------------
// OpenFile function Windows
//------------------------------------------------------------------------------
Array< std::string > OpenFile( const OpenFileParams& params )
{
  SDL_SysWMinfo wmInfo;
  if ( params.window )
  {
    SDL_VERSION( &wmInfo.version );
    SDL_GetWindowWMInfo( (SDL_Window*)params.window->window, &wmInfo );
  }

  ae::Array< char > filterStr = CreateFilterString( params.filters );

  char fileNameBuf[ MAX_PATH ];
  fileNameBuf[ 0 ] = 0;

  // Set parameters for Windows function call
  OPENFILENAMEA winParams;
  ZeroMemory( &winParams, sizeof( winParams ) );
  winParams.lStructSize = sizeof( winParams );
  winParams.hwndOwner = params.window ? wmInfo.info.win.window : nullptr;
  if ( params.windowTitle && params.windowTitle[ 0 ] )
  {
    winParams.lpstrTitle = params.windowTitle;
  }
  winParams.lpstrFile = fileNameBuf;
  winParams.nMaxFile = sizeof( fileNameBuf );
  winParams.lpstrFilter = filterStr.Length() ? &filterStr[ 0 ] : "All Files (*.*)\0*.*\0";
  winParams.nFilterIndex = 1;
  winParams.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  if ( params.useLegacy )
  {
    winParams.FlagsEx = OFN_EX_NOPLACESBAR;
  }

  // Open window
  if ( GetOpenFileNameA( &winParams ) )
  {
    return Array< std::string >( 1, winParams.lpstrFile );
  }

  return {};
}

//------------------------------------------------------------------------------
// SaveFile function Windows
//------------------------------------------------------------------------------
std::string SaveFile( const SaveFileParams& params )
{
  SDL_SysWMinfo wmInfo;
  if ( params.window )
  {
    SDL_VERSION( &wmInfo.version );
    SDL_GetWindowWMInfo( (SDL_Window*)params.window->window, &wmInfo );
  }

  ae::Array< char > filterStr = CreateFilterString( params.filters );

  char fileNameBuf[ MAX_PATH ];
  fileNameBuf[ 0 ] = 0;

  // Set parameters for Windows function call
  OPENFILENAMEA winParams;
  ZeroMemory( &winParams, sizeof( winParams ) );
  winParams.lStructSize = sizeof( winParams );
  winParams.hwndOwner = params.window ? wmInfo.info.win.window : nullptr;
  if ( params.windowTitle && params.windowTitle[ 0 ] )
  {
    winParams.lpstrTitle = params.windowTitle;
  }
  winParams.lpstrFile = fileNameBuf;
  winParams.nMaxFile = sizeof( fileNameBuf );
  winParams.lpstrFilter = filterStr.Length() ? &filterStr[ 0 ] : "All Files (*.*)\0*.*\0";
  winParams.nFilterIndex = 1;
  winParams.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  if ( params.useLegacy )
  {
    winParams.FlagsEx = OFN_EX_NOPLACESBAR;
  }

  if ( GetSaveFileNameA( &winParams ) )
  {
    std::filesystem::path result = winParams.lpstrFile;
    if ( winParams.nFilterIndex >= 1 )
    {
      winParams.nFilterIndex--;
      const char* ext = params.filters[ winParams.nFilterIndex ].extensions[ 0 ];
      
      FixPathExtension( ext, &result );
    }
    return result.string();
  }

  return "";
}

#else

//------------------------------------------------------------------------------
// OpenFile not implemented
//------------------------------------------------------------------------------
Array< std::string > OpenFile( const OpenFileParams& params )
{
  return {};
}

//------------------------------------------------------------------------------
// SaveFile not implemented
//------------------------------------------------------------------------------
std::string SaveFile( const SaveFileParams& params )
{
  return "";
}

#endif

} // ae namespace end
