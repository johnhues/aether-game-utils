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

//------------------------------------------------------------------------------
// InputType enum
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
  kInputType_Escape,

  kInputTypeCount
};

//------------------------------------------------------------------------------
// InputState class
//------------------------------------------------------------------------------
class InputState
{
public:
  InputState();
  bool Get( InputType type ) const;
  const char* GetName( InputType type ) const;

  bool gamepad;

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
  bool l;
  bool r;

  bool mouseLeft;
  bool mouseRight;
  bool mouseMiddle;

  bool space;
  bool ctrl;
  bool shift;
  bool del;
  bool esc;

  aeInt2 mousePixelPos;
  int32_t scroll;
};

//------------------------------------------------------------------------------
// aeInput class
//------------------------------------------------------------------------------
class aeInput
{
public:
  void Initialize( class aeWindow* window, class aeRenderer* renderer );
  void Terminate();
  void Pump();

  const InputState* GetState() const { return &m_input; }
  const InputState* GetPrevState() const { return &m_prevInput; }

private:
  class aeWindow* m_window;
  class aeRenderer* m_renderer;

  aeMap< uint32_t, bool* > m_keyMap;
  
  InputState m_input;
  InputState m_prevInput;
};

#endif
