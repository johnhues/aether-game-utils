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
#if !_AE_EMSCRIPTEN_
  #include "SDL_gamecontroller.h"
#endif

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const float kAnalogDeadzone = 0.2f;

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
aeFloat2 ApplyAnalogDeadzone( aeFloat2 analog, float deadzone )
{
  float originalLength = analog.Length();
  if ( originalLength <= kAnalogDeadzone )
  {
    return aeFloat2( 0.0f );
  }
  else
  {
    float renormalizedLength = aeMath::Delerp( deadzone, 1.0f, originalLength );
    renormalizedLength = aeMath::Clip01( renormalizedLength );

    return ( analog / originalLength ) * renormalizedLength;
  }
}

//------------------------------------------------------------------------------
// InputState member functions
//------------------------------------------------------------------------------
InputState::InputState()
{
  gamepad = false;
  gamepadBattery = aeBatteryLevel::None;

  leftAnalog = aeFloat2( 0.0f );
  rightAnalog = aeFloat2( 0.0f );
  dpad = aeInt2( 0 );

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
  windowFocus = false;

  exit = false;

  memset( m_keys, 0 ,sizeof( m_keys ) );
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

bool InputState::Get( aeKey key ) const
{
	return m_keys[ (uint32_t)key ];
}

//------------------------------------------------------------------------------
// aeInput member functions
//------------------------------------------------------------------------------
aeInput::aeInput()
{
  m_window = nullptr;

  m_textMode = 0;
  m_text = "";
  m_mouseCaptured = false;
  m_firstPump = true;

  m_joystickHandle = nullptr;
#if !_AE_EMSCRIPTEN_
  m_controller = nullptr;
#endif
  m_buttonCount = 0;
  m_hatCount = 0;
  m_axesCount = 0;
}

void aeInput::Initialize( aeWindow* window )
{
#if !_AE_EMSCRIPTEN_
  // This has to run for SDL_PumpEvents() to work
  if ( !window && SDL_Init( SDL_INIT_EVENTS ) < 0 )
  {
    AE_FAIL_MSG( "SDL could not initialize: #", SDL_GetError() );
  }
#endif

  SDL_JoystickEventState( SDL_ENABLE );
#if !_AE_IOS_
  // @HACK: Disable text input here to prevent keyboard from popping open on ios
  SDL_StartTextInput();
#endif

  m_window = window;

  m_mouseCaptured = SDL_GetRelativeMouseMode();
  m_firstPump = true;

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
{
  if ( m_joystickHandle )
  {
    SDL_JoystickClose( m_joystickHandle );
  }

#if !_AE_EMSCRIPTEN_
  if ( m_controller )
  {
    SDL_GameControllerClose( m_controller );
  }
#endif
}

void aeInput::Pump()
{
  SDL_PumpEvents();

#if !_AE_EMSCRIPTEN_
  m_prevInput = m_input;

  m_input.scroll = 0;

  m_textInput.Clear();

  bool ignoreMouseMovement = m_firstPump;
  SDL_Event events[ 32 ];
  // Get all events at once, this function can be very slow. Returns -1 while shutting down.
  int32_t eventCount = SDL_PeepEvents( events, countof( events ), SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT );
  if ( eventCount >= 1 )
  {
    for ( int32_t i = 0; i < eventCount; i++ )
    {
      const SDL_Event& event = events[ i ];

      if ( m_window && event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
      {
        int width = event.window.data1;
        int height = event.window.data2;
        SDL_GL_GetDrawableSize( (SDL_Window*)m_window->window, &width, &height );
        m_window->m_UpdateWidthHeight( width, height );
      }
      else if ( m_window && event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_MOVED )
      {
        aeInt2 pos( event.window.data1, event.window.data2 );
        pos = ( pos * m_window->GetDpiScale() ).TruncateCopy(); // @TODO: Should this be floor?
        m_window->m_UpdatePos( pos );
      }
      else if ( m_window && event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED )
      {
        m_input.windowFocus = true;
        // @NOTE: The first frame after window focus creates random movement when relative/capture mouse mode is enabled
        ignoreMouseMovement = true;
      }
      else if ( m_window && event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST )
      {
        m_input.windowFocus = false;
        SetMouseCaptured( false );
      }
      else if ( event.type == SDL_MOUSEMOTION )
      {
        if ( m_mouseCaptured )
        {
          m_prevInput.mousePixelPos = aeInt2( 0 );
          m_input.mousePixelPos = ignoreMouseMovement ? aeInt2( 0 ) : aeInt2( event.motion.xrel, -event.motion.yrel );
          m_input.mousePixelPos = ( m_input.mousePixelPos * m_window->GetDpiScale() ).TruncateCopy(); // @TODO: Should this be floor?
        }
        else
        {
          m_input.mousePixelPos = aeInt2( event.motion.x, event.motion.y );
          m_input.mousePixelPos = ( m_input.mousePixelPos * m_window->GetDpiScale() ).TruncateCopy(); // @TODO: Should this be floor?
          m_input.mousePixelPos.y = m_window->GetHeight() - m_input.mousePixelPos.y;
        }
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
      else if ( event.type == SDL_TEXTINPUT )
      {
        m_textInput.Append( event.text.text, (uint32_t)strlen( event.text.text ) + 1 );

        // @NOTE: Ignore keys while modifier is pressed so below copy and paste work as expected
        if ( m_textMode && !( SDL_GetModState() & KMOD_CTRL ) )
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

  int32_t numKeys = 0;
  const uint8_t* keys = SDL_GetKeyboardState( &numKeys );
  numKeys = aeMath::Min( (int32_t)kKeyCount, numKeys );
  for ( int32_t i = 0; i < numKeys; i++ )
  {
    m_input.m_keys[ i ] = ( keys[ i ] != 0 );
  }
#endif

  if ( !m_joystickHandle && SDL_NumJoysticks() )
  {
    uint32_t index = 0;

#if !_AE_EMSCRIPTEN_
    AE_ASSERT( !m_controller );
    for ( int32_t i = 0; i < SDL_NumJoysticks(); ++i )
    {
      if ( SDL_IsGameController( i ) )
      {
        index = i;
        m_controller = SDL_GameControllerOpen( i );
        break;
      }
    }

    if ( m_controller )
#endif
    {
      m_joystickHandle = SDL_JoystickOpen( index );
      m_buttonCount = SDL_JoystickNumButtons( m_joystickHandle );
      m_hatCount = SDL_JoystickNumHats( m_joystickHandle );
      m_axesCount = SDL_JoystickNumAxes( m_joystickHandle );
    }
  }
  // @TODO: Should check if specific controller is disconnected
  else if ( m_joystickHandle && !SDL_NumJoysticks() )
  {
    SDL_JoystickClose( m_joystickHandle );
    m_joystickHandle = nullptr;

#if !_AE_EMSCRIPTEN_
    AE_ASSERT( m_controller );
    SDL_GameControllerClose( m_controller );
    m_controller = nullptr;
#endif

    m_input.gamepadBattery = aeBatteryLevel::None;
  }

  m_input.gamepad = ( m_joystickHandle != nullptr );

  if ( m_joystickHandle )
  {
#if _AE_EMSCRIPTEN_
    bool* inputBtns[] =
    {
      // @NOTE: The order of these matter. SDL_JoystickGetButton() uses an index instead of named buttons.
      &m_input.a,
      &m_input.b,
      &m_input.x,
      &m_input.y,
      &m_input.l,
      &m_input.r,
      &m_input.start,
      &m_input.select
    };
    uint32_t btnCount = aeMath::Min( countof( inputBtns ), m_buttonCount );
    for ( uint32_t i = 0; i < btnCount; i++ )
    {
      *( inputBtns[ i ] ) = SDL_JoystickGetButton( m_joystickHandle, i );
    }
#else
    struct ControllerMapping
    {
      SDL_GameControllerButton btnCode;
      bool* input;
    };
    ControllerMapping inputBtns[] =
    {
      { SDL_CONTROLLER_BUTTON_A, &m_input.a },
      { SDL_CONTROLLER_BUTTON_B, &m_input.b },
      { SDL_CONTROLLER_BUTTON_X, &m_input.x },
      { SDL_CONTROLLER_BUTTON_Y, &m_input.y },
      { SDL_CONTROLLER_BUTTON_LEFTSHOULDER, &m_input.l },
      { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, &m_input.r },
      { SDL_CONTROLLER_BUTTON_START, &m_input.start },
      { SDL_CONTROLLER_BUTTON_BACK, &m_input.select }
    };
    for ( uint32_t i = 0; i < countof( inputBtns ); i++ )
    {
      ControllerMapping& mapping = inputBtns[ i ];
      *( mapping.input ) = SDL_GameControllerGetButton( m_controller, mapping.btnCode );
    }
#endif

    // dpad
    m_input.dpad = aeInt2( 0 );
#if !_AE_EMSCRIPTEN_
    if ( SDL_GameControllerGetButton( m_controller, SDL_CONTROLLER_BUTTON_DPAD_UP ) ) { m_input.dpad.y++; }
    if ( SDL_GameControllerGetButton( m_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN ) ) { m_input.dpad.y--; }
    if ( SDL_GameControllerGetButton( m_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT ) ) { m_input.dpad.x--; }
    if ( SDL_GameControllerGetButton( m_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) ) { m_input.dpad.x++; }
#endif

    if ( m_axesCount >= 2 )
    {
      aeFloat2 analog;
      analog.x = SDL_JoystickGetAxis( m_joystickHandle, 0 ) / (float)aeMath::MaxValue< int16_t >();
      analog.y = SDL_JoystickGetAxis( m_joystickHandle, 1 ) / (float)aeMath::MaxValue< int16_t >();
      analog.y = -analog.y; // @NOTE: Analog sticks are negative up for some reason on XBox controllers
      m_input.leftAnalog = ApplyAnalogDeadzone( analog, kAnalogDeadzone );
    }

    if ( m_axesCount == 6 ) // XBox type gamepad likely
    {
      aeFloat2 analog;
      analog.x = SDL_JoystickGetAxis( m_joystickHandle, 3 ) / (float)aeMath::MaxValue< int16_t >();
      analog.y = SDL_JoystickGetAxis( m_joystickHandle, 4 ) / (float)aeMath::MaxValue< int16_t >();
      analog.y = -analog.y; // @NOTE: Analog sticks are negative up for some reason on XBox controllers
      m_input.rightAnalog = ApplyAnalogDeadzone( analog, kAnalogDeadzone );
    }

#if !_AE_EMSCRIPTEN_
    switch ( SDL_JoystickCurrentPowerLevel( m_joystickHandle ) )
    {
      case SDL_JOYSTICK_POWER_EMPTY:
        m_input.gamepadBattery = aeBatteryLevel::Empty;
        break;
      case SDL_JOYSTICK_POWER_LOW:
        m_input.gamepadBattery = aeBatteryLevel::Low;
        break;
      case SDL_JOYSTICK_POWER_MEDIUM:
        m_input.gamepadBattery = aeBatteryLevel::Medium;
        break;
      case SDL_JOYSTICK_POWER_FULL:
      case SDL_JOYSTICK_POWER_MAX:
        m_input.gamepadBattery = aeBatteryLevel::Full;
        break;
      case SDL_JOYSTICK_POWER_WIRED:
        m_input.gamepadBattery = aeBatteryLevel::Wired;
        break;
      default:
        m_input.gamepadBattery = aeBatteryLevel::None;
        break;
    }
#endif
  }

  m_input.gamepad = ( m_joystickHandle != nullptr );

  if ( m_firstPump )
  {
    // Don't 'move' cursor on first frame
    m_prevInput.mousePixelPos = m_input.mousePixelPos;
    m_firstPump = false;
  }
}

void aeInput::SetTextMode( bool enabled )
{
  m_textMode = enabled;
}

void aeInput::SetMouseCaptured( bool captured )
{
  SDL_SetRelativeMouseMode( captured ? SDL_TRUE : SDL_FALSE );
  m_mouseCaptured = captured;
}
