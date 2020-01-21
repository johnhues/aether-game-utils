//------------------------------------------------------------------------------
// aeString.h
// Fixed length string
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
#ifndef AESTRING_H
#define AESTRING_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeLog.h"
#include "aePlatform.h"

//------------------------------------------------------------------------------
// String hash
//------------------------------------------------------------------------------
inline uint32_t aeFNV1a( const uint8_t* data, const uint32_t length, uint32_t hash = 0x811c9dc5 )
{
  for ( uint32_t i = 0; i < length; i++ )
  {
    hash = hash ^ data[ i ];
    hash *= 0x1000193;
  }
  return hash;
}

//------------------------------------------------------------------------------
// Fixed length string definition
//------------------------------------------------------------------------------
template < uint32_t N >
class aeStr
{
public:
  aeStr();

  template < uint32_t N2 >
  aeStr( const aeStr<N2>& str );

  aeStr( const char* str );

  aeStr( uint32_t length, const char* str );

  template < typename... Args >
  aeStr( const char* format, Args... args );

  explicit inline operator const char*() const;
  inline const char* c_str() const;

  template < uint32_t N2 >
  inline void operator =( const aeStr<N2>& str );

  inline aeStr<N> operator +( const char* str ) const;

  template < uint32_t N2 >
  inline aeStr<N> operator +( const aeStr<N2>& str ) const;

  inline void operator +=( const char* str );

  template < uint32_t N2 >
  inline void operator +=( const aeStr<N2>& str );

  template < uint32_t N2 >
  inline bool operator ==( const aeStr<N2>& str ) const;
  inline bool operator ==( const char* str ) const;

  template < uint32_t N2 >
  inline bool operator !=( const aeStr<N2>& str ) const;
  inline bool operator !=( const char* str ) const;

  template < uint32_t N2 >
  inline bool operator <( const aeStr<N2>& str ) const;
  inline bool operator <( const char* str ) const;

  template < uint32_t N2 >
  inline bool operator >( const aeStr<N2>& str ) const;
  inline bool operator >( const char* str ) const;

  template < uint32_t N2 >
  inline bool operator <=( const aeStr<N2>& str ) const;
  inline bool operator <=( const char* str ) const;

  template < uint32_t N2 >
  inline bool operator >=( const aeStr<N2>& str ) const;
  inline bool operator >=( const char* str ) const;

  inline char& operator[]( uint32_t i );
  inline const char operator[]( uint32_t i ) const;
  
  inline uint32_t Length() const;
  inline uint32_t Size() const;
  inline bool Empty() const;

  template < uint32_t N2 >
  inline void Append( const aeStr<N2>& str );
  inline void Append( const char* str );
  void Trim( uint32_t len );

  template < typename... Args >
  static aeStr< N > Format( const char* format, Args... args );

  static const uint32_t kMaxLength = N - 2;
  template < uint32_t N2 > friend class aeStr;

  inline uint32_t FNV1a() const;

private:
  void m_Format( const char* format );

  template < typename T, typename... Args >
  void m_Format( const char* format, T value, Args... args );

  uint16_t m_length;
  char m_str[ kMaxLength ];
};

//------------------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------------------
typedef aeStr< 16 > aeStr16;
typedef aeStr< 32 > aeStr32;
typedef aeStr< 64 > aeStr64;
typedef aeStr< 128 > aeStr128;
typedef aeStr< 256 > aeStr256;
typedef aeStr< 512 > aeStr512;

//------------------------------------------------------------------------------
// Non member functions
//------------------------------------------------------------------------------
template < uint32_t N >
std::ostream& operator<<( std::ostream& os, const aeStr< N >& str )
{
  return os << str.c_str();
}

template < uint32_t N >
inline const aeStr< N >& ToString( const aeStr< N >& value )
{
  return value;
}

inline const char* ToString( const char* value )
{
  return value;
}

inline aeStr16 ToString( int32_t value )
{
  char str[ aeStr16::kMaxLength ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%d", value );
  return aeStr16( length, str );
}

inline aeStr16 ToString( uint32_t value )
{
  char str[ aeStr16::kMaxLength ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%u", value );
  return aeStr16( length, str );
}

inline aeStr16 ToString( float value )
{
  char str[ aeStr16::kMaxLength ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%.2f", value );
  return aeStr16( length, str );
}

inline aeStr16 ToString( double value )
{
  char str[ aeStr16::kMaxLength ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%.2f", value );
  return aeStr16( length, str );
}

//------------------------------------------------------------------------------
// aeStr member functions
//------------------------------------------------------------------------------
template < uint32_t N >
aeStr< N >::aeStr()
{
  AE_STATIC_ASSERT_MSG( sizeof(*this) == N, "Incorrect aeStr size" );
}

template < uint32_t N >
template < uint32_t N2 >
aeStr< N >::aeStr( const aeStr<N2>& str )
{
  AE_ASSERT( str.m_length < kMaxLength );
  m_length = str.m_length;
  memcpy( m_str, str.m_str, m_length + 1 );
}

template < uint32_t N >
aeStr< N >::aeStr( const char* str )
{
  m_length = (uint16_t)strlen( str );
  AE_ASSERT_MSG( m_length < kMaxLength, "Length:# Max:#", m_length, kMaxLength );
  memcpy( m_str, str, m_length + 1 );
}

template < uint32_t N >
aeStr< N >::aeStr( uint32_t length, const char* str )
{
  AE_ASSERT( length < kMaxLength );
  m_length = length;
  memcpy( m_str, str, m_length );
  m_str[ length ] = 0;
}

template < uint32_t N >
template < typename... Args >
aeStr< N >::aeStr( const char* format, Args... args )
{
  m_length = 0;
  m_str[ 0 ] = 0;
  m_Format( format, args... );
}

template < uint32_t N >
inline aeStr< N >::operator const char*() const
{
  return m_str;
}

template < uint32_t N >
inline const char* aeStr< N >::c_str() const
{
  return m_str;
}

template < uint32_t N >
template < uint32_t N2 >
inline void aeStr< N >::operator =( const aeStr<N2>& str )
{
  AE_ASSERT( str.m_length < kMaxLength );
  m_length = str.m_length;
  memcpy( m_str, str.m_str, str.m_length + 1 );
}

template < uint32_t N >
inline aeStr<N> aeStr< N >::operator +( const char* str ) const
{
  aeStr<N> out( *this );
  out += str;
  return out;
}

template < uint32_t N >
template < uint32_t N2 >
inline aeStr<N> aeStr< N >::operator +( const aeStr<N2>& str ) const
{
  aeStr<N> out( *this );
  out += str;
  return out;
}

template < uint32_t N >
inline void aeStr< N >::operator +=( const char* str )
{
  uint32_t len = (uint32_t)strlen( str );
  AE_ASSERT( m_length + len < kMaxLength );
  memcpy( m_str + m_length, str, len + 1 );
  m_length += len;
}

template < uint32_t N >
template < uint32_t N2 >
inline void aeStr< N >::operator +=( const aeStr<N2>& str )
{
  AE_ASSERT( m_length + str.m_length < kMaxLength );
  memcpy( m_str + m_length, str.c_str(), str.m_length + 1 );
  m_length += str.m_length;
}

template < uint32_t N >
template < uint32_t N2 >
inline bool aeStr< N >::operator ==( const aeStr<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) == 0;
}

template < uint32_t N >
inline bool aeStr< N >::operator ==( const char* str ) const
{
  return strcmp( m_str, str ) == 0;
}

template < uint32_t N >
template < uint32_t N2 >
inline bool aeStr< N >::operator !=( const aeStr<N2>& str ) const
{
  return !operator==( str );
}

template < uint32_t N >
inline bool aeStr< N >::operator !=( const char* str ) const
{
  return !operator==( str );
}

template < uint32_t N >
template < uint32_t N2 >
inline bool aeStr< N >::operator <( const aeStr<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) < 0;
}

template < uint32_t N >
inline bool aeStr< N >::operator <( const char* str ) const
{
  return strcmp( m_str, str ) < 0;
}

template < uint32_t N >
template < uint32_t N2 >
inline bool aeStr< N >::operator >( const aeStr<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) > 0;
}

template < uint32_t N >
inline bool aeStr< N >::operator >( const char* str ) const
{
  return strcmp( m_str, str ) > 0;
}

template < uint32_t N >
template < uint32_t N2 >
inline bool aeStr< N >::operator <=( const aeStr<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) <= 0;
}

template < uint32_t N >
inline bool aeStr< N >::operator <=( const char* str ) const
{
  return strcmp( m_str, str ) <= 0;
}

template < uint32_t N >
template < uint32_t N2 >
inline bool aeStr< N >::operator >=( const aeStr<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) >= 0;
}

template < uint32_t N >
inline bool aeStr< N >::operator >=( const char* str ) const
{
  return strcmp( m_str, str ) >= 0;
}

template < uint32_t N >
inline char& aeStr< N >::operator[]( uint32_t i )
{
  AE_ASSERT( i < m_length ); return m_str[ i ];
}

template < uint32_t N >
inline const char aeStr< N >::operator[]( uint32_t i ) const
{
  AE_ASSERT( i < m_length ); return m_str[ i ];
}

template < uint32_t N >
inline uint32_t aeStr< N >::Length() const
{
  return m_length;
}

template < uint32_t N >
inline uint32_t aeStr< N >::Size() const
{
  return kMaxLength;
}

template < uint32_t N >
inline bool aeStr< N >::Empty() const
{
  return m_length == 0;
}

template < uint32_t N >
template < uint32_t N2 >
inline void aeStr< N >::Append( const aeStr<N2>& str )
{
  *this += str;
}

template < uint32_t N >
inline void aeStr< N >::Append( const char* str )
{
  *this += str;
}

template < uint32_t N >
void aeStr< N >::Trim( uint32_t len )
{
  if ( len == m_length )
  {
    return;
  }

  AE_ASSERT( len < m_length );
  m_length = len;
  m_str[ m_length ] = 0;
}

template < uint32_t N >
template < typename... Args >
aeStr< N > aeStr< N >::Format( const char* format, Args... args )
{
  aeStr< N > result( "" );
  result.m_Format( format, args... );
  return result;
}

template < uint32_t N >
void aeStr< N >::m_Format( const char* format )
{
  *this += format;
}

template < uint32_t N >
template < typename T, typename... Args >
void aeStr< N >::m_Format( const char* format, T value, Args... args )
{
  if ( !*format )
  {
    return;
  }
  
  const char* head = format;
  while ( *head && *head != '#' )
  {
    head++;
  }
  if ( head > format )
  {
    *this += aeStr( head - format, format );
  }

  if ( *head == '#' )
  {
    *this += ToString( value );
    head++;
  }
  m_Format( head, args... );
}

template < uint32_t N >
uint32_t aeStr< N >::FNV1a() const
{
  // @TODO: Return 0x811c9dc5 on empty string or 0?
  AE_ASSERT( m_length );
  return aeFNV1a( (const uint8_t*)m_str, m_length );
}

#endif