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
aeSpline::aeSpline( ae::Tag tag ) :
  m_controlPoints( tag ),
  m_segments( tag )
{}

aeSpline::aeSpline( ae::Tag tag, ae::Vec3* controlPoints, uint32_t count ) :
  m_controlPoints( tag ),
  m_segments( tag )
{
  for ( uint32_t i = 0; i < count; i++ )
  {
    m_controlPoints.Append( controlPoints[ i ] );
  }
}

void aeSpline::AppendControlPoint( ae::Vec3 p )
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

ae::Vec3 aeSpline::GetControlPoint( uint32_t index ) const
{
  return m_controlPoints[ index ];
}

uint32_t aeSpline::GetControlPointCount() const
{
  return m_controlPoints.Length();
}

ae::Vec3 aeSpline::GetPoint( float distance ) const
{
  if ( m_controlPoints.Length() == 0 )
  {
    return ae::Vec3( 0.0f );
  }
  else if ( m_controlPoints.Length() == 1 )
  {
    return m_controlPoints[ 0 ];
  }

  if ( m_length < 0.001f )
  {
    distance = 0.0f;
  }
  else if ( m_loop && ( distance < 0.0f || distance >= m_length ) )
  {
    distance = aeMath::Mod( distance, m_length );
  }

  for ( uint32_t i = 0; i < m_segments.Length(); i++ )
  {
    const Segment& segment = m_segments[ i ];
    if ( segment.GetLength() >= distance )
    {
      return segment.GetPoint( distance );
    }

    distance -= segment.GetLength();
  }

  return m_controlPoints[ m_controlPoints.Length() - 1 ];
}

float aeSpline::GetMinDistance( ae::Vec3 p, ae::Vec3* nearestOut )
{
  ae::Vec3 closest( 0.0f );
  float closestDistance = aeMath::MaxValue< float >();

  for ( uint32_t i = 0; i < m_segments.Length(); i++ )
  {
    const Segment& segment = m_segments[ i ];

    if ( segment.GetAABB().GetSignedDistanceFromSurface( p ) > closestDistance )
    {
      // @NOTE: Don't check segments that are further away than the already closest point
      continue;
    }

    ae::Vec3 segmentP;
    float d = segment.GetMinDistance( p, &segmentP );
    if ( d < closestDistance )
    {
      closest = segmentP;
      closestDistance = d;
    }
  }

  if ( nearestOut )
  {
    *nearestOut = closest;
  }
  return closestDistance;
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

  m_aabb = ae::AABB( ae::Vec3( aeMath::MaxValue< float >() ), ae::Vec3( aeMath::MinValue< float >() ) );

  for ( int32_t i = 0; i < segmentCount; i++ )
  {
    ae::Vec3 p0 = m_GetControlPoint( i - 1 );
    ae::Vec3 p1 = m_GetControlPoint( i );
    ae::Vec3 p2 = m_GetControlPoint( i + 1 );
    ae::Vec3 p3 = m_GetControlPoint( i + 2 );

    Segment* segment = &m_segments.Append( Segment() );
    segment->Init( p0, p1, p2, p3 );
    
    m_length += segment->GetLength();
    m_aabb.Expand( segment->GetAABB() );
  }
}

ae::Vec3 aeSpline::m_GetControlPoint( int32_t index ) const
{
  if ( m_loop )
  {
    return m_controlPoints[ aeMath::Mod( index, (int)m_controlPoints.Length() ) ];
  }
  else if ( index == -1 )
  {
    ae::Vec3 p0 = m_controlPoints[ 0 ];
    ae::Vec3 p1 = m_controlPoints[ 1 ];
    return ( p0 + p0 - p1 );
  }
  else if ( index == m_controlPoints.Length() )
  {
    ae::Vec3 p0 = m_controlPoints[ index - 2 ];
    ae::Vec3 p1 = m_controlPoints[ index - 1 ];
    return ( p1 + p1 - p0 );
  }
  else
  {
    return m_controlPoints[ index ];
  }
}

void aeSpline::Segment::Init( ae::Vec3 p0, ae::Vec3 p1, ae::Vec3 p2, ae::Vec3 p3 )
{
  const float alpha = 0.5f;
  const float tension = 0.0f;

  float t01 = pow( ( p0 - p1 ).Length(), alpha );
  float t12 = pow( ( p1 - p2 ).Length(), alpha );
  float t23 = pow( ( p2 - p3 ).Length(), alpha );

  ae::Vec3 m1 = ( p2 - p1 + ( ( p1 - p0 ) / t01 - ( p2 - p0 ) / ( t01 + t12 ) ) * t12 ) * ( 1.0f - tension );
  ae::Vec3 m2 = ( p2 - p1 + ( ( p3 - p2 ) / t23 - ( p3 - p1 ) / ( t12 + t23 ) ) * t12 ) * ( 1.0f - tension );

  m_a = ( p1 - p2 ) * 2.0f + m1 + m2;
  m_b = ( p1 - p2 ) * -3.0f - m1 - m1 - m2;
  m_c = m1;
  m_d = p1;

  m_length = ( p2 - p1 ).Length();
  m_resolution = 1;

  float nextLength = m_length;
  uint32_t nextResolution = m_resolution;
  do
  {
    m_aabb = ae::AABB( GetPoint0(), GetPoint0() );

    m_length = nextLength;
    m_resolution = nextResolution;

    nextResolution = m_resolution * 2;
    nextLength = 0.0f;
    for ( uint32_t i = 0; i < nextResolution; i++ )
    {
      ae::Vec3 s0 = GetPoint01( i / (float)nextResolution );
      ae::Vec3 s1 = GetPoint01( ( i + 1 ) / (float)nextResolution );
      nextLength += ( s1 - s0 ).Length();

      m_aabb.Expand( s1 );
    }
  } while ( aeMath::Abs( nextLength - m_length ) > 0.001f );
}

ae::Vec3 aeSpline::Segment::GetPoint01( float t ) const
{
  return ( m_a * t * t * t ) + ( m_b * t * t ) + ( m_c * t ) + m_d;
}

ae::Vec3 aeSpline::Segment::GetPoint0() const
{
  return m_d;
}

ae::Vec3 aeSpline::Segment::GetPoint1() const
{
  return m_a + m_b + m_c + m_d;
}

ae::Vec3 aeSpline::Segment::GetPoint( float d ) const
{
  if ( d <= 0.0f )
  {
    return GetPoint0();
  }
  else if ( d < m_length )
  {
    // @NOTE: Search is required here because even within a segment
    //        t (0-1) does not map linearly to arc length. This is
    //        an approximate mapping from arc length -> t based on
    //        the optimized resolution value calculated above.
    for ( uint32_t i = 0; i < m_resolution; i++ )
    {
      ae::Vec3 s0 = GetPoint01( i / (float)m_resolution );
      ae::Vec3 s1 = GetPoint01( ( i + 1 ) / (float)m_resolution );
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
  }
  
  return GetPoint1();
}

float aeSpline::Segment::GetMinDistance( ae::Vec3 p, ae::Vec3* pOut ) const
{
  uint32_t closestIndex = 0;
  ae::Vec3 closest = GetPoint0();
  float closestDistSq = aeMath::MaxValue< float >();
  for ( uint32_t i = 0; i < m_resolution; i++ )
  {
    ae::Vec3 s = GetPoint01( i / (float)m_resolution );
    float d = ( s - p ).LengthSquared();
    if ( d < closestDistSq )
    {
      closestIndex = i;
      closest = s;
      closestDistSq = d;
    }
  }

  ae::Vec3 other;
  if ( closestIndex == 0 )
  {
    other = GetPoint01( 1 / (float)m_resolution );
  }
  else if ( closestIndex == m_resolution )
  {
    other = GetPoint01( ( m_resolution - 1 ) / (float)m_resolution );
  }
  else
  {
    ae::Vec3 prev = GetPoint01( ( closestIndex - 1 ) / (float)m_resolution );
    ae::Vec3 next = GetPoint01( ( closestIndex + 1 ) / (float)m_resolution );
    float prevDist = ( prev - p ).LengthSquared();
    float nextDist = ( next - p ).LengthSquared();
    if ( prevDist < nextDist )
    {
      other = prev;
    }
    else
    {
      other = next;
    }
  }

  return aeLineSegment( closest, other ).GetMinDistance( p, pOut );
}
