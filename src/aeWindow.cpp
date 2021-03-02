//------------------------------------------------------------------------------
// aeWindow.cpp
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
#include "aeWindow.h"
#include "aeLog.h"
#include "aeString.h"
#include "SDL.h"

//------------------------------------------------------------------------------
// aeWindow member functions
//------------------------------------------------------------------------------
aeWindow::aeWindow()
{
  window = nullptr;
  m_pos = aeInt2( 0 );
  m_width = 0;
  m_height = 0;
  m_dpiScale = 1.0f; // Don't bother getting actual value until opengl context exists because SDL2 needs it
  m_fullScreen = false;
}

void aeWindow::Initialize( uint32_t width, uint32_t height, bool fullScreen, bool showCursor )
{
  AE_ASSERT( !window );

  m_pos = aeInt2( fullScreen ? 0 : (int)SDL_WINDOWPOS_CENTERED );
  m_width = width;
  m_height = height;
  m_fullScreen = fullScreen;

  m_Initialize();

  SDL_ShowCursor( showCursor ? SDL_ENABLE : SDL_DISABLE );
  SDL_GetWindowPosition( (SDL_Window*)window, &m_pos.x, &m_pos.y );
}

void aeWindow::Initialize( aeInt2 pos, uint32_t width, uint32_t height, bool showCursor )
{
  AE_ASSERT( !window );

  m_pos = pos;
  m_width = width;
  m_height = height;
  m_fullScreen = false;

  m_Initialize();
  
  SDL_ShowCursor( showCursor ? SDL_ENABLE : SDL_DISABLE );
}

void aeWindow::m_Initialize()
{
  if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER ) < 0 )
  {
    AE_FAIL_MSG( "SDL could not initialize: #", SDL_GetError() );
  }

#if _AE_IOS_
  m_pos = aeInt2( 0 );
  m_fullScreen = true;
  
  SDL_DisplayMode displayMode;
  if ( SDL_GetDesktopDisplayMode( 0, &displayMode ) == 0 )
  {
    m_width = displayMode.w;
    m_height = displayMode.h;
  }

  window = SDL_CreateWindow( "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_width, m_height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI );
#else
  uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
  flags |= m_fullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE;
  window = SDL_CreateWindow( "", m_pos.x, m_pos.y, m_width, m_height, flags );
#endif
  AE_ASSERT( window );
}

void aeWindow::Terminate()
{
  SDL_DestroyWindow( (SDL_Window*)window );
}

void aeWindow::SetTitle( const char* title )
{
  if ( window )
  {
    SDL_SetWindowTitle( (SDL_Window*)window, title );
  }
}

void aeWindow::SetFullScreen( bool fullScreen )
{
  if ( window )
  {
    uint32_t oldFlags = SDL_GetWindowFlags( (SDL_Window*)window );
    
    uint32_t newFlags = oldFlags;
    if ( fullScreen )
    {
      newFlags |= SDL_WINDOW_FULLSCREEN;
    }
    else
    {
      newFlags &= ~SDL_WINDOW_FULLSCREEN;
    }

    if ( newFlags != oldFlags )
    {
      SDL_SetWindowFullscreen( (SDL_Window*)window, newFlags );
    }

    m_fullScreen = fullScreen;
  }
}

void aeWindow::SetPosition( aeInt2 pos )
{
  if ( window )
  {
    SDL_SetWindowPosition( (SDL_Window*)window, pos.x, pos.y );
    m_pos = pos;
  }
}

void aeWindow::SetSize( uint32_t width, uint32_t height )
{
  if ( window )
  {
    SDL_SetWindowSize( (SDL_Window*)window, width, height );
    m_width = width;
    m_height = height;
    m_UpdateDpiScale();
  }
}

void aeWindow::m_UpdateDpiScale()
{
  int a = 0;
  int b = 0;
  SDL_GetWindowSize( (SDL_Window*)window, &a, nullptr );
  SDL_GL_GetDrawableSize( (SDL_Window*)window, &b, nullptr );
  
  if ( a * b )
  {
    m_dpiScale = b / (float)a;
  }
  else
  {
    m_dpiScale = 1.0f;
  }
}

// This function is required so aeInput etc can set width and height when it detects changes
void aeWindow::m_UpdateWidthHeight( int32_t width, int32_t height )
{
  m_width = width;
  m_height = height;
  m_UpdateDpiScale();
}
