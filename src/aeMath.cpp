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

void aeFloat2::Normalize()
{
  float l = sqrtf( x * x + y * y );
  x /= l;
  y /= l;
}

aeFloat2 aeFloat2::NormalizeCopy(void) const
{
  float l = sqrtf( x * x + y * y );
  return aeFloat2( x / l, y / l );
}

void aeFloat2::SafeNormalize()
{
  float l = sqrtf( x * x + y * y );
  if ( l < 0.00001f )
  {
    x = 0.0f;
    y = 0.0f;
    return;
  }
  x /= l;
  y /= l;
}

aeFloat2 aeFloat2::SafeNormalizeCopy(void) const
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
  if ( LengthSquared() < 0.01f )
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

void aeFloat3::Normalize()
{
  (*this) *= 1.0f / Length();
}

aeFloat3 aeFloat3::NormalizeCopy(void) const
{
  aeFloat3 copy = *this;
  copy.Normalize();
  return copy;
}

void aeFloat3::SafeNormalize()
{
  float length = Length();
  if ( length < 0.00001f )
  {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    return;
  }
  (*this) *= 1.0f / length;
}

aeFloat3 aeFloat3::SafeNormalizeCopy(void) const
{
  aeFloat3 copy = *this;
  copy.SafeNormalize();
  return copy;
}

void aeFloat3::SetZero()
{
  x = y = z = 0.0f;
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
  aeFloat3 v0 = NormalizeCopy();
  aeFloat3 v1 = end.NormalizeCopy();
  
  float d = aeMath::Clip( v0.Dot( v1 ), -1.0f, 1.0f );
  float angle = acosf( d ) * t;
  if ( d > 0.99f ) { return v1; }
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

aeFloat4& aeFloat4::Normalize()
{
  (*this) *= 1.0f / Length();

  return *this;
}

aeFloat4 aeFloat4::NormalizeCopy(void) const
{
  aeFloat4 copy = *this;
  copy.Normalize();
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

aeFloat4x4 aeFloat4x4::Scaling( const aeFloat3& s )
{
  aeFloat4x4 r = aeFloat4x4::Identity();
  r.SetScale( s );
  return r;
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

aeFloat4x4& aeFloat4x4::SetWorldToView(aeFloat3 forward, aeFloat3 up)
{
  *this = aeFloat4x4::Identity();

  //xaxis.x  xaxis.y  xaxis.z  dot(xaxis, eye)
  //yaxis.x  yaxis.y  yaxis.z  dot(yaxis, eye)
  //zaxis.x  zaxis.y  zaxis.z  dot(zaxis, eye)
  //0        0        0        1

  forward.Normalize();
  up.Normalize();

  aeFloat3 right = forward % up;
  right.Normalize();
  up = right % forward;

  SetRowVector(0, right);
  SetRowVector(1, up);
  SetRowVector(2, -forward);

  return *this;
}

aeFloat4x4& aeFloat4x4::SetViewToProjection(float fov, float aspectRatio, float nearPlane, float farPlane)
{
  // a  0  0  0
  // 0  b  0  0
  // 0  0  A  B
  // 0  0 -1  0
  
  memset(data, 0, sizeof(float)*16);
  
  float r = aspectRatio * tanf(fov * 0.5f) * 0.8f;
  float t = tanf(fov * 0.5f) * 0.8f;
  
  float a = nearPlane / r;
  float b = nearPlane / t;
  float A = -( farPlane + nearPlane ) / ( farPlane - nearPlane );
  float B = ( -2.0f * farPlane * nearPlane ) / ( farPlane - nearPlane );
  
  data[0]  = a;
  data[5]  = b;
  data[10] = A;
  data[11] = B;
  data[14] = -1;

  return *this;
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

aeFloat4x4& aeFloat4x4::RotateZ( float angle )
{
  *this = *this * aeFloat4x4::RotationZ( angle );
  return *this;
}

//------------------------------------------------------------------------------
// aeRect member functions
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// aeRect member functions
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

bool aePlane::IntersectRay( aeFloat3 pos, aeFloat3 dir, aeFloat3* out ) const
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

  *out = pos - dir * c;
  return true;
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
