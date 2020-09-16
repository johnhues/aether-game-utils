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
	aeEditorCamera();
	void Update( const class aeInput* input, float dt );

	void SetPosition( aeFloat3 pos ) { m_focusPos = pos - m_offset; }

	aeFloat3 GetPosition() const { return m_focusPos + m_offset; }
	aeFloat3 GetFocus() const { return m_focusPos; }
	aeFloat3 GetForward() const { return m_forward; }
	float GetDistance() const { return m_dist; }

	void SetFocusDistance( float distance );
	void Refocus( aeFloat3 pos );

private:
	enum class MoveMode
	{
		None,
		Rotate,
		Pan,
		Zoom,
	};

	void m_RecalculateOffset();

	MoveMode m_mode = MoveMode::None;

	aeFloat3 m_focusPos = aeFloat3( 0.0f );
	aeFloat3 m_offset = aeFloat3( 0.0f );
	aeFloat3 m_refocusPos = aeFloat3( 0.0f );
	bool m_refocus = false;

	aeFloat3 m_forward = aeFloat3( 0.0f, 1.0f, 0.0f );
	aeFloat3 m_right = aeFloat3( 1.0f, 0.0f, 0.0f );
	aeFloat3 m_up = aeFloat3( 0.0f, 0.0f, 1.0f );

	float m_yaw = 0.77f;
	float m_pitch = 0.5f;
	float m_dist = 5.0f;
};

#endif
