//------------------------------------------------------------------------------
// aeMath.cpp
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
#include <cmath>
#include "aeMath.h"

//------------------------------------------------------------------------------
// aePlane member functions
//------------------------------------------------------------------------------
aePlane::aePlane( ae::Vec4 pointNormal ) :
  m_plane( pointNormal / pointNormal.GetXYZ().Length() ) // Normalize
{}

aePlane::aePlane( ae::Vec3 point, ae::Vec3 normal )
{
  m_plane = ae::Vec4( normal.NormalizeCopy(), 0.0f );
  m_plane.w = GetSignedDistance( point );
}

ae::Vec3 aePlane::GetNormal() const
{
  return m_plane.GetXYZ();
}

ae::Vec3 aePlane::GetClosestPointToOrigin() const
{
  return m_plane.GetXYZ() * m_plane.w;
}

bool aePlane::IntersectRay( ae::Vec3 pos, ae::Vec3 dir, float* tOut, ae::Vec3* out ) const
{
  dir.SafeNormalize();
  
  ae::Vec3 n = m_plane.GetXYZ();
  ae::Vec3 p = n * m_plane.w;

  float a = dir.Dot( n );
  if ( a > -0.01f )
  {
    // Ray is pointing away from or parallel to plane
    return false;
  }

  ae::Vec3 diff = pos - p;
  float b = diff.Dot( n );
  float c = b / a;

  if ( tOut )
  {
    *tOut = c;
  }
  if ( out )
  {
    *out = pos - dir * c;
  }
  return true;
}

ae::Vec3 aePlane::GetClosestPoint( ae::Vec3 pos, float* distanceOut ) const
{
  ae::Vec3 n = m_plane.GetXYZ();
  float t = pos.Dot( n ) - m_plane.w;
  if ( distanceOut )
  {
    *distanceOut = t;
  }
  return pos - n * t;
}

float aePlane::GetSignedDistance( ae::Vec3 pos ) const
{
   return pos.Dot( m_plane.GetXYZ() ) - m_plane.w;
}

//------------------------------------------------------------------------------
// aeLineSegment member functions
//------------------------------------------------------------------------------
aeLineSegment::aeLineSegment( ae::Vec3 p0, ae::Vec3 p1 )
{
  m_p0 = p0;
  m_p1 = p1;
}

float aeLineSegment::GetMinDistance( ae::Vec3 p, ae::Vec3* nearestOut ) const
{
  float lenSq = ( m_p1 - m_p0 ).LengthSquared();
  if ( lenSq <= 0.001f )
  {
    if ( nearestOut )
    {
      *nearestOut = m_p0;
    }
    return ( p - m_p0 ).Length();
  }

  float t = aeMath::Clip01( ( p - m_p0 ).Dot( m_p1 - m_p0 ) / lenSq );
  ae::Vec3 linePos = aeMath::Lerp( m_p0, m_p1, t );

  if ( nearestOut )
  {
    *nearestOut = linePos;
  }
  return ( p - linePos ).Length();
}

//------------------------------------------------------------------------------
// aeCircle member functions
//------------------------------------------------------------------------------
aeCircle::aeCircle( ae::Vec2 point, float radius )
{
  m_point = point;
  m_radius = radius;
}

float aeCircle::GetArea( float radius )
{
  return aeMath::PI * radius * radius;
}

bool aeCircle::Intersect( const aeCircle& other, ae::Vec2* out ) const
{
  ae::Vec2 diff = other.m_point - m_point;
  float dist = diff.Length();
  if ( dist > m_radius + other.m_radius )
  {
    return false;
  }

  if ( out )
  {
    *out = m_point + diff.SafeNormalizeCopy() * ( ( m_radius + dist - other.m_radius ) * 0.5f );
  }
  return true;
}

// Test if point P lies inside the counterclockwise 3D triangle ABC
bool PointInTriangle( ae::Vec3 p, ae::Vec3 a, ae::Vec3 b, ae::Vec3 c )
{
  // Translate point and triangle so that point lies at origin
  a -= p;
  b -= p;
  c -= p;
  float ab = a.Dot( b );
  float ac = a.Dot( c );
  float bc = b.Dot( c );
  float cc = c.Dot( c );
  // Make sure plane normals for pab and pbc point in the same direction
  if ( bc * ac - cc * ab < 0.0f )
  {
    return false;
  }
  // Make sure plane normals for pab and pca point in the same direction
  float bb = b.Dot( b );
  if ( ab * bc - ac * bb < 0.0f )
  {
    return false;
  }
  // Otherwise P must be in (or on) the triangle
  return true;
}

#include "aeRender.h"
bool Sphere_SweepTriangle( const ae::Sphere& sphere, ae::Vec3 direction, const ae::Vec3* points, ae::Vec3 normal,
  float* outNearestDistance,
  ae::Vec3* outNearestIntersectionPoint,
  ae::Vec3* outNearestPolygonIntersectionPoint, ae::DebugLines* debug )
{
  direction.SafeNormalize(); // @TODO: Make sure following logic is isn't limited by direction length

  // Plane origin/normal
  aePlane triPlane( points[ 0 ], normal );

  // Determine the distance from the plane to the source
  ae::Vec3 planeIntersectionPoint( 0.0f );
  ae::Vec3 planeIntersectionPoint2( 0.0f );
  //float pDist = intersect( pOrigin, normal, source, -normal );
  float pDist = aeMath::MaxValue< float >();
  if ( triPlane.IntersectRay( sphere.center, -normal, &pDist, &planeIntersectionPoint ) )
  {
    // @TODO: Should be able to remove this
    pDist = ( planeIntersectionPoint - sphere.center ).Length();
    debug->AddLine( sphere.center, sphere.center - normal, ae::Color::Red() );
    debug->AddSphere( planeIntersectionPoint, 0.05f, ae::Color::Red(), 8 );

    planeIntersectionPoint2 = planeIntersectionPoint;
  }

  // Is the source point behind the plane?
  // [note that you can remove this condition if your visuals are not using backface culling]
  if ( pDist < 0.0 )
  {
    return false;
  }

  // Is the plane embedded (i.e. within the distance of radius of the sphere)?
  if ( pDist <= sphere.radius )
  {
    // Calculate the plane intersection point
    // @TODO: Is this already calculated above?
    planeIntersectionPoint = sphere.center - normal * pDist;
  }
  else
  {
    ae::Vec3 sphereIntersectionPoint = sphere.center - normal * sphere.radius;
    if ( !triPlane.IntersectRay( sphereIntersectionPoint, direction, nullptr, &planeIntersectionPoint ) )
    {
      return false;
    }

    //debug->AddLine( sphereIntersectionPoint, planeIntersectionPoint, ae::Color::PicoOrange() );
  }

  // Unless otherwise noted, our polygonIntersectionPoint is the
  // same point as planeIntersectionPoint
  ae::Vec3 polygonIntersectionPoint = planeIntersectionPoint;
  // So� are they the same?
  // @TODO: Check edges
  //if ( planeIntersectionPoint is not within the current polygon )
  if ( !PointInTriangle( polygonIntersectionPoint, points[ 0 ], points[ 1 ], points[ 2 ] ) )
  {
    // polygonIntersectionPoint = nearest point on polygon's perimeter to planeIntersectionPoint;

    ae::Vec3 p0, p1;
    ae::Vec3 c0, c1, c2;
    float d0 = aeLineSegment( points[ 0 ], points[ 1 ] ).GetMinDistance( planeIntersectionPoint, &c0 );
    float d1 = aeLineSegment( points[ 1 ], points[ 2 ] ).GetMinDistance( planeIntersectionPoint, &c1 );
    float d2 = aeLineSegment( points[ 2 ], points[ 0 ] ).GetMinDistance( planeIntersectionPoint, &c2 );
    if ( d0 <= d1 && d0 <= d2 )
    {
      polygonIntersectionPoint = c0;
      p0 = points[ 0 ];
      p1 = points[ 1 ];
    }
    else if ( d1 <= d0 && d1 <= d2 )
    {
      polygonIntersectionPoint = c1;
      p0 = points[ 1 ];
      p1 = points[ 2 ];
    }
    else
    {
      polygonIntersectionPoint = c2;
      p0 = points[ 2 ];
      p1 = points[ 0 ];
    }

    //debug->AddLine( planeIntersectionPoint, polygonIntersectionPoint, ae::Color::Green() );
    //debug->AddSphere( planeIntersectionPoint + normal * radius, radius, ae::Color::Gray(), 16 );

    ae::Vec3 flatDir = direction.ZeroAxisCopy( normal );
    debug->AddLine( planeIntersectionPoint, planeIntersectionPoint2 + flatDir * 4.0f, ae::Color::Blue() );

    float minDist = aeMath::MaxValue< float >();
    ae::Vec3 theRealThingTM;
    ae::Vec3 edgeNormalMinTemp;
    //ae::Vec3 flatDir = direction.ZeroAxisCopy( normal );
    for ( uint32_t i = 0; i < 3; i++ )
    {
      p0 = points[ i ];
      p1 = points[ ( i + 1 ) % 3 ];

      ae::Vec3 edgeNormal = normal.Cross( p0 - p1 ).SafeNormalizeCopy();
      aePlane edgePlane( p0, edgeNormal ); //  + edgeNormal * radius

      float distance = 0.0f;
      ae::Vec3 testPoint = planeIntersectionPoint2;
      edgePlane.IntersectRay( planeIntersectionPoint2, flatDir, &distance, &testPoint );
      if ( minDist > distance )
      {
        minDist = distance;
        theRealThingTM = testPoint;
        edgeNormalMinTemp = edgeNormal;
      }
    }

    debug->AddLine( planeIntersectionPoint2, planeIntersectionPoint2 + flatDir * 4.0f, ae::Color::Blue() );
    debug->AddLine( theRealThingTM, theRealThingTM + edgeNormalMinTemp, ae::Color::Blue() );
    debug->AddSphere( theRealThingTM, 0.05f, ae::Color::Blue(), 16 );

    polygonIntersectionPoint = theRealThingTM;

    float circleRad = sphere.radius; // Incorrect! This circle is the 'slice of sphere' at the point of contact with the edge
    ae::Vec3 circlePos = theRealThingTM + edgeNormalMinTemp * circleRad;
    debug->AddCircle( circlePos, normal, circleRad, ae::Color::Blue(), 16 );
  }
  // Invert the velocity vector
  //ae::Vec3 negativeVelocityVector = -velocityVector;
  // Using the polygonIntersectionPoint, we need to reverse-intersect
  // with the sphere (note: the 1.0 below is the unit-sphere�s
  // radius)
  //float t = intersectSphere( sourcePoint, 1.0f, polygonIntersectionPoint, negativeVelocityVector );
  // Was there an intersection with the sphere?
  //if ( t >= 0.0 ) // && t <= distanceToTravel ) // No limit?
  float t = 0.0f;
  debug->AddLine( polygonIntersectionPoint, polygonIntersectionPoint - direction * 4.0f, ae::Color::Green() );
  if ( sphere.Raycast( polygonIntersectionPoint, -direction, &t ) )
  {
    if ( outNearestDistance )
    {
      *outNearestDistance = t;
    }
    if ( outNearestIntersectionPoint )
    {
      // Where did sphere intersected
      *outNearestIntersectionPoint = polygonIntersectionPoint - direction * t;
    }
    if ( outNearestPolygonIntersectionPoint )
    {
      *outNearestPolygonIntersectionPoint = polygonIntersectionPoint;
    }
    return true;
  }

  return false;
}

/*
bool aeSphere::SweepTriangle( ae::Vec3 direction, const ae::Vec3* points, ae::Vec3 normal,
  float* outNearestDistance,
  ae::Vec3* outNearestIntersectionPoint,
  ae::Vec3* outNearestPolygonIntersectionPoint, ae::DebugLines* debug ) const
{
  direction.SafeNormalize(); // @TODO: Make sure following logic is isn't limited by direction length

  // Plane origin/normal
  aePlane triPlane( points[ 0 ], normal );

  // Determine the distance from the plane to the source
  ae::Vec3 planeIntersectionPoint( 0.0f );
  ae::Vec3 planeIntersectionPoint2( 0.0f );
  //float pDist = intersect( pOrigin, normal, source, -normal );
  float pDist = aeMath::MaxValue< float >();
  if ( triPlane.IntersectRay( center, -normal, &pDist, &planeIntersectionPoint ) )
  {
    // @TODO: Should be able to remove this
    pDist = ( planeIntersectionPoint - center ).Length();
    debug->AddLine( center, center - normal, ae::Color::Red() );
    debug->AddSphere( planeIntersectionPoint, 0.05f, ae::Color::Red(), 8 );

    planeIntersectionPoint2 = planeIntersectionPoint;
  }

  // Is the source point behind the plane?
  // [note that you can remove this condition if your visuals are not using backface culling]
  if ( pDist < 0.0 )
  {
    return false;
  }

  // Is the plane embedded (i.e. within the distance of radius of the sphere)?
  if ( pDist <= radius )
  {
    // Calculate the plane intersection point
    // @TODO: Is this already calculated above?
    planeIntersectionPoint = center - normal * pDist;
  }
  else
  {
    ae::Vec3 sphereIntersectionPoint = center - normal * radius;
    if ( !triPlane.IntersectRay( sphereIntersectionPoint, direction, nullptr, &planeIntersectionPoint ) )
    {
      return false;
    }

    //debug->AddLine( sphereIntersectionPoint, planeIntersectionPoint, ae::Color::PicoOrange() );
  }

  // Unless otherwise noted, our polygonIntersectionPoint is the
  // same point as planeIntersectionPoint
  ae::Vec3 polygonIntersectionPoint = planeIntersectionPoint;
  // So� are they the same?
  // @TODO: Check edges
  //if ( planeIntersectionPoint is not within the current polygon )
  if ( !PointInTriangle( polygonIntersectionPoint, points[ 0 ], points[ 1 ], points[ 2 ] ) )
  {
    // polygonIntersectionPoint = nearest point on polygon's perimeter to planeIntersectionPoint;

    ae::Vec3 p0, p1;
    ae::Vec3 c0, c1, c2;
    float d0 = aeLineSegment( points[ 0 ], points[ 1 ] ).GetMinDistance( planeIntersectionPoint, &c0 );
    float d1 = aeLineSegment( points[ 1 ], points[ 2 ] ).GetMinDistance( planeIntersectionPoint, &c1 );
    float d2 = aeLineSegment( points[ 2 ], points[ 0 ] ).GetMinDistance( planeIntersectionPoint, &c2 );
    if ( d0 <= d1 && d0 <= d2 )
    {
      polygonIntersectionPoint = c0;
      p0 = points[ 0 ];
      p1 = points[ 1 ];
    }
    else if ( d1 <= d0 && d1 <= d2 )
    {
      polygonIntersectionPoint = c1;
      p0 = points[ 1 ];
      p1 = points[ 2 ];
    }
    else
    {
      polygonIntersectionPoint = c2;
      p0 = points[ 2 ];
      p1 = points[ 0 ];
    }

    //debug->AddLine( planeIntersectionPoint, polygonIntersectionPoint, ae::Color::Green() );
    //debug->AddSphere( planeIntersectionPoint + normal * radius, radius, ae::Color::Gray(), 16 );

    ae::Vec3 flatDir = ae::Vec3( direction ).ZeroAxis( normal );
    debug->AddLine( planeIntersectionPoint, planeIntersectionPoint2 + flatDir * 4.0f, ae::Color::Blue() );

    float minDist = aeMath::MaxValue< float >();
    ae::Vec3 theRealThingTM;
    ae::Vec3 edgeNormalMinTemp;
    //ae::Vec3 flatDir = ae::Vec3( direction ).ZeroAxis( normal );
    for ( uint32_t i = 0; i < 3; i++ )
    {
      p0 = points[ i ];
      p1 = points[ ( i + 1 ) % 3 ];

      ae::Vec3 edgeNormal = ( normal % ( p0 - p1 ) ).SafeNormalizeCopy();
      aePlane edgePlane( p0, edgeNormal ); //  + edgeNormal * radius

      float distance = 0.0f;
      ae::Vec3 testPoint = planeIntersectionPoint2;
      edgePlane.IntersectRay( planeIntersectionPoint2, flatDir, &distance, &testPoint );
      if ( minDist > distance )
      {
        minDist = distance;
        theRealThingTM = testPoint;
        edgeNormalMinTemp = edgeNormal;
      }
    }

    debug->AddLine( planeIntersectionPoint2, planeIntersectionPoint2 + flatDir * 4.0f, ae::Color::Blue() );
    debug->AddLine( theRealThingTM, theRealThingTM + edgeNormalMinTemp, ae::Color::Blue() );
    debug->AddSphere( theRealThingTM, 0.05f, ae::Color::Blue(), 16 );

    ////polygonIntersectionPoint = theRealThingTM;
  }
  // Invert the velocity vector
  //ae::Vec3 negativeVelocityVector = -velocityVector;
  // Using the polygonIntersectionPoint, we need to reverse-intersect
  // with the sphere (note: the 1.0 below is the unit-sphere�s
  // radius)
  //float t = intersectSphere( sourcePoint, 1.0f, polygonIntersectionPoint, negativeVelocityVector );
  // Was there an intersection with the sphere?
  //if ( t >= 0.0 ) // && t <= distanceToTravel ) // No limit?
  float t = 0.0f;
  debug->AddLine( polygonIntersectionPoint, polygonIntersectionPoint - direction * 4.0f, ae::Color::Green() );
  if ( Raycast( polygonIntersectionPoint, -direction, &t ) )
  {
    if ( outNearestDistance )
    {
      *outNearestDistance = t;
    }
    if ( outNearestIntersectionPoint )
    {
      // Where did sphere intersected
      *outNearestIntersectionPoint = polygonIntersectionPoint - direction * t;
    }
    if ( outNearestPolygonIntersectionPoint )
    {
      *outNearestPolygonIntersectionPoint = polygonIntersectionPoint;
    }
    return true;
  }

  return false;
}
*/

//------------------------------------------------------------------------------
// aeFrustum member functions
//------------------------------------------------------------------------------
aeFrustum::aeFrustum( ae::Matrix4 worldToProjection )
{
  ae::Vec4 row0 = worldToProjection.GetRow( 0 );
  ae::Vec4 row1 = worldToProjection.GetRow( 1 );
  ae::Vec4 row2 = worldToProjection.GetRow( 2 );
  ae::Vec4 row3 = worldToProjection.GetRow( 3 );

  ae::Vec4 near = -row0 - row3;
  ae::Vec4 far = row0 - row3;
  ae::Vec4 left = -row1 - row3;
  ae::Vec4 right = row1 - row3;
  ae::Vec4 top = -row2 - row3;
  ae::Vec4 bottom = row2 - row3;
  near.w = -near.w;
  far.w = -far.w;
  left.w = -left.w;
  right.w = -right.w;
  top.w = -top.w;
  bottom.w = -bottom.w;
  
  m_planes[ (int)aeFrustumPlane::Near ] = near;
  m_planes[ (int)aeFrustumPlane::Far ] = far;
  m_planes[ (int)aeFrustumPlane::Left ] = left;
  m_planes[ (int)aeFrustumPlane::Right ] = right;
  m_planes[ (int)aeFrustumPlane::Top ] = top;
  m_planes[ (int)aeFrustumPlane::Bottom ] = bottom;
}

bool aeFrustum::Intersects( ae::Vec3 point ) const
{
  for ( uint32_t i = 0; i < countof(m_planes); i++ )
  {
    if ( m_planes[ i ].GetSignedDistance( point ) > 0.0f )
    {
      return false;
    }
  }
  return true;
}

bool aeFrustum::Intersects( const ae::Sphere& sphere ) const
{
  for( int i = 0; i < countof(m_planes); i++ )
  {
    float distance = m_planes[ i ].GetSignedDistance( sphere.center );
    if( distance > 0.0f && distance - sphere.radius > 0.0f ) 
    {
      return false;
    }
  }
  return true;
}

aePlane aeFrustum::GetPlane( aeFrustumPlane plane ) const
{
  return m_planes[ (int)plane ];
}
