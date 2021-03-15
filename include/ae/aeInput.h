//------------------------------------------------------------------------------
// aeInput.h
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
#ifndef AEINPUT_H
#define AEINPUT_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeMap.h"
#include "aeMath.h"
#include "aeString.h"

//------------------------------------------------------------------------------
// Input types
//------------------------------------------------------------------------------
enum InputType
{
  kInputType_Up,
  kInputType_Down,
  kInputType_Left,
  kInputType_Right,

  kInputType_Start,
  kInputType_Select,

  kInputType_A,
  kInputType_B,
  kInputType_X,
  kInputType_Y,
  kInputType_L,
  kInputType_R,

  kInputType_MouseLeft,
  kInputType_MouseRight,
  kInputType_MouseMiddle,

  kInputType_Space,
  kInputType_Control,
  kInputType_Shift,
  kInputType_Delete,
  kInputType_Tilde,
  kInputType_Escape,

  kInputTypeCount
};

enum class aeBatteryLevel
{
  None,
  Empty,
  Low,
  Medium,
  Full,
  Wired
};

//------------------------------------------------------------------------------
// aeKey enum
// Based on:
// https://hg.libsdl.org/SDL/file/eff2d605bef5/include/SDL_scancode.h
// https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
//------------------------------------------------------------------------------
enum class aeKey
{
  Unknown = 0,

  A = 4,
  B = 5,
  C = 6,
  D = 7,
  E = 8,
  F = 9,
  G = 10,
  H = 11,
  I = 12,
  J = 13,
  K = 14,
  L = 15,
  M = 16,
  N = 17,
  O = 18,
  P = 19,
  Q = 20,
  R = 21,
  S = 22,
  T = 23,
  U = 24,
  V = 25,
  W = 26,
  X = 27,
  Y = 28,
  Z = 29,

  Num1 = 30,
  Num2 = 31,
  Num3 = 32,
  Num4 = 33,
  Num5 = 34,
  Num6 = 35,
  Num7 = 36,
  Num8 = 37,
  Num9 = 38,
  Num0 = 39,

  Enter = 40,
  Escape = 41,
  Backspace = 42,
  Tab = 43,
  Space = 44,

  Minus = 45,
  Equals = 46,
  LeftBracket = 47,
  RightBracket = 48,
  Backslash = 49,

  Semicolon = 51,
  Apostrophe = 52,
  Tilde = 53,
  Comma = 54,
  Period = 55,
  Slash = 56,
  CapsLock = 57,

  F1 = 58,
  F2 = 59,
  F3 = 60,
  F4 = 61,
  F5 = 62,
  F6 = 63,
  F7 = 64,
  F8 = 65,
  F9 = 66,
  F10 = 67,
  F11 = 68,
  F12 = 69,

  PrintScreen = 70,
  ScrollLock = 71,
  Pause = 72,

  Insert = 73,
  Home = 74,
  PageUp = 75,
  Delete = 76,
  End = 77,
  PageDown = 78,

  Right = 79,
  Left = 80,
  Down = 81,
  Up = 82,

  NumLock = 84,
  NumPadDivide = 84,
  NumPadMultiply = 85,
  NumPadMinus = 86,
  NumPadPlus = 87,
  NumPadEnter = 88,
  NumPad1 = 89,
  NumPad2 = 90,
  NumPad3 = 91,
  NumPad4 = 92,
  NumPad5 = 93,
  NumPad6 = 94,
  NumPad7 = 95,
  NumPad8 = 96,
  NumPad9 = 97,
  NumPad0 = 98,
  NumPadPeriod = 99,
  NumPadEquals = 103,

  LeftControl = 224,
  LeftShift = 225,
  LeftAlt = 226,
  LeftSuper = 227,
  RightControl = 228,
  RightShift = 229,
  RightAlt = 230,
  RightSuper = 231
};

const uint32_t kKeyCount = 256;

//------------------------------------------------------------------------------
// InputState class
//------------------------------------------------------------------------------
class InputState
{
public:
  InputState();
  bool Get( InputType type ) const;
  const char* GetName( InputType type ) const;
  bool Get( aeKey key ) const;

  bool gamepad;
  aeBatteryLevel gamepadBattery;

  aeFloat2 leftAnalog;
  aeFloat2 rightAnalog;
  aeInt2 dpad;
  
  bool up;
  bool down;
  bool left;
  bool right;

  bool start;
  bool select;

  bool a;
  bool b;
  bool x;
  bool y;
  
  float leftTrigger;
  float rightTrigger;
  bool l;
  bool r;

  bool mouseLeft;
  bool mouseRight;
  bool mouseMiddle;

  bool space;
  bool ctrl;
  bool shift;
  bool del;
  bool tilde;
  bool esc;

  aeInt2 mousePixelPos;
  int32_t scroll;
  bool windowFocus;

  bool exit; // Window 'X' etc

private:
  friend class aeInput;
  bool m_keys[ kKeyCount ];
};

//------------------------------------------------------------------------------
// aeInput class
//------------------------------------------------------------------------------
class aeInput
{
public:
  aeInput();
  void Initialize( class aeWindow* window = nullptr );
  void Terminate();
  void Pump();

  const InputState* GetState() const { return &m_input; }
  const InputState* GetPrevState() const { return &m_prevInput; }

  void SetTextMode( bool enabled );
  bool GetTextMode() const { return m_textMode; }
  void SetText( const char* text ) { m_text = text; }
  const char* GetText() const { return m_text.c_str(); }
  const char* GetTextInput() const { return m_textInput.Length() ? &m_textInput[ 0 ] : ""; }

  void SetMouseCaptured( bool captured );
  bool GetMouseCaptured() const { return m_mouseCaptured; }

private:
  class aeWindow* m_window;

  aeMap< uint32_t, bool* > m_keyMap;
  
  InputState m_input;
  InputState m_prevInput;

  bool m_textMode;
  aeStr512 m_text;
  aeArray< char > m_textInput;

  bool m_mouseCaptured;
  bool m_firstPump;

  struct _SDL_Joystick* m_joystickHandle;
#if !_AE_EMSCRIPTEN_
  struct _SDL_GameController* m_controller;
#endif
  uint32_t m_buttonCount;
  uint32_t m_hatCount;
  uint32_t m_axesCount;
};

#endif
