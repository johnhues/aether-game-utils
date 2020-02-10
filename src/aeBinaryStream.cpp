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

aeBinaryStream::aeBinaryStream( aeArray< uint8_t >* array )
{
  m_mode = Mode::WriteBuffer;
  if ( array )
  {
    m_extArray = array;
    m_position = m_extArray->Length();
    m_length = m_extArray->Size();
    m_isValid = true;
  }
}

aeBinaryStream aeBinaryStream::Writer( uint8_t* data, uint32_t length )
{
  return aeBinaryStream( Mode::WriteBuffer, data, length );
}

aeBinaryStream aeBinaryStream::Writer( aeArray< uint8_t >* array )
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

void aeBinaryStream::SerializeUint8( uint8_t& v )
{
  SerializeRaw( &v, sizeof(v) );
}

void aeBinaryStream::SerializeUint8( const uint8_t& v )
{
  SerializeRaw( &v, sizeof(v) );
}

void aeBinaryStream::SerializeUint16( uint16_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint16( const uint16_t& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint32( uint32_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint32( const uint32_t& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint64( uint64_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeUint64( const uint64_t& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt8( uint8_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt8( const uint8_t& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt16( uint16_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt16( const uint16_t& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt32( uint32_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt32( const uint32_t& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt64( uint64_t& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeInt64( const uint64_t& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeFloat( float& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeFloat( const float& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeDouble( double& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeDouble( const double& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeBool( bool& v )
{
  SerializeRaw( (uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeBool( const bool& v )
{
  SerializeRaw( (const uint8_t*)&v, sizeof(v) );
}

void aeBinaryStream::SerializeArray( uint8_t* data, uint32_t length )
{
  AE_FAIL_MSG( "Not implemented" );
}

void aeBinaryStream::SerializeArray( const uint8_t* data, uint32_t length )
{
  AE_FAIL_MSG( "Not implemented" );
}

void aeBinaryStream::SerializeArray( aeArray< uint8_t>& array )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    uint16_t length = 0;
    SerializeUint16( length );
    
    if ( !m_isValid || length == 0 )
    {
      return;
    }
    else if ( GetRemaining() < length )
    {
      Invalidate();
      return;
    }
    
    array.Append( PeakData(), length );
    Discard( length );
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    uint16_t length = array.Length();
    SerializeUint16( length );
    if ( length )
    {
      SerializeRaw( &array[ 0 ], length );
    }
  }
}

void aeBinaryStream::SerializeArray( const aeArray< uint8_t>& array )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const array." );
  
  uint16_t length = array.Length();
  SerializeUint16( length );
  if ( length )
  {
    SerializeRaw( &array[ 0 ], length );
  }
}

void aeBinaryStream::SerializeRaw( uint8_t* data, uint32_t length )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    if ( m_position + length <= m_length )
    {
      memcpy( data, m_data + m_position, length );
      m_position += length;
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
      if ( length <= m_length - m_position )
      {
        memcpy( m_data + m_position, data, length );
        m_position += length;
      }
      else
      {
        Invalidate();
      }
    }
    else
    {
      aeArray< uint8_t >& array = m_GetArray();
      array.Append( data, length );
      m_position = array.Length();
      m_length = array.Size();
    }
  }
  else
  {
    AE_FAIL_MSG( "Binary stream must be initialized with aeBinaryStream::Writer or aeBinaryStream::Reader static functions." );
  }
}

void aeBinaryStream::SerializeRaw( const uint8_t* data, uint32_t length )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const array." );
  SerializeRaw( const_cast< uint8_t* >( data ), length );
}

void aeBinaryStream::SerializeRaw( aeArray< uint8_t>& array )
{
  AE_FAIL_MSG( "Not implemented" );
}

void aeBinaryStream::SerializeRaw( const aeArray< uint8_t>& array )
{
  AE_FAIL_MSG( "Not implemented" );
}