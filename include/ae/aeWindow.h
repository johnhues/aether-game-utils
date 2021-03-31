//------------------------------------------------------------------------------
// aeWindow.h
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
#ifndef AEWINDOW_H
#define AEWINDOW_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeMath.h"
#include "aeString.h"

//------------------------------------------------------------------------------
// Window class
//------------------------------------------------------------------------------
class aeWindow
{
public:
  aeWindow();
  // @TODO: Init should take window title, since SetTitle is practically always called after Init()
  void Initialize( uint32_t width, uint32_t height, bool fullScreen, bool showCursor );
  void Initialize( aeInt2 pos, uint32_t width, uint32_t height, bool showCursor );
  void Terminate();

  void SetTitle( const char* title );
  void SetFullScreen( bool fullScreen );
  void SetPosition( aeInt2 pos );
  void SetSize( uint32_t width, uint32_t height );
  void SetMaximized( bool maximized );

  const char* GetTitle() const { return m_windowTitle.c_str(); }
  aeInt2 GetPosition() const { return m_pos; }
  int32_t GetWidth() const { return m_width; }
  int32_t GetHeight() const { return m_height; }
  bool GetFullScreen() const { return m_fullScreen; }
  bool GetMaximized() const { return m_maximized; }

private:
  void m_Initialize();
    
  aeInt2 m_pos;
  int32_t m_width;
  int32_t m_height;
  bool m_fullScreen;
  bool m_maximized;
  aeStr256 m_windowTitle;

public:
  // Internal
  void m_UpdatePos( aeInt2 pos ) { m_pos = pos; }
  void m_UpdateWidthHeight( int32_t width, int32_t height ) { m_width = width; m_height = height; }
  void m_UpdateMaximized( bool maximized ) { m_maximized = maximized; }

  void* window;
};

#endif
