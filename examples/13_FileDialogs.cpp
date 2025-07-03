//------------------------------------------------------------------------------
// 13_FileDialogs.cpp
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
#include "aether.h"

int main()
{
  ae::Window window;
  ae::Input input;
  ae::GraphicsDevice device;
  ae::FileSystem fs;
  window.Initialize( 800, 600, false, true, true );
  window.SetTitle( "Press Cmd+S or Cmd+O" );
  input.Initialize( &window );
  device.Initialize( &window );
  fs.Initialize( "data", "ae", "file_dialogs" );
  while( !input.quit )
  {
    input.Pump();
    device.Activate();
    device.Clear( ae::Color::PicoBlue() );
    device.Present();

    if( !input.GetPrev( ae::Key::Num1 ) && input.Get( ae::Key::Num1 ) )
    {
      ae::Str256 path = "unknown";
      fs.GetRootDir( ae::FileSystem::Root::Data, &path );
      AE_INFO( "Show dir '#'", path );
      fs.ShowFolder( ae::FileSystem::Root::Data, "" );
    }

    if( !input.GetPrev( ae::Key::Num2 ) && input.Get( ae::Key::Num2 ) )
    {
      ae::Str256 path = "unknown";
      fs.GetRootDir( ae::FileSystem::Root::User, &path );
      AE_INFO( "Show dir '#'", path );
      fs.ShowFolder( ae::FileSystem::Root::User, "" );
    }

    if( !input.GetPrev( ae::Key::Num3 ) && input.Get( ae::Key::Num3 ) )
    {
      ae::Str256 path = "unknown";
      fs.GetRootDir( ae::FileSystem::Root::Cache, &path );
      AE_INFO( "Show dir '#'", path );
      fs.ShowFolder( ae::FileSystem::Root::Cache, "" );
    }

    if( input.Get( ae::Key::LeftMeta ) && !input.GetPrev( ae::Key::O ) && input.Get( ae::Key::O ) )
    {
      ae::FileDialogParams params;
      //params.filters.Append( ae::FileFilter( "All Files", "*" ) );
      params.filters.Append( ae::FileFilter( "Text Files", "txt" ) );
      params.filters.Append( ae::FileFilter( "JSON Files", "json" ) );
      params.window = &window;
      params.windowTitle = "Open Some File To Do Things With";
      params.allowMultiselect = true;
      ae::Array< std::string > result = fs.OpenDialog( params );
      if( result.Length() )
      {
        AE_INFO( "Open dialog success" );
        for( auto&& r : result )
        {
          AE_INFO( "file #", r );
        }
      }
      else
      {
        AE_ERR( "User cancelled open" );
      }
    }

    if( input.Get( ae::Key::LeftMeta ) && !input.GetPrev( ae::Key::S ) && input.Get( ae::Key::S ) )
    {
      ae::FileDialogParams params;
      params.window = &window;
      params.filters.Append( ae::FileFilter( "Text Files", "txt" ) );
      std::string result = fs.SaveDialog( params );
      if( !result.empty() )
      {
        AE_INFO( "Save dialog success #", result );
      }
      else
      {
        AE_ERR( "User cancelled save" );
      }
    }
  }
  return 0;
}
