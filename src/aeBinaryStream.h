//------------------------------------------------------------------------------
// aeBinaryStream.h
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
#ifndef AEBINARYSTREAM_H
#define AEBINARYSTREAM_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeLog.h"

//------------------------------------------------------------------------------
// aeBinaryStream class
//------------------------------------------------------------------------------
class aeBinaryStream
{
public:
  enum Mode
  {
    ReadBuffer,
    WriteBuffer,
  };

  aeBinaryStream( Mode mode, void* data, uint32_t length )
  {
    m_mode = mode;
    m_data = (uint8_t*)data;
    m_length = length;
    m_position = 0;
  }

  template< typename T >
  void _Serialize( T& v, decltype(&T::Serialize) )
  {
    v.Serialize( this );
  }

  template< typename T >
  void _Serialize( T& v, ... )
  {
    if ( m_mode == ReadBuffer )
    {
      AE_ASSERT( m_position + sizeof(T) <= m_length );
      memcpy( &v, m_data + m_position, sizeof(T) );
      m_position += sizeof(T);
    }
    if ( m_mode == WriteBuffer )
    {
      AE_ASSERT( sizeof(T) <= m_length - m_position );
      memcpy( m_data + m_position, &v, sizeof(T) );
      m_position += sizeof(T);
    }
  }

  template< typename T >
  void Serialize( T& v )
  {
    _Serialize( v, 0 );
  }

  template< uint32_t N >
  void Serialize( char (&str)[ N ] )
  {
    uint16_t len;
    if ( m_mode == ReadBuffer )
    {
      AE_ASSERT( m_position + sizeof(len) <= m_length );
      memcpy( &len, m_data + m_position, sizeof(len) );
      m_position += sizeof(len);

      AE_ASSERT( m_position + len + 1 <= m_length );
      memcpy( str, m_data + m_position, len );
      str[ len ] = 0;
      m_position += len;
    }
    if ( m_mode == WriteBuffer )
    {
      len = strlen( str );
      AE_ASSERT( sizeof(len) <= m_length - m_position );
      memcpy( m_data + m_position, &len, sizeof(len) );
      m_position += sizeof(len);

      AE_ASSERT( len <= m_length - m_position );
      memcpy( m_data + m_position, str, len );
      m_position += len;
    }
  }

  void Serialize( void* data, uint32_t length )
  {
    if ( m_mode == ReadBuffer )
    {
      AE_ASSERT( m_position + length <= m_length );
      memcpy( data, m_data + m_position, length );
      m_position += length;
    }
    if ( m_mode == WriteBuffer )
    {
      AE_ASSERT( length <= m_length - m_position );
      memcpy( m_data + m_position, data, length );
      m_position += length;
    }
  }

  bool IsValid() const { return true; } // @TODO: Implement stream invalidation. Validation checks should replace all above asserts.
  Mode GetMode() const { return m_mode; }
  const uint8_t* GetData() const { return m_data; }
  uint32_t GetOffset() const { return m_position; }
  uint32_t GetLength() const { return m_length; }

private:
  Mode m_mode;
  uint8_t* m_data;
  uint32_t m_length;
  uint32_t m_position;
};

#endif
