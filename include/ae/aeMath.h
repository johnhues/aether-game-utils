//------------------------------------------------------------------------------
// aeMath.h
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
#ifndef AEMATH_H
#define AEMATH_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
// @TODO: Remove aeMath
#define aeMath ae

//------------------------------------------------------------------------------
// aeRandom
//------------------------------------------------------------------------------
class aeRandom
{
public:
  aeRandom( float min, float max ) :
    m_min( min ),
    m_max( max ),
    m_seed( 1 )
  {}
  
  void Seed() { m_seed = uint64_t( rand() ) << 31 | uint64_t( rand() ); }
  void Seed( uint32_t seed ) { m_seed = seed; }
  float GetMin() const { return m_min; }
  float GetMax() const { return m_max; }

  float Get()
  {
    // splitmix https://arvid.io/2018/07/02/better-cxx-prng/
    uint64_t z = ( m_seed += UINT64_C( 0x9E3779B97F4A7C15 ) );
    z = ( z ^ ( z >> 30 ) ) * UINT64_C( 0xBF58476D1CE4E5B9 );
    z = ( z ^ ( z >> 27 ) ) * UINT64_C( 0x94D049BB133111EB );
    float t = uint32_t( ( z ^ ( z >> 31 ) ) >> 31 ) / (float)UINT32_MAX;
    return m_min + t * ( m_max - m_min );
  }

private:
  float m_min;
  float m_max;
  uint64_t m_seed;
};

//------------------------------------------------------------------------------
// defs
//------------------------------------------------------------------------------
using aeFloat2 = ae::Vec2;
using aeFloat3 = ae::Vec3;
using aeInt2 = ae::Int2;
using aeInt3 = ae::Int3;
using aeFloat4 = ae::Vec4;
using aeFloat4x4 = ae::Matrix4;
using aeQuat = ae::Quaternion;

const static aeFloat3 aeFloat3Zero = aeFloat3( 0.0f );
const static aeFloat3 aeFloat3Up = aeFloat3( 0, 0, 1 );
const static aeFloat3 aeFloat3Down = aeFloat3( 0, 0, -1 );
const static aeFloat3 aeFloat3Left = aeFloat3( -1, 0, 0 );
const static aeFloat3 aeFloat3Right = aeFloat3( 1, 0, 0 );
const static aeFloat3 aeFloat3Forward = aeFloat3( 0, 1, 0 );
const static aeFloat3 aeFloat3Backward = aeFloat3( 0, -1, 0 );

namespace aeMath
{
  inline aeFloat3 Abs( aeFloat3 v )
  {
    return aeFloat3(
      Abs( v.x ),
      Abs( v.y ),
      Abs( v.z )
    );
  }

  inline aeInt3 Min( aeInt3 v0, aeInt3 v1 )
  {
    return aeInt3(
      Min( v0.x, v1.x ),
      Min( v0.y, v1.y ),
      Min( v0.z, v1.z )
    );
  }

  inline aeInt3 Max( aeInt3 v0, aeInt3 v1 )
  {
    return aeInt3(
      Max( v0.x, v1.x ),
      Max( v0.y, v1.y ),
      Max( v0.z, v1.z )
    );
  }

  inline aeInt3 Abs( aeInt3 v )
  {
    return aeInt3(
      Abs( v.x ),
      Abs( v.y ),
      Abs( v.z )
    );
  }

  inline aeFloat3 Ceil( aeFloat3 v )
  {
    return aeFloat3(
      (float)Ceil( v.x ),
      (float)Ceil( v.y ),
      (float)Ceil( v.z )
    );
  }

  inline aeFloat3 Floor( aeFloat3 v )
  {
    return aeFloat3(
      (float)Floor( v.x ),
      (float)Floor( v.y ),
      (float)Floor( v.z )
    );
  }

  inline aeFloat4 Ceil( aeFloat4 v )
  {
    return aeFloat4(
      (float)Ceil( v.x ),
      (float)Ceil( v.y ),
      (float)Ceil( v.z ),
      (float)Ceil( v.w )
    );
  }

  inline aeFloat4 Floor( aeFloat4 v )
  {
    return aeFloat4(
      (float)Floor( v.x ),
      (float)Floor( v.y ),
      (float)Floor( v.z ),
      (float)Floor( v.w )
    );
  }
}

//------------------------------------------------------------------------------
// aeRect class
//------------------------------------------------------------------------------
struct aeRect
{
  aeRect() = default;
  aeRect( const aeRect& ) = default;

  aeRect( float x, float y, float w, float h ) : x(x), y(y), w(w), h(h) {}
  aeRect( aeFloat2 p0, aeFloat2 p1 );
  static aeRect Zero();

  explicit operator aeFloat4() const { return aeFloat4( x, y, w, h ); }
  
  aeFloat2 GetMin() const { return aeFloat2( x, y ); }
  aeFloat2 GetMax() const { return aeFloat2( x + w, y + h ); }
  aeFloat2 GetSize() const { return aeFloat2( w, h ); }

  bool Contains( aeFloat2 pos ) const;
  void Expand( aeFloat2 pos ); // @NOTE: Zero size rect is maintained by Expand()

  bool GetIntersection( const aeRect& other, aeRect* intersectionOut ) const;
  
  float x;
  float y;
  float w;
  float h;
};

inline std::ostream& operator<<( std::ostream& os, aeRect r )
{
  return os << r.x << " " << r.y << " " << r.w << " " << r.h;
}

//------------------------------------------------------------------------------
// aeRectInt class
//------------------------------------------------------------------------------
struct aeRectInt
{
  aeRectInt() = default;
  aeRectInt( const aeRectInt& ) = default;

  aeRectInt( int32_t x, int32_t y, int32_t w, int32_t h ) : x(x), y(y), w(w), h(h) {}

  static aeRectInt Zero();
  aeInt2 GetPos() const { return aeInt2( x, y ); }
  aeInt2 GetSize() const { return aeInt2( w, h ); }
  bool Contains( aeInt2 pos ) const;
  bool Intersects( aeRectInt other ) const;
  void Expand( aeInt2 pos ); // @NOTE: Zero size rect is expanded to 1x1 grid square by Expand()
  
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;
};

inline std::ostream& operator<<( std::ostream& os, aeRectInt r )
{
  return os << r.x << " " << r.y << " " << r.w << " " << r.h;
}

//------------------------------------------------------------------------------
// aePlane class
//------------------------------------------------------------------------------
class aePlane
{
public:
  aePlane() = default;
  aePlane( aeFloat3 point, aeFloat3 normal );
  aePlane( aeFloat4 pointNormal );
  
  aeFloat3 GetNormal() const;
  aeFloat3 GetClosestPointToOrigin() const;

  bool IntersectRay( aeFloat3 pos, aeFloat3 dir, float* tOut, aeFloat3* out ) const;
  aeFloat3 GetClosestPoint( aeFloat3 pos, float* distanceOut = nullptr ) const;
  float GetSignedDistance( aeFloat3 pos ) const;

private:
  aeFloat4 m_plane;
};

//------------------------------------------------------------------------------
// aeLineSegment class
//------------------------------------------------------------------------------
class aeLineSegment
{
public:
  aeLineSegment() = default;
  aeLineSegment( aeFloat3 p0, aeFloat3 p1 );

  float GetMinDistance( aeFloat3 p, aeFloat3* nearestOut = nullptr ) const; // @TODO: GetDistance()

private:
  aeFloat3 m_p0;
  aeFloat3 m_p1;
};

//------------------------------------------------------------------------------
// aeCircle class
//------------------------------------------------------------------------------
class aeCircle
{
public:
  aeCircle() = default;
  aeCircle( aeFloat2 point, float radius );

  static float GetArea( float radius );

  aeFloat2 GetCenter() const { return m_point; }
  float GetRadius() const { return m_radius; }
  void SetCenter( aeFloat2 point ) { m_point = point; }
  void SetRadius( float radius ) { m_radius = radius; }

  bool Intersect( const aeCircle& other, aeFloat2* out ) const;

private:
  friend std::ostream& operator<<( std::ostream& os, const aeCircle& c );

  aeFloat2 m_point;
  float m_radius;
};

inline std::ostream& operator<<( std::ostream& os, const aeCircle& c )
{
  return os << "(" << c.m_point << " : " << c.m_radius << ")";
}

//------------------------------------------------------------------------------
// aeSphere class
//------------------------------------------------------------------------------
class aeSphere
{
public:
  aeSphere() = default;
  aeSphere( aeFloat3 center, float radius ) : center( center ), radius( radius ) {}

  bool Raycast( aeFloat3 origin, aeFloat3 direction, float* tOut = nullptr, aeFloat3* pOut = nullptr ) const;
  bool SweepTriangle( aeFloat3 direction, const aeFloat3* points, aeFloat3 normal,
    float* outNearestDistance, aeFloat3* outNearestIntersectionPoint, aeFloat3* outNearestPolygonIntersectionPoint, class aeDebugRender* debug ) const;
  bool IntersectTriangle( aeFloat3 t0, aeFloat3 t1, aeFloat3 t2, aeFloat3* outNearestIntersectionPoint ) const;

  aeFloat3 center = aeFloat3( 0.0f );
  float radius = 0.5f;
};

//------------------------------------------------------------------------------
// aeAABB class
//------------------------------------------------------------------------------
class aeAABB
{
public:
  aeAABB() = default;
  aeAABB( const aeAABB& ) = default;
  aeAABB( aeFloat3 p0, aeFloat3 p1 );
  explicit aeAABB( const aeSphere& sphere );

  void Expand( aeFloat3 p );
  void Expand( aeAABB other );
  void Expand( float boundary );

  aeFloat3 GetMin() const { return m_min; }
  aeFloat3 GetMax() const { return m_max; }
  aeFloat3 GetCenter() const { return ( m_min + m_max ) * 0.5f; }
  aeFloat3 GetHalfSize() const { return ( m_max - m_min ) * 0.5f; }
  aeFloat4x4 GetTransform() const;

  float GetMinDistance( aeFloat3 p ) const; // @TODO: GetDistanceFromSurface()
  bool Intersect( aeAABB other ) const;
  bool IntersectRay( aeFloat3 p, aeFloat3 d, aeFloat3* pOut = nullptr, float* tOut = nullptr ) const;

private:
  aeFloat3 m_min;
  aeFloat3 m_max;
};

inline std::ostream& operator<<( std::ostream& os, aeAABB aabb )
{
  return os << "[" << aabb.GetMin() << ", " << aabb.GetMax() << "]";
}

//------------------------------------------------------------------------------
// aeOBB class
//------------------------------------------------------------------------------
class aeOBB
{
public:
  aeOBB() = default;
  aeOBB( const aeOBB& ) = default;
  aeOBB( const aeFloat4x4& transform );

  void SetTransform( const aeFloat4x4& transform );
  const aeFloat4x4& GetTransform() const;

  float GetMinDistance( aeFloat3 p ) const; // @TODO: GetDistanceFromSurface()
  bool IntersectRay( aeFloat3 p, aeFloat3 d, aeFloat3* pOut = nullptr, float* tOut = nullptr ) const;

  aeAABB GetAABB() const;

private:
  aeFloat4x4 m_transform;
  aeFloat4x4 m_invTransRot;
  aeAABB m_scaledAABB;
};

//------------------------------------------------------------------------------
// aeFrustum class
//------------------------------------------------------------------------------
enum class aeFrustumPlane
{
  Near,
  Far,
  Left,
  Right,
  Top,
  Bottom
};
class aeFrustum
{
public:
  aeFrustum( aeFloat4x4 worldToProjection );
  bool Intersects( const aeSphere& sphere ) const;
  bool Intersects( aeFloat3 point ) const;
  aePlane GetPlane( aeFrustumPlane plane ) const;
  
private:
  aePlane m_planes[ 6 ];
};

#endif
