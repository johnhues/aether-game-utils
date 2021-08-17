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
// aeRect member functions
//------------------------------------------------------------------------------
aeRect::aeRect( aeFloat2 p0, aeFloat2 p1 )
{
  if ( p0.x < p1.x )
  {
    x = p0.x;
    w = p1.x - p0.x;
  }
  else
  {
    x = p1.x;
    w = p0.x - p1.x;
  }

  if ( p0.y < p1.y )
  {
    y = p0.y;
    h = p1.y - p0.y;
  }
  else
  {
    y = p1.y;
    h = p0.y - p1.y;
  }
}

aeRect aeRect::Zero()
{
  return aeRect( 0.0f, 0.0f, 0.0f, 0.0f );
}

bool aeRect::Contains( aeFloat2 pos ) const
{
  return !( pos.x < x || pos.x >= ( x + w ) || pos.y < y || pos.y >= ( y + h ) );
}

void aeRect::Expand( aeFloat2 pos )
{
  if ( w == 0.0f )
  {
    x = pos.x;
  }
  else
  {
    float x1 = aeMath::Max( x + w, pos.x );
    x = aeMath::Min( x, pos.x );
    w = x1 - x;
  }

  if ( h == 0.0f )
  {
    y = pos.y;
  }
  else
  {
    float y1 = aeMath::Max( y + h, pos.y );
    y = aeMath::Min( y, pos.y );
    h = y1 - y;
  }
}

bool aeRect::GetIntersection( const aeRect& other, aeRect* intersectionOut ) const
{
  float x0 = aeMath::Max( x, other.x );
  float x1 = aeMath::Min( x + w, other.x + other.w );
  float y0 = aeMath::Max( y, other.y );
  float y1 = aeMath::Min( y + h, other.y + other.h );
  if ( x0 < x1 && y0 < y1 )
  {
    if ( intersectionOut )
    {
      *intersectionOut = aeRect( aeFloat2( x0, y0 ), aeFloat2( x1, y1 ) );
    }
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------------------
// aeRectInt member functions
//------------------------------------------------------------------------------
aeRectInt aeRectInt::Zero()
{
  return aeRectInt( 0, 0, 0, 0 );
}

bool aeRectInt::Contains( aeInt2 pos ) const
{
  return !( pos.x < x || pos.x >= ( x + w ) || pos.y < y || pos.y >= ( y + h ) );
}

bool aeRectInt::Intersects( aeRectInt o ) const
{
  return !( o.x + o.w <= x || x + w <= o.x // No horizontal intersection
    || o.y + o.h <= y || y + h <= o.y ); // No vertical intersection
}

void aeRectInt::Expand( aeInt2 pos )
{
  if ( w == 0 )
  {
    x = pos.x;
    w = 1;
  }
  else
  {
    // @NOTE: One past input value to expand width by one column
    int x1 = aeMath::Max( x + w, pos.x + 1 );
    x = aeMath::Min( x, pos.x );
    w = x1 - x;
  }

  if ( h == 0 )
  {
    y = pos.y;
    h = 1;
  }
  else
  {
    // @NOTE: One past input value to expand width by one row
    int y1 = aeMath::Max( y + h, pos.y + 1 );
    y = aeMath::Min( y, pos.y );
    h = y1 - y;
  }
}

//------------------------------------------------------------------------------
// aePlane member functions
//------------------------------------------------------------------------------
aePlane::aePlane( aeFloat4 pointNormal ) :
  m_plane( pointNormal / pointNormal.GetXYZ().Length() ) // Normalize
{}

aePlane::aePlane( aeFloat3 point, aeFloat3 normal )
{
  m_plane = aeFloat4( normal.NormalizeCopy(), 0.0f );
  m_plane.w = GetSignedDistance( point );
}

aeFloat3 aePlane::GetNormal() const
{
  return m_plane.GetXYZ();
}

aeFloat3 aePlane::GetClosestPointToOrigin() const
{
  return m_plane.GetXYZ() * m_plane.w;
}

bool aePlane::IntersectRay( aeFloat3 pos, aeFloat3 dir, float* tOut, aeFloat3* out ) const
{
  dir.SafeNormalize();
  
  aeFloat3 n = m_plane.GetXYZ();
  aeFloat3 p = n * m_plane.w;

  float a = dir.Dot( n );
  if ( a > -0.01f )
  {
    // Ray is pointing away from or parallel to plane
    return false;
  }

  aeFloat3 diff = pos - p;
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

aeFloat3 aePlane::GetClosestPoint( aeFloat3 pos, float* distanceOut ) const
{
  aeFloat3 n = m_plane.GetXYZ();
  float t = pos.Dot( n ) - m_plane.w;
  if ( distanceOut )
  {
    *distanceOut = t;
  }
  return pos - n * t;
}

float aePlane::GetSignedDistance( aeFloat3 pos ) const
{
   return pos.Dot( m_plane.GetXYZ() ) - m_plane.w;
}

//------------------------------------------------------------------------------
// aeLineSegment member functions
//------------------------------------------------------------------------------
aeLineSegment::aeLineSegment( aeFloat3 p0, aeFloat3 p1 )
{
  m_p0 = p0;
  m_p1 = p1;
}

float aeLineSegment::GetMinDistance( aeFloat3 p, aeFloat3* nearestOut ) const
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
  aeFloat3 linePos = aeMath::Lerp( m_p0, m_p1, t );

  if ( nearestOut )
  {
    *nearestOut = linePos;
  }
  return ( p - linePos ).Length();
}

//------------------------------------------------------------------------------
// aeCircle member functions
//------------------------------------------------------------------------------
aeCircle::aeCircle( aeFloat2 point, float radius )
{
  m_point = point;
  m_radius = radius;
}

float aeCircle::GetArea( float radius )
{
  return aeMath::PI * radius * radius;
}

bool aeCircle::Intersect( const aeCircle& other, aeFloat2* out ) const
{
  aeFloat2 diff = other.m_point - m_point;
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

//------------------------------------------------------------------------------
// aeSphere member functions
//------------------------------------------------------------------------------
bool aeSphere::Raycast( aeFloat3 origin, aeFloat3 direction, float* tOut, aeFloat3* pOut ) const
{
  direction.SafeNormalize();

  aeFloat3 m = origin - center;
  float b = m.Dot( direction );
  float c = m.Dot( m ) - radius * radius;
  // Exit if r�s origin outside s (c > 0) and r pointing away from s (b > 0)
  if ( c > 0.0f && b > 0.0f )
  {
    return false;
  }

  // A negative discriminant corresponds to ray missing sphere
  float discr = b * b - c;
  if ( discr < 0.0f )
  {
    return false;
  }

  // Ray now found to intersect sphere, compute smallest t value of intersection
  float t = -b - sqrtf( discr );
  // If t is negative, ray started inside sphere so clamp t to zero
  if ( t < 0.0f )
  {
    t = 0.0f;
  }

  if ( tOut )
  {
    *tOut = t;
  }

  if ( pOut )
  {
    *pOut = origin + direction * t;
  }

  return true;
}

// Test if point P lies inside the counterclockwise 3D triangle ABC
bool PointInTriangle( aeFloat3 p, aeFloat3 a, aeFloat3 b, aeFloat3 c )
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
bool aeSphere::SweepTriangle( aeFloat3 direction, const aeFloat3* points, aeFloat3 normal,
  float* outNearestDistance,
  aeFloat3* outNearestIntersectionPoint,
  aeFloat3* outNearestPolygonIntersectionPoint, aeDebugRender* debug ) const
{
  direction.SafeNormalize(); // @TODO: Make sure following logic is isn't limited by direction length

  // Plane origin/normal
  aePlane triPlane( points[ 0 ], normal );

  // Determine the distance from the plane to the source
  aeFloat3 planeIntersectionPoint( 0.0f );
  aeFloat3 planeIntersectionPoint2( 0.0f );
  //float pDist = intersect( pOrigin, normal, source, -normal );
  float pDist = aeMath::MaxValue< float >();
  if ( triPlane.IntersectRay( center, -normal, &pDist, &planeIntersectionPoint ) )
  {
    // @TODO: Should be able to remove this
    pDist = ( planeIntersectionPoint - center ).Length();
    debug->AddLine( center, center - normal, aeColor::Red() );
    debug->AddSphere( planeIntersectionPoint, 0.05f, aeColor::Red(), 8 );

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
    aeFloat3 sphereIntersectionPoint = center - normal * radius;
    if ( !triPlane.IntersectRay( sphereIntersectionPoint, direction, nullptr, &planeIntersectionPoint ) )
    {
      return false;
    }

    //debug->AddLine( sphereIntersectionPoint, planeIntersectionPoint, aeColor::PicoOrange() );
  }

  // Unless otherwise noted, our polygonIntersectionPoint is the
  // same point as planeIntersectionPoint
  aeFloat3 polygonIntersectionPoint = planeIntersectionPoint;
  // So� are they the same?
  // @TODO: Check edges
  //if ( planeIntersectionPoint is not within the current polygon )
  if ( !PointInTriangle( polygonIntersectionPoint, points[ 0 ], points[ 1 ], points[ 2 ] ) )
  {
    // polygonIntersectionPoint = nearest point on polygon's perimeter to planeIntersectionPoint;

    aeFloat3 p0, p1;
    aeFloat3 c0, c1, c2;
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

    //debug->AddLine( planeIntersectionPoint, polygonIntersectionPoint, aeColor::Green() );
    //debug->AddSphere( planeIntersectionPoint + normal * radius, radius, aeColor::Gray(), 16 );

    aeFloat3 flatDir = direction.ZeroAxisCopy( normal );
    debug->AddLine( planeIntersectionPoint, planeIntersectionPoint2 + flatDir * 4.0f, aeColor::Blue() );

    float minDist = aeMath::MaxValue< float >();
    aeFloat3 theRealThingTM;
    aeFloat3 edgeNormalMinTemp;
    //aeFloat3 flatDir = direction.ZeroAxisCopy( normal );
    for ( uint32_t i = 0; i < 3; i++ )
    {
      p0 = points[ i ];
      p1 = points[ ( i + 1 ) % 3 ];

      aeFloat3 edgeNormal = normal.Cross( p0 - p1 ).SafeNormalizeCopy();
      aePlane edgePlane( p0, edgeNormal ); //  + edgeNormal * radius

      float distance = 0.0f;
      aeFloat3 testPoint = planeIntersectionPoint2;
      edgePlane.IntersectRay( planeIntersectionPoint2, flatDir, &distance, &testPoint );
      if ( minDist > distance )
      {
        minDist = distance;
        theRealThingTM = testPoint;
        edgeNormalMinTemp = edgeNormal;
      }
    }

    debug->AddLine( planeIntersectionPoint2, planeIntersectionPoint2 + flatDir * 4.0f, aeColor::Blue() );
    debug->AddLine( theRealThingTM, theRealThingTM + edgeNormalMinTemp, aeColor::Blue() );
    debug->AddSphere( theRealThingTM, 0.05f, aeColor::Blue(), 16 );

    polygonIntersectionPoint = theRealThingTM;

    float circleRad = radius; // Incorrect! This circle is the 'slice of sphere' at the point of contact with the edge
    aeFloat3 circlePos = theRealThingTM + edgeNormalMinTemp * circleRad;
    debug->AddCircle( circlePos, normal, circleRad, aeColor::Blue(), 16 );
  }
  // Invert the velocity vector
  //aeFloat3 negativeVelocityVector = -velocityVector;
  // Using the polygonIntersectionPoint, we need to reverse-intersect
  // with the sphere (note: the 1.0 below is the unit-sphere�s
  // radius)
  //float t = intersectSphere( sourcePoint, 1.0f, polygonIntersectionPoint, negativeVelocityVector );
  // Was there an intersection with the sphere?
  //if ( t >= 0.0 ) // && t <= distanceToTravel ) // No limit?
  float t = 0.0f;
  debug->AddLine( polygonIntersectionPoint, polygonIntersectionPoint - direction * 4.0f, aeColor::Green() );
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

/*
bool aeSphere::SweepTriangle( aeFloat3 direction, const aeFloat3* points, aeFloat3 normal,
  float* outNearestDistance,
  aeFloat3* outNearestIntersectionPoint,
  aeFloat3* outNearestPolygonIntersectionPoint, aeDebugRender* debug ) const
{
  direction.SafeNormalize(); // @TODO: Make sure following logic is isn't limited by direction length

  // Plane origin/normal
  aePlane triPlane( points[ 0 ], normal );

  // Determine the distance from the plane to the source
  aeFloat3 planeIntersectionPoint( 0.0f );
  aeFloat3 planeIntersectionPoint2( 0.0f );
  //float pDist = intersect( pOrigin, normal, source, -normal );
  float pDist = aeMath::MaxValue< float >();
  if ( triPlane.IntersectRay( center, -normal, &pDist, &planeIntersectionPoint ) )
  {
    // @TODO: Should be able to remove this
    pDist = ( planeIntersectionPoint - center ).Length();
    debug->AddLine( center, center - normal, aeColor::Red() );
    debug->AddSphere( planeIntersectionPoint, 0.05f, aeColor::Red(), 8 );

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
    aeFloat3 sphereIntersectionPoint = center - normal * radius;
    if ( !triPlane.IntersectRay( sphereIntersectionPoint, direction, nullptr, &planeIntersectionPoint ) )
    {
      return false;
    }

    //debug->AddLine( sphereIntersectionPoint, planeIntersectionPoint, aeColor::PicoOrange() );
  }

  // Unless otherwise noted, our polygonIntersectionPoint is the
  // same point as planeIntersectionPoint
  aeFloat3 polygonIntersectionPoint = planeIntersectionPoint;
  // So� are they the same?
  // @TODO: Check edges
  //if ( planeIntersectionPoint is not within the current polygon )
  if ( !PointInTriangle( polygonIntersectionPoint, points[ 0 ], points[ 1 ], points[ 2 ] ) )
  {
    // polygonIntersectionPoint = nearest point on polygon's perimeter to planeIntersectionPoint;

    aeFloat3 p0, p1;
    aeFloat3 c0, c1, c2;
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

    //debug->AddLine( planeIntersectionPoint, polygonIntersectionPoint, aeColor::Green() );
    //debug->AddSphere( planeIntersectionPoint + normal * radius, radius, aeColor::Gray(), 16 );

    aeFloat3 flatDir = aeFloat3( direction ).ZeroAxis( normal );
    debug->AddLine( planeIntersectionPoint, planeIntersectionPoint2 + flatDir * 4.0f, aeColor::Blue() );

    float minDist = aeMath::MaxValue< float >();
    aeFloat3 theRealThingTM;
    aeFloat3 edgeNormalMinTemp;
    //aeFloat3 flatDir = aeFloat3( direction ).ZeroAxis( normal );
    for ( uint32_t i = 0; i < 3; i++ )
    {
      p0 = points[ i ];
      p1 = points[ ( i + 1 ) % 3 ];

      aeFloat3 edgeNormal = ( normal % ( p0 - p1 ) ).SafeNormalizeCopy();
      aePlane edgePlane( p0, edgeNormal ); //  + edgeNormal * radius

      float distance = 0.0f;
      aeFloat3 testPoint = planeIntersectionPoint2;
      edgePlane.IntersectRay( planeIntersectionPoint2, flatDir, &distance, &testPoint );
      if ( minDist > distance )
      {
        minDist = distance;
        theRealThingTM = testPoint;
        edgeNormalMinTemp = edgeNormal;
      }
    }

    debug->AddLine( planeIntersectionPoint2, planeIntersectionPoint2 + flatDir * 4.0f, aeColor::Blue() );
    debug->AddLine( theRealThingTM, theRealThingTM + edgeNormalMinTemp, aeColor::Blue() );
    debug->AddSphere( theRealThingTM, 0.05f, aeColor::Blue(), 16 );

    ////polygonIntersectionPoint = theRealThingTM;
  }
  // Invert the velocity vector
  //aeFloat3 negativeVelocityVector = -velocityVector;
  // Using the polygonIntersectionPoint, we need to reverse-intersect
  // with the sphere (note: the 1.0 below is the unit-sphere�s
  // radius)
  //float t = intersectSphere( sourcePoint, 1.0f, polygonIntersectionPoint, negativeVelocityVector );
  // Was there an intersection with the sphere?
  //if ( t >= 0.0 ) // && t <= distanceToTravel ) // No limit?
  float t = 0.0f;
  debug->AddLine( polygonIntersectionPoint, polygonIntersectionPoint - direction * 4.0f, aeColor::Green() );
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

// 5.1 Closest-point Computations 139 and, in fact, be the orthogonal projection R, which can now be easily computed per the preceding. This information is now enough to produce a code solution.
aeFloat3 ClosestPtPointTriangle(aeFloat3 p, aeFloat3 a, aeFloat3 b, aeFloat3 c)
{
  aeFloat3 ab = b - a;
  aeFloat3 ac = c - a;
  aeFloat3 bc = c - b;
  
  // Compute parametric position s for projection P’ of P on AB,
  // P’ = A + s*AB, s = snom/(snom+sdenom)
  float snom = (p - a).Dot( ab );
  float sdenom = (p - b).Dot(a - b);
  
  // Compute parametric position t for projection P’ of P on AC,
  // P’ = A + t*AC, s = tnom/(tnom+tdenom)
  float tnom = (p - a).Dot( ac );
  float tdenom = (p - c).Dot( a - c);
  if (snom <= 0.0f && tnom <= 0.0f) return a; // Vertex region early out
  
  // Compute parametric position u for projection P’ of P on BC,
  // P’ = B + u*BC, u = unom/(unom+udenom)
  float unom = (p - b).Dot( bc ), udenom = (p - c).Dot(b - c);
  if (sdenom <= 0.0f && unom <= 0.0f) return b; // Vertex region early out
  if (tdenom <= 0.0f && udenom <= 0.0f) return c; // Vertex region early out
  
  // P is outside (or on) AB if the triple scalar product [N PA PB] <= 0
  aeFloat3 n = (b - a).Cross(c - a);
  float vc = n.Dot((a - p).Cross(b - p));
  // If P outside AB and within feature region of AB,
  // return projection of P onto AB
  if (vc <= 0.0f && snom >= 0.0f && sdenom >= 0.0f)
      return a + snom / (snom + sdenom) * ab;
  
  // P is outside (or on) BC if the triple scalar product [N PB PC] <= 0
  float va = n.Dot((b - p).Cross(c - p));
  // If P outside BC and within feature region of BC,
  // return projection of P onto BC
  if (va <= 0.0f && unom >= 0.0f && udenom >= 0.0f)
    return b + unom / (unom + udenom) * bc;
  
  // P is outside (or on) CA if the triple scalar product [N PC PA] <= 0
  float vb = n.Dot((c - p).Cross(a - p));
  // If P outside CA and within feature region of CA,
  // return projection of P onto CA
  if (vb <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f)
      return a + tnom / (tnom + tdenom) * ac;

  // P must project inside face region. Compute Q using barycentric coordinates
  float u = va / (va + vb + vc);
  float v = vb / (va + vb + vc);
  float w=1.0f-u-v; //=vc/(va+vb+vc)
  return u * a + v * b + w * c;
}

bool aeSphere::IntersectTriangle( aeFloat3 t0, aeFloat3 t1, aeFloat3 t2, aeFloat3* outNearestIntersectionPoint ) const
{
  aeFloat3 closest = ClosestPtPointTriangle( center, t0, t1, t2 );
  if ( ( closest - center ).LengthSquared() <= radius * radius )
  {
    if ( outNearestIntersectionPoint )
    {
      *outNearestIntersectionPoint = closest;
    }
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
// aeAABB member functions
//------------------------------------------------------------------------------
aeAABB::aeAABB( aeFloat3 p0, aeFloat3 p1 )
{
  m_min = aeMath::Min( p0, p1 );
  m_max = aeMath::Max( p0, p1 );
}

aeAABB::aeAABB( const aeSphere& sphere )
{
  aeFloat3 r( sphere.radius );
  m_min = sphere.center - r;
  m_max = sphere.center + r;
}

void aeAABB::Expand( aeFloat3 p )
{
  m_min = aeMath::Min( p, m_min );
  m_max = aeMath::Max( p, m_max );
}

void aeAABB::Expand( aeAABB other )
{
  m_min = aeMath::Min( other.m_min, m_min );
  m_max = aeMath::Max( other.m_max, m_max );
}

void aeAABB::Expand( float boundary )
{
  m_min -= aeFloat3( boundary );
  m_max += aeFloat3( boundary );
}

aeFloat4x4 aeAABB::GetTransform() const
{
  return aeFloat4x4::Translation( GetCenter() ) * aeFloat4x4::Scaling( m_max - m_min );
}

float aeAABB::GetMinDistance( aeFloat3 p ) const
{
  aeFloat3 center = GetCenter();
  aeFloat3 halfSize = GetHalfSize();

  aeFloat3 d = p - center;
  d.x = aeMath::Max( aeMath::Abs( d.x ) - halfSize.x, 0.0f );
  d.y = aeMath::Max( aeMath::Abs( d.y ) - halfSize.y, 0.0f );
  d.z = aeMath::Max( aeMath::Abs( d.z ) - halfSize.z, 0.0f );

  return d.Length();
}

bool aeAABB::Intersect( aeAABB other ) const
{
  if ( m_max.x >= other.m_min.x && m_max.y >= other.m_min.y && m_max.z >= other.m_min.z )
  {
    return true;
  }
  else if ( other.m_max.x >= m_min.x && other.m_max.y >= m_min.y && other.m_max.z >= m_min.z )
  {
    return true;
  }

  return false;
}

// Intersect ray R(t) = p + t*d against AABB a. When intersecting,
// return intersection distance tmin and point q of intersection
bool aeAABB::IntersectRay( aeFloat3 p, aeFloat3 d, aeFloat3* pOut, float* tOut ) const
{
  float tmin = 0.0f; // set to -FLT_MAX to get first hit on line
  float tmax = aeMath::MaxValue< float >(); // set to max distance ray can travel (for segment)
  for ( int32_t i = 0; i < 3; i++ ) // For all three slabs
  {
    if ( aeMath::Abs( d[ i ] ) < 0.001f )
    {
      // Ray is parallel to slab. No hit if origin not within slab
      if ( p[ i ] < m_min[ i ] || p[ i ] > m_max[ i ] )
      {
        return false;
      }
    }
    else
    {
      // Compute intersection t value of ray with near and far plane of slab
      float ood = 1.0f / d[ i ];
      float t1 = ( m_min[ i ] - p[ i ] ) * ood;
      float t2 = ( m_max[ i ] - p[ i ] ) * ood;
      // Make t1 be intersection with near plane, t2 with far plane
      if ( t1 > t2 )
      {
        std::swap( t1, t2 );
      }

      // Compute the intersection of slab intersection intervals
      tmin = aeMath::Max( tmin, t1 );
      tmax = aeMath::Min( tmax, t2 );

      // Exit with no collision as soon as slab intersection becomes empty
      if ( tmin > tmax )
      {
        return false;
      }
    }
  }

  // Ray intersects all 3 slabs. Return point (q) and intersection t value (tmin)
  if ( tOut )
  {
    *tOut = tmin;
  }
  if ( pOut )
  {
    *pOut = p + d * tmin;
  }
  return true;
}

//------------------------------------------------------------------------------
// aeOBB member functions
//------------------------------------------------------------------------------
aeOBB::aeOBB( const aeFloat4x4& transform )
{
  SetTransform( transform );
}

void aeOBB::SetTransform( const aeFloat4x4& transform )
{
  m_transform = transform;
  
  aeFloat3 scale = transform.GetScale();
  m_scaledAABB = aeAABB( scale * -0.5f, scale * 0.5f );
  
  m_invTransRot = transform;
  m_invTransRot = m_invTransRot.GetScaleRemoved();
  m_invTransRot.SetInverse();
}

const aeFloat4x4& aeOBB::GetTransform() const
{
  return m_transform;
}

float aeOBB::GetMinDistance( aeFloat3 p ) const
{
  p = ( m_invTransRot * aeFloat4( p, 1.0f ) ).GetXYZ();
  return m_scaledAABB.GetMinDistance( p );
}

bool aeOBB::IntersectRay( aeFloat3 _p, aeFloat3 _d, aeFloat3* pOut, float* tOut ) const
{
  aeFloat3 p = ( m_invTransRot * aeFloat4( _p, 1.0f ) ).GetXYZ();
  aeFloat3 d = ( m_invTransRot * aeFloat4( _d, 0.0f ) ).GetXYZ();

  float rayT = 0.0f;
  if ( m_scaledAABB.IntersectRay( p, d, nullptr, &rayT ) )
  {
    if ( tOut )
    {
      *tOut = rayT;
    }
    if ( pOut )
    {
      *pOut = _p + _d * rayT;
    }
    return true;
  }

  return false;
}

aeAABB aeOBB::GetAABB() const
{
  aeFloat4 corners[] =
  {
    m_transform * aeFloat4( -0.5f, -0.5f, -0.5f, 1.0f ),
    m_transform * aeFloat4( 0.5f, -0.5f, -0.5f, 1.0f ),
    m_transform * aeFloat4( 0.5f, 0.5f, -0.5f, 1.0f ),
    m_transform * aeFloat4( -0.5f, 0.5f, -0.5f, 1.0f ),
    m_transform * aeFloat4( -0.5f, -0.5f, 0.5f, 1.0f ),
    m_transform * aeFloat4( 0.5f, -0.5f, 0.5f, 1.0f ),
    m_transform * aeFloat4( 0.5f, 0.5f, 0.5f, 1.0f ),
    m_transform * aeFloat4( -0.5f, 0.5f, 0.5f, 1.0f ),
  };
  
  aeAABB result( corners[ 0 ].GetXYZ(), corners[ 1 ].GetXYZ() );
  for ( uint32_t i = 2; i < countof( corners ); i++ )
  {
    result.Expand( corners[ i ].GetXYZ() );
  }

  return result;
}

//------------------------------------------------------------------------------
// aeFrustum member functions
//------------------------------------------------------------------------------
aeFrustum::aeFrustum( aeFloat4x4 worldToProjection )
{
  aeFloat4 row0 = worldToProjection.GetRow( 0 );
  aeFloat4 row1 = worldToProjection.GetRow( 1 );
  aeFloat4 row2 = worldToProjection.GetRow( 2 );
  aeFloat4 row3 = worldToProjection.GetRow( 3 );

  aeFloat4 near = -row0 - row3;
  aeFloat4 far = row0 - row3;
  aeFloat4 left = -row1 - row3;
  aeFloat4 right = row1 - row3;
  aeFloat4 top = -row2 - row3;
  aeFloat4 bottom = row2 - row3;
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

bool aeFrustum::Intersects( aeFloat3 point ) const
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

bool aeFrustum::Intersects( const aeSphere& sphere ) const
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
