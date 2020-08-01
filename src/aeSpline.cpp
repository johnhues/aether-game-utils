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
  m_RecalculateSegments();
}

void aeSpline::RemoveControlPoint( uint32_t index )
{
  m_controlPoints.Remove( index );
  m_RecalculateSegments();
}

void aeSpline::SetLooping( bool enabled )
{
  if ( m_loop != enabled )
  {
    m_loop = enabled;
    m_RecalculateSegments();
  }
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

  if ( m_length < 0.001f )
  {
    distance = 0.0f;
  }
  else if ( m_loop && distance >= m_length )
  {
    distance = aeMath::Mod( distance, m_length );
  }

  for ( uint32_t i = 0; i < m_segments.Length(); i++ )
  {
    const Segment& segment = m_segments[ i ];
    if ( segment.length >= distance )
    {
      return segment.GetPoint( distance );
    }

    distance -= segment.length;
  }

  return m_controlPoints[ m_controlPoints.Length() - 1 ];
}

float aeSpline::GetLength() const
{
  return m_length;
}

void aeSpline::m_RecalculateSegments()
{
  m_segments.Clear();
  m_length = 0.0f;

  if ( m_controlPoints.Length() < 2 )
  {
    return;
  }

  int32_t segmentCount = m_controlPoints.Length();
  if ( !m_loop )
  {
    segmentCount--;
  }
  for ( int32_t i = 0; i < segmentCount; i++ )
  {
    aeFloat3 p0 = m_GetControlPoint( i - 1 );
    aeFloat3 p1 = m_GetControlPoint( i );
    aeFloat3 p2 = m_GetControlPoint( i + 1 );
    aeFloat3 p3 = m_GetControlPoint( i + 2 );

    Segment* segment = &m_segments.Append( Segment() );
    segment->Init( p0, p1, p2, p3 );
    
    m_length += segment->length;
  }
}

aeFloat3 aeSpline::m_GetControlPoint( int32_t index ) const
{
  if ( m_loop )
  {
    return m_controlPoints[ aeMath::Mod( index, (int)m_controlPoints.Length() ) ];
  }
  else if ( index == -1 )
  {
    aeFloat3 p0 = m_controlPoints[ 0 ];
    aeFloat3 p1 = m_controlPoints[ 1 ];
    return ( p0 + p0 - p1 );
  }
  else if ( index == m_controlPoints.Length() )
  {
    aeFloat3 p0 = m_controlPoints[ index - 2 ];
    aeFloat3 p1 = m_controlPoints[ index - 1 ];
    return ( p1 + p1 - p0 );
  }
  else
  {
    return m_controlPoints[ index ];
  }
}

void aeSpline::Segment::Init( aeFloat3 p0, aeFloat3 p1, aeFloat3 p2, aeFloat3 p3 )
{
  const float alpha = 0.5f;
  const float tension = 0.0f;

  float t01 = pow( ( p0 - p1 ).Length(), alpha );
  float t12 = pow( ( p1 - p2 ).Length(), alpha );
  float t23 = pow( ( p2 - p3 ).Length(), alpha );

  aeFloat3 m1 = ( p2 - p1 + ( ( p1 - p0 ) / t01 - ( p2 - p0 ) / ( t01 + t12 ) ) * t12 ) * ( 1.0f - tension );
  aeFloat3 m2 = ( p2 - p1 + ( ( p3 - p2 ) / t23 - ( p3 - p1 ) / ( t12 + t23 ) ) * t12 ) * ( 1.0f - tension );

  a = ( p1 - p2 ) * 2.0f + m1 + m2;
  b = ( p1 - p2 ) * -3.0f - m1 - m1 - m2;
  c = m1;
  d = p1;

  length = ( p2 - p1 ).Length();
  resolution = 1;

  float nextLength = length;
  uint32_t nextResolution = resolution;
  do
  {
    length = nextLength;
    resolution = nextResolution;

    nextResolution = resolution * 2;
    nextLength = 0.0f;
    for ( uint32_t i = 0; i < nextResolution; i++ )
    {
      aeFloat3 s0 = GetPoint01( i / (float)nextResolution );
      aeFloat3 s1 = GetPoint01( ( i + 1 ) / (float)nextResolution );
      nextLength += ( s1 - s0 ).Length();
    }
  } while ( aeMath::Abs( nextLength - length ) > 0.001f );
}

aeFloat3 aeSpline::Segment::GetPoint01( float t ) const
{
  return ( a * t * t * t ) + ( b * t * t ) + ( c * t ) + d;
}

aeFloat3 aeSpline::Segment::GetPoint( float d ) const
{
  if ( d <= 0.0f )
  {
    return GetPoint01( 0.0f );
  }

  for ( uint32_t i = 0; i < resolution; i++ )
  {
    aeFloat3 s0 = GetPoint01( i / (float)resolution );
    aeFloat3 s1 = GetPoint01( ( i + 1 ) / (float)resolution );
    float l = ( s1 - s0 ).Length();
    if ( l >= d )
    {
      return aeMath::Lerp( s0, s1, d / l );
    }
    else
    {
      d -= l;
    }
  }

  return GetPoint01( 1.0f );
}
