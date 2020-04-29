//------------------------------------------------------------------------------
// aeInput.cpp
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
#include "aeInput.h"
#include "aeLog.h"
#include "aePlatform.h"
#include "aeRender.h"
#include "aeWindow.h"
#include "SDL.h"

//------------------------------------------------------------------------------
// InputState member functions
//------------------------------------------------------------------------------
InputState::InputState()
{
  gamepad = false;

  up = false;
  down = false;
  left = false;
  right = false;

  start = false;
  select = false;

  a = false;
  b = false;
  x = false;
  y = false;
  l = false;
  r = false;

  mouseLeft = false;
  mouseRight = false;
  mouseMiddle = false;

  space = false;
  ctrl = false;
  shift = false;
  del = false;
  tilde = 0;
  esc = false;

  mousePixelPos = aeInt2( 0 );
  scroll = 0;

  exit = false;
}

bool InputState::Get( InputType type ) const
{
  switch ( type )
  {
  case kInputType_Up:
    return up;
  case kInputType_Down:
    return down;
  case kInputType_Left:
    return left;
  case kInputType_Right:
    return right;
  case kInputType_Start:
    return start;
  case kInputType_Select:
    return select;
  case kInputType_A:
    return a;
  case kInputType_B:
    return b;
  case kInputType_X:
    return x;
  case kInputType_Y:
    return y;
  case kInputType_L:
    return l;
  case kInputType_R:
    return r;
  case kInputType_MouseLeft:
    return mouseLeft;
  case kInputType_MouseRight:
    return mouseRight;
  case kInputType_MouseMiddle:
    return mouseMiddle;
  case kInputType_Space:
    return space;
  case kInputType_Control:
    return ctrl;
  case kInputType_Shift:
    return shift;
  case kInputType_Delete:
    return del;
  case kInputType_Tilde:
    return tilde;
  case kInputType_Escape:
    return esc;
  default:
    return false;
  }
}

const char* InputState::GetName( InputType type ) const
{
  switch ( type )
  {
  case kInputType_Up:
    return gamepad ? "Stick" : "Arrows";
  case kInputType_Down:
    return gamepad ? "Stick" : "Arrows";
  case kInputType_Left:
    return gamepad ? "Stick" : "Arrows";
  case kInputType_Right:
    return gamepad ? "Stick" : "Arrows";
  case kInputType_Start:
    return gamepad ? "Start" : "Enter";
  case kInputType_Select:
    return gamepad ? "Select" : "Shift";
  case kInputType_A:
    return gamepad ? "A" : "X";
  case kInputType_B:
    return gamepad ? "B" : "Z";
  case kInputType_X:
    return gamepad ? "X" : "A";
  case kInputType_Y:
    return gamepad ? "Y" : "S";
  case kInputType_L:
    return gamepad ? "L" : "Q";
  case kInputType_R:
    return gamepad ? "R" : "W";
  case kInputType_MouseLeft:
    return "Left Click";
  case kInputType_MouseRight:
    return "Right Click";
  case kInputType_MouseMiddle:
    return "Middle Click";
  case kInputType_Space:
    return "Space";
  case kInputType_Control:
    return "Ctrl";
  case kInputType_Shift:
    return "Shift";
  case kInputType_Delete:
    return "Delete";
  case kInputType_Tilde:
    return "Tilde";
  case kInputType_Escape:
    return "Escape";
  default:
    return "";
  }
}

//------------------------------------------------------------------------------
// aeInput member functions
//------------------------------------------------------------------------------
aeInput::aeInput()
{
  m_window = nullptr;
  m_render = nullptr;
  m_textMode = 0;
}

void aeInput::Initialize( aeWindow* window, aeRender* render )
{
  SDL_JoystickEventState( SDL_ENABLE );

  m_window = window;
  m_render = render;

  // Input key mapping
  {
    m_keyMap.Set( SDL_SCANCODE_UP, &m_input.up );
    m_keyMap.Set( SDL_SCANCODE_DOWN, &m_input.down );
    m_keyMap.Set( SDL_SCANCODE_LEFT, &m_input.left );
    m_keyMap.Set( SDL_SCANCODE_RIGHT, &m_input.right );

    m_keyMap.Set( SDL_SCANCODE_RETURN, &m_input.start );
    m_keyMap.Set( SDL_SCANCODE_RSHIFT, &m_input.select );

    m_keyMap.Set( SDL_SCANCODE_Z, &m_input.b );
    m_keyMap.Set( SDL_SCANCODE_X, &m_input.a );
    m_keyMap.Set( SDL_SCANCODE_A, &m_input.x );
    m_keyMap.Set( SDL_SCANCODE_S, &m_input.y );
    m_keyMap.Set( SDL_SCANCODE_Q, &m_input.l );
    m_keyMap.Set( SDL_SCANCODE_W, &m_input.r );

    m_keyMap.Set( SDL_SCANCODE_SPACE, &m_input.space );
#if _AE_OSX_
    m_keyMap.Set( SDL_SCANCODE_LGUI, &m_input.ctrl ); // Command
#else
    m_keyMap.Set( SDL_SCANCODE_LCTRL, &m_input.ctrl );
#endif
    m_keyMap.Set( SDL_SCANCODE_LSHIFT, &m_input.shift );
    m_keyMap.Set( SDL_SCANCODE_BACKSPACE, &m_input.del );
    m_keyMap.Set( SDL_SCANCODE_GRAVE, &m_input.tilde );
    m_keyMap.Set( SDL_SCANCODE_ESCAPE, &m_input.esc );
  }
}

void aeInput::Terminate()
{}

void aeInput::Pump()
{
  SDL_PumpEvents();

#if !_AE_EMSCRIPTEN_
  m_prevInput = m_input;

  m_input.scroll = 0;

  bool sdlTextMode = SDL_IsTextInputActive();
  if ( m_textMode && !sdlTextMode )
  {
    SDL_StartTextInput();
  }
  else if ( !m_textMode && sdlTextMode )
  {
    SDL_StopTextInput();
  }

  SDL_Event events[ 32 ];
  // Get all events at once, this function can be very slow. Returns -1 while shutting down.
  int32_t eventCount = SDL_PeepEvents( events, countof( events ), SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT );
  if ( eventCount >= 1 )
  {
    for ( int32_t i = 0; i < eventCount; i++ )
    {
      const SDL_Event& event = events[ i ];

      if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
      {
        m_window->m_UpdateWidthHeight( event.window.data1, event.window.data2 );
      }
      else if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_MOVED )
      {
        m_window->m_UpdatePos( aeInt2( event.window.data1, event.window.data2 ) );
      }
      else if ( event.type == SDL_MOUSEMOTION )
      {
        m_input.mousePixelPos = aeInt2( event.motion.x, m_window->GetHeight() - event.motion.y );
      }
      else if ( event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP )
      {
        bool pressed = ( event.button.state == SDL_PRESSED );
        switch ( event.button.button )
        {
        case SDL_BUTTON_LEFT:
          m_input.mouseLeft = pressed;
          break;
        case SDL_BUTTON_MIDDLE:
          m_input.mouseMiddle = pressed;
          break;
        case SDL_BUTTON_RIGHT:
          m_input.mouseRight = pressed;
          break;
        }
      }
      else if ( event.type == SDL_MOUSEWHEEL )
      {
        m_input.scroll = event.wheel.y * ( event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1 );
      }
      else if ( m_textMode && event.type == SDL_TEXTINPUT )
      {
        // @NOTE: Ignore keys while modifier is pressed so below copy and paste work as expected
        if ( !( SDL_GetModState() & KMOD_CTRL ) )
        {
          m_text += event.text.text;
        }
      }
      else if ( event.type == SDL_KEYDOWN || event.type == SDL_KEYUP )
      {
        // @NOTE: Always capture key ups even while in text mode to avoid creating inconsistent input state.
        //        Don't capture key downs to avoid creating game inputs while typing.
        if ( !m_textMode || event.type == SDL_KEYUP )
        {
          bool** target = m_keyMap.TryGet( event.key.keysym.scancode );
          if ( target )
          {
            **target = ( event.type == SDL_KEYDOWN );
          }
        }

        if ( m_textMode && event.type == SDL_KEYDOWN )
        {
          if ( event.key.keysym.sym == SDLK_BACKSPACE && m_text.Length() )
          {
            m_text.Trim( m_text.Length() - 1 );
          }
          else if ( event.key.keysym.sym == SDLK_RETURN )
          {
            m_text += "\n";
          }
          else if ( event.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
          {
            SDL_SetClipboardText( m_text.c_str() );
          }
          else if ( event.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
          {
            m_text += SDL_GetClipboardText();
          }
        }
      }
      else if ( event.type == SDL_QUIT )
      {
        m_input.exit = true;
      }
    }
  }
#endif
}

void aeInput::SetTextMode( bool enabled )
{
  m_textMode = enabled;
}
