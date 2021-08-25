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

void aeEditorCamera::SetDistanceLimits( float min, float max )
{
  m_min = min;
  m_max = max;
}

void aeEditorCamera::Update( const ae::Input* input, float dt )
{
	if ( !m_inputEnabled )
	{
		input = nullptr;
	}

	// Input
	ae::Vec2 mouseMovement( 0.0f );
	bool mousePan = false;
	bool mouseZoom = false;
	bool mouseRotate = false;
	if ( input )
	{
		ae::Key panKey = ae::Key::LeftAlt;
		mouseMovement = ae::Vec2( input->mouse.movement );
		mousePan = input->mousePrev.middleButton && input->mouse.middleButton;
		if ( !mousePan
			&& input->mousePrev.leftButton && input->mouse.leftButton
			&& input->Get( panKey ) )
		{
			mousePan = true;
		}
		mouseZoom = input->mousePrev.rightButton && input->mouse.rightButton;

		if ( input->GetMouseCaptured() && !mousePan && !mouseZoom )
		{
			mouseRotate = true;
		}
		else if ( !input->Get( panKey ) )
		{
			mouseRotate = input->mousePrev.leftButton && input->mouse.leftButton;
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
		// Assume right handed coordinate system
		// The focal point should move in the direction that the users hand is moving
		m_yaw -= mouseMovement.x * 0.005f; // Positive horizontal input should result in clockwise rotation around the z axis
		m_pitch += mouseMovement.y * 0.005f; // Positive vertical input should result in counter clockwise rotation around cameras right vector
		m_pitch = aeMath::Clip( m_pitch, -aeMath::HALF_PI * 0.99f, aeMath::HALF_PI * 0.99f ); // Don't let camera flip
	}

	// Zoom
	float zoomSpeed = m_dist / 75.0f;
	if ( m_mode == Mode::Zoom )
	{
		m_dist += mouseMovement.y * 0.1f * zoomSpeed;
		m_dist -= mouseMovement.x * 0.1f * zoomSpeed;
	}
	m_dist -= input ? input->mouse.scroll.y * 2.5f * zoomSpeed : 0.0f;
  m_dist = aeMath::Clip( m_dist, m_min, m_max );

	// Recalculate camera offset from focus and local axis'
	m_Precalculate();

	// Translation
	if ( m_mode == Mode::Pan )
	{
		AE_ASSERT( !m_refocus );
		float panSpeed = m_dist / 750.0f;
		m_focusPos -= m_right * ( mouseMovement.x * panSpeed );
		m_focusPos -= m_up * ( mouseMovement.y * panSpeed );
	}

	// Refocus
	if ( m_refocus )
	{
		AE_ASSERT( m_mode != Mode::Pan );
		m_focusPos = aeMath::DtLerp( m_focusPos, 4.0f, dt, m_refocusPos );
		if ( ( m_refocusPos - m_focusPos ).Length() < 0.01f )
		{
			m_refocus = false;
			m_focusPos = m_refocusPos;
		}
	}
}

void aeEditorCamera::Reset( ae::Vec3 up, ae::Vec3 focus, ae::Vec3 pos )
{
	m_refocus = false;
	m_refocusPos = focus;
	
	m_focusPos = focus;
	
	ae::Vec3 diff = focus - pos;
	m_dist = diff.Length();
	
	if ( m_dist > 0.01f ) // Only update rotation if focus is different than position
	{
		m_yaw = diff.GetXY().GetAngle();
		m_pitch = asinf( diff.z / m_dist );
	}
	
	m_Precalculate();
}

void aeEditorCamera::SetDistanceFromFocus( float distance )
{
	m_dist = distance;
	m_Precalculate();
}

void aeEditorCamera::Refocus( ae::Vec3 focus )
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

void aeEditorCamera::SetRotation( ae::Vec2 angle )
{
  m_yaw = angle.x;
  m_pitch = angle.y;
  m_Precalculate();
}

void aeEditorCamera::m_Precalculate()
{
  m_forward = ae::Vec3( aeMath::Cos( m_yaw ), aeMath::Sin( m_yaw ), 0.0f );
  m_forward *= aeMath::Cos( m_pitch );
  m_forward.z = aeMath::Sin( m_pitch );
  m_offset = -m_forward;
	m_offset *= m_dist;
	m_right = m_forward.Cross( m_worldUp ).SafeNormalizeCopy();
	m_up = m_right.Cross( m_forward ).SafeNormalizeCopy();
}
