//------------------------------------------------------------------------------
// aeEditorCamera.cpp
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
#include "aeEditorCamera.h"
#include "aeInput.h"

//------------------------------------------------------------------------------
// aeEditorCamera member functions
//------------------------------------------------------------------------------
aeEditorCamera::aeEditorCamera()
{
	m_RecalculateOffset();
}

void aeEditorCamera::Update( const aeInput* input, float dt )
{
	// Refocus
	if ( m_mode == MoveMode::Refocus )
	{
		m_focusPos = aeMath::DtLerp( m_focusPos, 4.5f, dt, m_refocusPos );
		if ( ( m_refocusPos - m_focusPos ).Length() < 0.01f )
		{
			m_mode = MoveMode::None;
			m_focusPos = m_refocusPos;
		}
		else
		{
			return;
		}
	}

	// Input
	aeFloat2 mouseMovement( input->GetState()->mousePixelPos - input->GetPrevState()->mousePixelPos );
	bool mouseRotate = input->GetPrevState()->mouseLeft && input->GetState()->mouseLeft;
	bool mousePan = input->GetPrevState()->mouseMiddle && input->GetState()->mouseMiddle;
	bool mouseZoom = input->GetPrevState()->mouseRight && input->GetState()->mouseRight;

	if ( m_mode == MoveMode::Rotate && !mouseRotate )
	{
		m_mode = MoveMode::None;
	}
	else if ( m_mode == MoveMode::Pan && !mousePan )
	{
		m_mode = MoveMode::None;
	}
	else if ( m_mode == MoveMode::Zoom && !mouseZoom )
	{
		m_mode = MoveMode::None;
	}

	if ( m_mode == MoveMode::None )
	{
		if ( mouseRotate )
		{
			m_mode = MoveMode::Rotate;
		}
		else if ( mousePan )
		{
			m_mode = MoveMode::Pan;
		}
		else if ( mouseZoom )
		{
			m_mode = MoveMode::Zoom;
		}
	}

	// Rotation
	if ( m_mode == MoveMode::Rotate )
	{
		mouseMovement *= 0.01f;
		m_yaw -= mouseMovement.x;
		m_pitch -= mouseMovement.y;
		m_pitch = aeMath::Clip( m_pitch, -aeMath::HALF_PI * 0.99f, aeMath::HALF_PI * 0.99f );
	}

	float speed = 0.1f / aeMath::Clip( 1.0f - m_dist / 100.0f, 0.1f, 1.0f );

	// Zoom
	if ( m_mode == MoveMode::Zoom )
	{
		m_dist -= mouseMovement.y * 0.1f * speed;
	}
	m_dist -= input->GetState()->scroll * 2.5f * speed;
	m_dist = aeMath::Clip( m_dist, 1.0f, 100.0f );

	// Recalculate camera offset from focus and local axis'
	m_RecalculateOffset();

	// Translation
	if ( m_mode == MoveMode::Pan )
	{
		m_focusPos -= m_right * ( mouseMovement.x * speed );
		m_focusPos -= m_up * ( mouseMovement.y * speed );
	}
}

void aeEditorCamera::Refocus( aeFloat3 pos )
{
	m_mode = MoveMode::Refocus;
	m_refocusPos = pos;
}

void aeEditorCamera::m_RecalculateOffset()
{
	m_offset = aeFloat3( aeMath::Cos( m_yaw ), aeMath::Sin( m_yaw ), 0.0f );
	m_offset *= aeMath::Cos( m_pitch );
	m_offset.z = aeMath::Sin( m_pitch );
	m_forward = -m_offset;
	m_offset *= m_dist;
	m_right = ( m_forward % aeFloat3::Up ).SafeNormalizeCopy();
	m_up = ( m_right % m_forward ).SafeNormalizeCopy();
}
