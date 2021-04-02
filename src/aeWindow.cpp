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
  m_fullScreen = false;
  m_maximized = false;
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

  window = SDL_CreateWindow( "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_width, m_height, SDL_WINDOW_SHOWN );
#else
  aeRect windowRect( m_pos.x, m_pos.y, m_width, m_height );
  bool overlapsAny = false;
  uint32_t displayCount = SDL_GetNumVideoDisplays();
  for ( uint32_t i = 0; i < displayCount; i++ )
  {
    SDL_Rect rect;
    int result = SDL_GetDisplayBounds( i, &rect );
    if ( result == 0 )
    {
      aeRect screenRect( rect.x, rect.y, rect.w, rect.h );
      aeRect intersection;
      if ( windowRect.GetIntersection( screenRect, &intersection ) )
      {
        // Check how much window overlaps. This prevent windows that are barely overlapping from appearing offscreen.
        float intersectionArea = intersection.w * intersection.h;
        float screenArea = screenRect.w * screenRect.h;
        float windowArea = windowRect.w * windowRect.h;
        float screenOverlap = intersectionArea / screenArea;
        float windowOverlap = intersectionArea / windowArea;
        if ( screenOverlap > 0.1f || windowOverlap > 0.1f )
        {
          overlapsAny = true;
          break;
        }
      }
    }
  }

  if ( !overlapsAny && displayCount )
  {
    SDL_Rect screenRect;
    if ( SDL_GetDisplayBounds( 0, &screenRect ) == 0 )
    {
      int32_t border = screenRect.w / 16;

      m_width = screenRect.w - border * 2;
      int32_t h0 = screenRect.h - border * 2;
      int32_t h1 = m_width * ( 10.0f / 16.0f );
      m_height = aeMath::Min( h0, h1 );

      m_pos.x = border;
      m_pos.y = ( screenRect.h - m_height ) / 2;
      m_pos.x += screenRect.x;
      m_pos.y += screenRect.y;

      m_fullScreen = false;
    }
  }

  uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
  flags |= m_fullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE;
  window = SDL_CreateWindow( "", m_pos.x, m_pos.y, m_width, m_height, flags );
#endif
  AE_ASSERT( window );
  
  SDL_SetWindowTitle( (SDL_Window*)window, "" );
  m_windowTitle = "";
}

void aeWindow::Terminate()
{
  SDL_DestroyWindow( (SDL_Window*)window );
}

void aeWindow::SetTitle( const char* title )
{
  if ( window && m_windowTitle != title )
  {
    SDL_SetWindowTitle( (SDL_Window*)window, title );
    m_windowTitle = title;
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
  }
}

void aeWindow::SetMaximized( bool maximized )
{
  if ( maximized )
  {
    SDL_MaximizeWindow( (SDL_Window*)window );
  }
  else
  {
    SDL_RestoreWindow( (SDL_Window*)window );
  }
  m_maximized = maximized;
}
