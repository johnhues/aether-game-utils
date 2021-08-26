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

namespace aeMath
{
  inline ae::Vec3 Abs( ae::Vec3 v )
  {
    return ae::Vec3(
      Abs( v.x ),
      Abs( v.y ),
      Abs( v.z )
    );
  }

  inline ae::Int3 Min( ae::Int3 v0, ae::Int3 v1 )
  {
    return ae::Int3(
      Min( v0.x, v1.x ),
      Min( v0.y, v1.y ),
      Min( v0.z, v1.z )
    );
  }

  inline ae::Int3 Max( ae::Int3 v0, ae::Int3 v1 )
  {
    return ae::Int3(
      Max( v0.x, v1.x ),
      Max( v0.y, v1.y ),
      Max( v0.z, v1.z )
    );
  }

  inline ae::Int3 Abs( ae::Int3 v )
  {
    return ae::Int3(
      Abs( v.x ),
      Abs( v.y ),
      Abs( v.z )
    );
  }

  inline ae::Vec3 Ceil( ae::Vec3 v )
  {
    return ae::Vec3(
      (float)Ceil( v.x ),
      (float)Ceil( v.y ),
      (float)Ceil( v.z )
    );
  }

  inline ae::Vec3 Floor( ae::Vec3 v )
  {
    return ae::Vec3(
      (float)Floor( v.x ),
      (float)Floor( v.y ),
      (float)Floor( v.z )
    );
  }

  inline ae::Vec4 Ceil( ae::Vec4 v )
  {
    return ae::Vec4(
      (float)Ceil( v.x ),
      (float)Ceil( v.y ),
      (float)Ceil( v.z ),
      (float)Ceil( v.w )
    );
  }

  inline ae::Vec4 Floor( ae::Vec4 v )
  {
    return ae::Vec4(
      (float)Floor( v.x ),
      (float)Floor( v.y ),
      (float)Floor( v.z ),
      (float)Floor( v.w )
    );
  }
}

//------------------------------------------------------------------------------
// aePlane class
//------------------------------------------------------------------------------
class aePlane
{
public:
  aePlane() = default;
  aePlane( ae::Vec3 point, ae::Vec3 normal );
  aePlane( ae::Vec4 pointNormal );
  
  ae::Vec3 GetNormal() const;
  ae::Vec3 GetClosestPointToOrigin() const;

  bool IntersectRay( ae::Vec3 pos, ae::Vec3 dir, float* tOut, ae::Vec3* out ) const;
  ae::Vec3 GetClosestPoint( ae::Vec3 pos, float* distanceOut = nullptr ) const;
  float GetSignedDistance( ae::Vec3 pos ) const;

private:
  ae::Vec4 m_plane;
};

//------------------------------------------------------------------------------
// aeLineSegment class
//------------------------------------------------------------------------------
class aeLineSegment
{
public:
  aeLineSegment() = default;
  aeLineSegment( ae::Vec3 p0, ae::Vec3 p1 );

  float GetMinDistance( ae::Vec3 p, ae::Vec3* nearestOut = nullptr ) const; // @TODO: GetDistance()

private:
  ae::Vec3 m_p0;
  ae::Vec3 m_p1;
};

//------------------------------------------------------------------------------
// aeCircle class
//------------------------------------------------------------------------------
class aeCircle
{
public:
  aeCircle() = default;
  aeCircle( ae::Vec2 point, float radius );

  static float GetArea( float radius );

  ae::Vec2 GetCenter() const { return m_point; }
  float GetRadius() const { return m_radius; }
  void SetCenter( ae::Vec2 point ) { m_point = point; }
  void SetRadius( float radius ) { m_radius = radius; }

  bool Intersect( const aeCircle& other, ae::Vec2* out ) const;

private:
  friend std::ostream& operator<<( std::ostream& os, const aeCircle& c );

  ae::Vec2 m_point;
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
  aeSphere( ae::Vec3 center, float radius ) : center( center ), radius( radius ) {}

  bool Raycast( ae::Vec3 origin, ae::Vec3 direction, float* tOut = nullptr, ae::Vec3* pOut = nullptr ) const;
  bool SweepTriangle( ae::Vec3 direction, const ae::Vec3* points, ae::Vec3 normal,
    float* outNearestDistance, ae::Vec3* outNearestIntersectionPoint, ae::Vec3* outNearestPolygonIntersectionPoint, ae::DebugLines* debug ) const;
  bool IntersectTriangle( ae::Vec3 t0, ae::Vec3 t1, ae::Vec3 t2, ae::Vec3* outNearestIntersectionPoint ) const;

  ae::Vec3 center = ae::Vec3( 0.0f );
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
  aeAABB( ae::Vec3 p0, ae::Vec3 p1 );
  explicit aeAABB( const aeSphere& sphere );

  void Expand( ae::Vec3 p );
  void Expand( aeAABB other );
  void Expand( float boundary );

  ae::Vec3 GetMin() const { return m_min; }
  ae::Vec3 GetMax() const { return m_max; }
  ae::Vec3 GetCenter() const { return ( m_min + m_max ) * 0.5f; }
  ae::Vec3 GetHalfSize() const { return ( m_max - m_min ) * 0.5f; }
  ae::Matrix4 GetTransform() const;

  float GetMinDistance( ae::Vec3 p ) const; // @TODO: GetDistanceFromSurface()
  bool Intersect( aeAABB other ) const;
  bool IntersectRay( ae::Vec3 p, ae::Vec3 d, ae::Vec3* pOut = nullptr, float* tOut = nullptr ) const;

private:
  ae::Vec3 m_min;
  ae::Vec3 m_max;
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
  aeOBB( const ae::Matrix4& transform );

  void SetTransform( const ae::Matrix4& transform );
  const ae::Matrix4& GetTransform() const;

  float GetMinDistance( ae::Vec3 p ) const; // @TODO: GetDistanceFromSurface()
  bool IntersectRay( ae::Vec3 p, ae::Vec3 d, ae::Vec3* pOut = nullptr, float* tOut = nullptr ) const;

  aeAABB GetAABB() const;

private:
  ae::Matrix4 m_transform;
  ae::Matrix4 m_invTransRot;
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
  aeFrustum( ae::Matrix4 worldToProjection );
  bool Intersects( const aeSphere& sphere ) const;
  bool Intersects( ae::Vec3 point ) const;
  aePlane GetPlane( aeFrustumPlane plane ) const;
  
private:
  aePlane m_planes[ 6 ];
};

#endif
