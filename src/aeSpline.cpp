//------------------------------------------------------------------------------
// aeSpline.cpp
// Utilities for allocating objects. Provides functionality to track current and
// past allocations.
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
#include "aeSpline.h"

//------------------------------------------------------------------------------
// aeSpline member functions
//------------------------------------------------------------------------------
aeSpline::aeSpline( aeFloat3* controlPoints, uint32_t count ) :
  m_controlPoints( count )
{
  for ( uint32_t i = 0; i < count; i++ )
  {
    m_controlPoints.Append( controlPoints[ i ] );
  }
}

void aeSpline::AppendControlPoint( aeFloat3 p )
{
  m_controlPoints.Append( p );
}

aeFloat3 aeSpline::GetControlPoint( uint32_t index ) const
{
  return m_controlPoints[ index ];
}

uint32_t aeSpline::GetControlPointCount() const
{
  return m_controlPoints.Length();
}

aeFloat3 aeSpline::GetPoint( float distance ) const
{
  if ( m_controlPoints.Length() == 0 )
  {
    return aeFloat3( 0.0f );
  }
  else if ( m_controlPoints.Length() == 1 )
  {
    return m_controlPoints[ 0 ];
  }

  for ( uint32_t i = 0; i < m_controlPoints.Length() - 1; i++ )
  {
    aeFloat3 p0 = m_controlPoints[ i ];
    aeFloat3 p1 = m_controlPoints[ i + 1 ];

    float length = ( p1 - p0 ).Length();
    if ( length >= distance )
    {
      return aeMath::Lerp( p0, p1, distance / length );
    }

    distance -= length;
  }

  return m_controlPoints[ m_controlPoints.Length() - 1 ];
}

float aeSpline::GetLength() const
{
  if ( m_controlPoints.Length() <= 1 )
  {
    return 0.0f;
  }

  float length = 0.0f;
  for ( uint32_t i = 0; i < m_controlPoints.Length() - 1; i++ )
  {
    aeFloat3 p0 = m_controlPoints[ i ];
    aeFloat3 p1 = m_controlPoints[ i + 1 ];
    length += ( p1 - p0 ).Length();
  }
  return length;
}
