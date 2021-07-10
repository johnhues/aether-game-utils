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
// aeFloat2 struct
//------------------------------------------------------------------------------
struct AE_ALIGN(16) aeFloat2
{
  union
  {
    struct
    {
      float x;
      float y;
    };
    float data[ 2 ];
  };

  const static aeFloat2 Zero;

  aeFloat2() = default;
  aeFloat2( const aeFloat2& ) = default;
  aeFloat2( bool ) = delete;
  explicit aeFloat2( float v ) : x( v ), y( v ) {}
  aeFloat2( float x, float y ) : x( x ), y( y ) {}
  explicit aeFloat2( const struct aeInt2& v );
  static aeFloat2 FromAngle( float angle ) { return aeFloat2( aeMath::Cos( angle ), aeMath::Sin( angle ) );}

  bool operator ==( const aeFloat2& o ) const { return x == o.x && y == o.y; }
  bool operator !=( const aeFloat2& o ) const { return !( *this == o ); }
  
  aeFloat2 operator- () const;
  aeFloat2 operator+ ( const aeFloat2 &v ) const;
  void operator+= (const aeFloat2 &v);
  aeFloat2 operator- ( const aeFloat2 &v ) const;
  void operator-= ( const aeFloat2 &v );
  
  aeFloat2 operator* (const float s) const;
  void operator*=( const float s );
  aeFloat2 operator* (const aeFloat2 s) const;
  void operator*=( const aeFloat2 s );
  aeFloat2 operator/ (const float s) const;
  void operator/= (const float s);
  aeFloat2 operator/ ( const aeFloat2 s ) const;
  void operator/= ( const aeFloat2 s );
  
  aeFloat2 Lerp( const aeFloat2& end, float t ) const;
  
  float Length() const;
  float LengthSquared() const;
  float Normalize();
  aeFloat2 NormalizeCopy() const;
  float SafeNormalize();
  aeFloat2 SafeNormalizeCopy() const;
  void Trim( float s );
  
  aeInt2 NearestCopy() const;
  aeInt2 FloorCopy() const;
  aeInt2 CeilCopy() const;
  
  float Dot( const aeFloat2 &v ) const;
  static float Dot( const aeFloat2& a, const aeFloat2& b );
  aeFloat2 RotateCopy( float rotation ) const;
  float GetAngle() const;
  float Atan2() const;
  static aeFloat2 Reflect( aeFloat2 v, aeFloat2 n );
  
  // @TODO: Remove
  aeFloat2( const ae::Vec2 v ) : x(v.x), y(v.y) {}
  operator ae::Vec2() const { return ae::Vec2( x, y ); }
};

inline std::ostream& operator<<( std::ostream& os, aeFloat2 v )
{
  return os << v.x << " " << v.y;
}

//------------------------------------------------------------------------------
// aeInt2 struct
//------------------------------------------------------------------------------
struct aeInt2
{
  union
  {
    struct
    {
      int32_t x;
      int32_t y;
    };
    int32_t data[ 2 ];
  };
  int32_t pad[ 2 ];

  const static aeInt2 Zero;
  const static aeInt2 Up;
  const static aeInt2 Down;
  const static aeInt2 Left;
  const static aeInt2 Right;

  aeInt2() = default;
  aeInt2( const aeInt2& ) = default;
  aeInt2( bool ) = delete;

  explicit aeInt2( int32_t _v ) : x( _v ), y( _v ) {}
  explicit aeInt2( const struct aeInt3& v );
  aeInt2( int32_t _x, int32_t _y ) : x( _x ), y( _y ) {}
  // @NOTE: No automatic conversion from aeFloat2 because rounding type should be explicit!
  
  int32_t& operator[]( int32_t i ) { return data[ i ]; }
  const int32_t& operator[]( int32_t i ) const { return data[ i ]; }
  
  bool operator ==( const aeInt2& o ) const { return x == o.x && y == o.y; }
  bool operator !=( const aeInt2& o ) const { return !( *this == o ); }
  
  aeInt2 operator- () const;

  aeInt2& operator+=( const aeInt2 &v );
  aeInt2& operator-=( const aeInt2 &v );
  aeInt2 operator+( const aeInt2 &v ) const;
  aeInt2 operator-( const aeInt2 &v ) const;

  // @TODO: aeInt * 2u causes ambiguous overloaded operator issue
  aeInt2 operator* ( const int32_t s ) const;
  void operator*= ( const int32_t s );
  aeInt2 operator* ( const aeInt2& v ) const;
  aeInt2& operator*= ( const aeInt2& v );
  aeInt2 operator/ ( const int32_t s ) const;
  void operator/= ( const int32_t s );
  aeInt2 operator/ ( const aeInt2& v ) const;
  aeInt2& operator/= ( const aeInt2& v );

  aeFloat2 operator* ( const float s ) const;
  void operator*= ( const float s ) = delete;
  aeFloat2 operator/ ( const float s ) const;
  void operator/= ( const float s ) = delete;
};

inline std::ostream& operator<<( std::ostream& os, aeInt2 v )
{
  return os << v.x << " " << v.y;
}

//------------------------------------------------------------------------------
// aeFloat3 struct
//------------------------------------------------------------------------------
class AE_ALIGN(16) aeFloat3
{
public:
  union
  {
    struct
    {
      float x;
      float y;
      float z;
    };
    float data[ 3 ];
  };
  float pad;

  const static aeFloat3 Zero;
  const static aeFloat3 Up;
  const static aeFloat3 Down;
  const static aeFloat3 Left;
  const static aeFloat3 Right;
  const static aeFloat3 Forward;
  const static aeFloat3 Backward;

  aeFloat3() = default;
  aeFloat3( const aeFloat3& ) = default;
  aeFloat3( bool ) = delete;

  explicit aeFloat3( float _v ) : x( _v ), y( _v ), z( _v ), pad( 0.0f ) {}
  aeFloat3( float _x, float _y, float _z ) : x( _x ), y( _y ), z( _z ), pad( 0.0f ) {}
  aeFloat3( const float( &v )[ 3 ] ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0.0f ) {}
  aeFloat3( const float( &v )[ 4 ] ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0.0f ) {}
  explicit aeFloat3( float*& v ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0.0f ) {}
  explicit aeFloat3( const float*& v ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0.0f ) {}
  aeFloat3( aeFloat2 xy, float z ) : x( xy.x ), y( xy.y ), z( z ), pad( 0.0f ) {}
  explicit aeFloat3( aeFloat2 xy ) : x( xy.x ), y( xy.y ), z( 0.0f ), pad( 0.0f ) {}
  explicit operator aeFloat2() const { return aeFloat2( x, y ); }
  explicit aeFloat3( const struct aeInt3& v );
  void SetZero();

  float& operator[]( int32_t i ) { return data[ i ]; }
  float operator[]( int32_t i ) const { return data[ i ]; }
  aeFloat2 GetXY() const { return aeFloat2( x, y ); }

  bool operator==(const aeFloat3& v) const;
  bool operator!=(const aeFloat3& v) const;
  bool operator<(const aeFloat3& v) const;
  bool operator>(const aeFloat3& v) const;
  bool operator<=(const aeFloat3& v) const;
  bool operator>=(const aeFloat3& v) const;

  aeFloat3 operator+ (const aeFloat3& v) const;
  void operator+= (const aeFloat3& v);
  void operator+= (const aeFloat2& v);
  aeFloat3 operator- (void) const;
  aeFloat3 operator- (const aeFloat3& v) const;
  void operator-= (const aeFloat3& v);
  
  aeFloat3 operator* (const float s) const;
  void operator*= (const float s);
  aeFloat3 operator/ (const float s) const;
  void operator/= (const float s);
  
  aeFloat3 operator* ( const aeFloat3& v ) const;
  aeFloat3& operator*= ( const aeFloat3& v );
  static aeFloat3 Multiply( const aeFloat3& a, const aeFloat3& b );
  aeFloat3 operator/ ( const aeFloat3& v ) const;
  aeFloat3& operator/= ( const aeFloat3& v );
  
  float Dot(const aeFloat3& v) const;
  static float Dot( const aeFloat3& a, const aeFloat3& b );
  aeFloat3 operator% (const aeFloat3& v) const;
  void AddScaledVector(const aeFloat3& v, const float s);
  aeFloat3 ComponentProduct(const aeFloat3& v) const;
  void ComponentProductUpdate(const aeFloat3& v);

  float Length() const;
  float LengthSquared() const;
  float GetDistance(const aeFloat3 &other) const;
  float GetDistanceSquared(const aeFloat3 &other) const;
  
  aeFloat3& Trim(const float s);
  aeFloat3& ZeroAxis( aeFloat3 axis ); // Zero component along arbitrary axis (ie vec dot axis == 0)
  aeFloat3& ZeroDirection( aeFloat3 direction ); // Zero component along positive half of axis (ie vec dot dir > 0)
  float Normalize();
  aeFloat3 NormalizeCopy() const;
  float SafeNormalize();
  aeFloat3 SafeNormalizeCopy() const;
  
  aeInt3 NearestCopy() const;
  aeInt3 FloorCopy() const;
  aeInt3 CeilCopy() const;

  float GetAngleBetween(const aeFloat3& v) const;
  void AddRotationXY(float rotation);
  aeFloat3 RotateCopy( aeFloat3 axis, float angle ) const;
  aeFloat3 Lerp(const aeFloat3& end, float t) const;
  aeFloat3 Slerp(const aeFloat3& end, float t) const;

  static aeFloat3 ProjectPoint( const class aeFloat4x4& projection, aeFloat3 p );
  
  // @TODO: Remove
  aeFloat3( const ae::Vec3 v ) : x(v.x), y(v.y), z(v.z) {}
  operator ae::Vec3() const { return ae::Vec3( x, y, z ); }
};

inline aeFloat3 operator * ( float f, aeFloat3 v ) { return v * f; }

inline std::ostream& operator<<( std::ostream& os, aeFloat3 v )
{
  return os << v.x << " " << v.y << " " << v.z;
}

//------------------------------------------------------------------------------
// aeInt3 struct
//------------------------------------------------------------------------------
struct AE_ALIGN(16) aeInt3
{
  union
  {
    struct
    {
      int32_t x;
      int32_t y;
      int32_t z;
    };
    int32_t data[ 3 ];
  };
  int32_t pad;

  aeInt3() = default;
  aeInt3( const aeInt3& ) = default;
  aeInt3( bool ) = delete;

  explicit aeInt3( int32_t _v ) : x( _v ), y( _v ), z( _v ), pad( 0 ) {}
  aeInt3( int32_t _x, int32_t _y, int32_t _z ) : x( _x ), y( _y ), z( _z ), pad( 0 ) {}
  aeInt3( aeInt2 xy, int32_t _z ) : x( xy.x ), y( xy.y ), z( _z ), pad( 0 ) {}
  aeInt3( const int32_t( &v )[ 3 ] ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0 ) {}
  aeInt3( const int32_t( &v )[ 4 ] ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0 ) {}
  explicit aeInt3( int32_t*& v ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0 ) {}
  explicit aeInt3( const int32_t*& v ) : x( v[ 0 ] ), y( v[ 1 ] ), z( v[ 2 ] ), pad( 0 ) {}
  // @NOTE: No conversion from aeFloat3 because rounding type should be explicit!
  
  int32_t& operator[]( int32_t i ) { return data[ i ]; }
  const int32_t& operator[]( int32_t i ) const { return data[ i ]; }
  aeInt2 GetXY() const { return aeInt2( x, y ); }
  
  bool operator ==( const aeInt3& o ) const { return x == o.x && y == o.y && z == o.z; }
  bool operator !=( const aeInt3& o ) const { return !( *this == o ); }
  
  aeInt3 operator+( const aeInt3& v ) const;
  aeInt3 operator-( const aeInt3& v ) const;
  aeInt3& operator+=( const aeInt3& v );
  aeInt3& operator-=( const aeInt3& v );

  aeInt3 operator*( int32_t s ) const;
  aeInt3 operator*( const aeInt3& s ) const;
  aeInt3 operator/( int32_t s ) const;
  aeInt3 operator/( const aeInt3& s ) const;
  void operator*=( int32_t s );
  void operator*=( const aeInt3& s );
  void operator/=( int32_t s );
  void operator/=( const aeInt3& s );
};

inline std::ostream& operator<<( std::ostream& os, aeInt3 v )
{
  return os << v.x << " " << v.y << " " << v.z;
}

namespace aeMath
{
  inline aeFloat3 Min( aeFloat3 v0, aeFloat3 v1 )
  {
    return aeFloat3(
      Min( v0.x, v1.x ),
      Min( v0.y, v1.y ),
      Min( v0.z, v1.z )
    );
  }

  inline aeFloat3 Max( aeFloat3 v0, aeFloat3 v1 )
  {
    return aeFloat3(
      Max( v0.x, v1.x ),
      Max( v0.y, v1.y ),
      Max( v0.z, v1.z )
    );
  }

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
}

//------------------------------------------------------------------------------
// aeFloat4 struct
//------------------------------------------------------------------------------
class AE_ALIGN(16) aeFloat4
{
public:
  union
  {
    struct
    {
      float x;
      float y;
      float z;
      float w;
    };
    float data[4];
  };

  const static aeFloat4 Zero;

  aeFloat4() = default;
  aeFloat4( const aeFloat4& ) = default;
  aeFloat4( bool ) = delete;

  explicit aeFloat4(float v) : x(v), y(v), z(v), w(v) {}
  aeFloat4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
  aeFloat4( aeFloat3 xyz, float w ) : x( xyz.x ), y( xyz.y ), z( xyz.z ), w( w ) {}
  aeFloat4( aeFloat2 xy, float z, float w ) : x( xy.x ), y( xy.y ), z( z ), w( w ) {}
  explicit aeFloat4( aeFloat3 xyz ) : x( xyz.x ), y( xyz.y ), z( xyz.z ), w( 0.0f ) {}
  explicit aeFloat4( aeFloat2 xy ) : x( xy.x ), y( xy.y ), z( 0.0f ), w( 0.0f ) {}
  explicit operator aeFloat3() const { return aeFloat3( x, y, z ); }
  explicit operator aeFloat2() const { return aeFloat2( x, y ); }
  aeFloat4(const float (&v)[3]) : x(v[0]), y(v[1]), z(v[2]), w(0.0f) {}
  aeFloat4(const float (&v)[4]) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}
  aeFloat2 GetXY() const { return aeFloat2( x, y ); }
  aeFloat2 GetZW() const { return aeFloat2( z, w ); }
  aeFloat3 GetXYZ() const { return aeFloat3( x, y, z ); }
  float operator[](unsigned i) const;
  float& operator[](unsigned i);
  aeFloat4 operator-(void) const;
  aeFloat4 operator+ (const aeFloat4& v) const;
  void operator+= (const aeFloat4& v);
  aeFloat4 operator- (const aeFloat4& v) const;
  void operator-= (const aeFloat4& v);
  aeFloat4 operator* (const float s) const;
  void operator*= (const float s);
  aeFloat4 operator/ (const float s) const;
  void operator/= (const float s);
  float operator* (const aeFloat4& v) const;
  void AddScaledVector(const aeFloat4& v, const float s);
  aeFloat4 ComponentProduct(const aeFloat4& v) const;
  void ComponentProductUpdate(const aeFloat4& v);
  void Invert();
  float Length() const;
  float LengthSquared() const;
  void Trim(const float s);
  float Normalize();
  aeFloat4 NormalizeCopy() const;
  float SafeNormalize();
  aeFloat4 SafeNormalizeCopy() const;
  void SetZero();
  bool operator==(const aeFloat4& v) const;
  bool operator!=(const aeFloat4& v) const;
  bool operator<(const aeFloat4& v) const;
  bool operator>(const aeFloat4& v) const;
  bool operator<=(const aeFloat4& v) const;
  bool operator>=(const aeFloat4& v) const;
  
  // @TODO: Remove
  aeFloat4( const ae::Vec4 v ) : x(v.x), y(v.y), z(v.z), w(v.w) {}
  operator ae::Vec4() const { return ae::Vec4( x, y, z, w ); }
};

inline std::ostream& operator<<( std::ostream& os, aeFloat4 v )
{
  return os << v.x << " " << v.y << " " << v.z << " " << v.w;
}

namespace aeMath
{
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

template< typename T >
struct _aeInt4
{
  union
  {
    struct
    {
      T x;
      T y;
      T z;
      T w;
    };
    T data[ 4 ];
  };
  
  _aeInt4() = default;
  _aeInt4( const _aeInt4& ) = default;
  _aeInt4( bool ) = delete;
  
  _aeInt4( T _x ) : x( _x ), y( _x ), z( _x ), w( _x ) {}
  _aeInt4( T _x, T _y, T _z, T _w ) : x( _x ), y( _y ), z( _z ), w( _w ) {}
  T& operator[]( T i ) { return data[ i ]; }
  const T& operator[]( T i ) const { return data[ i ]; }
  bool operator ==( const _aeInt4& o ) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
  bool operator !=( const _aeInt4& o ) const { return x != o.x || y != o.y || z != o.z || w != o.w; }
};

typedef _aeInt4< int32_t > aeInt4;

//------------------------------------------------------------------------------
// aeFloat4x4 struct
//------------------------------------------------------------------------------
class AE_ALIGN(16) aeFloat4x4
{
public:
  float data[ 16 ];

  aeFloat4x4() = default;
  aeFloat4x4( const aeFloat4x4& ) = default;

  aeFloat4x4(float v0,  float v1,  float v2,  float v3,
    float v4,  float v5,  float v6,  float v7,
    float v8,  float v9,  float v10, float v11,
    float v12, float v13, float v14, float v15)
  {
    data[0]  = v0;  data[1]  = v1;  data[2]  = v2;  data[3]  = v3;
    data[4]  = v4;  data[5]  = v5;  data[6]  = v6;  data[7]  = v7;
    data[8]  = v8;  data[9]  = v9;  data[10] = v10; data[11] = v11;
    data[12] = v12; data[13] = v13; data[14] = v14; data[15] = v15;
  }

  static aeFloat4x4 Identity()
  {
    aeFloat4x4 r;
    r.data[0]  = 1; r.data[1]  = 0; r.data[2]  = 0; r.data[3]  = 0;
    r.data[4]  = 0; r.data[5]  = 1; r.data[6]  = 0; r.data[7]  = 0;
    r.data[8]  = 0; r.data[9]  = 0; r.data[10] = 1; r.data[11] = 0;
    r.data[12] = 0; r.data[13] = 0; r.data[14] = 0; r.data[15] = 1;
    return r;
  }

  // Constructor helpers
  static aeFloat4x4 Translation( const aeFloat3& p );
  static aeFloat4x4 Rotation( aeFloat3 forward0, aeFloat3 up0, aeFloat3 forward1, aeFloat3 up1 );
  static aeFloat4x4 RotationX( float angle );
  static aeFloat4x4 RotationY( float angle );
  static aeFloat4x4 RotationZ( float angle );
  static aeFloat4x4 Scaling( const aeFloat3& s );

  static aeFloat4x4 WorldToView( aeFloat3 position, aeFloat3 forward, aeFloat3 up ); // @TODO: Verify handedness
  static aeFloat4x4 ViewToProjection( float fov, float aspectRatio, float nearPlane, float farPlane );

  // Operators
  bool operator== ( const aeFloat4x4& o ) const { return memcmp( o.data, data, sizeof(data) ) == 0; }
  bool operator!= ( const aeFloat4x4& o ) const { return !operator== ( o ); }
  aeFloat4 operator*(const aeFloat4& v) const;
  aeFloat4x4 operator*(const aeFloat4x4& m) const;
  void operator*=(const aeFloat4x4& m);
  bool SetInverse(const aeFloat4x4& m);

  aeFloat4x4 Inverse() const; //TODO: Rename this! Caused a bunch of problems debugging graphics...
  bool Invert();           //TODO: Rename. See above
  aeFloat4x4& SetTranspose();
  aeFloat4x4 GetTransposeCopy() const;
  aeFloat4x4 GetNormalMatrix() const;
  aeFloat4x4& SetOrientation( const class aeQuat& q );
  aeFloat3 TransformInverse(const aeFloat3& v) const;
  aeFloat3 TransformDirection(const aeFloat3& v) const;
  aeFloat3 TransformInverseDirection(const aeFloat3& v) const;
  void SetDiagonal(float a, float b, float c);
  aeFloat3 GetAxisVector(int col) const;
  aeFloat3 GetPosition() const;
  aeQuat GetRotation() const;
  void SetPosition( const aeFloat3& p );
  void SetAxisVector(unsigned col, const aeFloat3 &v);
  aeFloat4 GetRowVector(int row) const;
  void SetRowVector(unsigned row, const aeFloat3 &v);
  void SetRowVector(unsigned row, const aeFloat4 &v);
  aeFloat4x4& SetIdentity();
  aeFloat4x4& SetTranslate(float X, float Y, float Z);
  aeFloat4x4& SetTranslate(const aeFloat3 &translation);
  aeFloat4x4& SetTranslation(float X, float Y, float Z);
  aeFloat4x4& SetTranslation(const aeFloat3 &translation);
  aeFloat3 GetTranslation() const;
  aeFloat3 GetScale() const;
  aeFloat4x4& SetScale(float X, float Y, float Z);
  aeFloat4x4& SetScale(const aeFloat3 &scale);
  aeFloat4x4& SetScaleKeepTranslate(float X, float Y, float Z);
  aeFloat4x4& SetScaleKeepTranslate(const aeFloat3 &scale);
  aeFloat4x4& SetRotateX(float angle);
  aeFloat4x4& SetRotateY(float angle);
  aeFloat4x4& SetRotateZ(float angle);
  aeFloat4x4& RemoveScaling();

  // Transformation helpers
  aeFloat4x4& Translate( aeFloat3 t );
  aeFloat4x4& Scale( aeFloat3 s );
  aeFloat4x4& RotateX( float angle );
  aeFloat4x4& RotateY( float angle );
  aeFloat4x4& RotateZ( float angle );
};

inline std::ostream& operator << ( std::ostream& os, const aeFloat4x4& mat )
{
  os << mat.data[ 0 ] << " " << mat.data[ 1 ] << " " << mat.data[ 2 ] << " " << mat.data[ 3 ]
    << " " << mat.data[ 4 ] << " " << mat.data[ 5 ] << " " << mat.data[ 6 ] << " " << mat.data[ 7 ]
    << " " << mat.data[ 8 ] << " " << mat.data[ 9 ] << " " << mat.data[ 10 ] << " " << mat.data[ 11 ]
    << " " << mat.data[ 12 ] << " " << mat.data[ 13 ] << " " << mat.data[ 14 ] << " " << mat.data[ 15 ];
  return os;
}

//------------------------------------------------------------------------------
// aeQuat class
//------------------------------------------------------------------------------
class AE_ALIGN(16) aeQuat
{
public:
  union
  {
    struct
    {
      float i;
      float j;
      float k;
      float r;
    };
    float data[ 4 ];
  };
  
  aeQuat() = default;
  aeQuat( const aeQuat& ) = default;

  aeQuat( const float i, const float j, const float k, const float r ) : i(i), j(j), k(k), r(r) {}
  explicit aeQuat( aeFloat3 v ) : i(v.x), j(v.y), k(v.z), r(0.0f) {}
  aeQuat( aeFloat3 forward, aeFloat3 up, bool prioritizeUp = true );
  aeQuat( aeFloat3 axis, float angle );
  static aeQuat Identity() { return aeQuat( 0.0f, 0.0f, 0.0f, 1.0f ); }

  void Normalize();
  bool operator==( const aeQuat& q ) const;
  bool operator!=( const aeQuat& q ) const;
  aeQuat& operator*= ( const aeQuat& q );
  aeQuat operator* ( const aeQuat& q ) const;
  float Dot( const aeQuat& q ) const;
  aeQuat const operator* ( float s ) const;
  void AddScaledVector( const aeFloat3& v, float s );
  void RotateByVector( const aeFloat3& v );
  void SetDirectionXY( const aeFloat3& v );
  aeFloat3 GetDirectionXY() const;
  void ZeroXY();
  void GetAxisAngle( aeFloat3* axis, float* angle ) const;
  void AddRotationXY( float rotation);
  aeQuat Nlerp( aeQuat end, float t ) const;
  aeFloat4x4 GetTransformMatrix() const;
  aeQuat  GetInverse() const;
  aeQuat& SetInverse();
  aeFloat3 Rotate( aeFloat3 v ) const;
};

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
  aePlane( aeFloat4 pointNormal );
  aePlane( aeFloat3 point, aeFloat3 normal );

  bool IntersectRay( aeFloat3 pos, aeFloat3 dir, float* tOut, aeFloat3* out ) const;
  float GetSignedDistance( aeFloat3 pos ) const;

private:
  aeFloat3 m_point;
  aeFloat3 m_normal;
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

  aeFloat3 center = aeFloat3::Zero;
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
class aeFrustum
{
public:
  aeFrustum( aeFloat4x4 worldToProjection );
  bool Intersects( const aeSphere& sphere ) const;
  bool Intersects( aeFloat3 point ) const;
  
private:
  aePlane m_faces[ 6 ];
};

#endif
