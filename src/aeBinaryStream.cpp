//------------------------------------------------------------------------------
// aeBinaryStream.cpp
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
#include "aeBinaryStream.h"
#include "aeMath.h"

//------------------------------------------------------------------------------
// aeBBinaryStream member functions
//------------------------------------------------------------------------------
aeBinaryStream::aeBinaryStream( Mode mode, uint8_t* data, uint32_t length )
{
  m_mode = mode;
  m_data = data;
  m_length = length;
  m_isValid = m_data && m_length;
}

aeBinaryStream::aeBinaryStream( Mode mode, const uint8_t* data, uint32_t length )
{
  AE_ASSERT_MSG( mode == Mode::ReadBuffer, "Only read mode can be used with a constant data buffer." );
  m_mode = mode;
  m_data = const_cast< uint8_t* >( data );
  m_length = length;
  m_isValid = m_data && m_length;
}

aeBinaryStream::aeBinaryStream( Mode mode )
{
  AE_ASSERT_MSG( mode == Mode::WriteBuffer, "Only write mode can be used when a data buffer is not provided." );
  m_mode = mode;
  m_isValid = true;
}

aeBinaryStream::aeBinaryStream( ae::Array< uint8_t >* array )
{
  m_mode = Mode::WriteBuffer;
  if ( array )
  {
    m_extArray = array;
    m_offset = m_extArray->Length();
    m_length = m_extArray->Size();
    m_isValid = true;
  }
}

aeBinaryStream aeBinaryStream::Writer( uint8_t* data, uint32_t length )
{
  return aeBinaryStream( Mode::WriteBuffer, data, length );
}

aeBinaryStream aeBinaryStream::Writer( ae::Array< uint8_t >* array )
{
  return aeBinaryStream( array );
}

aeBinaryStream aeBinaryStream::Writer()
{
  return aeBinaryStream( Mode::WriteBuffer );
}

aeBinaryStream aeBinaryStream::Reader( const uint8_t* data, uint32_t length )
{
  return aeBinaryStream( Mode::ReadBuffer, const_cast< uint8_t* >( data ), length );
}

aeBinaryStream aeBinaryStream::Reader( const ae::Array< uint8_t >& data )
{
  if ( !data.Length() )
  {
    return aeBinaryStream::Reader( nullptr, 0 );
  }
  return aeBinaryStream( Mode::ReadBuffer, &data[ 0 ], data.Length() );
}

void aeBinaryStream::SerializeUint8( uint8_t& v )
{
  SerializeRaw( &v, sizeof(v) );
}

void aeBinaryStream::SerializeUint8( const uint8_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( &v, sizeof(v) );
}

void aeBinaryStream::SerializeUint16( uint16_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint16( const uint16_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint32( uint32_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint32( const uint32_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint64( uint64_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint64( const uint64_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt8( int8_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt8( const int8_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt16( int16_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt16( const int16_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt32( int32_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt32( const int32_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt64( int64_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt64( const int64_t& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof( v ) );
}

void aeBinaryStream::SerializeFloat( float& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof( v ) );
}

void aeBinaryStream::SerializeFloat( const float& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof( v ) );
}

void aeBinaryStream::SerializeDouble( double& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof( v ) );
}

void aeBinaryStream::SerializeDouble( const double& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof( v ) );
}

void aeBinaryStream::SerializeBool( bool& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof( v ) );
}

void aeBinaryStream::SerializeBool( const bool& v )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::m_SerializeArrayLength( uint32_t& length, uint32_t maxLength )
{
  if ( maxLength <= aeMath::MaxValue< uint8_t >() )
  {
    uint8_t len = (uint8_t)length;
    SerializeUint8( len );
    length = len;
  }
  else if ( maxLength <= aeMath::MaxValue< uint16_t >() )
  {
    uint16_t len = (uint16_t)length;
    SerializeUint16( len );
    length = len;
  }
  else
  {
    uint32_t len = length;
    SerializeUint32( len );
    length = len;
  }
}

void aeBinaryStream::SerializeArray( ae::Array< uint8_t >& array, uint32_t maxLength )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    uint32_t length = 0;
    m_SerializeArrayLength( length, maxLength );
    if ( !m_isValid || length == 0 )
    {
      return;
    }
    else if ( GetRemaining() < length )
    {
      Invalidate();
      return;
    }
    
    array.Append( PeekData(), length );
    Discard( length );
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    uint32_t length = array.Length();
    m_SerializeArrayLength( length, maxLength );
    if ( length )
    {
      SerializeRaw( &array[ 0 ], length );
    }
  }
}

void aeBinaryStream::SerializeArray( const ae::Array< uint8_t >& array, uint32_t maxLength )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const array." );
  
  uint32_t length = array.Length();
  m_SerializeArrayLength( length, maxLength );
  if ( length )
  {
    SerializeRaw( &array[ 0 ], length );
  }
}

void aeBinaryStream::SerializeRaw( void* data, uint32_t length )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    if ( m_offset + length <= m_length )
    {
      memcpy( data, m_data + m_offset, length );
      m_offset += length;
    }
    else
    {
      Invalidate();
    }
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    if ( m_data )
    {
      if ( length <= m_length - m_offset )
      {
        memcpy( m_data + m_offset, data, length );
        m_offset += length;
      }
      else
      {
        Invalidate();
      }
    }
    else
    {
      ae::Array< uint8_t >& array = m_GetArray();
      array.Append( (uint8_t*)data, length );
      m_offset = array.Length();
      m_length = array.Size();
    }
  }
  else
  {
    AE_FAIL_MSG( "Binary stream must be initialized with aeBinaryStream::Writer or aeBinaryStream::Reader static functions." );
  }
}

void aeBinaryStream::SerializeRaw( const void* data, uint32_t length )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const array." );
  SerializeRaw( (void*)data, length );
}

void aeBinaryStream::SerializeRaw( ae::Array< uint8_t>& array )
{
  AE_FAIL_MSG( "Not implemented" );
}

void aeBinaryStream::SerializeRaw( const ae::Array< uint8_t>& array )
{
  AE_FAIL_MSG( "Not implemented" );
}

void aeBinaryStream::Discard( uint32_t length )
{
  if ( !length )
  {
    return;
  }
  else if ( GetRemaining() < length )
  {
    Invalidate();
  }
  else
  {
    m_offset += length;
  }
}
