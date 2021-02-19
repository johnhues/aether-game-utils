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
	m_Precalculate();
}

void aeEditorCamera::Update( const aeInput* input, float dt )
{
	if ( !m_inputEnabled )
	{
		input = nullptr;
	}

	// Input
	aeFloat2 mouseMovement( 0.0f );
	bool mousePan = false;
	bool mouseZoom = false;
	bool mouseRotate = false;
	if ( input )
	{
		aeKey panKey = aeKey::LeftAlt;
		mouseMovement = aeFloat2( input->GetState()->mousePixelPos - input->GetPrevState()->mousePixelPos );
		mousePan = input->GetPrevState()->mouseMiddle && input->GetState()->mouseMiddle;
		if ( !mousePan
			&& input->GetPrevState()->mouseLeft && input->GetState()->mouseLeft
			&& input->GetState()->Get( panKey ) )
		{
			mousePan = true;
		}
		mouseZoom = input->GetPrevState()->mouseRight && input->GetState()->mouseRight;

		if ( input->GetMouseCaptured() && !mousePan && !mouseZoom )
		{
			mouseRotate = true;
		}
		else if ( !input->GetState()->Get( panKey ) )
		{
			mouseRotate = input->GetPrevState()->mouseLeft && input->GetState()->mouseLeft;
		}
	}

	if ( m_mode == Mode::Rotate && !mouseRotate )
	{
		m_mode = Mode::None;
	}
	else if ( m_mode == Mode::Pan && !mousePan )
	{
		m_mode = Mode::None;
	}
	else if ( m_mode == Mode::Zoom && !mouseZoom )
	{
		m_mode = Mode::None;
	}

	if ( m_mode == Mode::None )
	{
		if ( mouseRotate )
		{
			m_mode = Mode::Rotate;
		}
		else if ( mousePan )
		{
			m_mode = Mode::Pan;
			m_refocus = false;
		}
		else if ( mouseZoom )
		{
			m_mode = Mode::Zoom;
		}
	}

	// Rotation
	if ( m_mode == Mode::Rotate )
	{
		m_yaw -= mouseMovement.x * 0.01f;
		m_pitch -= mouseMovement.y * 0.01f;
		m_pitch = aeMath::Clip( m_pitch, -aeMath::HALF_PI * 0.99f, aeMath::HALF_PI * 0.99f );
	}

	// Zoom
	float zoomSpeed = 0.15f / aeMath::Clip( 1.0f - m_dist / 100.0f, 0.15f, 1.0f );
	if ( m_mode == Mode::Zoom )
	{
		m_dist += mouseMovement.y * 0.1f * zoomSpeed;
		m_dist -= mouseMovement.x * 0.1f * zoomSpeed;
	}
	m_dist -= input ? input->GetState()->scroll * 2.5f * zoomSpeed : 0.0f;
	m_dist = aeMath::Clip( m_dist, 1.0f, 1000.0f );

	// Recalculate camera offset from focus and local axis'
	m_Precalculate();

	// Translation
	if ( m_mode == Mode::Pan )
	{
		AE_ASSERT( !m_refocus );
		float panSpeed = 0.035f / aeMath::Clip( 1.0f - m_dist / 100.0f, 0.035f, 1.0f );
		m_focusPos -= m_right * ( mouseMovement.x * panSpeed );
		m_focusPos -= m_up * ( mouseMovement.y * panSpeed );
	}

	// Refocus
	if ( m_refocus )
	{
		AE_ASSERT( m_mode != Mode::Pan );
		m_focusPos = aeMath::DtLerp( m_focusPos, 4.5f, dt, m_refocusPos );
		if ( ( m_refocusPos - m_focusPos ).Length() < 0.01f )
		{
			m_refocus = false;
			m_focusPos = m_refocusPos;
		}
	}
}

void aeEditorCamera::Reset( aeFloat3 focus, aeFloat3 pos )
{
	m_refocus = false;
	m_refocusPos = focus;
	
	m_focusPos = focus;
	
	aeFloat3 diff = focus - pos;
	m_dist = diff.Length();
	
	if ( m_dist > 0.01f ) // Don't rotate camera if focus is same as pos
	{
		m_yaw = diff.GetXY().GetAngle() + aeMath::PI;
		m_pitch = -std::asinf( diff.z / m_dist );
	}
	
	m_Precalculate();
}

void aeEditorCamera::SetDistanceFromFocus( float distance )
{
	m_dist = distance;
	m_Precalculate();
}

void aeEditorCamera::Refocus( aeFloat3 focus )
{
	m_refocus = true;
	m_refocusPos = focus;

	if ( m_mode == Mode::Pan )
	{
		m_mode = Mode::None;
	}
}

void aeEditorCamera::SetInputEnabled( bool enabled )
{
	m_inputEnabled = enabled;
}

void aeEditorCamera::m_Precalculate()
{
	m_offset = aeFloat3( aeMath::Cos( m_yaw ), aeMath::Sin( m_yaw ), 0.0f );
	m_offset *= aeMath::Cos( m_pitch );
	m_offset.z = aeMath::Sin( m_pitch );
	m_forward = -m_offset;
	m_offset *= m_dist;
	m_right = ( m_forward % aeFloat3::Up ).SafeNormalizeCopy();
	m_up = ( m_right % m_forward ).SafeNormalizeCopy();
}
