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
#include "aePlatform.h"

//------------------------------------------------------------------------------
// aeMath functions
//------------------------------------------------------------------------------
namespace aeMath
{
  extern const float PI;
  extern const float TWO_PI;
  extern const float HALF_PI;

  template< typename T >
  T&& Min( T&& v )
  {
    return std::forward< T >( v );
  }
  template< typename T0, typename T1, typename... Tn >
  auto Min( T0&& v0, T1&& v1, Tn&&... vn )
  {
    return ( v0 < v1 ) ? Min( v0, std::forward< Tn >( vn )... ) : Min( v1, std::forward< Tn >( vn )... );
  }
  
  template< typename T >
  T&& Max( T&& v )
  {
    return std::forward< T >( v );
  }
  template< typename T0, typename T1, typename... Tn >
  auto Max( T0&& v0, T1&& v1, Tn&&... vn )
  {
    return ( v0 > v1 ) ? Max( v0, std::forward< Tn >( vn )... ) : Max( v1, std::forward< Tn >( vn )... );
  }

  template<typename T>
  inline T Abs(const T &x)
  {
    if(x < static_cast<T>(0))
    {
      return x * static_cast<T>(-1);
    }
    return x;
  }

  template < typename T >
  inline T Clip( T x, T min, T max )
  {
    return Min( Max( x, min ), max );
  }

  inline float Clip01( float x )
  {
    return Clip( x, 0.0f, 1.0f );
  }

  inline float DegToRad( float degrees )
  {
    return degrees * PI / 180.0f;
  }

  inline float RadToDeg( float radians )
  {
    return radians * 180.0f / PI;
  }

  inline int32_t Ceil( float f )
  {
    bool positive = f >= 0.0f;
    if(positive)
    {
      int i = static_cast<int>(f);
      if( f > static_cast<float>(i) ) return i + 1;
      else return i;
    }
    else return static_cast<int>(f);
  }

  inline int32_t Floor( float f )
  {
    bool negative = f < 0.0f;
    if(negative)
    {
      int i = static_cast<int>(f);
      if( f < static_cast<float>(i) ) return i - 1;
      else return i;
    }
    else return static_cast<int>(f);
  }

  inline int32_t Round( float f )
  {
    if( f >= 0.0f ) return (int32_t)( f + 0.5f );
    else return (int32_t)( f - 0.5f );
  }

  inline uint32_t Mod( uint32_t i, uint32_t n )
  {
    return i % n;
  }

  inline int Mod( int32_t i, int32_t n )
  {
    if( i < 0 )
    {
      return ( ( i % n ) + n ) % n;
    }
    else
    {
      return i % n;
    }
  }

  inline float Mod( float f, float n )
  {
    return fmodf( fmodf( f, n ) + n, n );
  }

  inline float Pow( float x, float e )
  {
    return powf( x, e );
  }

  inline float Cos( float x )
  {
    return cosf( x );
  }

  inline float Sin( float x )
  {
    return sinf( x );
  }

  inline float Atan2( float y, float x )
  {
    return atan2( y, x );
  }

  template< typename T >
  inline T MaxValue()
  {
    return std::numeric_limits<T>::max();
  }

  template< typename T >
  inline T MinValue()
  {
    return std::numeric_limits<T>::min();
  }

  template<>
  inline float MaxValue< float >()
  {
    return std::numeric_limits<float>::infinity();
  }

  template<>
  inline float MinValue< float >()
  {
    return -1 * std::numeric_limits<float>::infinity();
  }

  template<>
  inline double MaxValue< double >()
  {
    return std::numeric_limits<double>::infinity();
  }

  template<>
  inline double MinValue< double >()
  {
    return -1 * std::numeric_limits<double>::infinity();
  }

  template< typename T >
  T Lerp( T start, T end, float t )
  {
    return start + ( end - start ) * t;
  }

  inline float Delerp( float start, float end, float value )
  {
    return ( value - start ) / ( end - start );
  }

  inline float Delerp01( float start, float end, float value )
  {
    return Clip01( ( value - start ) / ( end - start ) );
  }

  template< typename T >
  T DtLerp( T value, float snappiness, float dt, T target )
  {
    return aeMath::Lerp( target, value, exp2( -exp2( snappiness ) * dt ) );
  }

  inline float DtLerpAngle( float value, float snappiness, float dt, float target )
  {
    target = aeMath::Mod( target, aeMath::TWO_PI );
    float innerDist = aeMath::Abs( target - value );
    float preDist = aeMath::Abs( ( target - aeMath::TWO_PI ) - value );
    float postDist = aeMath::Abs( ( target + aeMath::TWO_PI ) - value );
    if ( innerDist >= preDist || innerDist >= postDist )
    {
      if ( preDist < postDist )
      {
        target -= aeMath::TWO_PI;
      }
      else
      {
        target += aeMath::TWO_PI;
      }
    }
    value = aeMath::DtLerp( value, snappiness, dt, target );
    return aeMath::Mod( value, aeMath::TWO_PI );
  }

  inline int32_t Random( int32_t min, int32_t max )
  {
    if ( min >= max )
    {
      return min;
    }
    return min + ( rand() % ( max - min ) );
  }

  inline float Random( float min, float max )
  {
    if ( min >= max )
    {
      return min;
    }
    return min + ( ( rand() / (float)RAND_MAX ) * ( max - min ) );
  }

  inline bool RandomBool()
  {
    return Random( 0, 2 );
  }

  template < typename T >
  class RandomValue
  {
  public:
    RandomValue() {}
    RandomValue( T min, T max );
    RandomValue( T value );

    void SetMin( T min );
    void SetMax( T max );

    T GetMin() const;
    T GetMax() const;

    T Get() const;
    operator T() const;

  private:
    T m_min;
    T m_max;
  };

  namespace Interpolation
  {
    template< typename T >
    T Linear( T start, T end, float t )
    {
      return start + ( ( end - start ) * t );
    }

    template< typename T >
    T Cosine( T start, T end, float t )
    {
      t = (1-cos(t*aeMath::PI))/2;

      // float angle = ( t * aeMath::PI ) + aeMath::PI;
      // t = aeMath::Cos( angle );
      // t = ( t + 1 ) / 2.0f;

      // return start + ( ( end - start ) * t );
      return start.Lerp( end, t );
    }
  }
}

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
  bool operator ==( const aeFloat2& o ) const { return x == o.x && y == o.y; }
  bool operator !=( const aeFloat2& o ) const { return !( *this == o ); }
  aeFloat2 operator- () const;
  aeFloat2 operator+ ( const aeFloat2 &v ) const;
  void operator+= (const aeFloat2 &v);
  aeFloat2 operator- ( const aeFloat2 &v ) const;
  void operator-= ( const aeFloat2 &v );
  float Dot( const aeFloat2 &v ) const;
  static float Dot( const aeFloat2& a, const aeFloat2& b );
  float Length() const;
  float LengthSquared() const;
  float Atan2() const;
  aeFloat2 operator* (const float s) const;
  void operator*=( const float s );
  aeFloat2 operator* (const aeFloat2 s) const;
  void operator*=( const aeFloat2 s );
  aeFloat2 operator/ (const float s) const;
  void operator/= (const float s);
  aeFloat2 operator/ ( const aeFloat2 s ) const;
  void operator/= ( const aeFloat2 s );
  aeFloat2 Lerp(const aeFloat2& end, float t) const;
  void Normalize();
  aeFloat2 NormalizeCopy() const;
  void SafeNormalize();
  aeFloat2 SafeNormalizeCopy() const;
  aeInt2 NearestCopy() const;
  aeInt2 FloorCopy() const;
  aeInt2 CeilCopy() const;
  aeFloat2 RotateCopy( float rotation ) const;
  float GetAngle() const;
  static aeFloat2 Reflect( aeFloat2 v, aeFloat2 n );
};

inline std::ostream& operator<<( std::ostream& os, aeFloat2 v )
{
  return os << "<" << v.x << ", " << v.y << ">";
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
  return os << "<" << v.x << ", " << v.y << ">";
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

  explicit aeFloat3(const float _v) : x(_v), y(_v), z(_v), pad(0.0f) {}
  aeFloat3(const float _x, const float _y, const float _z ) : x(_x), y(_y), z(_z), pad(0.0f) {}
  aeFloat3(const float (&v)[3]) : x(v[0]), y(v[1]), z(v[2]), pad(0.0f) {}
  aeFloat3(const float (&v)[4]) : x(v[0]), y(v[1]), z(v[2]), pad(0.0f) {}
  explicit aeFloat3(const float* v) : x(v[0]), y(v[1]), z(v[2]), pad(0.0f) {}
  aeFloat3( aeFloat2 xy, float z ) : x( xy.x ), y( xy.y ), z( z ), pad(0.0f) {}
  explicit aeFloat3( aeFloat2 xy ) : x( xy.x ), y( xy.y ), z( 0.0f ), pad(0.0f) {}
  explicit operator aeFloat2() const { return aeFloat2( x, y ); }
  explicit aeFloat3( const struct aeInt3& v );
  void SetZero();

  float& operator[]( int32_t i ) { return data[ i ]; }
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
  aeFloat3 operator* ( const aeFloat3& v ) const;
  aeFloat3& operator*= ( const aeFloat3& v );
  static aeFloat3 Multiply( const aeFloat3& a, const aeFloat3& b );
  aeFloat3 operator/ (const float s) const;
  void operator/= (const float s);
  
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
  void Normalize();
  aeFloat3 NormalizeCopy() const;
  void SafeNormalize();
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
};

inline std::ostream& operator<<( std::ostream& os, aeFloat3 v )
{
  return os << "<" << v.x << ", " << v.y << ", " << v.z << ">";
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
  return os << "<" << v.x << ", " << v.y << ", " << v.z << ">";
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
  aeFloat4& Normalize();
  aeFloat4 NormalizeCopy() const;
  void SetZero();
  bool operator==(const aeFloat4& v) const;
  bool operator!=(const aeFloat4& v) const;
  bool operator<(const aeFloat4& v) const;
  bool operator>(const aeFloat4& v) const;
  bool operator<=(const aeFloat4& v) const;
  bool operator>=(const aeFloat4& v) const;
};

inline std::ostream& operator<<( std::ostream& os, aeFloat4 v )
{
  return os << "<" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ">";
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
  os << "[ " << mat.data[ 0 ] << ", " << mat.data[ 1 ] << ", " << mat.data[ 2 ] << ", " << mat.data[ 3 ]
    << ", " << mat.data[ 4 ] << ", " << mat.data[ 5 ] << ", " << mat.data[ 6 ] << ", " << mat.data[ 7 ]
    << ", " << mat.data[ 8 ] << ", " << mat.data[ 9 ] << ", " << mat.data[ 10 ] << ", " << mat.data[ 11 ]
    << ", " << mat.data[ 12 ] << ", " << mat.data[ 13 ] << ", " << mat.data[ 14 ] << ", " << mat.data[ 15 ] << " ]";
  return os;
}

//------------------------------------------------------------------------------
// aeQuat class
//------------------------------------------------------------------------------
class AE_ALIGN(16) aeQuat
{
public:
  float i;
  float j;
  float k;
  float r;
  
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
  return os << "<" << r.x << ", " << r.y << ", " << r.w << ", " << r.h << ">";
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
  return os << "<" << r.x << ", " << r.y << ", " << r.w << ", " << r.h << ">";
}

//------------------------------------------------------------------------------
// aePlane class
//------------------------------------------------------------------------------
class aePlane
{
public:
  aePlane() = default;
  aePlane( aeFloat3 point, aeFloat3 normal );

  bool IntersectRay( aeFloat3 pos, aeFloat3 dir, aeFloat3* out ) const;

private:
  aeFloat3 m_point;
  aeFloat3 m_normal;
};

class aeLineSegment
{
public:
  aeLineSegment() = default;
  aeLineSegment( aeFloat3 p0, aeFloat3 p1 );

  float GetMinDistance( aeFloat3 p, aeFloat3* nearestOut = nullptr );

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
// aeAABB class
//------------------------------------------------------------------------------
class aeAABB
{
public:
  aeAABB() = default;
  aeAABB( const aeAABB& ) = default;
  aeAABB( aeFloat3 min, aeFloat3 max );

  void Expand( aeFloat3 p );
  void Expand( aeAABB other );

  aeFloat3 GetMin() const { return m_min; }
  aeFloat3 GetMax() const { return m_max; }

  float GetMinDistance( aeFloat3 p ) const;

private:
  aeFloat3 m_min;
  aeFloat3 m_max;
};

//------------------------------------------------------------------------------
// aeMath::RandomValue member functions
//------------------------------------------------------------------------------
template < typename T >
inline aeMath::RandomValue< T >::RandomValue( T min, T max ) : m_min(min), m_max(max) {}

template < typename T >
inline aeMath::RandomValue< T >::RandomValue( T value ) : m_min(value), m_max(value) {}

template < typename T >
inline void aeMath::RandomValue< T >::SetMin( T min )
{
  m_min = min;
}

template < typename T >
inline void aeMath::RandomValue< T >::SetMax( T max )
{
  m_max = max;
}

template < typename T >
inline T aeMath::RandomValue< T >::GetMin() const
{
  return m_min;
}

template < typename T >
inline T aeMath::RandomValue< T >::GetMax() const
{
  return m_max;
}

template < typename T >
inline T aeMath::RandomValue< T >::Get() const
{
  return Random( m_min, m_max );
}

template < typename T >
inline aeMath::RandomValue< T >::operator T() const
{
  return Get();
}

//------------------------------------------------------------------------------
// aeHash class (fnv1a)
// @NOTE: Empty strings and zero-length data buffers do not hash to zero
//------------------------------------------------------------------------------
class aeHash
{
public:
  aeHash() = default;
  explicit aeHash( uint32_t initialValue );

  aeHash& HashString( const char* str );
  aeHash& HashData( const uint8_t* data, const uint32_t length );

  void Set( uint32_t hash );
  uint32_t Get() const;

private:
  uint32_t m_hash = 0x811c9dc5;
};

#endif
