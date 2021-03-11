//------------------------------------------------------------------------------
// aeEditorCamera.h
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
#ifndef AEEDITORCAMERA_H
#define AEEDITORCAMERA_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeMath.h"

//------------------------------------------------------------------------------
// aeEditorCamera class
//------------------------------------------------------------------------------
class aeEditorCamera
{
public:
	enum class Mode
	{
		None,
		Rotate,
		Pan,
		Zoom,
	};
	
	aeEditorCamera();
	void Update( const class aeInput* input, float dt ); // @TODO: This should take input values, not the whole input system

	void Reset( aeFloat3 focus, aeFloat3 pos ); // Interupts refocus. Does not affect input mode.
	void SetDistanceFromFocus( float distance ); // Updates position. Does not affect input mode or refocus.
	void Refocus( aeFloat3 focus ); // Updates focus and position over time
	void SetInputEnabled( bool enabled ); // True by default
  void SetRotation( aeFloat2 angle );

	Mode GetMode() const { return m_mode; }
	aeFloat3 GetPosition() const { return m_focusPos + m_offset; }
	aeFloat3 GetFocus() const { return m_focusPos; }
	aeFloat3 GetForward() const { return m_forward; }
	float GetDistanceFromFocus() const { return m_dist; }
  aeFloat2 GetRotation() const { return aeFloat2( m_yaw, m_pitch ); }

private:
	// Called when focus, distance, yaw, or pitch is changed
	void m_Precalculate();

	// Mode
	bool m_inputEnabled = true;
	Mode m_mode = Mode::None;
	aeFloat3 m_refocusPos = aeFloat3( 0.0f );
	bool m_refocus = false;

	// Positioning
	aeFloat3 m_focusPos = aeFloat3( 0.0f );
	float m_dist = 5.0f;

	// Rotation
	float m_yaw = 0.77f;
	float m_pitch = 0.5f;

	// Pre-calculated values for getters
	aeFloat3 m_offset;
	aeFloat3 m_forward;
	aeFloat3 m_right;
	aeFloat3 m_up;
};

#endif
