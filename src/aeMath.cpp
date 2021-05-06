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
// Defines
//------------------------------------------------------------------------------
const float aeMath::PI = 3.14159265358979323846f;
const float aeMath::TWO_PI = 2.0f * aeMath::PI;
const float aeMath::HALF_PI = 0.5f * aeMath::PI;
const float aeMath::QUARTER_PI = 0.25f * aeMath::PI;

//------------------------------------------------------------------------------
// aeFloat2 struct
//------------------------------------------------------------------------------
const aeFloat2 aeFloat2::Zero = aeFloat2( 0.0f );

aeFloat2::aeFloat2( const aeInt2& v )
  : x( v.x ), y( v.y )
{}

aeFloat2 aeFloat2::operator- () const
{
  return aeFloat2( -x, -y );
}

aeFloat2 aeFloat2::operator+ ( const aeFloat2 &v ) const
{
  return aeFloat2( x+v.x, y+v.y );
}

void aeFloat2::operator+= ( const aeFloat2 &v )
{
  x += v.x;
  y += v.y;
}

aeFloat2 aeFloat2::operator- ( const aeFloat2 &v ) const
{
  return aeFloat2( x - v.x, y - v.y );
}

void aeFloat2::operator-= (const aeFloat2 &v)
{
  x -= v.x;
  y -= v.y;
}

float aeFloat2::Dot( const aeFloat2 &v ) const
{
  return x * v.x + y * v.y;
}

float aeFloat2::Dot( const aeFloat2& a, const aeFloat2& b )
{
  return a.x * b.x + a.y * b.y;
}

float aeFloat2::Length() const
{
  return sqrtf( x * x + y * y );
}

float aeFloat2::LengthSquared() const
{
  return x * x + y * y;
}

float aeFloat2::Atan2() const
{
  return atan2( y, x );
}

aeFloat2 aeFloat2::operator* ( const float s ) const
{
  return aeFloat2( x*s, y*s );
}

void aeFloat2::operator*= ( const float s )
{
  x *= s;
  y *= s;
}

aeFloat2 aeFloat2::operator*( const aeFloat2 s ) const
{
  return aeFloat2( x * s.x, y * s.y );
}

void aeFloat2::operator*=( const aeFloat2 v )
{
  x *= v.x;
  y *= v.y;
}

aeFloat2 aeFloat2::operator/ ( const float s ) const
{
  return aeFloat2( x/s, y/s );
}

void aeFloat2::operator/= ( const float s )
{
  x /= s;
  y /= s;
}

aeFloat2 aeFloat2::operator/ ( const aeFloat2 s ) const
{
  return aeFloat2( x / s.x, y / s.y ); 
}

void aeFloat2::operator/= ( const aeFloat2 s )
{
  x /= s.x;
  y /= s.y;
}

aeFloat2 aeFloat2::Lerp(const aeFloat2& end, float t) const
{
  return aeFloat2( x * (1.0f-t) + end.x * t, y * (1.0f-t) + end.y * t );
}

float aeFloat2::Normalize()
{
  float length = sqrtf( x * x + y * y );
  x /= length;
  y /= length;
  return length;
}

aeFloat2 aeFloat2::NormalizeCopy() const
{
  float length = sqrtf( x * x + y * y );
  return aeFloat2( x / length, y / length );
}

float aeFloat2::SafeNormalize()
{
  float length = sqrtf( x * x + y * y );
  if ( length < 0.00001f )
  {
    x = 0.0f;
    y = 0.0f;
    return 0.0f;
  }

  x /= length;
  y /= length;
  return length;
}

aeFloat2 aeFloat2::SafeNormalizeCopy() const
{
  float l = sqrtf( x * x + y * y );
  if ( l < 0.00001f )
  {
    return aeFloat2( 0.0f );
  }
  return aeFloat2( x / l, y / l );
}

aeInt2 aeFloat2::NearestCopy() const
{
  return aeInt2( x + 0.5f, y + 0.5f );
}

aeInt2 aeFloat2::FloorCopy() const
{
  return aeInt2( aeMath::Floor( x ), aeMath::Floor( y ) );
}

aeInt2 aeFloat2::CeilCopy() const
{
  return aeInt2( aeMath::Ceil( x ), aeMath::Ceil( y ) );
}

aeFloat2 aeFloat2::RotateCopy( float rotation ) const
{
  float sinTheta = std::sin( rotation );
  float cosTheta = std::cos( rotation );
  return aeFloat2( x * cosTheta - y * sinTheta, x * sinTheta + y * cosTheta );
}

float aeFloat2::GetAngle() const
{
  if ( LengthSquared() < 0.0001f )
  {
    return 0.0f;
  }
  return aeMath::Atan2( y, x );
}

aeFloat2 aeFloat2::Reflect( aeFloat2 v, aeFloat2 n )
{
  return n * ( 2.0f * v.Dot( n ) / n.LengthSquared() ) - v;
}

//------------------------------------------------------------------------------
// aeInt2
//------------------------------------------------------------------------------
const aeInt2 aeInt2::Zero = aeInt2( 0 );
const aeInt2 aeInt2::Up = aeInt2( 0, 1 );
const aeInt2 aeInt2::Down = aeInt2( 0, -1 );
const aeInt2 aeInt2::Left = aeInt2( -1, 0 );
const aeInt2 aeInt2::Right = aeInt2( 1, 0 );

aeInt2::aeInt2( const aeInt3& v ) : x( v.x ), y( v.y ) {}

aeInt2 aeInt2::operator- () const
{
  return aeInt2( -x, -y );
}

aeInt2& aeInt2::operator+=( const aeInt2 &v )
{
  x += v.x;
  y += v.y;
  return *this;
}

aeInt2& aeInt2::operator-=( const aeInt2 &v )
{
  x -= v.x;
  y -= v.y;
  return *this;
}

aeInt2 aeInt2::operator+( const aeInt2 &v ) const
{
  return aeInt2( x + v.x, y + v.y );
}

aeInt2 aeInt2::operator-( const aeInt2 &v ) const
{
  return aeInt2( x - v.x, y - v.y );
}

aeInt2 aeInt2::operator* ( int32_t s ) const
{
  return aeInt2( x * s, y * s );
}

void aeInt2::operator*= ( int32_t s )
{
  x *= s;
  y *= s;
}

aeInt2 aeInt2::operator* ( const aeInt2& v ) const
{
  return aeInt2( x * v.x, y * v.y );
}

aeInt2& aeInt2::operator*= ( const aeInt2& v )
{
  x *= v.x;
  y *= v.y;
  return *this;
}

aeInt2 aeInt2::operator/ ( int32_t s ) const
{
  return aeInt2( x / s, y / s );
}

void aeInt2::operator/= ( int32_t s )
{
  x /= s;
  y /= s;
}

aeInt2 aeInt2::operator/ ( const aeInt2& v ) const
{
  return aeInt2( x / v.x, y / v.y );
}

aeInt2& aeInt2::operator/= ( const aeInt2& v )
{
  x /= v.x;
  y /= v.y;
  return *this;
}

aeFloat2 aeInt2::operator* ( const float s ) const
{
  return aeFloat2( x * s, y * s );
}

aeFloat2 aeInt2::operator/ ( const float s ) const
{
  return aeFloat2( x / s, y / s );
}

//------------------------------------------------------------------------------
// aeFloat3 struct
//------------------------------------------------------------------------------
const aeFloat3 aeFloat3::Zero     = aeFloat3(0.0f, 0.0f, 0.0f);
const aeFloat3 aeFloat3::Up       = aeFloat3(0.0f, 0.0f, 1.0f);
const aeFloat3 aeFloat3::Down     = aeFloat3(0.0f, 0.0f, -1.0f);
const aeFloat3 aeFloat3::Left     = aeFloat3(-1.0f, 0.0f, 0.0f);
const aeFloat3 aeFloat3::Right    = aeFloat3(1.0f, 0.0f, 0.0f);
const aeFloat3 aeFloat3::Forward  = aeFloat3(0.0f, 1.0f, 0.0f);
const aeFloat3 aeFloat3::Backward = aeFloat3(0.0f, -1.0f, 0.0f);

aeFloat3::aeFloat3( const aeInt3& v ) :
  x( v.x ),
  y( v.y ),
  z( v.z )
{}

aeFloat3 aeFloat3::operator+ (const aeFloat3& v) const
{
  return aeFloat3(x+v.x, y+v.y, z+v.z);
}

void aeFloat3::operator+= (const aeFloat3& v)
{
  x += v.x;
  y += v.y;
  z += v.z;
}

void aeFloat3::operator+=( const aeFloat2& v )
{
  x += v.x;
  y += v.y;
}

aeFloat3 aeFloat3::operator-( void ) const
{
  return aeFloat3(-x, -y, -z);
}

aeFloat3 aeFloat3::operator- (const aeFloat3& v) const
{
  return aeFloat3(x-v.x, y-v.y, z-v.z);
}

void aeFloat3::operator-= (const aeFloat3& v)
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
}

aeFloat3 aeFloat3::operator* (const float s) const
{
  return aeFloat3(x*s, y*s, z*s);
}

void aeFloat3::operator*= (const float s)
{
  x *= s;
  y *= s;
  z *= s;
}

aeFloat3 aeFloat3::operator/ (const float s) const
{
  return aeFloat3(x/s, y/s, z/s);
}

void aeFloat3::operator/= (const float s)
{
  x /= s;
  y /= s;
  z /= s;
}

aeFloat3 aeFloat3::operator* ( const aeFloat3& v ) const
{
  return aeFloat3( x * v.x, y * v.y, z * v.z );
}

aeFloat3& aeFloat3::operator*= ( const aeFloat3& v )
{
  x *= v.x;
  y *= v.y;
  z *= v.z;
  return *this;
}

aeFloat3 aeFloat3::Multiply( const aeFloat3& a, const aeFloat3& b )
{
  return aeFloat3( a.x * b.x, a.y * b.y, a.z * b.z );
}

aeFloat3 aeFloat3::operator/ ( const aeFloat3& v ) const
{
  return aeFloat3( x / v.x, y / v.y, z / v.z );
}

aeFloat3& aeFloat3::operator/= ( const aeFloat3& v )
{
  x /= v.x;
  y /= v.y;
  z /= v.z;
  return *this;
}

float aeFloat3::Dot(const aeFloat3& v) const
{
  return x * v.x + y * v.y + z * v.z;
}

float aeFloat3::Dot( const aeFloat3& a, const aeFloat3& b )
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

aeFloat3 aeFloat3::operator% (const aeFloat3& v) const
{
  return aeFloat3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

void aeFloat3::AddScaledVector(const aeFloat3& v, const float s)
{
  x += v.x * s;
  y += v.y * s;
  z += v.z * s;
}

aeFloat3 aeFloat3::ComponentProduct(const aeFloat3& v) const
{
  return aeFloat3(x*v.x, y*v.y, z*v.z);
}

void aeFloat3::ComponentProductUpdate(const aeFloat3& v)
{
  x *= v.x;
  y *= v.y;
  z *= v.z;
}

float aeFloat3::Length() const
{
  return std::sqrt(x*x + y*y + z*z);
}

float aeFloat3::LengthSquared() const
{
  return x*x + y*y + z*z;
}

float aeFloat3::GetDistance(const aeFloat3 &other) const
{
  return (*this - other).Length();
}

float aeFloat3::GetDistanceSquared(const aeFloat3 &other) const
{
  return (*this - other).LengthSquared();
}

aeFloat3& aeFloat3::Trim(const float s)
{
  if (LengthSquared() > s*s)
  {
    SafeNormalize();
    (*this) *= s;
  }
  return *this;
}

aeFloat3& aeFloat3::ZeroAxis( aeFloat3 axis )
{
  axis.SafeNormalize();
  *this -= axis * Dot( axis );
  return *this;
}

aeFloat3& aeFloat3::ZeroDirection( aeFloat3 direction )
{
  float d = Dot( direction );
  if ( d > 0.0f )
  {
    direction.SafeNormalize();
    *this -= direction * d;
  }
  return *this;
}

float aeFloat3::Normalize()
{
  float length = Length();
  x /= length;
  y /= length;
  z /= length;
  return length;
}

aeFloat3 aeFloat3::NormalizeCopy() const
{
  aeFloat3 copy = *this;
  copy.Normalize();
  return copy;
}

float aeFloat3::SafeNormalize()
{
  float length = Length();
  if ( length < 0.00001f )
  {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    return 0.0f;
  }
  
  x /= length;
  y /= length;
  z /= length;

  return length;
}

aeFloat3 aeFloat3::SafeNormalizeCopy() const
{
  aeFloat3 copy = *this;
  copy.SafeNormalize();
  return copy;
}

void aeFloat3::SetZero()
{
  x = y = z = 0.0f;
}

aeInt3 aeFloat3::NearestCopy() const
{
  return aeInt3(
    x + 0.5f,
    y + 0.5f,
    z + 0.5f
  );
}

aeInt3 aeFloat3::FloorCopy() const
{
  return aeInt3(
    aeMath::Floor( x ),
    aeMath::Floor( y ),
    aeMath::Floor( z )
  );
}

aeInt3 aeFloat3::CeilCopy() const
{
  return aeInt3(
    aeMath::Ceil( x ),
    aeMath::Ceil( y ),
    aeMath::Ceil( z )
  );
}

float aeFloat3::GetAngleBetween(const aeFloat3& v) const
{
  aeFloat3 crossProduct = *this % v;
  float dotProduct = aeFloat3::Dot( *this, v );
  if (crossProduct.LengthSquared() < 0.001f && dotProduct > 0.0f)
  {
    return 0.0f;
  }
  else if (crossProduct.LengthSquared() < 0.001f && dotProduct < 0.0f)
  {
    return aeMath::PI;
  }

  float angle = aeFloat3::Dot( *this, v );
  angle /= (*this).Length() * v.Length();
  angle = acos(angle);
  angle = std::abs(angle);
    
  return angle;
}

void aeFloat3::AddRotationXY(float rotation)
{
  float sinTheta = std::sin(rotation);
  float cosTheta = std::cos(rotation);
  float newX = x*cosTheta - y*sinTheta;
  float newY = x*sinTheta + y*cosTheta;
  x = newX;
  y = newY;
}

aeFloat3 aeFloat3::RotateCopy( aeFloat3 axis, float angle ) const
{
  // http://stackoverflow.com/questions/6721544/circular-rotation-around-an-arbitrary-axis
  axis.Normalize();
  float cosA = cosf( angle );
  float mCosA = 1.0f - cosA;
  float sinA = sinf( angle );
  
  aeFloat3 r0(
    cosA + axis.x * axis.x * mCosA,
    axis.x * axis.y * mCosA - axis.z * sinA,
    axis.x * axis.z * mCosA + axis.y * sinA );
  aeFloat3 r1(
    axis.y * axis.x * mCosA + axis.z * sinA,
    cosA + axis.y * axis.y * mCosA,
    axis.y * axis.z * mCosA - axis.x * sinA );
  aeFloat3 r2(
    axis.z * axis.x * mCosA - axis.y * sinA,
    axis.z * axis.y * mCosA + axis.x * sinA,
    cosA + axis.z * axis.z * mCosA );
  
  return aeFloat3( r0.Dot( *this ), r1.Dot( *this ), r2.Dot( *this ) );
}

aeFloat3 aeFloat3::Lerp(const aeFloat3& end, float t) const
{
  t = aeMath::Clip01(t);
  return aeFloat3( x * (1.0f-t) + end.x * t, y * (1.0f-t) + end.y * t, z * (1.0f-t) + end.z * t );
}

aeFloat3 aeFloat3::Slerp( const aeFloat3& end, float t ) const
{
  if ( Length() < 0.001f || end.Length() < 0.001f )
  {
    return aeFloat3( 0.0f );
  }

  aeFloat3 v0 = NormalizeCopy();
  aeFloat3 v1 = end.NormalizeCopy();
  
  float d = aeMath::Clip( v0.Dot( v1 ), -1.0f, 1.0f );
  if ( d > 0.999f )
  {
    return v1;
  }

  if ( d < -0.99f )
  {
    return v0;
  }

  float angle = acosf( d ) * t;
  aeFloat3 v2 = v1 - v0 * d;
  v2.Normalize();
  return ( ( v0 * cosf( angle ) ) + ( v2 * sinf( angle ) ) );
}

bool aeFloat3::operator==(const aeFloat3& v) const
{
  return x == v.x && y == v.y && z == v.z;
}

bool aeFloat3::operator!=(const aeFloat3& v) const
{
  return !(*this == v);
}

bool aeFloat3::operator<(const aeFloat3& v) const
{
  return x < v.x && y < v.y && z < v.z;
}

bool aeFloat3::operator>(const aeFloat3& v) const
{
  return x > v.x && y > v.y && z > v.z;
}

bool aeFloat3::operator<=(const aeFloat3& v) const
{
  return x <= v.x && y <= v.y && z <= v.z;
}

bool aeFloat3::operator>=(const aeFloat3& v) const
{
  return x >= v.x && y >= v.y && z >= v.z;
}

aeFloat3 aeFloat3::ProjectPoint( const aeFloat4x4& projection, aeFloat3 p )
{
  aeFloat4 projected = projection * aeFloat4( p, 1.0f );
  return projected.GetXYZ() / projected.w;
}

//------------------------------------------------------------------------------
// aeInt3
//------------------------------------------------------------------------------
aeInt3 aeInt3::operator+( const aeInt3 &v ) const
{
  return aeInt3( x + v.x, y + v.y, z + v.z );
}

aeInt3 aeInt3::operator-( const aeInt3 &v ) const
{
  return aeInt3( x - v.x, y - v.y, z - v.z );
}

aeInt3& aeInt3::operator+=( const aeInt3 &v )
{
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

aeInt3& aeInt3::operator-=( const aeInt3 &v )
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}

aeInt3 aeInt3::operator*( int32_t s ) const
{
  return aeInt3( x * s, y * s, z * s );
}

aeInt3 aeInt3::operator*( const aeInt3& s ) const
{
  return aeInt3( x * s.x, y * s.y, z * s.z );
}

aeInt3 aeInt3::operator/( int32_t s ) const
{
  return aeInt3( x / s, y / s, z / s );
}

aeInt3 aeInt3::operator/( const aeInt3& s ) const
{
  return aeInt3( x / s.x, y / s.y, z / s.z );
}

void aeInt3::operator*=( int32_t s )
{
  x *= s;
  y *= s;
  z *= s;
}

void aeInt3::operator*=( const aeInt3& s )
{
  x *= s.x;
  y *= s.y;
  z *= s.z;
}

void aeInt3::operator/=( int32_t s )
{
  x /= s;
  y /= s;
  z /= s;
}

void aeInt3::operator/=( const aeInt3& s )
{
  x /= s.x;
  y /= s.y;
  z /= s.z;
}

//------------------------------------------------------------------------------
// aeFloat4 struct
//------------------------------------------------------------------------------
const aeFloat4 aeFloat4::Zero = aeFloat4( 0.0f );

float aeFloat4::operator[] (unsigned i) const
{
  if (i == 0) return x;
  if (i == 1) return y;
  if (i == 2) return z;
  return w;
}

float& aeFloat4::operator[] (unsigned i)
{    
  if (i == 0) return x;
  if (i == 1) return y;
  if (i == 2) return z;
  return w;
}

aeFloat4 aeFloat4::operator-(void) const
{
  return aeFloat4(-x, -y, -z, -w);
}
  
aeFloat4 aeFloat4::operator+ (const aeFloat4& v) const
{
  return aeFloat4(x+v.x, y+v.y, z+v.z, w+v.w);
}
  
void aeFloat4::operator+= (const aeFloat4& v)
{
  x += v.x;
  y += v.y;
  z += v.z;
  w += v.w;
}
  
aeFloat4 aeFloat4::operator- (const aeFloat4& v) const
{
  return aeFloat4(x-v.x, y-v.y, z-v.z, w-v.w);
}

void aeFloat4::operator-= (const aeFloat4& v)
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  w -= v.w;
}

aeFloat4 aeFloat4::operator* (const float s) const
{
  return aeFloat4(x*s, y*s, z*s, w*s);
}
  
void aeFloat4::operator*= (const float s)
{
  x *= s;
  y *= s;
  z *= s;
  w *= s;
}

aeFloat4 aeFloat4::operator/ (const float s) const
{
  return aeFloat4(x/s, y/s, z/s, w/s);
}

void aeFloat4::operator/= (const float s)
{
  x /= s;
  y /= s;
  z /= s;
  w /= s;
}

float aeFloat4::operator* (const aeFloat4& v) const
{
  return x*v.x + y*v.y + z*v.z + w*v.w;
}

void aeFloat4::AddScaledVector(const aeFloat4& v, const float s)
{
  x += v.x * s;
  y += v.y + s;
  z += v.z + s;
  w += v.w + s;
}

aeFloat4 aeFloat4::ComponentProduct(const aeFloat4& v) const
{
  return aeFloat4(x*v.x, y*v.y, z*v.z, w*v.w);
}

void aeFloat4::ComponentProductUpdate(const aeFloat4& v)
{
  x *= v.x;
  y *= v.y;
  z *= v.z;
  w *= v.w;
}

void aeFloat4::Invert()
{
  x = -x;
  y = -y;
  z = -z;
  w = -w;
}

float aeFloat4::Length() const
{
  return std::sqrt(x*x + y*y + z*z + w*w);
}

float aeFloat4::LengthSquared() const
{
  return x*x + y*y + z*z + w*w;
}

void aeFloat4::Trim(const float s)
{
  if (LengthSquared() > s*s)
  {
    Normalize();
    (*this) *= s;
  }
}

float aeFloat4::Normalize()
{
  float length = Length();
  x /= length;
  y /= length;
  z /= length;
  return length;
}

aeFloat4 aeFloat4::NormalizeCopy() const
{
  aeFloat4 copy = *this;
  copy.Normalize();
  return copy;
}

float aeFloat4::SafeNormalize()
{
  float length = Length();
  if ( length < 0.00001f )
  {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    return 0.0f;
  }

  x /= length;
  y /= length;
  z /= length;

  return length;
}

aeFloat4 aeFloat4::SafeNormalizeCopy( void ) const
{
  aeFloat4 copy = *this;
  copy.SafeNormalize();
  return copy;
}

void aeFloat4::SetZero()
{
  x = y = z = w = 0.0f;
}

bool aeFloat4::operator==(const aeFloat4& v) const
{
  return x == v.x && y == v.y && z == v.z && w == v.w;
}

bool aeFloat4::operator!=(const aeFloat4& v) const
{
  return x != v.x || y != v.y || z != v.z || w != v.w;
}
  
bool aeFloat4::operator<(const aeFloat4& v) const
{
  return x < v.x && y < v.y && z < v.z && w < v.w;
}
  
bool aeFloat4::operator>(const aeFloat4& v) const
{
  return x > v.x && y > v.y && z > v.z && w > v.w;
}
  
bool aeFloat4::operator<=(const aeFloat4& v) const
{
  return x <= v.x && y <= v.y && z <= v.z && w <= v.w;
}
  
bool aeFloat4::operator>=(const aeFloat4& v) const
{
  return x >= v.x && y >= v.y && z >= v.z && w >= v.w;
}

//------------------------------------------------------------------------------
// aeQuat member functions
//------------------------------------------------------------------------------
aeQuat::aeQuat(aeFloat3 forward, aeFloat3 up, bool prioritizeUp)
{
  forward.Normalize();
  up.Normalize();

  aeFloat3 right = forward % up;
  right.Normalize();
  if (prioritizeUp)
  {
    up = right % forward;
  }
  else
  {
    forward = up % right;
  }

  #define m0 right
  #define m1 forward
  #define m2 up
  #define m00 m0.x
  #define m01 m1.x
  #define m02 m2.x
  #define m10 m0.y
  #define m11 m1.y
  #define m12 m2.y
  #define m20 m0.z
  #define m21 m1.z
  #define m22 m2.z

  //http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
  float trace = m00 + m11 + m22;
  if (trace > 0.0f)
  { 
    float S = sqrtf(trace + 1.0f) * 2;
    r = 0.25f * S;
    i = (m21 - m12) / S;
    j = (m02 - m20) / S; 
    k = (m10 - m01) / S; 
  }
  else if ((m00 > m11)&(m00 > m22))
  { 
    float S = sqrtf(1.0f + m00 - m11 - m22) * 2;
    r = (m21 - m12) / S;
    i = 0.25f * S;
    j = (m01 + m10) / S; 
    k = (m02 + m20) / S; 
  }
  else if (m11 > m22)
  { 
    float S = sqrtf(1.0f + m11 - m00 - m22) * 2;
    r = (m02 - m20) / S;
    i = (m01 + m10) / S; 
    j = 0.25f * S;
    k = (m12 + m21) / S; 
  }
  else
  { 
    float S = sqrtf(1.0f + m22 - m00 - m11) * 2;
    r = (m10 - m01) / S;
    i = (m02 + m20) / S;
    j = (m12 + m21) / S;
    k = 0.25f * S;
  }

  #undef m0
  #undef m1
  #undef m2
  #undef m00
  #undef m01
  #undef m02
  #undef m10
  #undef m11
  #undef m12
  #undef m20
  #undef m21
  #undef m22
}

aeQuat::aeQuat(aeFloat3 axis, float angle)
{
  axis.Normalize();
  float sinAngleDiv2 = sinf(angle / 2.0f);
  i = axis.x * sinAngleDiv2;
  j = axis.y * sinAngleDiv2;
  k = axis.z * sinAngleDiv2;
  r = cosf(angle / 2.0f);
}

void aeQuat::Normalize()
{
  float invMagnitude = r*r + i*i + j*j + k*k;
    
  if (invMagnitude == 0.0f)
  {
    r = 1;
    return;
  }
    
  invMagnitude = 1.0f / std::sqrt(invMagnitude);
  r *= invMagnitude;
  i *= invMagnitude;
  j *= invMagnitude;
  k *= invMagnitude;
}

bool aeQuat::operator==(const aeQuat& q) const
{
  return ( i == q.i ) && ( j == q.j ) && ( k == q.k ) && ( r == q.r );
}

bool aeQuat::operator!=(const aeQuat& q) const
{
  return !operator==(q);
}
  
aeQuat& aeQuat::operator*= (const aeQuat& q)
{
  //http://www.mathworks.com/help/aeroblks/quaternionmultiplication.html
  aeQuat copy = *this;
  r = copy.r*q.r - copy.i*q.i - copy.j*q.j - copy.k*q.k;
  i = copy.r*q.i + copy.i*q.r + copy.j*q.k - copy.k*q.j;
  j = copy.r*q.j + copy.j*q.r + copy.k*q.i - copy.i*q.k;
  k = copy.r*q.k + copy.k*q.r + copy.i*q.j - copy.j*q.i;
  return *this;
}

aeQuat aeQuat::operator* (const aeQuat& q) const
{
  return aeQuat(*this) *= q;
}

aeQuat const aeQuat::operator*(float s) const
{
  return aeQuat(s*i,s*j,s*k,s*r);
}

void aeQuat::AddScaledVector(const aeFloat3& v, float t)
{
  aeQuat q(v.x*t, v.y*t, v.z*t, 0.0f);
  q *= *this;

  r += q.r * 0.5f;
  i += q.i * 0.5f;
  j += q.j * 0.5f;
  k += q.k * 0.5f;
}

void aeQuat::RotateByVector(const aeFloat3& v)
{
  aeQuat q = aeQuat::Identity();

  float s = v.Length();
  //ASSERT_MSG(s > 0.001f, "Can't rotate by a zero vector!");
  q.r = cosf(s * 0.5f);

  aeFloat3 n = v.NormalizeCopy() * sinf(s * 0.5f);
  q.i = n.x;
  q.j = n.y;
  q.k = n.z;

  (*this) *= q;

}

void aeQuat::SetDirectionXY(const aeFloat3& v)
{
  float theta = std::atan(v.y/v.x);
  if (v.x < 0 && v.y >= 0)
    theta += aeMath::PI;
  else if (v.x < 0 && v.y < 0)
    theta -= aeMath::PI;

  r = std::cos(theta/2.0f);
  i = 0.0f;
  j = 0.0f;
  k = std::sin(theta/2.0f);
}

aeFloat3 aeQuat::GetDirectionXY() const
{
  float theta;
  if (k >= 0.0f)
    theta = 2.0f * std::acos(r);
  else
    theta = -2.0f * std::acos(r);

  return aeFloat3(std::cos(theta), std::sin(theta), 0.0f);
}

void aeQuat::ZeroXY()
{
  i = 0.0f;
  j = 0.0f;
}

void aeQuat::GetAxisAngle( aeFloat3* axis, float* angle ) const
{
  *angle = 2 * acos(r);
  axis->x = i / sqrt(1-r*r);
  axis->y = j / sqrt(1-r*r);
  axis->z = k / sqrt(1-r*r);
}

void aeQuat::AddRotationXY(float rotation)
{
  float sinThetaOver2 = std::sin(rotation/2.0f);
  float cosThetaOver2 = std::cos(rotation/2.0f);

  // create a quaternion representing the amount to rotate
  aeQuat change(0.0f, 0.0f, sinThetaOver2, cosThetaOver2 );
  change.Normalize();

  // apply the change in rotation
  (*this) *= change;
}

aeQuat aeQuat::Nlerp( aeQuat d, float t ) const
{
  float epsilon = this->Dot(d);
  aeQuat end = d;

  if(epsilon < 0.0f)
  {
    epsilon = -epsilon;

    end = aeQuat(-d.i, -d.j, -d.k, -d.r);
  }

  aeQuat result = (*this) * (1.0f - t);
  end = end * t;

  result.i += end.i;
  result.j += end.j;
  result.k += end.k;
  result.r += end.r;
  result.Normalize();

  return result;
}

aeFloat4x4 aeQuat::GetTransformMatrix(void) const
{
  aeQuat n = *this;
  n.Normalize();

  aeFloat4x4 matrix = aeFloat4x4::Identity();

  matrix.data[0]  = 1.0f-2.0f*n.j*n.j - 2.0f*n.k*n.k;
  matrix.data[1]  = 2.0f*n.i*n.j - 2.0f*n.r*n.k;
  matrix.data[2]  = 2.0f*n.i*n.k + 2.0f*n.r*n.j;

  matrix.data[4]  = 2.0f* n.i* n.j + 2.0f* n.r* n.k;
  matrix.data[5]  = 1.0f-2.0f* n.i* n.i- 2.0f* n.k* n.k;
  matrix.data[6]  = 2.0f* n.j* n.k - 2.0f* n.r* n.i;

  matrix.data[8]  = 2.0f* n.i* n.k - 2.0f* n.r* n.j;
  matrix.data[9]  = 2.0f* n.j* n.k + 2.0f* n.r* n.i;
  matrix.data[10] = 1.0f-2.0f* n.i* n.i - 2.0f* n.j* n.j;

  return matrix;
}

aeQuat aeQuat::GetInverse( void ) const
{
  return aeQuat(*this).SetInverse();
}

aeQuat& aeQuat::SetInverse( void )
{
  //http://www.mathworks.com/help/aeroblks/quaternioninverse.html
  float d = r*r + i*i + j*j + k*k;
  r /=  d;
  i /= -d;
  j /= -d;
  k /= -d;

  return *this;
}

aeFloat3 aeQuat::Rotate( aeFloat3 v ) const
{
  //http://www.mathworks.com/help/aeroblks/quaternionrotation.html
  aeQuat q = (*this) * aeQuat(v) * this->GetInverse();
  return aeFloat3(q.i, q.j, q.k);
}

float aeQuat::Dot( const aeQuat& q ) const
{
  return (q.r*r) + (q.i*i) + (q.j*j) + (q.k*k);
}

//------------------------------------------------------------------------------
// aeFloat4x4 member functions
//------------------------------------------------------------------------------
aeFloat4x4 aeFloat4x4::Translation( const aeFloat3& p )
{
  aeFloat4x4 r = aeFloat4x4::Identity();
  r.SetTranslate( p );
  return r;
}

aeFloat4x4 aeFloat4x4::Rotation( aeFloat3 forward0, aeFloat3 up0, aeFloat3 forward1, aeFloat3 up1 )
{
  // Remove rotation
  forward0.Normalize();
  up0.Normalize();

  aeFloat3 right0 = forward0 % up0;
  right0.Normalize();
  up0 = right0 % forward0;

  aeFloat4x4 removeRotation;
  memset( &removeRotation, 0, sizeof( removeRotation ) );
  removeRotation.SetRowVector( 0, right0 ); // right -> ( 1, 0, 0 )
  removeRotation.SetRowVector( 1, forward0 ); // forward -> ( 0, 1, 0 )
  removeRotation.SetRowVector( 2, up0 ); // up -> ( 0, 0, 1 )
  removeRotation.data[ 15 ] = 1;

  // Rotate
  forward1.Normalize();
  up1.Normalize();

  aeFloat3 right1 = forward1 % up1;
  right1.Normalize();
  up1 = right1 % forward1;

  aeFloat4x4 newRotation;
  memset( &newRotation, 0, sizeof( newRotation ) );
  // Set axis vector to invert (transpose)
  newRotation.SetAxisVector( 0, right1 ); // ( 1, 0, 0 ) -> right
  newRotation.SetAxisVector( 1, forward1 ); // ( 0, 1, 0 ) -> forward
  newRotation.SetAxisVector( 2, up1 ); // ( 0, 0, 1 ) -> up
  newRotation.data[ 15 ] = 1;

  return newRotation * removeRotation;
}

aeFloat4x4 aeFloat4x4::RotationX( float angle )
{
  aeFloat4x4 result; return result.SetRotateX( angle );
}

aeFloat4x4 aeFloat4x4::RotationY( float angle )
{
  aeFloat4x4 result; return result.SetRotateY( angle );
}

aeFloat4x4 aeFloat4x4::RotationZ( float angle )
{
  aeFloat4x4 result; return result.SetRotateZ( angle );
}

aeFloat4x4 aeFloat4x4::Scaling( const aeFloat3& s )
{
  aeFloat4x4 r = aeFloat4x4::Identity();
  r.SetScale( s );
  return r;
}

aeFloat4x4 aeFloat4x4::WorldToView( aeFloat3 position, aeFloat3 forward, aeFloat3 up )
{
  //xaxis.x  xaxis.y  xaxis.z  dot(xaxis, -eye)
  //yaxis.x  yaxis.y  yaxis.z  dot(yaxis, -eye)
  //zaxis.x  zaxis.y  zaxis.z  dot(zaxis, -eye)
  //0        0        0        1

  position = -position;
  forward.Normalize();
  up.Normalize();

  aeFloat3 right = forward % up;
  right.Normalize();
  up = right % forward;

  aeFloat4x4 result;
  memset( &result, 0, sizeof( result ) );
  result.SetRowVector( 0, right );
  result.SetRowVector( 1, up );
  result.SetRowVector( 2, -forward ); // @TODO: Seems a little sketch to flip handedness here
  result.SetAxisVector( 3, aeFloat3( position.Dot( right ), position.Dot( up ), position.Dot( -forward ) ) );
  result.data[ 15 ] = 1;
  return result;
}

// this hack comes in from the renderer
extern bool gReverseZ;

// fix the projection matrix, when false fov scales up/down with nearPlane
// if this breaks stuff, then can set to false
bool gFixProjection = true;

aeFloat4x4 aeFloat4x4::ViewToProjection( float fov, float aspectRatio, float nearPlane, float farPlane )
{
  // a  0  0  0
  // 0  b  0  0
  // 0  0  A  B
  // 0  0 -1  0

  // this is assuming a symmetric frustum, in this case nearPlane cancels out
  
  float halfAngleTangent = tanf( fov * 0.5f);
  float r = aspectRatio * halfAngleTangent; // scaled by view aspect ratio
  float t = halfAngleTangent; // tan of half angle fit vertically
	  
  if ( gFixProjection )
  {
	  r *= nearPlane;
	  t *= nearPlane;
  }
  float a = nearPlane / r;
  float b = nearPlane / t;
 	
  float A;
  float B;
  if (gReverseZ)
  {
	  A = 0;
	  B = nearPlane;
  }
  else
  {
	  A = -( farPlane + nearPlane ) / ( farPlane - nearPlane );
	  B = ( -2.0f * farPlane * nearPlane ) / ( farPlane - nearPlane );
  }
  
  aeFloat4x4 result;
  memset( &result, 0, sizeof( result ) );
  result.data[ 0 ] = a;
  result.data[ 5 ] = b;
  result.data[ 10 ] = A;
  result.data[ 11 ] = B;
  result.data[ 14 ] = -1;
  return result;
}

aeFloat4 aeFloat4x4::operator*(const aeFloat4& v) const
{
  return aeFloat4(
    v.x*data[0]  + v.y*data[1]  + v.z*data[2]  + v.w*data[3],
    v.x*data[4]  + v.y*data[5]  + v.z*data[6]  + v.w*data[7],
    v.x*data[8]  + v.y*data[9]  + v.z*data[10] + v.w*data[11],
    v.x*data[12] + v.y*data[13] + v.z*data[14] + v.w*data[15]);
}

aeFloat4x4 aeFloat4x4::operator*(const aeFloat4x4& m) const
{
  return aeFloat4x4(
    (data[0]*m.data[0])  + (data[1]*m.data[4])  + (data[2]*m.data[8])  + (data[3]*m.data[12]),
    (data[0]*m.data[1])  + (data[1]*m.data[5])  + (data[2]*m.data[9])  + (data[3]*m.data[13]),
    (data[0]*m.data[2])  + (data[1]*m.data[6])  + (data[2]*m.data[10])  + (data[3]*m.data[14]),
    (data[0]*m.data[3])  + (data[1]*m.data[7])  + (data[2]*m.data[11])  + (data[3]*m.data[15]),

    (data[4]*m.data[0])  + (data[5]*m.data[4])  + (data[6]*m.data[8])  + (data[7]*m.data[12]),
    (data[4]*m.data[1])  + (data[5]*m.data[5])  + (data[6]*m.data[9])  + (data[7]*m.data[13]),
    (data[4]*m.data[2])  + (data[5]*m.data[6])  + (data[6]*m.data[10])  + (data[7]*m.data[14]),
    (data[4]*m.data[3])  + (data[5]*m.data[7])  + (data[6]*m.data[11])  + (data[7]*m.data[15]),

    (data[8]*m.data[0])  + (data[9]*m.data[4])  + (data[10]*m.data[8]) + (data[11]*m.data[12]),
    (data[8]*m.data[1])  + (data[9]*m.data[5])  + (data[10]*m.data[9]) + (data[11]*m.data[13]),
    (data[8]*m.data[2])  + (data[9]*m.data[6])  + (data[10]*m.data[10]) + (data[11]*m.data[14]),
    (data[8]*m.data[3])  + (data[9]*m.data[7])  + (data[10]*m.data[11]) + (data[11]*m.data[15]),

    (data[12]*m.data[0]) + (data[13]*m.data[4]) + (data[14]*m.data[8]) + (data[15]*m.data[12]),
    (data[12]*m.data[1]) + (data[13]*m.data[5]) + (data[14]*m.data[9]) + (data[15]*m.data[13]),
    (data[12]*m.data[2]) + (data[13]*m.data[6]) + (data[14]*m.data[10]) + (data[15]*m.data[14]),
    (data[12]*m.data[3]) + (data[13]*m.data[7]) + (data[14]*m.data[11]) + (data[15]*m.data[15]));
}

void aeFloat4x4::operator*=(const aeFloat4x4& m)
{
  float t1;
  float t2;
  float t3;
  float t4;

  t1 = (data[0]*m.data[0]) + (data[1]*m.data[4]) + (data[2]*m.data[8])  + (data[3]*m.data[12]);
  t2 = (data[0]*m.data[1]) + (data[1]*m.data[5]) + (data[2]*m.data[9])  + (data[3]*m.data[13]);
  t3 = (data[0]*m.data[2]) + (data[1]*m.data[6]) + (data[2]*m.data[10]) + (data[3]*m.data[14]);
  t4 = (data[0]*m.data[3]) + (data[1]*m.data[7]) + (data[2]*m.data[11]) + (data[3]*m.data[15]);
  data[0] = t1;
  data[1] = t2;
  data[2] = t3;
  data[3] = t4;

  t1 = (data[4]*m.data[0]) + (data[5]*m.data[4]) + (data[6]*m.data[8])  + (data[7]*m.data[12]);
  t2 = (data[4]*m.data[1]) + (data[5]*m.data[5]) + (data[6]*m.data[9])  + (data[7]*m.data[13]);
  t3 = (data[4]*m.data[2]) + (data[5]*m.data[6]) + (data[6]*m.data[10]) + (data[7]*m.data[14]);
  t4 = (data[4]*m.data[3]) + (data[5]*m.data[7]) + (data[6]*m.data[11]) + (data[7]*m.data[15]);
  data[4] = t1;
  data[5] = t2;
  data[6] = t3;
  data[7] = t4;

  t1 = (data[8]*m.data[0]) + (data[9]*m.data[4]) + (data[10]*m.data[8])  + (data[11]*m.data[12]);
  t2 = (data[8]*m.data[1]) + (data[9]*m.data[5]) + (data[10]*m.data[9])  + (data[11]*m.data[13]);
  t3 = (data[8]*m.data[2]) + (data[9]*m.data[6]) + (data[10]*m.data[10]) + (data[11]*m.data[14]);
  t4 = (data[8]*m.data[3]) + (data[9]*m.data[7]) + (data[10]*m.data[11]) + (data[11]*m.data[15]);
  data[8]  = t1;
  data[9]  = t2;
  data[10] = t3;
  data[11] = t4;

  t1 = (data[12]*m.data[0]) + (data[13]*m.data[4]) + (data[14]*m.data[8])  + (data[15]*m.data[12]);
  t2 = (data[12]*m.data[1]) + (data[13]*m.data[5]) + (data[14]*m.data[9])  + (data[15]*m.data[13]);
  t3 = (data[12]*m.data[2]) + (data[13]*m.data[6]) + (data[14]*m.data[10]) + (data[15]*m.data[14]);
  t4 = (data[12]*m.data[3]) + (data[13]*m.data[7]) + (data[14]*m.data[11]) + (data[15]*m.data[15]);
  data[12] = t1;
  data[13] = t2;
  data[14] = t3;
  data[15] = t4;
}

bool aeFloat4x4::SetInverse(const aeFloat4x4& m)
{
  float inv[16], det;
  int i;

  inv[0] = m.data[5]  * m.data[10] * m.data[15] - 
    m.data[5]  * m.data[11] * m.data[14] - 
    m.data[9]  * m.data[6]  * m.data[15] + 
    m.data[9]  * m.data[7]  * m.data[14] +
    m.data[13] * m.data[6]  * m.data[11] - 
    m.data[13] * m.data[7]  * m.data[10];

  inv[4] = -m.data[4]  * m.data[10] * m.data[15] + 
    m.data[4]  * m.data[11] * m.data[14] + 
    m.data[8]  * m.data[6]  * m.data[15] - 
    m.data[8]  * m.data[7]  * m.data[14] - 
    m.data[12] * m.data[6]  * m.data[11] + 
    m.data[12] * m.data[7]  * m.data[10];

  inv[8] = m.data[4]  * m.data[9] * m.data[15] - 
    m.data[4]  * m.data[11] * m.data[13] - 
    m.data[8]  * m.data[5] * m.data[15] + 
    m.data[8]  * m.data[7] * m.data[13] + 
    m.data[12] * m.data[5] * m.data[11] - 
    m.data[12] * m.data[7] * m.data[9];

  inv[12] = -m.data[4]  * m.data[9] * m.data[14] + 
    m.data[4]  * m.data[10] * m.data[13] +
    m.data[8]  * m.data[5] * m.data[14] - 
    m.data[8]  * m.data[6] * m.data[13] - 
    m.data[12] * m.data[5] * m.data[10] + 
    m.data[12] * m.data[6] * m.data[9];

  inv[1] = -m.data[1]  * m.data[10] * m.data[15] + 
    m.data[1]  * m.data[11] * m.data[14] + 
    m.data[9]  * m.data[2] * m.data[15] - 
    m.data[9]  * m.data[3] * m.data[14] - 
    m.data[13] * m.data[2] * m.data[11] + 
    m.data[13] * m.data[3] * m.data[10];

  inv[5] = m.data[0]  * m.data[10] * m.data[15] - 
    m.data[0]  * m.data[11] * m.data[14] - 
    m.data[8]  * m.data[2] * m.data[15] + 
    m.data[8]  * m.data[3] * m.data[14] + 
    m.data[12] * m.data[2] * m.data[11] - 
    m.data[12] * m.data[3] * m.data[10];

  inv[9] = -m.data[0]  * m.data[9] * m.data[15] + 
    m.data[0]  * m.data[11] * m.data[13] + 
    m.data[8]  * m.data[1] * m.data[15] - 
    m.data[8]  * m.data[3] * m.data[13] - 
    m.data[12] * m.data[1] * m.data[11] + 
    m.data[12] * m.data[3] * m.data[9];

  inv[13] = m.data[0]  * m.data[9] * m.data[14] - 
    m.data[0]  * m.data[10] * m.data[13] - 
    m.data[8]  * m.data[1] * m.data[14] + 
    m.data[8]  * m.data[2] * m.data[13] + 
    m.data[12] * m.data[1] * m.data[10] - 
    m.data[12] * m.data[2] * m.data[9];

  inv[2] = m.data[1]  * m.data[6] * m.data[15] - 
    m.data[1]  * m.data[7] * m.data[14] - 
    m.data[5]  * m.data[2] * m.data[15] + 
    m.data[5]  * m.data[3] * m.data[14] + 
    m.data[13] * m.data[2] * m.data[7] - 
    m.data[13] * m.data[3] * m.data[6];

  inv[6] = -m.data[0]  * m.data[6] * m.data[15] + 
    m.data[0]  * m.data[7] * m.data[14] + 
    m.data[4]  * m.data[2] * m.data[15] - 
    m.data[4]  * m.data[3] * m.data[14] - 
    m.data[12] * m.data[2] * m.data[7] + 
    m.data[12] * m.data[3] * m.data[6];

  inv[10] = m.data[0]  * m.data[5] * m.data[15] - 
    m.data[0]  * m.data[7] * m.data[13] - 
    m.data[4]  * m.data[1] * m.data[15] + 
    m.data[4]  * m.data[3] * m.data[13] + 
    m.data[12] * m.data[1] * m.data[7] - 
    m.data[12] * m.data[3] * m.data[5];

  inv[14] = -m.data[0]  * m.data[5] * m.data[14] + 
    m.data[0]  * m.data[6] * m.data[13] + 
    m.data[4]  * m.data[1] * m.data[14] - 
    m.data[4]  * m.data[2] * m.data[13] - 
    m.data[12] * m.data[1] * m.data[6] + 
    m.data[12] * m.data[2] * m.data[5];

  inv[3] = -m.data[1] * m.data[6] * m.data[11] + 
    m.data[1] * m.data[7] * m.data[10] + 
    m.data[5] * m.data[2] * m.data[11] - 
    m.data[5] * m.data[3] * m.data[10] - 
    m.data[9] * m.data[2] * m.data[7] + 
    m.data[9] * m.data[3] * m.data[6];

  inv[7] = m.data[0] * m.data[6] * m.data[11] - 
    m.data[0] * m.data[7] * m.data[10] - 
    m.data[4] * m.data[2] * m.data[11] + 
    m.data[4] * m.data[3] * m.data[10] + 
    m.data[8] * m.data[2] * m.data[7] - 
    m.data[8] * m.data[3] * m.data[6];

  inv[11] = -m.data[0] * m.data[5] * m.data[11] + 
    m.data[0] * m.data[7] * m.data[9] + 
    m.data[4] * m.data[1] * m.data[11] - 
    m.data[4] * m.data[3] * m.data[9] - 
    m.data[8] * m.data[1] * m.data[7] + 
    m.data[8] * m.data[3] * m.data[5];

  inv[15] = m.data[0] * m.data[5] * m.data[10] - 
    m.data[0] * m.data[6] * m.data[9] - 
    m.data[4] * m.data[1] * m.data[10] + 
    m.data[4] * m.data[2] * m.data[9] + 
    m.data[8] * m.data[1] * m.data[6] - 
    m.data[8] * m.data[2] * m.data[5];

  det = m.data[0] * inv[0] + m.data[1] * inv[4] + m.data[2] * inv[8] + m.data[3] * inv[12];

  if (det == 0)
    return false;

  det = 1.0f / det;

  for (i = 0; i < 16; i++)
    data[i] = inv[i] * det;

  return true;
}

aeFloat4x4 aeFloat4x4::Inverse() const
{
  aeFloat4x4 inverse = aeFloat4x4::Identity();
  inverse.SetInverse(*this);
  return inverse;
}

bool aeFloat4x4::Invert()
{
  return SetInverse(*this);
}

aeFloat4x4& aeFloat4x4::SetOrientation( const aeQuat& q2 )
{
  aeQuat q = q2.GetInverse();

  data[0] = 1 - (2*q.j*q.j + 2*q.k*q.k);
  data[1] = 2*q.i*q.j + 2*q.k*q.r;
  data[2] = 2*q.i*q.k - 2*q.j*q.r;

  data[4] = 2*q.i*q.j - 2*q.k*q.r;
  data[5] = 1 - (2*q.i*q.i  + 2*q.k*q.k);
  data[6] = 2*q.j*q.k + 2*q.i*q.r;

  data[8] = 2*q.i*q.k + 2*q.j*q.r;
  data[9] = 2*q.j*q.k - 2*q.i*q.r;
  data[10] = 1 - (2*q.i*q.i  + 2*q.j*q.j);

  return *this;
}

aeFloat3 aeFloat4x4::TransformInverse(const aeFloat3& v) const
{
  aeFloat3 copy = v;

  copy.x -= data[3];
  copy.y -= data[7];
  copy.z -= data[11];

  return aeFloat3(
    copy.x*data[0] + copy.y*data[4] + copy.z*data[8],
    copy.x*data[1] + copy.y*data[5] + copy.z*data[9],
    copy.x*data[2] + copy.y*data[6] + copy.z*data[10]);
}

aeFloat3 aeFloat4x4::TransformDirection(const aeFloat3& v) const
{
  return aeFloat3(
    v.x*data[0] + v.y*data[1] + v.z*data[2],
    v.x*data[4] + v.y*data[5] + v.z*data[6],
    v.x*data[8] + v.y*data[9] + v.z*data[10]);
}

aeFloat3 aeFloat4x4::TransformInverseDirection(const aeFloat3& v) const
{
  return aeFloat3(
    v.x*data[0] + v.y*data[4] + v.z*data[8],
    v.x*data[1] + v.y*data[5] + v.z*data[9],
    v.x*data[2] + v.y*data[6] + v.z*data[10]);
}

void aeFloat4x4::SetDiagonal(float d1, float d2, float d3)
{
  data[0]  = d1;
  data[5]  = d2;
  data[10] = d3;
}

aeFloat3 aeFloat4x4::GetAxisVector(int col) const
{
  return aeFloat3(data[col], data[col+4], data[col+8]);
}

aeFloat3 aeFloat4x4::GetPosition() const
{
  return aeFloat3( data[ 3 ], data[ 7 ], data[ 11 ] );
}

aeQuat aeFloat4x4::GetRotation() const
{
  // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/

  aeFloat4x4 t = *this;
  t.RemoveScaling();

  #define m00 t.data[ 0 ]
  #define m01 t.data[ 1 ]
  #define m02 t.data[ 2 ]
  #define m10 t.data[ 4 ]
  #define m11 t.data[ 5 ]
  #define m12 t.data[ 6 ]
  #define m20 t.data[ 8 ]
  #define m21 t.data[ 9 ]
  #define m22 t.data[ 10 ]

  float trace = m00 + m11 + m22;
  if ( trace > 0.0f )
  { 
    float s = sqrt( trace + 1.0f ) * 2.0f;
    return aeQuat(
      ( m21 - m12 ) / s,
      ( m02 - m20 ) / s,
      ( m10 - m01 ) / s,
      0.25f * s
    );
  }
  else if ( ( m00 > m11 ) && ( m00 > m22 ) )
  { 
    float s = sqrt( 1.0f + m00 - m11 - m22 ) * 2.0f;
    return aeQuat(
      0.25f * s,
      ( m01 + m10 ) / s,
      ( m02 + m20 ) / s,
      ( m21 - m12 ) / s
    );
  }
  else if ( m11 > m22 )
  { 
    float s = sqrt( 1.0f + m11 - m00 - m22 ) * 2.0f;
    return aeQuat(
      ( m01 + m10 ) / s,
      0.25f * s,
      ( m12 + m21 ) / s,
      ( m02 - m20 ) / s
    );
  }
  else
  { 
    float s = sqrt( 1.0f + m22 - m00 - m11 ) * 2.0f;
    return aeQuat(
      ( m02 + m20 ) / s,
      ( m12 + m21 ) / s,
      0.25f * s,
      ( m10 - m01 ) / s
    );
  }

  #undef m00
  #undef m01
  #undef m02
  #undef m10
  #undef m11
  #undef m12
  #undef m20
  #undef m21
  #undef m22
}

void aeFloat4x4::SetPosition( const aeFloat3& p )
{
  data[ 3 ] = p.x;
  data[ 7 ] = p.y;
  data[ 11 ] = p.z;
}

void aeFloat4x4::SetAxisVector(unsigned col, const aeFloat3 &v)
{
  data[col] = v.x;
  data[col+4] = v.y;
  data[col+8] = v.z;
}

aeFloat4 aeFloat4x4::GetRowVector(int row) const
{
  return aeFloat4(data[row*4], data[row*4+1], data[row*4+2], data[row*4+3]);
}

void aeFloat4x4::SetRowVector(unsigned row, const aeFloat3 &v)
{
  data[row*4]   = v.x;
  data[row*4+1] = v.y;
  data[row*4+2] = v.z;
}

void aeFloat4x4::SetRowVector(unsigned row, const aeFloat4 &v)
{
  data[row*4]   = v.x;
  data[row*4+1] = v.y;
  data[row*4+2] = v.z;
  data[row*4+3] = v.w;
}

aeFloat4x4& aeFloat4x4::SetIdentity(void){
  data[0]  = 1.0f; data[1] = 0.0f;  data[2] = 0.0f;  data[3] = 0.0f;
  data[4]  = 0.0f; data[5] = 1.0f;  data[6] = 0.0f;  data[7] = 0.0f;
  data[8]  = 0.0f; data[9] = 0.0f;  data[10] = 1.0f; data[11] = 0.0f;
  data[12] = 0.0f; data[13] = 0.0f; data[14] = 0.0f; data[15] = 1.0f;
  return *this;
}

aeFloat4x4& aeFloat4x4::SetTranslate(float X, float Y, float Z){
  data[0]  = 1.0f; data[1] = 0.0f;  data[2] = 0.0f;  data[3] = X;
  data[4]  = 0.0f; data[5] = 1.0f;  data[6] = 0.0f;  data[7] = Y;
  data[8]  = 0.0f; data[9] = 0.0f;  data[10] = 1.0f; data[11] = Z;
  data[12] = 0.0f; data[13] = 0.0f; data[14] = 0.0f; data[15] = 1.0f;
  return *this;
}

aeFloat4x4& aeFloat4x4::SetTranslate(const aeFloat3 &translation){
  return SetTranslate(translation.x, translation.y, translation.z);
}

aeFloat4x4& aeFloat4x4::SetTranslation( float x, float y, float z )
{
  data[ 3 ] = x;
  data[ 7 ] = y;
  data[ 11 ] = z;
  data[ 15 ] = 1.0f;
  return *this;
}

aeFloat4x4& aeFloat4x4::SetTranslation( const aeFloat3& translation )
{
  data[ 3 ] = translation.x;
  data[ 7 ] = translation.y;
  data[ 11 ] = translation.z;
  data[ 15 ] = 1.0f;
  return *this;
}

aeFloat3 aeFloat4x4::GetTranslation() const
{
  return aeFloat3( data[ 3 ], data[ 7 ], data[ 11 ] );
}

aeFloat3 aeFloat4x4::GetScale() const
{
  return aeFloat3(
    aeFloat3( data[ 0 ], data[ 4 ], data[ 8 ] ).Length(),
    aeFloat3( data[ 1 ], data[ 5 ], data[ 9 ] ).Length(),
    aeFloat3( data[ 2 ], data[ 6 ], data[ 10 ] ).Length()
  );
}

aeFloat4x4& aeFloat4x4::SetScale(float X, float Y, float Z){
  data[0]  = X;    data[1] = 0.0f;  data[2] = 0.0f;  data[3] = 0.0f;
  data[4]  = 0.0f; data[5] = Y;     data[6] = 0.0f;  data[7] = 0.0f;
  data[8]  = 0.0f; data[9] = 0.0f;  data[10] = Z;    data[11] = 0.0f;
  data[12] = 0.0f; data[13] = 0.0f; data[14] = 0.0f; data[15] = 1.0f;
  return *this;
}

aeFloat4x4& aeFloat4x4::SetScaleKeepTranslate(float X, float Y, float Z){
  data[0]  = X;    data[1] = 0.0f;  data[2] = 0.0f;
  data[4]  = 0.0f; data[5] = Y;     data[6] = 0.0f;
  data[8]  = 0.0f; data[9] = 0.0f;  data[10] = Z;
  data[12] = 0.0f; data[13] = 0.0f; data[14] = 0.0f; data[15] = 1.0f;
  return *this;
}

aeFloat4x4& aeFloat4x4::SetScale(const aeFloat3 &scale){
  return SetScale(scale.x, scale.y, scale.z);
}

aeFloat4x4& aeFloat4x4::SetScaleKeepTranslate(const aeFloat3 &scale){
  return SetScaleKeepTranslate(scale.x, scale.y, scale.z);
}

aeFloat4x4& aeFloat4x4::SetRotateX(float angle){
  data[0]  = 1.0f; data[1] = 0.0f;             data[2] = 0.0f;              data[3] = 0.0f;
  data[4]  = 0.0f; data[5] = cosf(angle); data[6] = -sinf(angle); data[7] = 0.0f;
  data[8]  = 0.0f; data[9] = sinf(angle); data[10] = cosf(angle); data[11] = 0.0f;
  data[12] = 0.0f; data[13] = 0.0f;            data[14] = 0.0f;             data[15] = 1.0f;
  return *this;
}

aeFloat4x4& aeFloat4x4::SetRotateY(float angle){
  data[0]  = cosf(angle);  data[1] = 0.0f;  data[2] = sinf(angle);  data[3] = 0.0f;
  data[4]  = 0.0f;              data[5] = 1.0f;  data[6] = 0.0f;              data[7] = 0.0f;
  data[8]  = -sinf(angle); data[9] = 0.0f;  data[10] = cosf(angle); data[11] = 0.0f;
  data[12] = 0.0f;              data[13] = 0.0f; data[14] = 0.0f;             data[15] = 1.0f;
  return *this;
}

aeFloat4x4& aeFloat4x4::SetRotateZ(float angle){
  data[0]  = cosf(angle);  data[1] = -sinf(angle); data[2] = 0.0f;  data[3] = 0.0f;
  data[4]  = sinf(angle);  data[5] = cosf(angle);  data[6] = 0.0f;  data[7] = 0.0f;
  data[8]  = 0.0f;              data[9] = 0.0f;              data[10] = 1.0f; data[11] = 0.0f;
  data[12] = 0.0f;              data[13] = 0.0f;             data[14] = 0.0f; data[15] = 1.0f;
  return *this;
}

aeFloat4x4& aeFloat4x4::RemoveScaling(void)
{
  aeFloat3 col[4] = {GetAxisVector(0), GetAxisVector(1), GetAxisVector(2), GetAxisVector(3)};
  col[0].Normalize();
  col[1].Normalize();
  col[2].Normalize();

  for(unsigned i = 0; i < 4; ++i)
  {
    SetAxisVector(i, col[i]);
  }

  return *this;
}

aeFloat4x4& aeFloat4x4::SetTranspose( void )
{
  for(int i = 0; i < 4; ++i)
  {
    for(int j = i + 1; j < 4; ++j)
    {
      float temp = data[i * 4 + j];
      data[i * 4 + j] = data[j * 4 + i];
      data[j * 4 + i] = temp;
    }
  }

  return *this;
}

aeFloat4x4 aeFloat4x4::GetTransposeCopy(void) const
{
  aeFloat4x4 temp(*this);
  return temp.SetTranspose();
}

aeFloat4x4 aeFloat4x4::GetNormalMatrix() const
{
  return Inverse().GetTransposeCopy();
}

aeFloat4x4& aeFloat4x4::Translate( aeFloat3 t )
{
  *this = *this * aeFloat4x4::Translation( t );
  return *this;
}

aeFloat4x4& aeFloat4x4::Scale( aeFloat3 s )
{
  *this = *this * aeFloat4x4::Scaling( s );
  return *this;
}

aeFloat4x4& aeFloat4x4::RotateX( float angle )
{
  *this = *this * aeFloat4x4::RotationX( angle );
  return *this;
}

aeFloat4x4& aeFloat4x4::RotateY( float angle )
{
  *this = *this * aeFloat4x4::RotationY( angle );
  return *this;
}

aeFloat4x4& aeFloat4x4::RotateZ( float angle )
{
  *this = *this * aeFloat4x4::RotationZ( angle );
  return *this;
}

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
aePlane::aePlane( aeFloat3 point, aeFloat3 normal )
{
  m_point = point;
  m_normal = normal.SafeNormalizeCopy();
}

bool aePlane::IntersectRay( aeFloat3 pos, aeFloat3 dir, float* tOut, aeFloat3* out ) const
{
  dir.SafeNormalize();

  float a = dir.Dot( m_normal );
  if ( a > -0.01f )
  {
    // Ray is pointing away from or parallel to plane
    return false;
  }

  aeFloat3 diff = pos - m_point;
  float b = diff.Dot( m_normal );
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
  aeFloat3 n = (b - a) % (c - a);
  float vc = n.Dot((a - p) % (b - p));
  // If P outside AB and within feature region of AB,
  // return projection of P onto AB
  if (vc <= 0.0f && snom >= 0.0f && sdenom >= 0.0f)
      return a + snom / (snom + sdenom) * ab;
  
  // P is outside (or on) BC if the triple scalar product [N PB PC] <= 0
  float va = n.Dot((b - p) % (c - p));
  // If P outside BC and within feature region of BC,
  // return projection of P onto BC
  if (va <= 0.0f && unom >= 0.0f && udenom >= 0.0f)
    return b + unom / (unom + udenom) * bc;
  
  // P is outside (or on) CA if the triple scalar product [N PC PA] <= 0
  float vb = n.Dot((c - p) %(a - p));
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
  aeFloat3 c = ( m_min + m_max ) * 0.5f;
  aeFloat3 hs = ( m_max - m_min ) * 0.5f;

  aeFloat3 d = p - c;
  d.x = aeMath::Max( aeMath::Abs( d.x ) - hs.x, 0.0f );
  d.y = aeMath::Max( aeMath::Abs( d.y ) - hs.y, 0.0f );
  d.z = aeMath::Max( aeMath::Abs( d.z ) - hs.z, 0.0f );

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
  m_invTransRot.RemoveScaling();
  m_invTransRot.Invert();
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

bool aeOBB::IntersectRay( aeFloat3 p, aeFloat3 d, aeFloat3* pOut, float* tOut ) const
{
  p = ( m_invTransRot * aeFloat4( p, 1.0f ) ).GetXYZ();
  d = ( m_invTransRot * aeFloat4( d, 0.0f ) ).GetXYZ();

  float rayT = 0.0f;
  if ( m_scaledAABB.IntersectRay( p, d, nullptr, &rayT ) )
  {
    if ( tOut )
    {
      *tOut = rayT;
    }
    if ( pOut )
    {
      *pOut = p + d * rayT;
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
// aeHash member functions
//------------------------------------------------------------------------------
aeHash::aeHash( uint32_t initialValue )
{
  m_hash = initialValue;
}

aeHash& aeHash::HashString( const char* str )
{
  while ( *str )
  {
    m_hash = m_hash ^ str[ 0 ];
    m_hash *= 0x1000193;
    str++;
  }

  return *this;
}

aeHash& aeHash::HashData( const uint8_t* data, const uint32_t length )
{
  for ( uint32_t i = 0; i < length; i++ )
  {
    m_hash = m_hash ^ data[ i ];
    m_hash *= 0x1000193;
  }

  return *this;
}

void aeHash::Set( uint32_t hash )
{
  m_hash = hash;
}

uint32_t aeHash::Get() const
{
  return m_hash;
}
